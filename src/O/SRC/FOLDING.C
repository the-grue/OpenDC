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
		       /*  Start of file FOLDING.C  */

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

typedef struct FOLDING
	{
	struct FOLDING	*next ;
	CHAR		*data ;
	unsigned	len ;
	unsigned	bytes ;
	CHAR		lbl[1] ;
	} FOLDING ;

PRIVATE FOLDING *fold_head = NULL ;

#ifdef	_lint

PRIVATE unsigned Align_Even(CHAR *, unsigned, unsigned, BUFFER *) ;
PRIVATE VOID Enter_Const(CHAR *, unsigned, unsigned) ;
PRIVATE CHAR *Find_Data(CHAR *, unsigned) ;
PRIVATE unsigned Real_Const(CHAR *, CHAR *) ;
PRIVATE unsigned String_Const(CHAR *, CHAR *, unsigned) ;
PRIVATE unsigned String_Bytes(CHAR *) ;
PRIVATE BOOLEAN Digits(CHAR *) ;

#else

PRIVATE unsigned Align_Even() ;
PRIVATE VOID Enter_Const() ;
PRIVATE CHAR *Find_Data() ;
PRIVATE unsigned Real_Const() ;
PRIVATE unsigned String_Const() ;
PRIVATE unsigned String_Bytes() ;
PRIVATE BOOLEAN Digits() ;

#endif

VOID Fold_Constants()	/* PUBLIC ENTRY POINT */
	{
	CHAR *equiv ;
	CHAR bfr[200], *data ;
	unsigned bytes, len, length ;
	unsigned ds_bytes, es_bytes ;
	unsigned *segp ;
	BOOLEAN equ, dseg ;

	Init_Heap(1000) ;

	Reverse_Buffer(&data_bfr) ;

	es_bytes = ds_bytes = 0 ;
	dseg = FALSE ;
	while ((len = Get_Line(bfr, &data_bfr)) != 0)
		{
		equ = FALSE ;
		data = index(bfr, ' ') + 1 ;
		length = len - (data - bfr) ;
		if (EqualN(data, "DSEG", 4)) dseg = TRUE ;
		else if (EqualN(data, "ESEG", 4)) dseg = FALSE ;
		else if (option.fold.enabled && dseg &&
		    !EqualN(data, "PUBLIC", 6) &&
		    ((bytes = Real_Const(bfr, data)) != 0 ||
		     (bytes = String_Const(bfr, data, length)) != 0))
			{
			if ((equiv = Find_Data(data, length)) != NULL)
				{
				/* Duplicate constant already exists */
				if (option.comments.enabled)
					{
					Put_Str(folded, &inp_bfr) ;
					Put_Line(bfr, &inp_bfr) ;
					}
				_move(4, "EQU ", data) ;
				strcpy(data + 4, equiv) ;
				stats.dseg_del += bytes ;
				equ = TRUE ;
				}
			else
				{
				/* First time this constant was found */
				Enter_Const(bfr, len, bytes) ;
				}
			}

		if (option.time.enabled && bfr[0]!=';' && !equ)
			{
			segp = dseg ? &ds_bytes : &es_bytes ;
			*segp = Align_Even(bfr, len, *segp, &inp_bfr) ;
			}

		Put_Line(bfr, &inp_bfr) ;
		}

	/* ---------------------------------------------------- */
	/* There's no need to release memory: Routines that	*/
	/* follow and need heap space, re-initialize the heap.	*/
	/* ---------------------------------------------------- */

	Close_Input(&data_bfr) ;
	Delete_File(data_bfr.filename) ;
	}


PRIVATE VOID Enter_Const(str, len, bytes)
CHAR *str ;
unsigned len ;
unsigned bytes ;
	{
	FOLDING *entry, *node, *previous ;

	entry = (FOLDING *) Allocate(sizeof(FOLDING) + len) ;
	_move(len + 1, str, entry->lbl) ;
	entry->data = index(entry->lbl, ' ') ;
	*entry->data++ = '\0' ;
	entry->len = len - (entry->data - entry->lbl) ;
	entry->bytes = bytes ;

	/* Ordered insertion: largest 'len' first */
	previous = (FOLDING *) &fold_head ;
	for (node = fold_head; node != NULL; node = node->next)
		{
		if (node->len <= entry->len) break ;
		previous = node ;
		}

	entry->next = previous->next ;
	previous->next = entry ;
	}


PRIVATE CHAR *Find_Data(data, len)
CHAR *data ;
unsigned len ;
	{
	static CHAR equiv[20] ;
	static CHAR offset[] = "00000" ;
	CHAR *ep, *dp ;
	FOLDING *entry ;
	unsigned off, i ;

	for (entry = fold_head; entry!=NULL; entry = entry->next)
		{
		if (entry->len < len)
			{
			/* List is ordered by descending 'len' */
			break ;
			}
		if (data[1] == 'W')
			{
			/* Real Constant */
			if (Equal(entry->data, data)) return entry->lbl ;
			}
		else
			{
			/* String Constant */
			ep = &entry->data[entry->len] ;
			dp = &data[len] ;
			for (i = 0; i < len-3; i++)
				{
				if (*--dp != *--ep) break ;
				}

			if (dp != &data[3]) continue ;
			if (ep == &entry->data[3]) return entry->lbl ;

			/* Substring Match! */
			strcpy(equiv, entry->lbl) ;
			strcat(equiv, "+") ;

			/* Determine Offset from beginning */
			off = entry->bytes - String_Bytes(&data[3]);
			strcat(equiv, Convert(off, &offset[4])) ;

			return equiv ;
			}
		}

	return NULL ;
	}


PRIVATE unsigned Real_Const(bfr, data)
CHAR *bfr ;
CHAR *data ;
	{
	unsigned len ;

	if (bfr[0]=='_' && bfr[1]=='F' && Digits(bfr + 2) &&
	    EqualN(data, "DW ", 3))
	    	{
		len = 8 ;
		}
	else len = 0 ;
	return len ;
	}


PRIVATE unsigned String_Bytes(str)
CHAR *str ;
	{
	BOOLEAN quoted ;
	unsigned commas, literals, quotes ;
	CHAR ch ;

	quoted = FALSE ;
	quotes = literals = commas = 0 ;
	while ((ch = *str++) != '\0')
		{
		if (ch == '\'')
			{
			quoted = !quoted ;
			quotes++ ;
			}
		else if (quoted) literals++ ;
		else if (ch == ',') commas++ ;
		}

	return (commas + 1) - (quotes >> 1) + literals ;
	}


PRIVATE unsigned String_Const(bfr, data, len)
CHAR *bfr ;
CHAR *data ;
unsigned len ;
	{
	if (bfr[0]=='_' && bfr[1]=='_' && Digits(bfr + 2) &&
	    EqualN(data, "DB ", 3) &&
	    data[len - 1]=='0')
		{
		len = String_Bytes(&data[3]) ;
		}
	else
		{
		len = 0 ;
		}
	return len ;
	}


PRIVATE BOOLEAN Digits(label)
CHAR *label ;
	{
	while (isdigit(*label)) label++ ;
	return *label == ' ' ;
	}

PRIVATE unsigned Align_Even(line, len, bytes, bfr)
CHAR *line ;
unsigned len ;
unsigned bytes ;
BUFFER *bfr ;
	{
	static CHAR align[] = " DB 0 ; {O88: Even address alignment}" ;
	CHAR *ptr ;
	unsigned n ;

	ptr = index(line, ' ') + 4 ;
	len -= (ptr - line) ;

	if (ptr[-2] != 'B')
		{
		/* Not a byte operand (e.g., DW). Is this data labelled ? */
		if (line[0] != ' ')
			{
			/* Yes.  Are we on even adrs ? */
			if ((bytes & 1) != 0)
				{
				/* No.  Make it even. */
				Put_Line(align, bfr) ;
				stats.dseg_del-- ;
				}
			bytes = 0 ;	/* Whatever .. we're even now! */
			}
		}
	else if (ptr[-3]=='R')
		{
		/* RB pseudo-op */
		n = Value(ptr, len) ;
		if ((n & 1)==0 && line[0]!=' ')
			{
			/* Labelled RB (2N) - OK to align */
			if ((bytes & 1) != 0)
				{
				/* Odd address - force even */
				Put_Line(align, bfr) ;
				stats.dseg_del-- ;
				}
			bytes = 0 ;	/* Is now aligned */
			}
		else bytes += n ; /* RB (2N+1) or not labelled */
		}
	else /* if (ptr[-3]=='D') */
		{
		/* DB pseudo-op */
		bytes += String_Bytes(ptr) ;
		}

	return bytes ;
	}


			/*  End of file FOLDING.C  */

