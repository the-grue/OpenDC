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
		       /*  Start of file SQUEEZE.C  */

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

#define	SPAN	64
#define	MASK	0x3F
#define	INCR(i)	((i)++, (i) &= MASK)
#define	DECR(i)	((i)--, (i) &= MASK)

typedef	struct STMT
	{
	unsigned	target ;
	CHAR		*cmt ;
	CHAR		src[124] ;
	} STMT ;

typedef struct QUEUE
	{
	STMT		span[SPAN] ;
	unsigned	nq ;
	unsigned	dq ;
	unsigned	len ;
	} QUEUE ;


typedef struct JUMP
	{
	struct JUMP	*next ;
	int	 	count ;
	CHAR	 	dst[1] ;
	} JUMP ;

#define	OFF_DST	4
#define	FIND_DST(d)	((JUMP*)LL_Search((LINKLIST**)&jump_head,OFF_DST,d))

PRIVATE JUMP *jump_head = NULL ;


/* Functions PRIVATE to SQUEEZE.C					*/
/* -------------------------------------------------------------------- */

#ifdef	_lint

PRIVATE VOID Enter_Dst(CHAR *) ;
PRIVATE VOID Flush_Queue(QUEUE *, BUFFER *) ;
PRIVATE unsigned Match(VOID) ;
PRIVATE VOID Delete_Code(unsigned) ;
PRIVATE unsigned Label_Code(unsigned) ;
PRIVATE VOID Dequeue(QUEUE *, BUFFER *) ;
PRIVATE CHAR *Enqueue(QUEUE *, BUFFER *) ;
PRIVATE BOOLEAN One_Pass(VOID) ;
PRIVATE VOID Compress(VOID) ;
PRIVATE CHAR *Extract(VOID) ;
PRIVATE VOID Mingle(CHAR *) ;

#else

PRIVATE VOID Enter_Dst() ;
PRIVATE VOID Flush_Queue() ;
PRIVATE unsigned Match() ;
PRIVATE VOID Delete_Code() ;
PRIVATE unsigned Label_Code() ;
PRIVATE VOID Dequeue() ;
PRIVATE CHAR *Enqueue() ;
PRIVATE BOOLEAN One_Pass() ;
PRIVATE VOID Compress() ;
PRIVATE CHAR *Extract() ;
PRIVATE VOID Mingle() ;

#endif


PRIVATE QUEUE *q1, *q2 ;
PRIVATE BUFFER *in_bfr, *pt1_bfr, *pt2_bfr ;

#define	MARKER	":O88"

VOID Squeeze()
	{
	CHAR savename[100] ;

	Init_Heap(1000) ;

	q1 = (QUEUE *) Allocate(sizeof(QUEUE)) ;
	q2 = (QUEUE *) Allocate(sizeof(QUEUE)) ;

	strcpy(savename, Extract()) ;
	Compress() ;
	Mingle(savename) ;
	}


PRIVATE CHAR *Extract()
	{
	CHAR *str ;
	BOOLEAN jmp ;
	JUMP *entry ;

	/* Try to re-use data already in the code buffer */
	Reverse_Buffer(in_bfr = &code_bfr) ;

	Reuse_Buffer(&temp_bfr, Make_Filename()) ;
	Reuse_Buffer(&inp_bfr, Make_Filename()) ;

	q1->nq = q1->dq = q1->len = 0 ;
	while ((str = Enqueue(q1, &temp_bfr)) != NULL)
		{
		jmp = EqualN(str, " JMP", 4) ;
		if (jmp || EqualN(str, " _RET", 5))
			{
			/* End of code fragment */
			/* -------------------- */
			Put_Line(MARKER, &temp_bfr) ;
			Flush_Queue(q1, &inp_bfr) ;
			Put_Line(MARKER, &inp_bfr) ;
			if (jmp)
				{
				if ((entry = FIND_DST(&str[5])) != NULL)
					{
					entry->count++ ;
					}
				else
					{
					Enter_Dst(&str[5]) ;
					}
				}
			}

		else if (str[0]==' ' && str[1]=='J')	/* Conditional Jump */
			{
			/* Start of code fragment */
			/* ---------------------- */
			Flush_Queue(q1, &temp_bfr) ;
			}
		}

	Close_Input(&code_bfr) ;
	Delete_File(code_bfr.filename) ;

	Flush_Queue(q1, &temp_bfr) ;
	Flush_Buffer(&temp_bfr) ;
	Close_Output(&temp_bfr) ;

	return temp_bfr.filename ;
	}


PRIVATE VOID Compress()
	{
	BUFFER *swap ;

	Reverse_Buffer(in_bfr = &inp_bfr) ;
	Reuse_Buffer(pt1_bfr = &temp_bfr, Make_Filename()) ;
	Reuse_Buffer(pt2_bfr = &code_bfr, Make_Filename()) ;

	while (One_Pass())
		{
		/* Swap input and part 2 output buffers */
		swap = in_bfr ;
		in_bfr = pt2_bfr ;
		pt2_bfr = swap ;

		/* Reset the buffers pointers to the beginning */
		Reverse_Buffer(in_bfr) ;
		Reuse_Buffer(pt2_bfr, pt2_bfr->filename) ;
		}

	Delete_File(in_bfr->filename) ;
	Delete_File(pt2_bfr->filename) ;
	}


PRIVATE VOID Mingle(savename)
CHAR *savename ;
	{
	CHAR line[200] ;

	Reverse_Buffer(&temp_bfr) ;		/* Compressed redundant */
	Reuse_Buffer(&inp_bfr, savename) ;	/* Non-redundant code */
	Reuse_Buffer(&code_bfr, Make_Filename()) ;	/* Merged result */

	while (Get_Line(line, &inp_bfr) != 0)
		{
		if (Equal(line, MARKER))
			{
			while (Get_Line(line, &temp_bfr) != 0)
				{
				if (Equal(line, MARKER)) break ;
				Put_Line(line, &code_bfr) ;
				}
			}
		else Put_Line(line, &code_bfr) ;
		}

	Close_Input(&inp_bfr) ;
	Delete_File(inp_bfr.filename) ;

	Close_Input(&temp_bfr) ;
	Delete_File(temp_bfr.filename) ;
	}


PRIVATE BOOLEAN One_Pass()
	{
	static CHAR bfr[] = "00000" ;
	CHAR dst[10] ;
	unsigned len ;
	CHAR *str1, *str2 ;
	int pair ;
	BOOLEAN jmp ;
	JUMP *entry, *newjmp ;


	q1->nq = q1->dq = q1->len = 0 ;
	while ((str1 = Enqueue(q1, pt1_bfr)) != NULL)
		{
		if (*str1 != ' ') continue ;
		jmp = EqualN(str1, " JMP", 4) ;
		if (!jmp && !EqualN(str1, " _RET", 5))
			{
			continue ;
			}
		if (jmp)
			{
			entry = FIND_DST(&str1[5]) ;
			if (entry!=NULL && entry->count==0) continue ;
			}
		pair = *((int *) &str1[1]) ;
		q2->nq = q2->dq = q2->len = 0 ;
		while ((str2 = Enqueue(q2, pt2_bfr)) != NULL)
			{
			if (*str2 == ';') continue ;
			if (*((int *) &str2[1]) != pair) continue ;
			if ((len = Match()) < 2) continue ;
			Delete_Code(len);
			Flush_Queue(q2, pt2_bfr) ;
			Put_Str(" JMP ", pt2_bfr) ;
			_move(5, "_O88_", dst) ;
			strcpy(&dst[5], Convert(Label_Code(len),
					&bfr[sizeof(bfr) - 2])) ;
			Put_Line(dst, pt2_bfr) ;
			stats.cseg_ins++ ;
			if ((newjmp = FIND_DST(dst)) != NULL)
				{
				newjmp->count++ ;
				}
			else Enter_Dst(dst) ;
			}

		if (jmp && entry!=NULL)
			{
			jump_head = entry->next ;
			My_Free((CHAR *) entry) ;
			}

		break ;
		}
	Close_Input(in_bfr) ;
	Flush_Queue(q1, pt1_bfr) ;
	Flush_Queue(q2, pt2_bfr) ;
	return (str1 != NULL) ;
	}


PRIVATE unsigned Match()
	{
	unsigned matched ;
	CHAR *str1, *str2 ;

	STMT *span1 = q1->span ;
	unsigned len1 = q1->len ;
	unsigned nq1 = q1->nq ;

	STMT *span2 = q2->span ;
	unsigned len2 = q2->len ;
	unsigned nq2 = q2->nq ;

	matched = 0 ;
	while (len1!=0 && len2!=0)
		{
		do
			{
			DECR(nq1) ;
			str1 = span1[nq1].src ;
			} while (--len1!=0 && *str1==';') ;

		if (*str1!=' ') break ;

		do
			{
			DECR(nq2) ;
			str2 = span2[nq2].src ;
			} while (--len2!=0 && *str2==';') ;

		if (*str2!=' ') break ;

		if (!Equal(str1, str2)) break ;

		matched++ ;
		}

	return matched ;
	}


PRIVATE VOID Delete_Code(len)
unsigned len ;
	{
	unsigned nq = q2->nq ;
	STMT *span = q2->span ;

	stats.cseg_del += len ;
	while (len != 0)
		{
		DECR(nq) ;
		if (span[nq].src[0] != ';')
			{
			span[nq].cmt = redundant ;
			len-- ;
			}
		}
	}


PRIVATE VOID Flush_Queue(q, bfr)
QUEUE *q ;
BUFFER *bfr ;
	{
	while (q->len != 0) Dequeue(q, bfr) ;
	}


PRIVATE VOID Dequeue(q, bfr)
QUEUE *q ;
BUFFER *bfr ;
	{
	STMT *ptr ;
	static CHAR nmbr[] = "00000" ;

	q->len-- ;
	ptr = &q->span[q->dq] ;
	INCR(q->dq) ;
	if (ptr->cmt != NULL)
		{
		if (!option.comments.enabled) return ;
		Put_Str(ptr->cmt, bfr) ;
		}
	if (ptr->target != 0)
		{
		Put_Str("_O88_", bfr) ;
		Put_Str(
			Convert(ptr->target, &nmbr[sizeof(nmbr) - 2]),
			bfr) ;
		Put_Line(":", bfr) ;
		}
	Put_Line(ptr->src, bfr) ;
	}


PRIVATE unsigned Label_Code(len)
unsigned len ;
	{
	unsigned nq = q1->nq ;
	STMT *span = q1->span ;
	STMT *ptr ;

	while (len != 0)
		{
		DECR(nq) ;
		if (span[nq].src[0] == ' ') len-- ;
		}
	ptr = &span[nq] ;

	if (ptr->target == 0) ptr->target = ++o88_labels ;

	return ptr->target ;
	}

PRIVATE CHAR *Enqueue(q, bfr)
QUEUE *q ;
BUFFER *bfr ;
	{
	STMT *stmt ;
	CHAR *str ;

	if (q->len == SPAN) Dequeue(q, bfr) ;
	stmt = &q->span[q->nq] ;
	if (Get_Line(str = stmt->src, in_bfr) == 0) return NULL ;
	q->len++ ;
	INCR(q->nq) ; 
	stmt->target = 0 ;
	stmt->cmt = NULL ;
	if (*str!=';' && *str!=' ') Flush_Queue(q, bfr) ;
	return str ;
	}

PRIVATE VOID Enter_Dst(dst)
CHAR *dst ;
	{
	JUMP *entry ;
	unsigned len ;

	entry = (JUMP *) Allocate(sizeof(JUMP) + (len = strlen(dst))) ;
	_move(len + 1, dst, entry->dst) ;
	entry->count = 0 ;
	entry->next = jump_head ;
	jump_head = entry ;
	}

		    /*  End of file SQUEEZE.C  */
