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
		       /*  Start of file 8088-PT3.C  */

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

PRIVATE REG *wset[] = {BX, DI, CX, DX, AX, SI} ;

/* Functions PRIVATE to 8088-PT3.C...					*/
/* -------------------------------------------------------------------- */

#ifdef	_lint

REG		*Choose(FLAGS, FLAGS, CHAR *, REG *) ;
FLAGS		Eliminate(REG *, FLAGS) ;
VOID		Replace(CHAR *, CHAR *, CHAR *, CHAR *) ;
FLAGS		Restrict(FLAGS, FLAGS) ;

#else

REG		*Choose() ;
FLAGS		Eliminate() ;
VOID		Replace() ;
FLAGS		Restrict() ;

#endif


BOOLEAN Preserve()
	{
	/* Strategy: Try to preserve availability of known data in
	   registers, but without making register substitutions that
	   contradict what the compiler is likely to do later.
	*/
	REG *new ;
	CHAR bfr[80] ;
	PARTS *stmt, *parts ;
	CHAR *src, *dst, *load, *newsrc ;
	BOOLEAN replaced ;
	OPCODE *opc ;


	if (dreg1==NULL || dreg1==BP || dreg1==SP ||
	    (dreg1->attb & _SEGREG)!=0 ||
	    BYTE(dattb1) || (stmt1->attb & NEWREG)!=0)
		{
		/* ------------------------------- */
		/* Return if register is BP or SP. */
		/* ------------------------------- */
		return FALSE ;
		}

	/*
	-------------------------------------------------------
	"load" is the data about to be loaded into the register
	-------------------------------------------------------
	*/
	if (opc1 == XOR)
		{
		if (sreg1 == dreg1) load = zero ;
		else return FALSE ; /* Doesn't load dreg1 */
		}
	else if (opc1 == POP) load = NULL ;
	else if (sreg1!=NULL) load = sreg1->content ;
	else load = sorig1 ;

	new = NULL ;

	if ((opc1==MOV || opc1==LEA) && stmt2!=NULL && sreg1==NULL &&
	    stmt2->dst.reg!=NULL && !VARDST(sattb1) && INDEX(dreg1))
		{
		if ((stmt2->opcode==MOV || stmt2->opcode==LEA) &&
	            (stmt2->dst.reg==dreg1) &&
		    (stmt2->src.attb & DEPEND_FLAG(dreg1))!=0)
		    	{
			/*
			==================================
			Load  r1,src1	=>  Load  r2,src1
			Load  r1,f(r1)  =>  Load  r1,f(r2)
			==================================
			*/
			new = Choose(0, stmt2->src.attb, load, NULL) ;
			}

		else if (stmt3!=NULL && stmt2->opcode==ADD &&
			 (stmt3->opcode==MOV || stmt3->opcode==LEA) &&
			 (stmt2->dst.reg==dreg1) && (stmt3->dst.reg==dreg1) &&
			 stmt2->src.reg==NULL && CONSTANT(stmt2->src.attb) &&
			 (stmt3->src.attb & DEPEND_FLAG(dreg1))!=0)
			{
			/*
			==================================
			Load  r1,src1	=>  Load  r2,src1
			Add   r1,imm        Add   r2,imm
			Load  r1,f(r1)  =>  Load  r1,f(r2)
			==================================
			*/
			new = Choose(REG_FLAG(dreg1), stmt3->src.attb,
				load, NULL) ;
			}

		else if (stmt3!=NULL &&
			 (stmt2->opcode==MOV || stmt2->opcode==LEA) &&
	        	 (stmt3->opcode==MOV || stmt3->opcode==LEA) && 
			 stmt2->dst.reg!=dreg1 && stmt3->dst.reg==dreg1 &&
			 (stmt2->src.attb & DEPEND_FLAG(dreg1))==0 &&
			 (stmt3->src.attb & DEPEND_FLAG(dreg1))!=0)
			{
			/*
			==================================
			Load  r1,src1   =>  Load  r3,src1
			Load  r2,src2   =>  Load  r2,src2
			Load  r1,f(r1)  =>  Load  r1,f(r3)
			==================================
			*/
			new = Choose(REG_FLAG(stmt2->dst.reg),
				stmt3->src.attb, load, NULL) ;
			}
		}

	if (new==NULL && VALID(dattb1))
		{
		/*
		===========================
		LOAD R1,src  ->  LOAD r,src
		===========================
		*/
		new = Find_Free(load, NULL) ;
		}

	if (new==NULL && sreg1!=NULL && opc1!=XOR &&
	    (sreg1->attb & _SEGREG)==0)
		{
		/*
		===========================
		LOAD R1,R2  ->  LOAD R2,R2
		===========================
		*/
		new = Find_Free(load, sreg1) ;
		}

	if (new==NULL) return FALSE ;	/* No registers available */

	/* Now make the replacement ... */
	for (parts=stmt1; (stmt=Reg_Needed(dreg1, parts))!=NULL; parts=stmt)
		{
		opc = stmt->opcode ;
		if (opc==RET || (opc->attb & CONDJMP)!=0) break ;

		dst = stmt->dst.orig ;
		src = stmt->src.orig ;
		replaced = FALSE ;
		if (stmt->src.reg!=NULL)
			{
			if (stmt->src.reg == dreg1)
				{
				src = new->name ;
				replaced = TRUE ;
				}
			else if (Contains(dreg1, stmt->src.reg))
				{
				bfr[0] = new->name[0] ;
				bfr[1] = stmt->src.orig[1] ;
				bfr[2] = '\0' ;
				src = bfr ;
				replaced = TRUE ;
				}
			}
		else if ((stmt->src.attb & DEPEND_FLAG(dreg1)) != 0)
			{
			Replace(stmt->src.orig, src = bfr,
				dreg1->name, new->name) ;
			replaced = TRUE ;
			}

		if (stmt->dst.reg != NULL)
			{
			if ((opc->attb & LOADS) == 0)
				{
				if (stmt->dst.reg == dreg1)
					{
					dst = new->name ;
					replaced = TRUE ;
					}
				else if (Contains(dreg1, stmt->dst.reg))
					{
					bfr[0] = new->name[0] ;
					bfr[1] = stmt->dst.orig[1] ;
					bfr[2] = '\0' ;
					dst = bfr ;
					replaced = TRUE ;
					}
				}
			}
		else if ((stmt->dst.attb & DEPEND_FLAG(dreg1)) != 0)
			{
			Replace(stmt->dst.orig, dst = bfr,
				dreg1->name, new->name) ;
			replaced = TRUE ;
			}

		if (replaced)
			{
			Emit(stmt, null, stmt->mnemonic, dst, src) ;
			stmt->label = substitute ;
			}

		if (opc==JMP || opc==CALL) break ;

		if (stmt->dst.reg!=NULL && (opc->attb & LOADS)!=0 &&
		    Contains(dreg1, stmt->dst.reg))
			{
			break ;
			}
		}

	newsrc = (opc1==XOR) ? new->name : sorig1 ;
	Emit(stmt1, null, stmt1->mnemonic, new->name, newsrc)->attb = NEWREG ;
	stmt1->label = substitute ;
	return TRUE ;
	}


REG *Find_Free(load, try)
CHAR *load ;	/* new data about to be loaded into the old register */
REG *try ;	/* try this register if non-zero */
	{
	REG *result ;
	CHAR label[100] ;
	PARTS *parts ;
	FLAGS mdfd, ndex, dpnd, not_avail ;
	OPCODE *opc ;
	PRESERVE *entry ;
	CHAR *keep ;
	BOOLEAN needed ;
	PARTS *stmt ;
	REG *r ;
	REG *dreg, *sreg ;
	int i, defines, available ;


	dpnd = DEPEND_FLAG(dreg1) ;	/* Avoid recomputing in loop	*/
	keep = Skip_Prefix(dreg1->content) ; /* Data to be preserved	*/
	needed = (try != NULL) ;	/* 1=> "keep" is needed below	*/
	mdfd = 0 ;			/* Don't use these registers!	*/
	ndex = 0 ;			/* dreg1 (&others) used as ndex	*/
	defines = 0 ;			/* # times dreg1 is redefined	*/

	result = NULL ;
	for (parts=stmt1; (stmt=Find_Next(parts))!=NULL; parts=stmt)
		{
		opc = stmt->opcode ;
		dreg = stmt->dst.reg ;
		sreg = stmt->src.reg ;

		if (dreg==NULL)
			{
			if (defines==0 && (stmt->dst.attb & dpnd)!=0)
				{
				ndex |= stmt->dst.attb ;
				}
			if (!needed &&
			    Equal(Skip_Prefix(stmt->dst.orig), keep))
				{
				needed = TRUE ;
				}
			}
		else if (defines==0)
			{
			if (BYTE(dreg->attb) && Contains(dreg1, dreg))
				{
				mdfd |= (_SI | _DI) ;
				}
			}

		if (sreg==NULL)
			{
			if (defines==0 && (stmt->src.attb & dpnd)!=0)
				{
				ndex |= stmt->src.attb ;
				}
			if (!needed &&
			    Equal(Skip_Prefix(stmt->src.orig), keep))
				{
				needed = TRUE ;
				}
			}
		else if (defines==0)
			{
			if (BYTE(sreg->attb) && Contains(dreg1, sreg))
				{
				mdfd |= (_SI | _DI) ;
				}
			if ((opc->attb & MDFYSRC) != 0)
				{
				mdfd |= REG_FLAG(sreg) ;
				}
			}

		if (defines==0 && (opc->attb & MDFYDST)!=0)
			{
			if (dreg!=NULL)
				{
				mdfd |= REG_FLAG(dreg) ;
				if (INDEX(dreg))
					{
					mdfd = Eliminate(dreg, mdfd) ;
					}
				}
			else if (VARDST(stmt->dst.attb)) goto out ;
			else
				{
				for (i = 0; i < 6; i++)
					{
					r = wset[i] ;
					if (VALID(r->attb) &&
					    Equal(
					      Skip_Prefix(stmt->dst.orig),
					      Skip_Prefix(r->content)))
					    	{
						mdfd = Eliminate(r, mdfd) ;
						}
					}
				}
			}

		if (opc==JMP || (opc->attb & CONDJMP)!=0 || opc==CALL)
			{
			/*
			--------------------------------------------
			Must worry about future references to dreg1!
			--------------------------------------------
			*/
			if ((opc != CALL) &&
			    ((entry = FIND_LABEL(stmt->dst.orig))) != NULL)
				{
				if ((entry->status & DEFINED) != 0)
					{
					/*
					-----------------------------
					Backward Reference:  Possible
					downstream use of old content
					if jump is conditional.
					-----------------------------
					*/
					break ;
					}
				}
			/*
			---------------------------------------------
			Forward Reference: Possible future reference
			to dreg1 if jump to compiler-generated label,
			else may need old value below.
			---------------------------------------------
			*/
			if (defines==0 && stmt->dst.orig[0]=='_') goto out ;
			else if (opc==CALL || opc==JMP) break ;
			}

		else if (opc == RET)
			{
			if (defines==0)
				{
				/* Might need old reg! */
				if (ACCUM(dreg1) ||
				    (option.big_model.enabled &&
				     (dreg1==SI || dreg1==ES))) goto out ;
				mdfd |= _AX|_DX ;
				if (option.big_model.enabled) mdfd |= _SI|_ES;
				}
			break ;
			}

		else if (opc == REP)
			{
			if (dreg1==DI || dreg1==CX) goto out ;
			if (EqualN(stmt->dst.orig, "MOVS", 4))
				{
				if (dreg1==SI) goto out ;
				mdfd |= _SI ;
				}
			else if (EqualN(stmt->dst.orig, "STOS", 4))
				{
				if (dreg1==AX || dreg1==AL) goto out ;
				}
			mdfd |= _DI | _CX ;
			}

		else if (opc==MOVSB || opc==MOVSW)
			{
			if (dreg1==DI || dreg1==SI) goto out ;
			mdfd |= (_SI | _DI) ;
			}

		else if (opc==STOSB || opc==STOSW)
			{
			if (dreg1==DI || dreg1==AX || dreg1==AL) goto out ;
			mdfd |= _DI ;
			}

		if (defines == 0)
			{
			if (opc->fnc == Shift)
				{
				if (Contains(dreg1, sreg)) goto out ;
				else if (sreg != NULL) mdfd |= _CX ;
				}

			else if ((opc->attb & MDFYACC) != 0)
				{
				if (ACCUM(dreg1)) goto out ;
				mdfd |= (_AX|_DX) ;	/* Can't use ACC! */
				}

			else if (opc==OUT || opc==SAHF)
				{
				if (dreg1==AX || dreg==dreg1) goto out ;
				}

			else if (opc==IN)
				{
				if (sreg==dreg1) goto out ;
				}
			}			

		if (REDEFINED(dreg1, stmt)) defines++ ;
		else if (dreg1->name[1]=='X')
			{
			if (REDEFINED(RL(dreg1), stmt) ||
			    REDEFINED(RH(dreg1), stmt))
			    	{
				defines++ ;
				}
			}

		if (defines == 1)
			{
			not_avail = Restrict(ndex, mdfd) | REG_FLAG(dreg1) ;
			available = 0 ;
			for (i = 0; i < 6; i++)
				{
				if ((not_avail & REG_FLAG(wset[i])) == 0)
					{
					available++ ;
					}
				}
			if (available == 0) goto out ;
			}

		if (defines!=0 && (defines >= available-1)) break ;
		}

	/*
	-------------------------------------------------------------
	Can't replace if terminated on compiler-generated (fwd) label
	-------------------------------------------------------------
	*/
	if ((parts = Get_Next(parts)) != NULL)
		{
		if (*parts->label == '_')
			{
			/* Compiler-generated label: Fwd Ref ? */
			/* Remove the trailing ':' */
			label[strlen(strcpy(label, parts->label)) - 1] = '\0';

			/* Fwd reference - can't replace! */
			if (FIND_LABEL(label) != NULL) goto out ;

			/* Useful below (Rev Ref) label? */
			else needed = TRUE ;
			}

		/* Useful below (user) label? */
		else if (*parts->label != '\0') needed = TRUE ;
		}

	/*
	-----------------------------------------------------------
	No need to substitute if preserved value not re-referenced!
	-----------------------------------------------------------
	*/
	if (needed) result = Choose(mdfd, ndex, load, try) ;
out:
	return result ;
	}


PRIVATE FLAGS Restrict(ndex, mdfd)
FLAGS ndex ;
FLAGS mdfd ;
	{
	/*
	---------------------------------------------------
	If dreg1 is USED as an index register, then we must
	rule out replacement using non-index registers!
	---------------------------------------------------
	*/
	if ((ndex & DEPEND_FLAG(dreg1)) != 0) mdfd |= _AX|_CX|_DX ;

	/*
	-------------------------------------------
	Rule out registers double indexed with 'r'!
	-------------------------------------------
	*/
	if (dreg1==BX)
		{
		if ((ndex & (_DEPSI|_DEPDI)) != 0) mdfd |= _SI|_DI ;
		}
	else if (INDEX(dreg1) && (ndex & (_DEPBX|_DEPBP))!=0) mdfd |= _BX ;

	return mdfd ;
	}


PRIVATE FLAGS Eliminate(r, m)
REG *r ;
FLAGS m ;
	{
	REG *t ;
	unsigned int i ;
	FLAGS d, f ;

	d = DEPEND_FLAG(r) ;
	m |= REG_FLAG(r) ;
	for (i = 0; i < 6; i++)
		{
		f = REG_FLAG(t = wset[i]) ;
		if ((m & f) != 0) continue ;
		if ((t->attb & d) != 0)
			{
			m |= f ;
			if (INDEX(t)) m = Eliminate(t, m) ;
			}
		}
	return m ;
	}


PRIVATE REG *Choose(mdfd, ndex, load, try)
FLAGS mdfd ;
FLAGS ndex ;
CHAR *load ;
REG *try ;
	{
	REG **set ;
	REG *r ;
	REG *new ;
	int i, n ;
	int level ;
	BOOLEAN dreg1_mdfd ;


	dreg1_mdfd = (mdfd & REG_FLAG(dreg1)) != 0 ;	/* For convenience */
	if (dreg1_mdfd && try!=NULL && (try==SP || try==BP || SEGREG(try)))
		{
		return NULL ;
		}

	mdfd = Restrict(ndex, mdfd) | REG_FLAG(dreg1) ;

	/*
	-------------------------------------------------------------
	Data on register use has been collected; Choose new register:
	-------------------------------------------------------------
	*/

	new = NULL ;
	level = 0 ;
	if (try != NULL)
		{
		set = &try ;
		n = 1 ;
		}
	else if (WORD(dattb1))
		{
		set = wset ;
		n = 6 ;
		}

	for (i = 0; i < n; i++)
		{
		r = set[i] ;

		/* Ignore any that are already ruled out */
		if ((REG_FLAG(r) & mdfd) != 0) continue ;

		/* 1st priority: Register containing same data. */
		if ((level < 4) && VALID(r->attb) &&
		    (!dreg1_mdfd || AVAIL(r)) &&
		    load!=NULL && Equal(r->content, load))
			{
			new = r ;
			level = 4 ;
			}

		/* 2nd priority: Empty register. */
		else if ((level < 3) && EMPTY(r))
			{
			new = r ;
			level = 3 ;
			}

		else if ((level < 2) && AVAIL(r) && UNKNOWN(r))
			{
			new = r ;
			level = 2 ;
			}

		else if ((level < 1) && AVAIL(r))
			{
			new = r ;
			level = 1 ;
			}

		/* All the rest require an unneeded register... */
		if (Reg_Needed(r, stmt1) != NULL) continue ;

		/* 1st priority: Register containing same data. */
		if ((level < 4) && VALID(r->attb) && load!=NULL &&
		    Equal(r->content, load))
			{
			new = r ;
			level = 4 ;
			}

		/* 3rd priority: unneeded register w/unknown content */
		else if ((level < 2) && UNKNOWN(r))
			{
			new = r ;
			level = 2 ;
			}

		/* last resort: unneeded register w/known content */
		else if (level < 1)
			{
			new = r ;
			level = 1 ;
			}
		}

	return new ;
	}


PRIVATE VOID Replace(old_bfr, new_bfr, old_reg, new_reg)
CHAR *old_bfr ;
CHAR *new_bfr ;
CHAR *old_reg ;
CHAR *new_reg ;
	{
	CHAR *ptr ;
	int *pair ;

	for (ptr = index(strcpy(new_bfr, old_bfr),'['); *ptr!='\0'; ptr++)
		{
		pair = (int *) ptr ;
		if (*pair == *((int *) old_reg)) *pair = *((int *) new_reg) ;
		}
	}


			/*  End of file 8088-PT3.C  */
