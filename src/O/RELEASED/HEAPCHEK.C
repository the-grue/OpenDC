/*
 *  Released under the GNU GPL.  See http://www.gnu.org/licenses/gpl.txt
 *
 *  This program is part of the DeSmet C Compiler
 *
 *  DeSmet C is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundatation; either version 2 of the License, or any
 *  later version.
 *
 *  DeSmet C is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 */
/*
			       File: HEAPCHEK.C
			       ----------------

			      Copyright (c) 1987

				      by

			    Key Software Products
		    440 Ninth Avenue, Menlo Park, CA 94025
				(415) 364-9847

			    (All rights reserved)

This file contains code that can be used to discover semantic bugs in your
programs due to the stack growing down into the heap.  This is usually the
result of either unbounded recursion, or deeply nested calls to functions
containing large automatic storage class arrays.

To use this debugging feature, recompile your program with the O88 option
+H (CALL _HELP) enabled.  This causes O88 to insert a "CALL _HELP_" at the
entrance of every function.  You must also compile THIS file (using -H)
and link it with your program using BIND.

If the stack comes within 'MARGIN' (see below) bytes of the heap, the
function _HELP() (provided below) will print out a reverse list of
function return addresses and then terminate your program.  This list can 
be used with the .MAP file that BIND produces (an option) to determine the
sequence of function calls that led to disaster.

WARNING! Use of this feature will significantly slow down the execution of
your program.  Once you are confident that you have eliminated any such
problems, be sure to recompile your program WITHOUT +H, and BIND without the
compiled version of this file.

NOTE: This source is provided in case you wish to modify the action of the
function _HELP().  Anything may be done, but the function _HELP() must have
no arguments, and must not return a value.

*/


/* The following are for use with Gimpel LINT ...			*/
/* -------------------------------------------------------------------- */
/*lint	+fcu		char-is-unsigned				*/
/*lint	+fsu		string-is-unsigned				*/
/*lint	+fzu		sizeof-is-unsigned				*/
/*lint	-library	don't worry about unreferenced or undefined	*/

/* -------------------------------------------------------------- */
/* The following #define establishes the minimum distance allowed */
/* between the bottom of the stack and the top of the heap before */
/* triggering the error message and stack trace ...		  */
/* -------------------------------------------------------------- */
#define	MARGIN	500


/* ------------------------------------------------------------- */
/* This structure defines the control block format for the heap. */
/* ------------------------------------------------------------- */
typedef struct
	{
	char		status ;
	unsigned	size ;
	char		data[1] ;
	} HEAP ;


/* --------------------------------------------------------------- */
/* These are the three legal values for the status of a heap block */
/* --------------------------------------------------------------- */
#define	UNALL	0x9D
#define	ALLOC	0xAB
#define	EOA	0xC6


/* ------------------------------- */
/* Functions used by this file ... */
/* ------------------------------- */
extern unsigned		_showsp() ;
extern unsigned 	_showcs() ;
extern char		*_memory() ;
extern void		freeall() ;
extern unsigned char	_peek() ;
extern void		puts() ;
extern void		exit() ;

static char		*utoh() ;
static unsigned		_showbp() ;
static void		show_bp() ;	/* (Not actually used - see below) */


/* -------------------------------------------- */
/* Option +H (CALL _HELP) of O88 causes a CALL  */
/* to this function to be inserted at the entry */
/* point of every function.		  	*/
/*						*/
/*   DO NOT CHANGE THE NAME OF THIS FUNCTION!	*/
/* -------------------------------------------- */
void _HELP()
	{
	/* ---------------------------------------------------- */
	/* All of these variables are assigned static storage	*/
	/* allocation so that they won't be allocated from the	*/
	/* stack when this function is called.  Remember, we're	*/
	/* trying to monitor stack growth!			*/
	/* ---------------------------------------------------- */

	static int virgin = 1 ;		/* to control initialization	*/
	static unsigned cs ;		/* a C var to hold a copy of CS	*/
	static unsigned *bp ;		/* a C var to hold a copy of BP */
	static unsigned sp ;		/* a C var to hold a copy of SP */
	static unsigned rtn_adrs ;	/* a function's return address	*/
	static unsigned *last_bp = 0 ;	/* used to end the stack trace	*/
	static HEAP *blk ;		/* ptr to a memory control blk	*/

	if (virgin)
		{
		/* ------------------------------------------------ */
		/* This will only be executed once, when _HELP()    */
		/* is called at the entrance to main().  It's       */
		/* required to be sure that malloc's linked list    */
		/* of memory control blocks gets set up.  Normally  */
		/* this occurs on the first malloc call anyway, but */
		/* this freeall call is necessary if you use this   */
		/* _HELP() function with a program that does not    */
		/* use malloc.					    */
		/* ------------------------------------------------ */
		freeall(1000) ;
		virgin = 0 ;
		}


	/* ---------------------------------------------------- */
	/* Look at the instruction that immediately follows the	*/
	/* CALL instruction that invoked this function.  If it	*/
	/* is an SUB SP,n then it indicates that the function	*/
	/* uses n bytes of the stack for its auto storage class	*/
	/* variables.						*/
	/*							*/
	/* (There may be other SUB SP,n instructions if other	*/
	/* variables are declared within nested curly braces of	*/
	/* the same function, but these we can't find.)		*/
	/*							*/
	/* These n bytes should be subtracted from the current	*/
	/* value of the stack pointer to better anticipate when	*/
	/* (and if) the stack is overrunning the heap.		*/
	/* ---------------------------------------------------- */

	sp = _showsp() ;		/* SP is the current stack ptr	*/
	bp = (unsigned *) _showbp() ;	/* BP points to _HELP()'s frame	*/
	rtn_adrs = bp[1] ;		/* BP[1] is the return address	*/
	cs = _showcs() ;		/* CS is the code segment.	*/

	if (_peek(rtn_adrs + 2, cs) == 0xEC)
		{
		/* ---------------------------------------------------- */
		/* There are two forms of the SUB SP,n instruction.	*/
		/* They differ in the first opcode byte, but the	*/
		/* second opcode byte is always 0xEC.			*/
		/* ---------------------------------------------------- */
		if (_peek(rtn_adrs + 1, cs) == 0x83)
			{
			/* ---------------------------------------------- */
			/* This version of SUB SP,n is used when n < 256. */
			/* ---------------------------------------------- */
			sp -= (unsigned) _peek(rtn_adrs + 3, cs) ;
			}
		else if (_peek(rtn_adrs + 1, cs) == 0x81)
			{
			/* ---------------------------------------------- */
			/* This version of SUB SP,n is used when n > 255. */
			/* ---------------------------------------------- */
			sp -= (unsigned) (_peek(rtn_adrs + 4, cs) << 8) |
			      _peek(rtn_adrs + 3, cs) ;
			}
		}
	
	

	/* ---------------------------- */
	/* Find the top of the heap.	*/
	/* The technique is slow but	*/
	/* necessary since subsequent	*/
	/* freeall() calls may move it.	*/
	/* ---------------------------- */
	blk = (HEAP *) _memory() ;
	do
		{
		if (blk->status==UNALL || blk->status==ALLOC)
			{
			blk = (HEAP *) (blk->data + blk->size) ;
			}
		else if (blk->status!=EOA)
			{
			puts("\nHeap memory control blocks damaged!\7\n") ;
			goto backtrack ;
			}
		} while (blk->status!=EOA) ;

	/* ---------------------------- */
	/* Now compare lower bound of	*/
	/* stack to upper bound of heap */
	/* ---------------------------- */
	if ((sp - ((unsigned) blk)) > MARGIN) return ;	/* All is OK */

	puts("\nBtm of Stack [") ;	/* Stack too close to heap! */
	puts(utoh(sp)) ;
	puts("] near Top of Heap [") ;
	puts(utoh((unsigned) blk)) ;
	puts("]\7\n") ;

backtrack:
	/* ---------------------------------------------------- */
	/* This code backtracks through the stack frames, 	*/
	/* following the saved frame pointers.  Just above	*/
	/* the stack location of each saved frame pointer	*/
	/* (BP) is the return address for that function call.	*/
	/* This function prints out those return addresses so	*/
	/* you can determine from a .MAP file where the CALLs	*/
	/* were made.  Since the stack grows down, the reverse	*/
	/* sequence of saved frame pointers should have		*/
	/* increasing values; if not, we assume that we came	*/
	/* to the end.			  			*/
	/* ---------------------------------------------------- */
	for (bp=(unsigned *) _showbp(); bp>last_bp; bp=(unsigned *) (bp[0]))
		{
		puts("Frame pointer: BP=") ;
		puts(utoh((unsigned) bp)) ;
		puts(", Return address: SS:[BP+2]=") ;
		puts(utoh(bp[1])) ;
		puts("\n") ;
		last_bp = bp ;
		}
	exit(255) ;
	}


/* ---------------------------------------------------------- */
/* This function simply returns the value held in register BP */
/* Note that the entry and exit points are designed to avoid  */
/* the implied (but not indicated) C prologue and epilog to   */
/* the function.  The prologue is always a PUSH BP followed   */
/* by a MOV BP,SP that would destroy the original value of BP */
/* that we wish to discover!				      */
/* ---------------------------------------------------------- */
static void show_bp()
	{
#ifndef	_lint	/* For Gimpel LINT */
#asm
_showbp_:
	mov	ax,bp
	ret
#end
#else
	show_bp() ;	/* Eliminates "not referenced" message from LINT */
#endif
	}


/* ------------------------------------------------------------------------ */
/* This function converts a 16-bit number into a string of four hex digits. */
/* (Using printf might have added ~10k to the EXE file, so we roll our own) */
/* ------------------------------------------------------------------------ */
static char *utoh(u)
unsigned u ;
	{
	static char bfr[] = "0000" ;
	static char hex[] = "0123456789ABCDEF" ;
	int i ;

	for (i = 3; i >= 0; i--)
		{
		bfr[i] = hex[u & 0xF] ;
		u >>= 4 ;
		}
	return bfr ;
	}


			/* End of file: HEAPCHEK.C */
