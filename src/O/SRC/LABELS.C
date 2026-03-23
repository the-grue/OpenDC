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
		       /*  Start of file LABELS.C  */

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

#include "inc\o88.h"

/* Functions PRIVATE to LABELS.C...					*/
/* -------------------------------------------------------------------- */

#ifdef	_lint

/*lint +fvr	*/
PRESERVE	*Remember_Ref(CHAR *) ;

/*lint -fvr	*/
PRESERVE	*Enter_Label(CHAR *) ;
VOID		Release_Regs(VOID) ;
VOID		Remember_Def(CHAR *, OPCODE *) ;
VOID		Remember_Del(CHAR *) ;
FLAGS		Merge(REG *, REG_STATE *) ;
BOOLEAN		Same_Label(CHAR *, CHAR *) ;

#else

PRESERVE	*Remember_Ref() ;
PRESERVE	*Enter_Label() ;
VOID		Release_Regs() ;
VOID		Remember_Def() ;
VOID		Remember_Del() ;
FLAGS		Merge() ;
BOOLEAN		Same_Label() ;

#endif


/* Variables PRIVATE to LABELS.C...					*/
/* -------------------------------------------------------------------- */
PRIVATE BOOLEAN		dead = TRUE ;


BOOLEAN Jump()
	{
#ifdef	FINISHED
	REG_STATE *s ;
#endif
	OPCODE *opc2 ;
	PRESERVE *entry ;
	REG *rh, *rl ;
	PARTS *stmt, *my2, *my3 ;
	FLAGS attb ;
	int i ;
	PACKED n ;
	static CHAR bfr[] = "000" ;
	static OPCODE *jmp[] =
		{JZ, JNZ, JAE, JA, JB, JBE, JG, JGE, JL, JLE} ;
	static CHAR *njmp[] =
		{
		"JNZ", "JZ", "JB", "JBE", "JAE",
		"JA", "JLE", "JL", "JGE", "JG"
		} ;

	if (index(dorig1, '[') != NULL)		/* "switch" statement? */
		{
		switch_stmt = 1 ;
		return TRUE ;
		}

	if (opc1 == JMP)
		{
		stmt = Get_Next(stmt1) ;
		if (Same_Label(stmt->label, dorig1))
			{
			/*
			-------------------------------------
				JMP lbl  =>       ###
			lbl:                 lbl: (if needed)
			-------------------------------------
			*/
			if (FIND_LABEL(dorig1)==NULL)
				{
				stmt->mnemonic = stmt->label ;
				stmt->label = deleted ;
				}
			stmt1->label = deleted ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}
	else if (stmt2!=NULL /* && (opc1->attb & CONDJMP) */)
		{
		my2 = stmt2 ;
		while (my2!=NULL && Unnecessary(my2))
			{
			my2->label = redundant ;
			stats.cseg_del++ ;
			my2 = Find_Next(my2) ;
			}

		opc2 = my2->opcode ;
		if (my2==stmt2 && ((my3 = Find_Next(my2)) != NULL) &&
		   ((stmt = Get_Next(my3)) != NULL) &&
		   (opc2==LEA || opc2==MOV ||
		    (opc2==XOR && Equal(my2->dst.orig, my2->src.orig))) &&
		    my3->opcode==JMP && Same_Label(stmt->label, dorig1))
		    	{
			stmt = Find_Next(stmt) ;
			if (stmt!=NULL &&
			    (stmt->opcode==LEA || stmt->opcode==MOV ||
			     (stmt->opcode==XOR &&
			      Equal(stmt->dst.orig, stmt->src.orig))) &&
			    Equal(my2->dst.orig, stmt->dst.orig) &&
			    Same_Label(Get_Next(stmt)->label,my3->dst.orig) &&
			    FIND_LABEL(dorig1)==NULL)
				{
				/*
				--------------------------------------------
					Jc lbl1            LEA|MOV dst,src2
					LEA|MOV dst,src1   Jc lbl1      
					JMP lbl2           LEA|MOV dst,src1
				lbl1:                    lbl1:
					LEA|MOV dst,src2
				lbl2:                    lbl2: (if needed)
				--------------------------------------------
				*/
				if (stmt->opcode == XOR)
					{
					for (i = 0; i < 10; i++)
						{
						if (jmp[i] == opc1)
							{
							break ;
							}
						}
					Emit(my3, null, stmt->mnemonic,
						stmt->dst.orig,
						stmt->src.orig) ;
					Emit(my3, null, njmp[i], dorig1,
						null) ;
					Emit(my3, null, my2->mnemonic,
						my2->dst.orig,
						my2->src.orig) ;
					}
				else
					{
					Emit(my3, null, my2->mnemonic,
						my2->dst.orig,
						my2->src.orig) ;
					Emit(my3, null, stmt1->mnemonic,
						dorig1, null) ;
					Emit(my3, null, stmt->mnemonic,
						stmt->dst.orig,
						stmt->src.orig) ;
					}
				stmt->label = prefetch ;
				if (FIND_LABEL(my3->dst.orig)==NULL)
					{
					stmt = Get_Next(stmt) ;
					stmt->mnemonic = stmt->label ;
					stmt->label = deleted ;
					}
				my3->label = prefetch ;
				my2->label = prefetch ;
				stmt1->label = prefetch ;
				stats.cseg_del++ ;
				return TRUE ;
				}
			}

		stmt = Get_Next(my2) ;
		if (my2->opcode==JMP &&
		    Same_Label(stmt->label, dorig1))
		    	{
			/*
			----------------------------------------
				Jc  lbl1	=>	JNc lbl2
				JMP lbl2
			lbl1:
			----------------------------------------
			*/
			for (i = 0; i < 10; i++)
				{
				if (jmp[i] == opc1)
					{
					Emit(my2, null, njmp[i],
					     my2->dst.orig, null);
					if (FIND_LABEL(dorig1)==NULL)
						{
						stmt->mnemonic = stmt->label ;
						stmt->label = deleted ;
						}
					my2->label = prefetch ;
					stmt1->label = prefetch ;
					stats.cseg_del++ ;
					return TRUE ;
					}
				}
			}
		}

	entry = Remember_Ref(dorig1) ;

	if (opc1==JMP)
		{
		}

	else if (jmp_reg==NULL)
		{
		}

	else if (opc1==JNZ)
		{
		_move((jmp_reg->len=cmp_len) + 1, cmp_val, jmp_reg->content) ;
		LOAD_ATTB(jmp_reg, cmp_att) ;
		if (jmp_reg->name[1]=='X' && CONSTANT(cmp_att))
			{
			n.word = (unsigned) Value(cmp_val, cmp_len) ;

			rh = RH(jmp_reg) ;
			strcpy(rh->content, Convert(n.byte.msb, &bfr[2])) ;
			rh->len = strlen(rh->content) ;
			attb = (n.byte.msb!=0) ? _NUMB : (_NUMB|_ZERO) ;
			LOAD_ATTB(rh, attb) ;

			rl = RL(jmp_reg) ;
			strcpy(rl->content, Convert(n.byte.lsb, &bfr[2])) ;
			rl->len = strlen(rl->content) ;
			attb = (n.byte.lsb!=0) ? _NUMB : (_NUMB|_ZERO) ;
			LOAD_ATTB(rl, attb) ;
			}
		}


	else if (opc1==JZ)
		{
		/* ? */
		}

	else if (jmp_reg->name[1]!='X' || !CONSTANT(cmp_att))
		{
		}

	else if (opc1==JA)
		{
		n.word = (unsigned) Value(cmp_val, cmp_len) ;
		if (n.byte.msb == 0)
			{
			rh = RH(jmp_reg) ;
			rh->len = 1 ;
			rh->content[0] = '0' ;
			rh->content[1] = '\0' ;
			LOAD_ATTB(rh, _NUMB|_ZERO) ;
			}
		}

	else if (opc1==JAE)
		{
		n.word = (unsigned) Value(cmp_val, cmp_len) ;
		if (n.word <= 256)
			{
			rh = RH(jmp_reg) ;
			rh->len = 1 ;
			rh->content[0] = '0' ;
			rh->content[1] = '\0' ;
			LOAD_ATTB(rh, _NUMB|_ZERO) ;
			}
		}

	else if ((entry->status & DEFINED) != 0)
		{
		/* Backward Reference */
		return TRUE ;
		}

#ifdef	FINISHED
	else if (opc1==JB)
		{
		n.word = (unsigned) Value(cmp_val, cmp_len) ;
		if (n.word <= 256)
			{
			rh = RH(jmp_reg) ;
			s = entry->regs[rh - reg] ;
			/* ? */
			}
		}

	else if (opc1==JBE)
		{
		n.word = (unsigned) Value(cmp_val, cmp_len) ;
		if (n.word <= 255)
			{
			rh = RH(jmp_reg) ;
			s = entry->regs[rh - reg] ;
			/* ? */
			}
		}
#endif

	return TRUE ;
	}


PRIVATE VOID Remember_Del(label)
CHAR *label ;
	{
	PRESERVE *entry ;

	entry = FIND_LABEL(label) ;
	if (entry==NULL)
		{
		entry = Enter_Label(label) ;
		}

	entry->status |= DELETED ;
	}


PRIVATE PRESERVE *Remember_Ref(label)
CHAR *label ;
	{
	PRESERVE *entry ;
	REG *r ;
	REG_STATE **sp, *s ;
	unsigned bytes ;

	entry = FIND_LABEL(label) ;
	if (entry==NULL)
		{
		entry = Enter_Label(label) ;
		}

	if ((entry->status & DEFINED) != 0)
		{
		/* BACKWARD REFERENCE: Registers are */
		/* considered as EMPTY by compiler,  */
		/* but we will remember what was in  */
		/* them by marking them AVAILABLE!   */
		/* --------------------------------- */
		Release_Regs() ;
		entry->status |= REFERENCE ;
		return entry ;
		}

	if ((entry->status & REFERENCE) != 0)
		{
		/* SUBSEQUENT FORWARD REFERENCE: Merge */
		/* current and preserved states.       */
		/* ----------------------------------- */
		sp = entry->regs ;
		for (r = AH; r <= ES; r++)
			{
			s = *sp++ ;
			s->attb = (s->attb & _FXD) | Merge(r, s) ;
			}
		return entry ;
		}

	/* FIRST FORWARD REFERENCE:  Load register state. */
	/* ---------------------------------------------- */
	sp = entry->regs ;
	for (r = AH; r <= ES; r++)
		{
		bytes = r->len + sizeof(REG_STATE) ;
		*sp++ = s = (REG_STATE *) Allocate(bytes) ;
		_move(bytes, (CHAR *) &r->attb, (CHAR *) &s->attb) ;
		}

	entry->status |= REFERENCE ;

	return entry ;
	}


PRIVATE VOID Remember_Def(label, last_opc)
CHAR *label ;
OPCODE *last_opc ;
	{
	FLAGS state ;
	REG *r ;
	PRESERVE *entry ;
	REG_STATE **sp, *s ;
	CHAR lbl[80] ;

	/* Remove the trailing ':' */
	lbl[strlen(strcpy(lbl, label)) - 1] = '\0' ;
	entry = FIND_LABEL(lbl) ;
	if (entry==NULL)
		{
		entry = Enter_Label(lbl) ;
		}

	if ((entry->status & REFERENCE) != 0)
		{
		/* FORWARD REFERENCE LABEL: Modify */
		/* current register state.          */
		/* -------------------------------- */

		sp = entry->regs ;	/* Preserved register state */

		if (dead)
			{
			/* This code can only be reached by a	*/
			/* forward jump.  The initial register	*/
			/* state for this code may be loaded	*/
			/* from that in effect at the jump.	*/
			/* ------------------------------------ */
			for (r = AH; r <= ES; r++)
				{
				s = *sp++ ;
				_move(s->len + sizeof(REG_STATE),
					(CHAR *) &s->attb,
					(CHAR *) &r->attb) ;
				}
			}
		else
			{
			/* This code can either be reached by a forward	*/
			/* jump, or by falling through from the code	*/
			/* directly above it.  The initial register	*/
			/* state is determined by merging the register	*/
			/* state from above with that in effect at the	*/
			/* jump.					*/

			for (r = AH; r <= ES; r++)
				{
				r->attb = (r->attb & _FXD) | Merge(r, *sp++) ;
				}
			}


		if (lbl[0] != '_')
			{
			/* USER LABEL: Technically, all	regis-	*/
			/* ters are "free" at this point.	*/
			/* Mark them as AVAILABLE to try to use	*/
			/* their present contents if possible.	*/
			/* ------------------------------------ */
			Release_Regs() ;
			}

		/* Label no longer needed - recover memory */
		/* --------------------------------------- */
		Remove_Entry() ;
		return ;
		}

	/* BACKWARD REFERENCE LABEL:  Assume that */
	/* all registers may be considered EMPTY  */
	/* unless preceded by JMP: "if (1) ... "  */
	/* -------------------------------------- */
	entry->status |= DEFINED ;
	state = (last_opc == JMP) ? _UNKN : _EMPTY ;
	Free_Registers(state, state) ;
	}


PRIVATE FLAGS Merge(r, s)
REG *r ;
REG_STATE *s ;
	{
	FLAGS attb ;

	attb = r->attb | s->attb ;
	if ((attb & _EMPTY) != 0)
		{
		return _EMPTY ;
		}
	else if ((attb & _UNKN)!=0 || ((r->attb ^ s->attb) & ~_AVAIL)!=0 ||
		 (r->len != s->len) || !Equal(r->content, s->content))
		{
		return _UNKN | (attb & _AVAIL) ;
		}
	return r->attb | (attb & _AVAIL) ; /* No change */
	}


BOOLEAN Labels()
	{
	CHAR lbl[100] ;
	PRESERVE *entry ;
	CHAR *ptr, *label = stmt1->label ;
	static OPCODE *prev_opc = NULL ;
	OPCODE *last_opc ;

	last_opc = prev_opc ;
	prev_opc = opc1 ;

	/* Process label list of switch statement */
	/* -------------------------------------- */
	if (switch_stmt != 0)
		{
		switch_stmt++ ;
		}

	if (switch_stmt == 2)	/* jump vector label */
		{
		return TRUE ;
		}
	else if (switch_stmt > 2)
		{
		if (*label != '\0')	/* beyond jump vector */
			{
			switch_stmt = 0 ;
			}
		else		/* jump vector itself */
			{
			if ((ptr = index(dorig1, ',')) != NULL)
				{
				Remember_Ref(ptr + 1) ;
				}
			else
				{
				Remember_Ref(dorig1) ;
				}
			return TRUE ;
			}
		}

	/* Now begin actual label processing */
	/* --------------------------------- */

	if (*label=='\0')
		{
		/* No label - do nothing */
		return FALSE ;
		}

	flg_sts = 0 ;
	flg_reg = NULL ;
	cld_set = FALSE ;

	/* If this statement has a label, see if that label is	*/
	/* to be deleted (previous forward references deleted).	*/
	/* ---------------------------------------------------- */
	lbl[strlen(strcpy(lbl, label)) - 1] = '\0' ; /* (Strip ':') */
	if ((entry = FIND_LABEL(lbl)) != NULL)	/* (fwd REFERENCE label?) */
		{
		if ((entry->status & (DELETED|REFERENCE|DEFINED)) ==
			DELETED)
			{
			/* Delete the entry */
			Remove_Entry() ;

			/* Delete the label */
			stmt1->mnemonic = stmt1->label ;
			stmt1->label = deadcode ;
			return TRUE ;
			}
		}

	if ((opc1==PUSH && dreg1==BP) || opc1==ENTER)
	    	{
		/* Entrance to new function */
		while (Remove_Entry()) ;
		Free_Registers(_EMPTY, _EMPTY) ;
		Clean_Function(lbl) ;
		if (option.call_help.enabled && opc1==PUSH)
			{
			Emit(stmt2, null, call, "_HELP_", null) ;
			stats.cseg_ins++ ;
			}
		return (dead = FALSE) ;
		}

	Remember_Def(label, last_opc) ;
	dead = FALSE ;
	return !opc1 ;	/* Finished w/this statement if no opcode */
	}


BOOLEAN Dead_Code()
	{
	/* Kill unreachable code following RET */
	/* ----------------------------------- */
	if (!dead)
		{
		if (opc1==RET)
			{
			if (Clean_Ret()) return TRUE ;
			dead = TRUE ;
			}

		/* Kill unreachable code after (non-switch vectoring) JMP */
		/* ------------------------------------------------------ */
		else if (opc1==JMP)
			{
			dead = !Same_Label(Get_Next(stmt1)->label, dorig1) ;
			}
		}

	/* Remove any dead code */
	/* -------------------- */
	else if (opc1!=NULL)
		{
		/* If removed code is a jump, mark label as dead */
		/* --------------------------------------------- */
		if (opc1==JMP || (opc1->attb & CONDJMP))
			{
			Remember_Del(dorig1) ;
			}
		stmt1->label = deadcode ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	return FALSE ;
	}


PRIVATE VOID Release_Regs()
	{
	AH->attb |= _AVAIL ;	AL->attb |= _AVAIL ;	AX->attb |= _AVAIL ;
	BH->attb |= _AVAIL ;	BL->attb |= _AVAIL ;	BX->attb |= _AVAIL ;
	CH->attb |= _AVAIL ;	CL->attb |= _AVAIL ;	CX->attb |= _AVAIL ;
	DH->attb |= _AVAIL ;	DL->attb |= _AVAIL ;	DX->attb |= _AVAIL ;
	SI->attb |= _AVAIL ;	DI->attb |= _AVAIL ;	ES->attb |= _AVAIL ;
	}


BOOLEAN Remove_Entry()
	{
	PRESERVE *entry ;
	REG *r ;
	REG_STATE **sp ;

	/* Assumes that this function was called */
	/* immediately after FIND_LABEL(), which */
	/* moved what it found to the front.	 */

	if ((entry = states) != NULL)
		{
		states = entry->next ;
		if ((entry->status & REFERENCE) != 0)
			{
			if (*(sp = entry->regs) != NULL)
				{
				for (r = AH; r <= ES; r++)
					{
					My_Free((CHAR *) *sp++) ;
					}
				}
			}
		My_Free((CHAR *) entry) ;
		}

	return (states != NULL) ;
	}


PRIVATE PRESERVE *Enter_Label(label)
CHAR *label ;
	{
	PRESERVE *entry ;
	REG_STATE **sp ;
	REG *r ;
	unsigned len ;

	entry = (PRESERVE *) Allocate(sizeof(PRESERVE) + (len=strlen(label)));
	entry->status = 0 ;
	_move(len + 1, label, entry->label) ;
	entry->next = states ;
	states = entry ;

	sp = entry->regs ;
	for (r = AH; r <= ES; r++)
		{
		*sp++ = NULL ;	/* Mark as empty! */
		}

	return entry ;
	}

PRIVATE BOOLEAN Same_Label(label, dst)
CHAR *label ;
CHAR *dst ;
	{
	while (*label == *dst)
		{
		label++ ;
		dst++ ;
		}
	return (*label==':' && *dst=='\0') ;
	}

			 /*  End of file LABELS.C  */


