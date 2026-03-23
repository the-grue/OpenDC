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
		       /*  Start of file CLEANUP.C  */

/*
		 O88 Optimizer for DeSmet (C-Ware) C-Compiler
		 --------------------------------------------

			      Copyright (c) 1986

				      by

			    Key Software Products
		    440 Ninth Avenue, Menlo Park, CA 94025
				(415) 364-9847

			    (All rights reserved)


*/

#include	"inc\o88.h"

typedef struct CLEAN
	{
	struct CLEAN	*next ;
	unsigned	bytes ;
	CHAR		name[1] ;
	} CLEAN ;

#define	KEYOFFSET	4

#ifdef	_lint

PRIVATE VOID Clean_Enter(CHAR *, unsigned, unsigned) ;
PRIVATE	CHAR *Hash(CHAR *) ;

#else

PRIVATE VOID Clean_Enter() ;
PRIVATE	CHAR *Hash() ;

#endif

PRIVATE CLEAN		*cln_head = NULL ;
PRIVATE CHAR		*cleaning = NULL ;

#define	FIND_BYTES(n)	((CLEAN*)LL_Search((LINKLIST**)&cln_head,KEYOFFSET,n))

VOID Clean_Setup()
	{
	CHAR line[100], *number, savename[100], *ptr, *function ;
	unsigned len, len_str, len_num ;

	if (option.stack.enabled && clean_stack!=NULL)
		{
		strcpy(savename, temp_bfr.filename) ;
		Reuse_Buffer(&temp_bfr, clean_stack) ;
		while ((len = Get_Line(line, &temp_bfr)) != 0)
			{
			function = Leading(line) ;
			if ((ptr = index(function, ' ')) == NULL) continue ;
			*ptr = '\0' ;
			len_str = (unsigned) (ptr - function) ;
			number = Leading(ptr + 1) ;
			len_num = len - (unsigned) (number - line) ;
			len_num = Trailing(number, len_num) ;
			Clean_Enter(function, len_str,
				(unsigned) Value(number,len_num)) ;
			}
		Reuse_Buffer(&temp_bfr, savename) ;
		}
	}


PRIVATE VOID Clean_Enter(name, len_str, bytes)
CHAR *name ;
unsigned len_str ;
unsigned bytes ;
	{
	CLEAN *entry ;

	entry = (CLEAN *) Allocate(sizeof(CLEAN) + len_str) ;
	entry->bytes = bytes ;
	_move(len_str, name, entry->name) ;
	entry->name[len_str] = '\0' ;
	entry->next = cln_head ;
	cln_head = entry ;

	*Hash(name) = 1 ;
	}


VOID Clean_Function(name)
CHAR *name ;
	{
	static CHAR bfr[] = "00000" ;
	CLEAN *entry ;
	
	if ((*Hash(name) != 0) &&
	    ((entry = FIND_BYTES(name)) != NULL))
		{
		cleaning = Convert(entry->bytes, &bfr[4]) ;
		}
	else
		{
		cleaning = NULL ;
		}
	}


BOOLEAN Clean_Call()
	{
	unsigned bytes ;
	REG *dreg2, *sreg2 ;
	OPCODE *opc2 ;
	OPERAND *sop2 ;
	CLEAN *entry ;

	if ((stmt1->attb & CLEAND) != 0)
		{
		/* Already cleaned! */
		return FALSE ;
		}

	if ((*Hash(dorig1) == 0) ||
	    ((entry = FIND_BYTES(dorig1)) == NULL))
		{
		/* Not one of ours! */
		return FALSE ;
		}

	bytes = entry->bytes ;

	if (stmt2==NULL)
		{
		/* Screwy code: There's gotta be SOMETHING after a CALL! */
		stmt1->label = conflict ;
		stmt1->attb |= ERRMSG ;
		return TRUE ;
		}

	opc2 = stmt2->opcode ;
	dreg2 = stmt2->dst.reg ;
	sop2 = &stmt2->src ;
	sreg2 = sop2->reg ;

	if (opc2==MOV && dreg2==SP && sreg2==BP)
	    	{
		/* ----------------------------- */
		/* CALL XYZ	=>	CALL XYZ */
		/* MOV SP,BP		###	 */
		/* ----------------------------- */
		Emit(stmt2, null, call, dorig1, null)->attb |= CLEAND ;
		stmt1->label = cleaned ;
		stmt2->label = cleaned ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	if (opc2==ADD && dreg2==SP && sreg2==NULL && CONSTANT(sop2->attb))
	    	{
		/* ----------------------------- */
		/* CALL XYZ	=>	CALL XYZ */
		/* ADD SP,n		###	 */
		/* ----------------------------- */
		if (((unsigned) Value(sop2->orig, sop2->len)) != bytes)
			{
			stmt1->label = conflict ;
			stmt1->attb |= ERRMSG ;
			return TRUE ;
			}

		Emit(stmt2, null, call, dorig1, null)->attb |= CLEAND ;
		stmt1->label = cleaned ;
		stmt2->label = cleaned ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	/* ---------------------- */
	/* Should NEVER get here! */
	/* ---------------------- */
	stmt1->label = conflict ;
	stmt1->attb |= ERRMSG ;
	return TRUE ;
	}


BOOLEAN Clean_Ret()
	{
	if ((stmt1->attb & CLEAND) != 0)
		{
		/* Already cleaned! */
		return FALSE ;
		}

	if (cleaning!=NULL)
		{
		Emit(stmt1, null, ret, cleaning, null)->attb |= CLEAND ;
		stmt1->label = cleaned ;
		return TRUE ;
		}

	stmt1->attb |= CLEAND ;	/* Mark as processed */
	return FALSE ;
	}

VOID Clean_Free()
	{
	cln_head = (CLEAN *) LL_Free((CHAR *) cln_head, 0, 0) ;
	}

PRIVATE CHAR *Hash(str)
CHAR *str ;
	{
	static CHAR hash[256] = {FALSE} ;
	CHAR code, ch ;

	code = 0 ;
	while ((ch = *str++) != '\0')
		{
		code += ch ;
		}
	return &hash[code] ;
	}


			/*  End of file CLEANUP.C  */

