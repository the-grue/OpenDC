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
		       /*  Start of file JUMPS.C  */

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

typedef struct JUMP
	{
	struct JUMP	*next ;
	CHAR		*to ;
	CHAR		fm[1] ;
	} JUMP ;

#define	OFF_TO	2
#define	OFF_FM	4

#define	FIND_JUMP(fm)	((JUMP*)LL_Search((LINKLIST**)&jmp_head,OFF_FM,fm))

PRIVATE JUMP *jmp_head = NULL ;

#ifdef	_lint

PRIVATE VOID Jump_Pass1(VOID) ;
PRIVATE VOID Jump_Pass2(VOID) ;
PRIVATE VOID Enter_Fm(CHAR *) ;
PRIVATE VOID Enter_To(CHAR *) ;
PRIVATE CHAR *Switch(CHAR *) ;
PRIVATE CHAR *Hash(CHAR *) ;
PRIVATE unsigned Get_Len(CHAR *, CHAR) ;

#else

PRIVATE VOID Jump_Pass1() ;
PRIVATE VOID Jump_Pass2() ;
PRIVATE VOID Enter_Fm() ;
PRIVATE VOID Enter_To() ;
PRIVATE CHAR *Switch() ;
PRIVATE CHAR *Hash() ;
PRIVATE unsigned Get_Len() ;

#endif


VOID Jump_Chains()	/* PUBLIC ENTRY POINT */
	{
	Init_Heap(1000) ;
	Jump_Pass1() ;
	Jump_Pass2() ;
	}

PRIVATE VOID Jump_Pass1()
	{
	/* ---------------------------------------------------------------- */
        /*                 JUMP CHAIN COLLAPSING - PASS 1                   */
	/*								    */
	/* Build the JMP chain equivalence base: Every time a labelled JMP  */
	/* instruction is found, enter its label (fm) and destination (to)  */
	/* into the data base.						    */
	/* ---------------------------------------------------------------- */
	CHAR str[200] ;
	BOOLEAN labelled ;

	/* Try to re-use data already in the code buffer */
	Reverse_Buffer(&code_bfr) ;

	labelled = FALSE ;
	while (Get_Line(str, &code_bfr) != 0)
		{
		if (str[0] == ';') continue ;

		if (str[0] != ' ')
			{
			Enter_Fm(str) ;
			labelled = TRUE ;
			continue ;
			}

		if (EqualN(str, " JMP", 4) ||
		    EqualN(str, " jmp", 4))
			{
			if (labelled) Enter_To(str) ;
			}

		jmp_head = (JUMP *) LL_Free((CHAR *) jmp_head, OFF_TO, 0);
		labelled = FALSE ;
		}
	}


PRIVATE VOID Jump_Pass2()
	{
	/* ---------------------------------------------------------------- */
        /*                 JUMP CHAIN COLLAPSING - PASS 2                   */
	/*								    */
	/* Make code replacements:					    */
	/*								    */
	/*     If a JMP or switch table (DW's) are encountered, check to    */
	/*     check to see if the destination is entered as a 'fm' in      */
	/*     the table.  If it is, replace the destination with the	    */
	/*     corresponding 'to' member.  Do this recursively until the    */
	/*     last search does not find a match in the table.  Then re-    */
	/*     place the destination of the instruction by that obtained    */
	/*     from the table.						    */
	/*								    */
	/* ---------------------------------------------------------------- */
	unsigned len ;
	JUMP *this ;
	CHAR str[200], *new_dst, *old_dst, bfr[100] ;


	Rewind_Buffer(&code_bfr) ;

	if (jmp_head == NULL)
		{
		Copy_Buffer(&code_bfr, &inp_bfr) ;
		return ;
		}

	/* Build the hash table */
	for (this = jmp_head; this != NULL; this = this->next)
		{
		*Hash(this->fm) = 1 ;
		}

	while (Get_Line(str, &code_bfr) != 0)
		{
		if (str[0] == ';') goto output ;

		if ((old_dst = Switch(str)) != NULL)
			{
			}

		else if (EqualN(str, " JMP", 4) ||
		         EqualN(str, " jmp", 4))
			{
			old_dst = &str[5] ;
			}

		if (old_dst != NULL)
			{
			len = Get_Len(old_dst, ';') ;
			_move(len, old_dst, new_dst = bfr) ;
			bfr[len] = '\0' ;
			while (*Hash(new_dst) != 0 &&
			       (this = FIND_JUMP(new_dst)) != NULL)
				{
				new_dst = this->to ;
				stats.cseg_smp++ ;
				}

			if (new_dst != bfr)
				{
				if (option.comments.enabled)
					{
					Put_Str(new_jmp_dst, &inp_bfr) ;
					Put_Line(str, &inp_bfr) ;
					}
				strcpy(old_dst, new_dst) ;
				}
			}

		output:	Put_Line(str, &inp_bfr) ;
		}

	jmp_head = (JUMP *) LL_Free((CHAR *) jmp_head, 0, OFF_TO) ;

	Close_Input(&code_bfr) ;
	Delete_File(code_bfr.filename) ;
	}


PRIVATE CHAR *Switch(str)
CHAR *str ;
	{
	static unsigned switch_stmt = 0 ;
	CHAR *ptr, *data ;

	if (switch_stmt != 0) switch_stmt++ ;

	else if (EqualN(str, " _CALL _SWITCH", 13) ||
	         (EqualN(str, " JMP", 4) && index(str, '[')!=NULL))
		{
		switch_stmt = 1 ;	/* Start of switch */
		}

	if (switch_stmt > 2)	/* DW's with labels start on 3rd line */
		{
		if (*str != ' ')	/* End of switch ? */
			{
			switch_stmt = 0 ;
			}
		else		/* Point to label! */
			{
			if ((ptr = index(data = &str[4], ',')) != NULL)
				{
				return ptr + 1 ;
				}
			return data ;
			}
		}

	return NULL ;
	}


PRIVATE VOID Enter_Fm(str)
CHAR *str ;
	{
	JUMP *entry ;
	unsigned len ;

	entry = (JUMP *) Allocate(sizeof(JUMP) + (len = Get_Len(str, ':'))) ;
	_move(len, str, entry->fm) ;
	entry->fm[len] = '\0' ;
	entry->to = NULL ;
	entry->next = jmp_head ;
	jmp_head = entry ;
	}


PRIVATE VOID Enter_To(str)
CHAR *str ;
	{
	JUMP *entry ;
	CHAR *target ;
	unsigned len ;

	for (target = str + 4; *target==' '; target++)
		{
		/* Locate start of destination label */
		}

	len = Get_Len(target, ';') ;
	for (entry = jmp_head; entry != NULL; entry = entry->next)
		{
		if (entry->to != NULL) break ;
		entry->to = Make_String(target, len + 1) ;
		}
	}


PRIVATE CHAR *Hash(lbl)
CHAR *lbl ;
	{
	unsigned code, digit ;
	static CHAR hash[100] = {0} ;

	code = 0 ;
	if (lbl[0]=='_' && lbl[1]=='L')
		{
		if ((digit = (unsigned) (lbl[2] - '0')) < 10)
			{
			code = digit ;
			if ((digit = (unsigned) (lbl[3] - '0')) < 10)
				{
				code = 10 * code + digit ;
				}
			}
		}
	return &hash[code] ;
	}

PRIVATE unsigned Get_Len(str, ch)
CHAR *str ;
CHAR ch ;
	{
	CHAR *ptr ;
	unsigned len ;

	if ((ptr = index(str, ch)) != NULL)
		{
		len = (unsigned) (ptr - str) ;
		}
	else
		{
		ptr = &str[len = strlen(str)] ;
		}
	while (*--ptr == ' ') len-- ;
	return len ;
	}


		    /*  End of file JUMPS.C  */

