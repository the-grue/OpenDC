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
		       /*  Start of file 8088-PT2.C  */

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


/* Functions PRIVATE to 8088-PT2.C...					*/
/* -------------------------------------------------------------------- */

typedef struct RECODING
	{
	CHAR		table[16] ;
	unsigned	length ;
	BOOLEAN		negate ;
	unsigned	clocks ;
	unsigned	bytes ;
	CHAR		dummy[8] ;	/* For fast subscripting */
	} RECODING ;

#ifdef	_lint

BOOLEAN		CL_Is_Usable(unsigned, PARTS *) ;
BOOLEAN		Quotient(PARTS *, unsigned, unsigned, CHAR *) ;
BOOLEAN		Remainder(PARTS *, CHAR *) ;
BOOLEAN		Multiply(PARTS *, REG *, unsigned) ;
unsigned	Recode(unsigned, CHAR []) ;
BOOLEAN		Expand(RECODING *, PARTS *) ;
RECODING	*Useful(unsigned, PARTS *) ;
unsigned	Log_Base2(unsigned) ;
BOOLEAN		Need_Remainder(PARTS *) ;
BOOLEAN		Pairs(VOID) ;
BOOLEAN		Quads(VOID) ;
BOOLEAN		Shift8(PARTS *, REG *, unsigned) ;
VOID		Rotate8(PARTS *, REG *) ;
VOID		Sar8(PARTS *) ;
VOID		Shl8(PARTS *, REG *) ;
VOID		Shr8(PARTS *, REG *) ;
BOOLEAN		Triples(VOID) ;
BOOLEAN		Usable(REG *, CHAR *) ;
CHAR		*Copy_Prefix(CHAR *, CHAR *) ;

#else

BOOLEAN		CL_Is_Usable() ;
BOOLEAN		Quotient() ;
BOOLEAN		Remainder() ;
BOOLEAN		Multiply() ;
unsigned	Recode() ;
BOOLEAN		Expand() ;
RECODING	*Useful() ;
unsigned	Log_Base2() ;
BOOLEAN		Need_Remainder() ;
BOOLEAN		Pairs() ;
BOOLEAN		Quads() ;
BOOLEAN		Shift8() ;
VOID		Sar8() ;
VOID		Shl8() ;
VOID		Shr8() ;
BOOLEAN		Triples() ;
BOOLEAN		Usable() ;
CHAR		*Copy_Prefix() ;

#endif

PARTS *Reg_Needed(r, parts)
REG *r ;
PARTS *parts ;
	{
	/* NOTE: This routine is called within a recursive loop!
		 To prevent the stack from growing too large, I
		 made the following temporary variables static.
		 None of them need to be preserved during the
		 recursion.
	*/
	static CHAR label[100] ;
	static PARTS *stmt, *result = NULL ;
	static PRESERVE *entry ;
	static REG_STATE *s ;
	static OPCODE *opc, *last_opc ;
	static REG *last_r = NULL ;
	static int last_id = -1 ;

	if (r==last_r && parts->id==last_id) return result ;
	last_r = r ;
	last_id = parts->id ;

	result = NULL ;
	opc = parts->opcode ;
	for (stmt=parts; (stmt=Get_Next(parts)) != NULL; parts=stmt)
		{
		last_opc = opc ;
		opc = stmt->opcode ;
		if (islower(stmt->mnemonic[0])) goto out ;
		if (opc==LINE) goto out ;
		if (*stmt->label == ';') continue ;
		if (*stmt->label!='\0') break ;
		if ((opc->attb & CONDJMP)!=0 || opc==JMP)
			{
			if ((entry = FIND_LABEL(stmt->dst.orig)) != NULL)
				{
				if ((entry->status & DEFINED) != 0)
					{
					/*
					BACKWARD REFERENCE:  Code below this
					jump starts with all registers EMPTY!
					Thus this register is not needed.
					*/
					goto out ;
					}
				if ((entry->status & REFERENCE) != 0)
					{
					/*
					SUBSEQUENT FORWARD REFERENCE: If
					previous forward reference left
					register EMPTY or AVAILable (at
					destination), then this register
					is not needed.
					*/
					s = entry->regs[r - reg] ;
					if ((s->attb & (_EMPTY|_AVAIL)) == 0)
						{
						result = stmt ;
						}
					goto out ;
					}
				}
			/*
			FIRST FORWARD REFERENCE:  Assume that code following 
			this jump may reference the register.
			*/
			result = stmt ;	/* (Be conservative) */
			goto out ;
			}

		if (REFERENCED(r, stmt))
			{
			result = stmt ;
			goto out ;
			}

		if (REDEFINED(r, stmt)) goto out ;
		}
	
	if (stmt == NULL)
		{
		/* ---------------------------------------- */
		/* No more code - register can't be needed! */
		/* ---------------------------------------- */
		goto out ;
		}

	/* ------------------------------------ */
	/* Here only if a label was encountered */
	/* ------------------------------------ */
	if (*stmt->label != '_')
		{
		/* ----------------------------------- */
		/* User-defined label: all regs empty! */
		/* ----------------------------------- */
		goto out ;
		}

	/* ------------------------------------------ */
	/* Compiler-generated label: Fwd or Rev Ref ? */
	/* ------------------------------------------ */

	/* First: Remove the trailing ':' */
	label[strlen(strcpy(label, stmt->label)) - 1] = '\0' ;

	if ((entry = FIND_LABEL(label)) == NULL)
		{
		/* --------------------------------------- */
		/* Backward referenced label: Reg is EMPTY */
		/* unless preceded by JMP: "if (1) ... "   */
		/* --------------------------------------- */
		if (last_opc == JMP) result = stmt ;
		goto out ;
		}

	/* ------------------------ */
	/* Forward referenced label */
	/* ------------------------ */

	if ((entry->status & DELETED) != 0)
		{
		/* -------------------------------- */
		/* DELETED label - Must start over! */
		/* -------------------------------- */
		result = Reg_Needed(r, stmt) ;
		goto out ;
		}

	/* ------------------------- */
	/* Register not needed if it */
	/* was EMPTY or AVAILable.   */
	/* ------------------------- */
	s = entry->regs[r - reg] ;
	if ((s->attb & (_EMPTY|_AVAIL)) == 0) result = stmt ;

out:
	return result ;
	}


BOOLEAN Contains(r1, r2)
REG *r1, *r2 ;
	{
	if (r1==NULL || r2==NULL) return FALSE ;
	if (r2 == r1) return TRUE ;
	if (r1->name[1] == 'X')
		{
		if (r2 == RH(r1) || r2 == RL(r1)) return TRUE ;
		}

	return FALSE ;
	}


BOOLEAN Mov_Pt2()
	{
	if (dreg1!=NULL && sreg1==NULL && OFFSET(sattb1))
		{
		if (Lea_Pt2()) return TRUE ;
		}
	return Quads() || Triples() || Pairs() ;
	}


PRIVATE BOOLEAN Pairs()
	{
	BOOLEAN ok ;
	PARTS *stmt, *at, *stmt6 ;
	REG *sreg2, *dreg2, *dreg, *r ;
	unsigned i ;
	CHAR *dst2, *ptr, *src2 ;
	CHAR new_src[80], new_dst[80] ;
	PACKED n ;
	OPCODE *opc ;
	static CHAR bfr[] = "00000" ;

	/*
		============================================================
		The following sequences consist of two or more instructions.
		============================================================
	*/

	if (stmt2==NULL) return FALSE ;

	src2  = stmt2->src.orig ;
	dst2  = stmt2->dst.orig ;

	sreg2 = stmt2->src.reg ;
	dreg2 = stmt2->dst.reg ;

	if (dreg1==sreg2 && sreg1!=NULL && dreg2!=NULL &&
	    stmt2->opcode==MOV && Reg_Needed(dreg1, stmt2)==NULL)
		{
		if (VALID(sattb1) && VALID(dreg2->attb) &&
		    Equal(sreg1->content, dreg2->content))
		    	{
			/*
			------------------
			MOV r,r1  =>   ###
			MOV r2,r       ###
			------------------
			*/
			stmt1->label = redundant ;
			stmt2->label = redundant ;
			stats.cseg_del += 2 ;
			return TRUE ;
			}

		if ((sattb1 & dreg2->attb & _SEGREG)==0)
		    	{
			/*
			-----------------------
			MOV r,r1      ###
			MOV r2,r  =>  MOV r2,r1
			-----------------------
			*/
			Emit(stmt2, stmt1->label, mov, dst2, sorig1) ;
			stmt2->label = combined ;
			stmt1->label = combined ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}

	if (sreg1==NULL && CONSTANT(sattb1) &&
	    ((stmt = Reg_Needed(dreg1, stmt1)) != NULL))
	    	{
		if (dreg1==CX && stmt->opcode==REP)
			{
			n.word = (unsigned) Value(sorig1, slen1) ;
			if (stmt->dst.orig[4]=='B')
				{
				stmt6 = Find_Next(stmt5) ;
				if (n.word==2 &&
				    stmt6->opcode==CLD &&
				    stmt4->opcode==MOV &&
				    stmt5->opcode==MOV &&
				    Equal(stmt->dst.orig, "MOVSB") &&
				    stmt4->dst.reg==AX &&
				    stmt4->src.reg==DS &&
				    stmt5->dst.reg==ES &&
				    stmt5->src.reg==AX)
					{
					/*
					-------------------------------
					MOV CX,2   =>  MOV AX,WORD [SI]
					...	       MOV WORD [DI],AX
					...
					MOV AX,DS
					MOV ES,AX
					CLD
					REP MOVSB
					-------------------------------
					*/
					Emit(stmt, null, mov,
						"WORD [DI]", ax) ;
					Emit(stmt, null, mov,
						ax, "WORD [SI]") ;
					stmt1->label = combined ;
					stmt4->label = combined ;
					stmt5->label = combined ;
					stmt6->label = combined ;
					stmt->label = combined ;
					stats.cseg_del += 3 ;
					return TRUE ;
					}
				/*
				---------------------------------------
				MOV CX,imm	=>	MOV CX,imm/2
				...			...
				REP STOSB/MOVSB		[MOV AH,AL]
							REP STOSW/MOVSW
							[STOSB/MOVSB]
				---------------------------------------
				*/
				if ((n.byte.lsb & 0x01) != 0)
					{
					Emit(stmt, null, stmt->dst.orig, 
						null, null) ;
					stats.cseg_ins++ ;
					}
				if ((n.word >>= 1) != 0)
					{
					Emit(stmt1, null, mov, "CX",
						Convert(n.word,	&bfr[4])) ;
					_move(stmt->dst.len + 1,
						stmt->dst.orig,	new_dst) ;
					new_dst[4] = 'W' ;
					Emit(stmt, null, "REP", new_dst, 
						null) ;
					if (Equal(new_dst, "STOSW"))
						{
						Emit(stmt, null, mov,
							"AH", "AL") ;
						stats.cseg_ins++ ;
						}
					stats.cseg_ins += 2 ;
					}
				stmt1->label = replaced ;
				stmt->label = replaced ;
				stats.cseg_del += 2 ;
				return TRUE ;
				}
			if (n.word <= 4)
				{
				stats.cseg_ins += n.word ;
				while (n.word-- > 0)
					{
					Emit(stmt, null, stmt->dst.orig,
						null, null) ;
					}
				stmt1->label = replaced ;
				stmt->label = replaced ;
				stats.cseg_del += 2 ;
				return TRUE ;
				}
			}

		if (dreg1==dreg2 && (stmt->opcode->attb & COMMUTE)!=0)
			{
			opc = stmt->opcode ;
			if (stmt->src.reg!=NULL &&
			    Reg_Needed(stmt->src.reg, stmt)==NULL)
			    	{
				/*
				-----------------------------
				MOV r1,imm  =>  opc r2,imm
				opc r1,r2       MOV r1,r2

				opc={ADD,ADC,OR,AND,XOR,TEST}
				-----------------------------
				*/
				Emit(stmt, null, mov, dorig1, stmt->src.orig);
				Emit(stmt, null, stmt->mnemonic,
					stmt->src.orig, sorig1);
				stmt->label = replaced ;
				stmt1->label = replaced ;
				return TRUE ;
				}

			n.word = (unsigned) Value(sorig1, slen1) ;
			if (stmt->src.reg==NULL && MEMREF(stmt->src.attb) &&
			    opc!=TEST &&
			    (dorig1[1]!='X' ||
			    (!(opc==AND &&
			       (n.byte.msb==0x00 || n.byte.lsb==0x00)) &&
			     !(opc==OR &&
			       (n.byte.msb==0xFF || n.byte.lsb==0xFF)))))
			       	{
				/*
				--------------------------
				MOV r1,imm  =>  MOV r1,mem
				opc r1,mem      opc r1,imm

				opc={ADD,ADC,OR,AND,XOR}
				--------------------------
				*/
				Emit(stmt, null, stmt->mnemonic, dorig1,
					sorig1) ;
				Emit(stmt, null, mov, dorig1, stmt->src.orig);
				stmt->label = replaced ;
				stmt1->label = replaced ;
				return TRUE ;
				}
			}
		}

	if (stmt2->opcode==SHL && dreg1==dreg2 &&
	    src2[0]=='1' && src2[1]=='\0')
		{
		_move(slen1, sorig1, new_src) ;
		_move((unsigned) 2, "<", &new_src[slen1]) ;
		if ((r = Find_Val(new_src, sattb1)) != NULL)
			{
			if (r != ES)
				{
				/*
					------------------------
					MOV r1,src
					SHL r1,1    =>  MOV r1,r
					------------------------
				*/
				Emit(stmt2, null, mov, dorig1, r->name) ;
				stmt2->label = replaced ;
				stmt1->label = replaced ;
				stats.cseg_del++ ;
				return TRUE ;
				}
			}
		}

	if (stmt2->opcode==MOV && sreg1==NULL && MEMREF(sattb1) &&
	    dreg2!=NULL && sreg2!=NULL && dorig1[1]=='X')
	    	{
		if (dreg2==RL(dreg1) && sreg2==RH(dreg1))
			{
			/*
			--------------------------------
			MOV rX,mem16  =>  MOV rL,mem8[1]
			MOV rL,rH
			--------------------------------
			*/
			Emit(stmt2, null, mov, dst2,
				Insert_String(Word_To_Byte(sorig1),
					slen1, 1, NULL)) ;
			stmt2->label = combined ;
			stmt1->label = combined ;
			stats.cseg_del++ ;
			return TRUE ;
			}

		if (dreg2==RH(dreg1) && sreg2==RL(dreg1))
			{
			/*
			--------------------------------------
			MOV rX,WORD mem  =>  MOV rH,BYTE mem
			MOV rH,rL
			--------------------------------------
			*/
			Emit(stmt2, null, mov, dst2, Word_To_Byte(sorig1)) ;
			stmt2->label = combined ;
			stmt1->label = combined ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}

	if ((stmt = Reg_Needed(dreg1, stmt1))!=NULL &&
	    Reg_Needed(dreg1, stmt)==NULL && stmt->opcode==MOV &&
	    stmt->src.reg==dreg1)
	    	{
		ok = TRUE ;
		for (at = stmt2; at != stmt ; at = Find_Next(at))
			{
			if (Invalidates(at, sop1)) break ;
			}
		ok = (at == stmt) ;
		dreg = stmt->dst.reg ;
		if (ok && ((dreg!=NULL && (!SEGREG(dreg) ||
				          (sreg1==NULL && MEMREF(sattb1)) ||
				          (sreg1!=NULL && !SEGREG(sreg1)))) ||
		     (dreg==NULL && MEMREF(stmt->dst.attb) &&
		      (sreg1!=NULL || !MEMREF(sattb1)))))
		       {
			/*
				--------------------------
				MOV r,src      ###
				...	       ...
				MOV dst,r  =>  MOV dst,src
				--------------------------
			*/
			Emit(stmt, null, mov, stmt->dst.orig, sorig1) ;
			stmt->label = combined ;
			stmt1->label = combined ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}

	else if ((stmt2->opcode->attb & RDWR)!=0 && dreg1==dreg2 &&
		 sreg2!=dreg1 && (stmt2->src.attb & DEPEND_FLAG(dreg1))==0 &&
		 (sreg1!=NULL || MEMREF(sattb1)) &&
		 (sreg1!=NULL || sreg2!=NULL ||
		  (stmt2->opcode!=CMP && stmt2->opcode!=TEST)) && 
		 Reg_Needed(dreg1, stmt2)==NULL &&
		 !(stmt2->opcode==PUSH && sreg1==SP))
		{
		/*
			-----------------------------
			MOV r1,r2  =>  ###
			opc r1[,src]   opc r2[,src]

				- or -

			MOV r1,mem  =>  ###
			opc r1[,src]    opc mem[,src]
			-----------------------------
		*/
		Emit(stmt2, null, stmt2->mnemonic, sorig1, src2) ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	if ((sattb1 & dattb1 & _INDEX)!=0 &&
	    ((stmt = Reg_Needed(dreg1, stmt1)) != NULL) &&
	    (((stmt->dst.attb | stmt->src.attb) & _DEPSRC) ==
	      DEPEND_FLAG(dreg1)) &&
	    ((stmt->dst.reg==dreg1 && (stmt->opcode->attb & LOADS)!=0) ||
	     Reg_Needed(dreg1, stmt)==NULL) &&
	    !(stmt2->opcode==PUSH && sreg1==SP))
		{
		for (at = stmt2; at != stmt; at = Find_Next(at))
			{
			if (Invalidates(at, sop1)) break ;
			}
		if (at == stmt)
			{
			/*
			-------------------------------------
			MOV r1,r2		###
			...		=>	...
			opc dst[,src]		opc dst[,src]
			-------------------------------------
			*/
			_move(stmt->src.len + 1, stmt->src.orig, new_src) ;
			_move(stmt->dst.len + 1, stmt->dst.orig, new_dst) ;
			ptr = new_src - 1 ;
			while ((ptr = index(++ptr, dorig1[0])) != NULL)
				{
				if (ptr[1] == dorig1[1])
					{
					ptr[0] = sorig1[0] ;
					ptr[1] = sorig1[1] ;
					break ;
					}
				}
			if (ptr==NULL)
				{
				ptr = new_dst - 1 ;
				while ((ptr=index(++ptr, dorig1[0])) != NULL)
					{
					if (ptr[1] == dorig1[1])
						{
						ptr[0] = sorig1[0] ;
						ptr[1] = sorig1[1] ;
						break ;
						}
					}
				}
			Emit(stmt, null, stmt->mnemonic, new_dst, new_src) ;
			stmt->label = replaced ;
			stmt1->label = deleted ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}

	if (sreg1==NULL && CONSTANT(sattb1))
		{
		n.word = (unsigned) Value(sorig1, slen1) ;
		if (((stmt = Reg_Needed(dreg1, stmt1)) != NULL) &&
			Reg_Needed(dreg1, stmt)==NULL)
			{
			dreg = stmt->dst.reg ;
			if (Multiply(stmt, dreg1, n.word))
				{
				stmt1->label = reduced ;
				stats.cseg_del++ ;
				return TRUE ;
				}
			else if (stmt->opcode==OUT)
				{
				if (dreg1==DX && n.byte.msb==0)
					{
					/*
					-----------------------------
					MOV DX,imm	###
					...         =>  ...
					OUT DX,AL/AX	OUT imm,AL/AX
					-----------------------------
					*/
					Emit(stmt, null, "OUT",
						sorig1, stmt->src.orig) ;
					stmt->label = replaced ;
					stmt1->label = absorbed ;
					stats.cseg_del++ ;
					return TRUE ;
					}
				return FALSE ;
				}
			else if (stmt->opcode==IN)
				{
				if (dreg1==DX && n.byte.msb==0)
					{
					/*
					----------------------------
					MOV DX,imm	###
					...         =>  ...
					IN AL/AX,DX	IN AL/AX,imm
					----------------------------
					*/
					Emit(stmt, null, "IN",
						stmt->dst.orig, sorig1) ;
					stmt->label = replaced ;
					stmt1->label = absorbed ;
					stats.cseg_del++ ;
					return TRUE ;
					}
				return FALSE ;
				}
			else if (stmt->src.reg==CL && dreg!=NULL &&
				 (dreg1==CX || dreg1==CL))
				{
				/*
				-----------------------------
				MOV CL|CX,imm      shift r,1
				...	       =>  ...
				shift r,CL         shift r,1
				-----------------------------
				*/
				if (Shift8(stmt, dreg, n.word)) return TRUE ;
				if ((n.word >= 4) || !option.time.enabled)
				     	{
					return FALSE ;
					}
				for (i = 0; i < n.word ; i++)
					{
					Emit(stmt, null, stmt->mnemonic,
						dreg->name, one) ;
					stats.cseg_ins++ ;
					}
				stmt->label = replaced ;
				stmt1->label = absorbed ;
				stats.cseg_del += 2 ;
				return TRUE ;
				}
			else if (stmt->src.reg==dreg1 &&
				 (stmt->opcode->attb & MDFYSRC)==0 &&
				 (stmt->dst.attb & DEPEND_FLAG(dreg1))==0 &&
				 stmt->opcode->fnc!=Shift &&
				 (dreg==NULL || !SEGREG(dreg)))
				{
				/*
				--------------------------
				MOV r,imm      ###
				...	   =>  ...
				opc dst,r      opc dst,imm
				--------------------------
				*/
				Emit(stmt, null, stmt->mnemonic,
					stmt->dst.orig, sorig1) ;
				stmt->label = replaced ;
				stmt1->label = absorbed ;
				stats.cseg_del++ ;
				return TRUE ;
				}
			}
		}
	return FALSE ;
	}


BOOLEAN Mul_Op()
	{
	unsigned n ;
	REG *r ;

	if (dreg1!=NULL && CONSTANT(dattb1))
		{
		n = (unsigned) Value(dreg1->content, dreg1->len) ;
		return Multiply(stmt1, dreg1, n) ;
		}

	else if (CONSTANT(AX->attb))
		{
		n = (unsigned) Value(AX->content, AX->len) ;
		return Multiply(stmt1, AX, n) ;
		}

	if ((r = Bad_Ref(dop1)) != NULL) Undefined_Reg(r) ;

	Update_Flg_Reg() ;

	MODIFY(AX) ; MODIFY(AH) ; MODIFY(AL) ;
	if (WORD(dattb1))
		{
		/* Most significant half never used */
		LOAD_ATTB(DX, _EMPTY) ;
		LOAD_ATTB(DH, _EMPTY) ;
		LOAD_ATTB(DL, _EMPTY) ;
		}
	else Updates(BYTE(dattb1)) ;
	return TRUE ;
	}

PRIVATE BOOLEAN Multiply(stmt, r, n)
PARTS *stmt ;
REG *r ;
unsigned n ;
	{
	PARTS *at ;
	RECODING *recode ;

	if (WORD(stmt->dst.attb) &&
	    (stmt->opcode==IMUL || stmt->opcode==MUL) &&
	    (stmt->dst.reg==r || (r==AX && WORD(stmt->dst.attb))))
	     	{
		if ((recode = Useful(n, stmt)) == NULL) return FALSE ;
		/*
			-------------------------------------
			MOV AX,imm	   MOV AX,WORD mem
			...
			[I]MUL WORD mem => shifts,adds,&subs

					-or-

			MOV AX,imm	   MOV AX,WORD mem
			...
			[I]MUL r        => shifts,adds,&subs

					-or-

			MOV r,imm
			...	      =>  ...
			[I]MUL r          shifts,adds,&subs
			-------------------------------------
		*/
		if (stmt->dst.reg==CX) CX->attb |= _AVAIL ;
		if (((at = Reg_Needed(DX, stmt)) != NULL) &&
		    (at->opcode==CALL ||
		     (Contains(DX, at->dst.reg) ||
		      Contains(DX, at->src.reg))))
			{
			Emit(stmt, null, cwd, null, null) ;
			stats.cseg_ins++ ;
			}

		if (n == 0)
			{
			Emit(stmt, null, xor, ax, ax) ;
			stats.cseg_ins++ ;
			}
		else
			{
			if (recode->negate)
				{
				Emit(stmt, null, "NEG", ax, null) ;
				stats.cseg_ins++ ;
				}

			if (Expand(recode, stmt))
				{
				/*
				--------------------------------
				DX was used as an operand in an
				ADD or SUB as part of the expan-
				sion, and needs to be loaded.
				--------------------------------
				*/
				Emit(stmt, null, mov, dx, ax) ;
				stats.cseg_ins++ ;
				}

			if (r == AX)
				{
				/*
				--------------------------
				If constant was in AX, we 
				must load AX with operand.
				--------------------------
				*/
				Emit(stmt, null, mov, ax, stmt->dst.orig) ;
				stats.cseg_ins++ ;
				}
			}

		stmt->label = reduced ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	return FALSE ;
	}


PRIVATE unsigned Recode(mpier, table)
unsigned mpier ;
CHAR table[] ;
	{
	unsigned i, cy, pos ;
	static CHAR ncy_tbl[] = {0, 0, 0, 4, 0, 4, 4, 4} ;
	static CHAR add_tbl[] = {0, 1, 0, 0, 1, 0, 0, 0} ;
	static CHAR sub_tbl[] = {0, 0, 0, 1, 0, 0, 1, 0} ;

	cy = 0 ;
	pos = 0 ;
	do
		{
		i = cy | (mpier & 0x03) ;
		if (add_tbl[i] != 0)
			{
			if (mpier >= 2) table[pos++] = 1 ; /* shift & add */
			}
		else if (sub_tbl[i] != 0)
			{
			if (mpier <= 3)
				{
				table[pos++] = 1 ; /* shift & add */
				break ;
				}
			table[pos++] = 2 ; /* shift & sub */
			}
		else table[pos++] = 0 ; /* shift only */
		mpier >>= 1 ;
		cy = ncy_tbl[i] ;
		} while ((mpier | cy) != 0) ;
	return pos ;
	}


/*lint -save -e525 -e725 (disable indentation checking) */
PRIVATE RECODING *Useful(n, stmt)
unsigned n ;
PARTS *stmt ;
	{
	static RECODING recode[2] ;
	unsigned i, clocks, shift_span, bytes ;
	unsigned pass, best ;
	BOOLEAN unused ;

	for (pass = 0; pass < 2; pass++)
		{
		/* Pass 0: no negation */
		/* Pass 1: use negation */

		recode[pass].length = Recode(n, recode[pass].table) ;
		recode[pass].negate = (BOOLEAN) pass ;
		n = (unsigned) -((int) n) ;

		/* Sum the clocks and opcode bytes required */

		unused = TRUE ;
		bytes = 0 ;
		clocks = 0 ;
		shift_span = 0 ;
		for (i = 0; i < recode[pass].length; i++)
			{
			shift_span++ ;
			if (recode[pass].table[i] == 0) continue ;

/* ADD/SUB AX,DX */	bytes += 2 ;
/* 2 bytes, */		clocks += 3 ;
/* 3 clocks */
			if (shift_span >= 8)
				{
/* MOV AH,AL */			bytes += 4 ;
/* XOR AL,AL */			clocks += 5 ;
/* 4 bytes, */			if ((shift_span -= 8) == 0) continue ;
/* 5 clocks */			}

/* 80188 */		if (option.superset.enabled)
/* ----- */			{
				if (shift_span > 1)
					{
/* SHL AX,n */				bytes += 3 ;
/* 3 bytes, */				clocks += 5 + shift_span ;
/* 5+N clks */				}
				else
					{
/* SHL AX,1  */				bytes += 2 * shift_span ;
/* ... ....  */				clocks += 2 * shift_span ;
/* 2N bytes, */				}
/* 2N clocks */			}

/* 8088 */		else if (unused && (shift_span >= 4) &&
/* ---- */			(stmt->dst.reg==CX ||
				 CL_Is_Usable(shift_span, stmt)))
			     	{
/* MOV CL,n   */		unused = FALSE ;
/* SHL AX,CL  */		bytes += 4 ;
/* 4 bytes,   */		clocks += 12 + 4 * shift_span ;
/* 12+4N clks */		}
			else
				{
/* SHL AX,1  */			bytes += 2 * shift_span ;
/* ... ....  */			clocks += 2 * shift_span ;
/* 2N bytes, */			}
/* 2N clocks */		shift_span = 0 ;
			}

		recode[pass].clocks = clocks ;
		recode[pass].bytes = bytes ;
		}

	/* Take NEG AX into account ... */
	recode[1].bytes += 2 ;
	recode[1].clocks += 3 ;

	/* Best code uses fewest clocks */
	best = (recode[1].clocks < recode[0].clocks) ;

	/* Not always faster on 80188 */
	if (option.superset.enabled)
		{
		if (recode[best].clocks < 35) return &recode[best] ;
		}

	/* Always faster on an 8088 */
	else
		{
		if (option.time.enabled) return &recode[best] ;

		/* Use anyway if negligible increase in size */
		/* (Best code uses fewest bytes) */

		best = (recode[1].bytes < recode[0].bytes) ;
		if (recode[best].bytes <= 4) return &recode[best] ;
		}

	return NULL ;
	}
/*lint -restore */


PRIVATE BOOLEAN Expand(recode, stmt)
RECODING *recode ;
PARTS *stmt ;
	{
	BOOLEAN used_op ;
	unsigned pos ;

	used_op = FALSE ;
	for (pos = 0; pos < recode->length; pos++)
		{
		if (recode->table[pos] == 1)
			{
			Emit(stmt, null, add, ax, dx) ;
			stats.cseg_ins++ ;
			used_op = TRUE ;
			}
		else if (recode->table[pos] == 2)
			{
			Emit(stmt, null, sub, ax, dx) ;
			stats.cseg_ins++ ;
			used_op = TRUE ;
			}
		Emit(stmt, null, shl, ax, one) ;
		stats.cseg_ins++ ;
		}

	return used_op ;
	}


PRIVATE BOOLEAN Triples()
	{
	OPCODE *opc2, *opc3 ;
	REG *sreg2, *dreg2, *sreg3, *dreg3, *r ;
	CHAR *src2, *dst2, *src3, *dst3, *mne ;
	BOOLEAN dependent ;
	CHAR *ptr, cnst[80] ;
	FLAGS sattb2, dattb2 ;
	unsigned op ;
	PARTS *stmt, *temp ;
	static OPCODE *bad_ops[] =
		{
		JMP, RET, CALL, REP, MOVSW, MOVSB, STOSW, STOSB, IMULI
		} ;

	/*
		==============================================================
		The following sequences consist of three or more instructions.
		==============================================================
	*/

	if (stmt3==NULL || SEGREG(dreg1)) return FALSE ;

	for (stmt = stmt2; stmt != NULL; stmt = Find_Next(stmt))
		{
		if (stmt->opcode==SHL && stmt->dst.reg==dreg1 &&
		    stmt->src.orig[0]=='1' && stmt->src.orig[1]=='\0')
		    	{
			continue ;
			}
		if (stmt==stmt2 || stmt->opcode!=MOV ||
		    (r = stmt->dst.reg)==NULL || stmt->src.reg!=dreg1 ||
		    Reg_Needed(dreg1, stmt))
		    	{
			break ;
			}
		for (temp = stmt2; temp != stmt; temp = Find_Next(temp))
			{
			Emit(stmt, null, temp->mnemonic, r->name, "1") ;
			temp->label = substitute ;
			}
		Emit(stmt, null, opc1->name, r->name, sorig1) ;
		stmt1->label = substitute ;
		stmt->label = deleted ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	opc2 = stmt2->opcode ;
	opc3 = stmt3->opcode ;

	sreg2 = stmt2->src.reg ;
	src2 = stmt2->src.orig ;
	sattb2 = stmt2->src.attb ;

	dreg2 = stmt2->dst.reg ;
	dst2 = stmt2->dst.orig ;
	dattb2 = stmt2->dst.attb ;
	mne = stmt2->mnemonic ;

	dst3 = stmt3->dst.orig ;
	dreg3 = stmt3->dst.reg ;
	sreg3 = stmt3->src.reg ;
	src3 = stmt3->src.orig ;


	if (opc2==SHL && dreg2==dreg1 &&
	    opc3==SHL && dreg3==dreg1 &&
	    src2[0]=='1' && src2[1]=='\0'
	    && src3[0]=='1' && src3[1]=='\0')
		{
		_move(slen1, sorig1, cnst) ;
		_move((unsigned) 3, "<<", &cnst[slen1]) ;
		if ((r = Find_Val(cnst, sattb1)) != NULL)
			{
			Emit(stmt3, null, mov, dorig1, r->name) ;
			stmt3->label = replaced ;
			stmt2->label = replaced ;
			stmt1->label = replaced ;
			stats.cseg_del += 2 ;
			return TRUE ;
			}
		}

	if (Reg_Needed(dreg1, stmt3) != NULL) return FALSE ;

	if (opc2==LEA && opc3==PUSH && dreg2==dreg1 &&
	    dreg3==dreg1 && sreg1!=NULL && INDEX(dreg1) &&
	    (sattb2 & _DEPSRC)==DEPEND_FLAG(dreg1) &&
	    Reg_Needed(sreg1, stmt3)==NULL)
	    	{
		/*
			OK if r1 & r2 aren't needed (r1 deleted, and
			r2 modified)
			----------------------------------------------
			MOV	r1,r2		->	###
			LEA	r1,src[r1]	->	ADD	r2,xxx
			PUSH	r1		->	PUSH	r2
			----------------------------------------------
		*/
		ptr = index(src2, ' ') + 1 ;
		if (*ptr == '[') cnst[0] = '\0' ;
		else
			{
			_move((unsigned) 7, offset, cnst) ;
			strcpy(&cnst[7], ptr) ;
			*index(&cnst[7], '[') = '\0' ;
			}
		ptr = index(strcat(cnst, index(src2, '[') + 3), ']') ;
		strcpy(ptr, ptr + 1) ;
		if (*(ptr = cnst) == '+') ptr++ ;
		Emit(stmt3, null, push, sorig1, null) ;
		Emit(stmt3, null, add, sorig1, ptr) ;
		stmt3->label = combined ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	/*
		====================================================
		The following are common to 3-instruction sequences:
		====================================================
	*/
	if (sreg2!=dreg1 && (sattb2 & DEPEND_FLAG(dreg1))==0 &&
	    (opc2->attb & CONDJMP)==0)
	    	{
		for (op = 0; op < 9; op++)
			{
			if (opc2 == bad_ops[op]) return FALSE ;
			}
		}
	else return FALSE ;

	if (opc3==PUSH)
		{
		/*
			-------------------------
		 	MOV r,r1       ###
			opc r,src  =>  opc r1,src
			PUSH r         PUSH r1
			-------------------------
		*/
		if (dreg2==dreg1 && dreg3==dreg1 && sreg1!=NULL &&
		    sreg1!=SP && Reg_Needed(sreg1, stmt3)==NULL)
		    	{
			Emit(stmt3, null, push, sorig1, null) ;
			Emit(stmt3, null, mne, sorig1, src2) ;
			stmt3->label = combined ;
			stmt2->label = combined ;
			stmt1->label = combined ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		return FALSE ;
		}

	/*
		=====================================================
		The following conditions are common to patterns 1 & 2
		=====================================================
	*/
	if (opc3==MOV && sreg3==dreg1 && *dst2!='\0')
		{
		/* OK, proceed ... */
		}
	else return FALSE ;

	if (dreg2==dreg1)
		{
		if (dreg3!=NULL)
			{
			if (sreg2==dreg3)
				{
				if ((opc2->attb & COMMUTE)!=0)
					{
					/*
						------------------------------
						MOV r1,src1    ###
						opc r1,r2  =>  opc r2,src1
						MOV r2,r1      ###
						------------------------------
					*/
					Emit(stmt3, null, mne, dst3, sorig1) ;
					stmt3->label = combined ;
					stmt2->label = combined ;
					stmt1->label = combined ;
					stats.cseg_del += 2 ;
					return TRUE ;
					}
				else if ((opc2==SUB || opc2==SBB) &&
					 !Flags_Needed(stmt3))
					{
					/*
						If <opc>==SUB or SBB, and the
						flags are not needed:
						------------------------------
						MOV r1,src1    ###
						opc r1,r2  =>  opc r2,src1
						MOV r2,r1      NEG r2
						------------------------------
					*/
					Emit(stmt3, null, "NEG", dst3, null) ;
					Emit(stmt3, null, mne, dst3, sorig1) ;
					stmt3->label = combined ;
					stmt2->label = combined ;
					stmt1->label = combined ;
					stats.cseg_del++ ;
					return TRUE ;
					}
				}
			else if (!SEGREG(dreg3))
				{
				if (sreg1==dreg3)
					{
					/*
						--------------------------
						MOV r1,r2       ###
						opc r1,src  =>  opc r2,src
						MOV r2,r1       ###
						--------------------------
					*/
					Emit(stmt3, null, mne, dst3, src2) ;
					stmt3->label = combined ;
					stmt2->label = combined ;
					stmt1->label = combined ;
					stats.cseg_del += 2 ;
					return TRUE ;
					}

				if (!(sattb2 & DEPEND_FLAG(dreg3)))
					{
					/*
					----------------------------
					MOV r1,src1      MOV r2,src1
					opc r1,src2  =>  opc r2,src2
					MOV r2,r1        ###
					----------------------------
					*/
					Emit(stmt3, null, mne, dst3, src2) ;
					Emit(stmt3, null, mov, dst3, sorig1) ;
					stmt3->label = combined ;
					stmt2->label = combined ;
					stmt1->label = combined ;
					stats.cseg_del++ ;
					return TRUE ;
					}
				}
			return FALSE ;
			}
		if (sreg1!=NULL)
			{
			if ((stmt3->dst.attb & DEPEND_FLAG(dreg1))==0 &&
			    !SEGREG(sreg1) && sreg1!=SP &&
			    Reg_Needed(sreg1, stmt3)==NULL)
				{
				/*
					---------------------------
					MOV r1,r2        ###
					opc r1,src2  =>  opc r2,src2
					MOV dst,r1       MOV dst,r2
					---------------------------
				*/
				Emit(stmt3, null, mov, dst3, sorig1) ;
				Emit(stmt3, null, mne, sorig1, src2) ;
			        stmt3->label = combined ;
			        stmt2->label = combined ;
			        stmt1->label = combined ;
				stats.cseg_del++ ;
				return TRUE ;
				}
			return FALSE ;
			}

		if (Equal(sorig1, dst3))
			{
			if ((sreg2!=NULL || *src2=='\0' ||
			    (sattb2 & _CONST)!=0) &&
			    (sattb1 & DEPEND_FLAG(dreg1))==0)
				{
				/*
					-----------------------------------
					MOV r1,mem          ###
					opc r1,(r|imm)  =>  opc mem,(r|imm)
					MOV mem,r1          ###
					-----------------------------------
				*/
				Emit(stmt3, null, mne, dst3, src2) ;
			        stmt3->label = combined ;
			        stmt2->label = combined ;
			        stmt1->label = combined ;
				stats.cseg_del += 2 ;
				return TRUE ;
				}
			else if ((sattb1 & DEPEND_FLAG(dreg1))==0)
				{
				/*
					--------------------------
					MOV r,mem1      MOV r,mem2
					opc r,mem2  =>  opc mem1,r
					MOV mem1,r      ###
					--------------------------
				*/
				Emit(stmt3, null, mne, dst3, dorig1) ;
				Emit(stmt3, null, mov, dorig1, src2) ;
				stmt3->label = combined ;
				stmt2->label = combined ;
				stmt1->label = combined ;
				stats.cseg_del++ ;
				return TRUE ;
				}
			}
		return FALSE ; /* from pattern 1 */
		}

	if (dreg3!=NULL &&
	    !REFERENCED(dreg1, stmt2) && !REDEFINED(dreg1, stmt2))
		{
		dependent = (dreg2 && (sattb1 & DEPEND_FLAG(dreg2))) ||
			    (!sreg1 && (opc2->attb & MDFYDST) &&
			     MEMREF(sattb1) && VARDST(dattb2)) ||
			    Equal(sorig1, dst2) ;
		if (!dependent)
			{
			/*
				--------------------------------
				MOV r,src1         ###
				opc dst2,src2  =>  opc dst2,src2
				MOV r3,r           MOV r3,src1
				--------------------------------
			*/
			Emit(stmt3, null, mov, dst3, sorig1) ;
			Emit(stmt3, null, mne, dst2, src2) ;
			stmt3->label = combined ;
			stmt2->label = combined ;
			stmt1->label = combined ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		else if (!REFERENCED(dreg3, stmt2))
			{
			/*
				--------------------------------
				MOV r,src1         MOV r3,src1
				opc dst2,src2  =>  opc dst2,src2
				MOV r3,r           ###
				--------------------------------
			*/
			Emit(stmt3, null, mne, dst2, src2) ;
			Emit(stmt3, null, mov, dst3, sorig1) ;
			stmt3->label = combined ;
			stmt2->label = combined ;
			stmt1->label = combined ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}
	return FALSE ;
	}


PRIVATE BOOLEAN Quads()
	{
	REG *sreg2, *dreg3, *sreg3 ;
	CHAR *src2 ;
	REG *r ;

	/*
		=============================================================
		The following sequences consist of four or more instructions.
		=============================================================
	*/
	if (stmt4==NULL || SEGREG(dreg1)) return FALSE ;

	sreg2 = stmt2->src.reg ;
	src2 = stmt2->src.orig ;

	dreg3 = stmt3->dst.reg ;
	sreg3 = stmt3->src.reg ;

	if (stmt2->opcode==AND && stmt2->dst.reg==dreg1 &&
	    (stmt2->src.attb & DEPEND_FLAG(dreg1))==0)
		{
		/*
			------------------------------
			MOV r,src1      ###
			AND r,src2  =>  ###
			     {followed by}
			CMP r,0		TEST src1,src2
			condjmp         condjmp
				{or}
			OR r,r		TEST src1,src2
			condjmp		condjmp
			------------------------------
		*/
		if (((stmt4->opcode->attb & CONDJMP)!=0 && dreg3==dreg1 && 
		    ((stmt3->opcode==OR && dreg3==sreg3) ||
		     (stmt3->opcode==CMP && ZERO(stmt3->src.attb)))) ||
		/*
			------------------------------
			MOV r1,src1     ###
			AND r1,src2  => ###
			OR  r1,r1	TEST src1,src2
			MOV r2,0	MOV r2,0
			condjmp         condjmp
			INC r2		INC r2
			------------------------------
		*/
		    (stmt3->opcode==OR && stmt4->opcode==MOV &&
		     stmt5 && (stmt5->opcode->attb & CONDJMP) &&
		     dreg3==dreg1 && sreg3==dreg1 && stmt4->dst.reg &&
		     ZERO(stmt4->src.attb)))
		    	{
			if ((sreg1!=NULL || MEMREF(sattb1)) &&
			    sreg2==NULL && CONSTANT(stmt2->src.attb))
				{
				Emit(stmt3, null, test, sorig1, src2) ;
				stmt3->label = combined ;
				stmt2->label = combined ;
				stmt1->label = combined ;
				stats.cseg_del += 2 ;
				return TRUE ;
				}
			else if (sreg1!=NULL || sreg2!=NULL ||
				 CONSTANT(sattb1) ||
				 CONSTANT(stmt2->src.attb))
				{
				Emit(stmt3, null, test, src2, sorig1) ;
				stmt3->label = combined ;
				stmt2->label = combined ;
				stmt1->label = combined ;
				stats.cseg_del += 2 ;
				return TRUE ;
				}
			}
		return FALSE ;
		}

	if (stmt3->opcode==SHL && !REFERENCED(dreg1, stmt2) && dreg3==dreg1)
	    	{
		if (stmt4->opcode==MOV)
			{
			if (((r = stmt4->dst.reg) != NULL) &&
			    stmt4->src.reg==dreg1 && !REFERENCED(r, stmt2) &&
			    !Reg_Needed(dreg1, stmt4))
				{
				/*
					(opc == ADD/SUB/INC/DEC)
					--------------------------------
					MOV r1,src1        MOV r2,src1
					opc dst2,src2  =>  opc dst2,src2
					SHL r1,imm         SHL r2,imm
					MOV r2,r1          ###
					--------------------------------
				*/
				Emit(stmt4, null, shl,
				     r->name, stmt3->src.orig) ;
				Emit(stmt4, null, stmt2->mnemonic,
				     stmt2->dst.orig, src2) ;
				Emit(stmt4, null, mov, r->name, sorig1) ;
				stmt4->label = combined ;
				stmt3->label = combined ;
				stmt2->label = combined ;
				stmt1->label = combined ;
				stats.cseg_del++ ;
				return TRUE ;
				}
			}

		else if (stmt4->opcode==SHL && stmt4->dst.reg==dreg1 &&
			 stmt5!=NULL && stmt5->opcode==MOV)
			{
			if (((r = stmt5->dst.reg) != NULL) &&
			    stmt5->src.reg==dreg1 &&
			    !REFERENCED(r, stmt2) &&
			    !Reg_Needed(dreg1, stmt5))
				{
				/*
					(opc == ADD/SUB/INC/DEC)
					--------------------------------
					MOV r1,src1        MOV r2,src1
					opc dst2,src2  =>  opc dst2,src2
					SHL r1,imm         SHL r2,imm
					SHL r1,imm         SHL r2,imm
					MOV r2,r1          ###
					--------------------------------
				*/
				Emit(stmt5, null, shl, r->name,
					stmt4->src.orig) ;
				Emit(stmt5, null, shl, r->name,
					stmt3->src.orig) ;
				Emit(stmt5, null, stmt2->mnemonic,
				     stmt2->dst.orig, src2) ;
				Emit(stmt5, null, mov, r->name, sorig1) ;
				stmt5->label = combined ;
				stmt4->label = combined ;
				stmt3->label = combined ;
				stmt2->label = combined ;
				stmt1->label = combined ;
				stats.cseg_del++ ;
				return TRUE ;
				}
			}
		}
	return FALSE ;
	}


BOOLEAN Add_Pt2()
	{
	CHAR operand[100] ;
	PARTS *stmt ;
	OPCODE *opc ;
	CHAR *ptr, *src, *dst ;
	REG *r ;


	if (dreg1==SP)
		{
		for (stmt=Get_Next(stmt1); stmt!=NULL ; stmt=Get_Next(stmt))
			{
			if (islower(stmt->mnemonic[0])) break ;
			opc = stmt->opcode ;
			if (opc==NULL || *stmt->label==';') continue ;
			if (opc==CALL || opc==PUSH || opc==JMP || opc==POP ||
			    (opc->attb & CONDJMP)!=0) break ;
			if ((opc==MOV && stmt->dst.reg==SP) ||
			    (opc==LEAVE))
				{
				stmt1->label = deleted ;
				stats.cseg_del++ ;
				return TRUE ;
				}
			}
		return FALSE ;
		}

	if (dreg1!=NULL && INDEX(dreg1))
		{
		if (sreg1==NULL && CONSTANT(sattb1))
			{
			if (stmt2->dst.reg==dreg1 &&
			    Reg_Needed(dreg1, stmt2)==NULL &&
			    ((r = Find_Free(NULL, NULL)) != NULL))
			    	{
				/*
				-----------------------------------
				ADD rI,imm  =>  LEA r,WORD [rI+imm]
				opc rI,src      opc r,src
				-----------------------------------
				*/
				Emit(stmt2, null, stmt2->mnemonic,
					r->name, stmt2->src.orig) ;
				}
			else if (stmt2->src.reg==dreg1 &&
			         Reg_Needed(dreg1, stmt2)==NULL &&
				 ((r = Find_Free(NULL, NULL)) != NULL))
				{
				/*
				-----------------------------------
				ADD rI,imm  =>  LEA r,WORD [rI+imm]
				opc dst,rI      opc dst,r
				-----------------------------------
				*/
				Emit(stmt2, null, stmt2->mnemonic,
					stmt2->dst.orig, r->name) ;
				}
			else
				{
				return FALSE ;
				}
			_move((unsigned) 6, "WORD [", operand) ;
			_move((unsigned) 2, dorig1, &operand[6]) ;
			ptr = &operand[8] ;
			*ptr++ = '+' ;
			_move(slen1, sorig1, ptr) ;
			ptr += slen1 ;
			ptr[0] = ']' ;
			ptr[1] = '\0' ;
			Emit(stmt2, null, lea, r->name, operand) ;
			stmt2->label = preserved ;
			stmt1->label = preserved ;
			return TRUE ;
			}

		else if ((VALID(dattb1) || VALID(sattb1)) &&
			 REFERENCED(dreg1, stmt2) &&
			 (REDEFINED(dreg1, stmt2) ||
			  Reg_Needed(dreg1, stmt2)==NULL))
		    	{
			if (dreg1 == BX)
				{
				if (sreg1!=NULL && sreg1!=BX &&
				    INDEX(sreg1) && Usable(sreg1, sorig1))
				    	{
					r = sreg1 ;
					}
				else if (Usable(DI, sorig1)) r = DI ;
				else if (Usable(SI, sorig1)) r = SI ;
				else return FALSE ;
				}
			else
				{
				if (Usable(BX, sorig1)) r = BX ;
				else return FALSE ;
				}
				
			if ((stmt2->src.attb & _DEPSRC) ==
			    DEPEND_FLAG(dreg1))
				{
				/*
				----------------------------------------
				ADD r1,r2        =>  opc dst,src[r1][r2]
				opc dst,src[r1]

						-or-

				ADD rI,mem	 => MOV BX,mem
				opc dst,src[rI]     opc dst,src[rI][BX]
				----------------------------------------
				*/
				dst = stmt2->dst.orig ;
				src = Insert_String(stmt2->src.orig,
					stmt2->src.len, 0, r) ;
				}
			else if ((stmt2->dst.attb & _DEPSRC) ==
				 DEPEND_FLAG(dreg1))
				{
				/*
				----------------------------------------
				ADD r1,r2        =>  opc dst[r1][r2],src
				opc dst[r1],src

						-or-

				ADD rI,mem	 => MOV BX,mem
				opc dst[rI],src     opc dst[rI][BX],src
				----------------------------------------
				*/
				dst = Insert_String(stmt2->dst.orig,
					stmt2->dst.len, 0, r) ;
				src = stmt2->src.orig ;
				}
			else
				{
				return FALSE ;
				}
			Emit(stmt2, null, stmt2->mnemonic, dst, src) ;
			if (r != sreg1)
				{
				Emit(stmt2, null, mov, r->name, sorig1) ;
				stats.cseg_ins++ ;
				}
			stmt2->label = preserved ;
			stmt1->label = preserved ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}

	return FALSE ;
	}


PRIVATE BOOLEAN Usable(r, op)
REG *r ;
CHAR *op ;
	{
	if (EMPTY(r) || AVAIL(r)) return TRUE ;
	if (!UNKNOWN(r) && Equal(op, r->content)) return TRUE ;
	return (Reg_Needed(r, stmt1) == NULL) ;
	}


BOOLEAN Sub_Pt2()
	{
	if (dreg1!=NULL && INDEX(dreg1) && CONSTANT(sattb1))
		{
		if (stmt3!=NULL && stmt2->opcode==SHL && stmt3->opcode==JMP &&
		    stmt2->dst.reg==dreg1 &&
		    stmt2->src.orig[0]=='1' && stmt2->src.orig[1]=='\0' &&
		    index(stmt3->dst.orig, '[') != NULL)
		    	{
			/*
			----------------------------------------------
			SUB rI,imm
			SHL rI,1	   => SHL rI,1
			JMP BYTE label[rI]    JMP BYTE label[rI]-2*imm
			----------------------------------------------
			*/
			Emit(stmt3, null, "JMP", 
				Insert_String(stmt3->dst.orig, stmt3->dst.len,
					-2 * Value(sorig1, slen1), NULL),
				null) ;
			stmt3->label = adjusted ;
			stmt1->label = absorbed ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}

	return FALSE ;
	}


BOOLEAN Lea_Pt2()	/* Also called by Mov_Pt2() */
	{		/* i.e., MOV AX,OFFSET xyz_ */
	PARTS *stmt, *at ;
	static CHAR newsrc[] = "[??]" ;
	REG *dreg2, *sreg2 ;
	CHAR bfr[100] ;
	unsigned len ;
	BOOLEAN ok ;

	if (stmt2==NULL) return FALSE ;

	dreg2 = stmt2->dst.reg ;
	sreg2 = stmt2->src.reg ;

	if (stmt2->opcode==ADD)
		{
		_move(5, "WORD ", bfr) ;
		strcpy(&bfr[5], Skip_Prefix(sorig1)) ;
		len = strlen(bfr) ;

		if (dreg2==dreg1 && sreg2==NULL && CONSTANT(stmt2->src.attb))
			{
			/*
				--------------------------------------
				LEA r,WORD src  => LEA r,WORD src[imm]
				ADD r,imm
				--------------------------------------
			*/
			Emit(stmt2, null, lea, dorig1,
				Insert_String(bfr, len,
					Value(stmt2->src.orig,
						stmt2->src.len), NULL)) ;
			stmt1->label = combined ;
			stmt2->label = combined ;
			stats.cseg_del++ ;
			return TRUE ;
			}

		if (dreg1==sreg2 && Reg_Needed(dreg1, stmt2)==NULL)
			{
			/*
			-------------------------------------------------
			LEA r,src             ###
			ADD (SI|DI|BX),r  =>  LEA (SI|DI),src[(SI|DI|BX)]
			-------------------------------------------------
			*/
			if (((dreg2==SI||dreg2==DI) &&
			    (sattb1 & (_DEPSI|_DEPDI))==0) ||
			    (dreg2==BX && (sattb1 & _DEPBX)==0))
			    	{
				Emit(stmt2, stmt1->label, lea,
					stmt2->dst.orig,
					Insert_String(bfr, len, 0, dreg2)) ;
				stmt2->label = combined ;
				stmt1->label = combined ;
				stats.cseg_del++ ;
				return TRUE ;
				}
			}
		}

	if ((stmt = Reg_Needed(dreg1, stmt1)) != NULL &&
	    Reg_Needed(dreg1, stmt) == NULL)
	    	{
		ok = TRUE ;
		for (at = stmt2; at != stmt ; at = Find_Next(at))
			{
			if (Invalidates(at, sop1)) break ;
			}
		ok = (at == stmt) ;
		newsrc[1] = dorig1[0] ;
		newsrc[2] = dorig1[1] ;
		if (ok && Equal(Skip_Prefix(stmt->src.orig), newsrc))
			{
			/*
			------------------------------------------
				LEA rI,src
				...
				opc dst,WORD [rI]  =>  opc dst,src
			------------------------------------------
			*/
			Emit(stmt, null, stmt->mnemonic,
				stmt->dst.orig,
				Copy_Prefix(stmt->src.orig, sorig1)) ;
			stmt1->label = combined ;
			stmt->label = combined ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		if (ok && Equal(Skip_Prefix(stmt->dst.orig), newsrc))
			{
			/*
			------------------------------------------
				LEA rI,dst
				...
				opc WORD [rI],src  =>  opc dst,src
			------------------------------------------
			*/
			Emit(stmt, null, stmt->mnemonic,
				Copy_Prefix(stmt->dst.orig, sorig1),
				stmt->src.orig) ;
			stmt1->label = combined ;
			stmt->label = combined ;
			stats.cseg_del++ ;
			return TRUE ;
			}

		if (OFFSET(sattb1) && stmt->src.reg==dreg1 &&
		    (stmt->dst.attb & DEPEND_FLAG(dreg1))==0 &&
		    (stmt->opcode->attb & MDFYSRC)==0 &&
		    (stmt->dst.reg==NULL || !SEGREG(stmt->dst.reg)))
			{
			/*
			--------------------------
			LEA r,src      ###
			...	   =>  ...
			opc dst,r      opc dst,imm
			--------------------------
			*/
			_move(7, "OFFSET ", bfr) ;
			strcpy(&bfr[7], Skip_Prefix(sorig1)) ;
			Emit(stmt, null, stmt->mnemonic, stmt->dst.orig, bfr);
			stmt->label = replaced ;
			stmt1->label = absorbed ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}

	return FALSE ;
	}


BOOLEAN Pop()
	{
	REG *dreg2, *sreg2, *dreg3, *sreg3, *dreg4 ;

	if (Preserve()) return TRUE ;
	if (dreg1==BP || dreg1==NULL || stmt4==NULL) return FALSE ;

	dreg2 = stmt2->dst.reg ;
	sreg2 = stmt2->src.reg ;
	dreg3 = stmt3->dst.reg ;
	sreg3 = stmt3->src.reg ;	
	dreg4 = stmt4->dst.reg ;

	if (dreg2!=NULL && dreg3!=NULL && dreg4==dreg2 && sreg2==dreg3 &&
	    sreg3==dreg1 && stmt2->opcode==MOV && stmt3->opcode==MOV &&
	    ((stmt4->opcode==CMP && ZERO(stmt4->src.attb)) ||
	     (stmt4->opcode==OR && dreg4==stmt4->src.reg)))
	    	{
		/*
			-----------------------
			POP r1
			MOV r2,r3
			MOV r3,r1  =>  OR r3,r3
			CMP r2,0       POP r3
			-----------------------
		*/
		Emit(stmt4, null, "POP", sreg2->name, null) ;
		Emit(stmt4, null, or, sreg2->name, sreg2->name) ;
		if (stmt4->opcode==CMP)
			{
			stats.cseg_smp++ ;	/* CMP r2,0 -> OR r3,r3 */
			}
		stmt4->label = combined ;
		stmt3->label = combined ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.cseg_del += 2 ;
		return TRUE ;
		}
	return FALSE ;
	}


BOOLEAN Divide()
	{
	REG *dreg2 ;
	PACKED n ;
	unsigned pwr, imm ;
	OPERAND *src2 ;
	OPCODE *opc3 ;
	static CHAR bfr[] = "00000" ;
	CHAR *mask ;

	if (stmt3==NULL) return FALSE ;

	dreg2 = stmt2->dst.reg ;
	src2 = &stmt2->src ;
	opc3 = stmt3->opcode ;

	n.word = (unsigned) Value(src2->orig, src2->len) ;
	if (opc3==IDIV && (n.byte.msb & 0x80)!=0) imm = -((int) n.word) ;
	else imm = n.word ;

	if (((dreg1==DX && sreg1==DX) || (opc1==CWD)) &&
	    stmt2->opcode==MOV && (opc3==DIV||opc3==IDIV) &&
	    dreg2!=NULL && stmt3->dst.reg==dreg2 && src2->reg==NULL &&
	    CONSTANT(src2->attb) &&
	    ((pwr = Log_Base2(imm)) != 0))
	    	{
		mask = Convert(imm - 1, &bfr[4]) ;
		if (Need_Remainder(stmt3)) return Remainder(stmt3, mask) ;
		else return Quotient(stmt3, n.word, pwr, mask) ;
		}
	return FALSE ;
	}


PRIVATE BOOLEAN Quotient(stmt3, imm, pwr, mask)
PARTS *stmt3 ;
unsigned imm, pwr ;
CHAR *mask ;
	{
	/*
	-------------------------------------
	CWD|XOR DX,DX     ###
	MOV r,pwrof2  =>  ###
	[I]DIV r          SHR|SAR AX,1 (repeated)
	-------------------------------------
	*/
	if (stmt3->opcode == IDIV)
		{
		if (!option.time.enabled) return FALSE ;

		if ((imm & 0x8000) != 0)
			{
			Emit(stmt3, null, "NEG", ax, null) ;
			stats.cseg_ins++ ;
			}
		stats.cseg_ins += pwr + 5 ;
		Emit(stmt3, null, "ADC", ax, zero) ;
		Emit(stmt3, null, "NEG", dx, null) ;
		while (pwr-- != 0) Emit(stmt3, null, "SAR", ax, one) ;
		Emit(stmt3, null, and, dx, mask) ;
		Emit(stmt3, null, and, dx, ax) ;
		Emit(stmt3, null, cwd, null, null) ;
		goto done ;
		}

	/* -------------------- */
	/* stmt3->opcode == DIV */
	/* -------------------- */
	if (!option.time.enabled &&
	    !(option.superset.enabled || pwr==8 || stmt3->dst.reg==CX ||
	      CL_Is_Usable(pwr, stmt3)))
		{
		return FALSE ;
		}

	stats.cseg_ins += pwr ;
	while (pwr-- != 0) Emit(stmt3, null, "SHR", ax, one) ;

done:
	if (stmt3->dst.reg==CX) CX->attb |= _AVAIL ;

	stmt3->label = reduced ;
	stmt2->label = reduced ;
	stmt1->label = reduced ;
	stats.cseg_del += 3 ;
	return TRUE ;
	}


PRIVATE BOOLEAN Remainder(stmt3, mask)
PARTS *stmt3 ;
CHAR *mask ;
	{
	if (stmt3->opcode==DIV)
		{
		Emit(stmt3, null, and, dx, mask) ;
		Emit(stmt3, null, mov, dx, ax) ;
		stats.cseg_ins += 2 ;
		}
	else /* if (stmt3->opcode==IDIV) */
		{
		if (!option.time.enabled) return FALSE ;
		Emit(stmt3, null, mov, dx, ax) ;
		Emit(stmt3, null, sub, ax, dx) ;
		Emit(stmt3, null, xor, ax, dx) ;
		Emit(stmt3, null, and, ax, mask) ;
		Emit(stmt3, null, sub, ax, dx) ;
		Emit(stmt3, null, xor, ax, dx) ;
		Emit(stmt3, null, cwd, null, null) ;
		stats.cseg_ins += 7 ;
		}
	stmt3->label = reduced ;
	stmt2->label = reduced ;
	stmt1->label = reduced ;
	stats.cseg_del += 3 ;
	return TRUE ;
	}


PRIVATE BOOLEAN Need_Remainder(stmt)
PARTS *stmt ;
	{
	OPCODE *opc ;

	while ((stmt = Get_Next(stmt)) != NULL)
		{
		if ((opc = stmt->opcode)==LINE) break ;
		if (opc==NULL) continue ;
		if (*stmt->label!='\0') continue ;
		if (islower(stmt->mnemonic[0])) break ;
		if (stmt->src.reg==DX && opc!=XOR) return TRUE ;
		if (stmt->dst.reg==DX && (opc->attb & LOADS)!=0) break ;
		if (stmt->src.reg==AX) break ;
		if (opc==RET || opc==CALL) break ;
		if (REFERENCED(AX, stmt)) break ;
		if (REFERENCED(DX, stmt)) return TRUE ;
		}

	return (stmt == NULL) ;
	}


PRIVATE unsigned Log_Base2(n)
unsigned n ;
	{
	int pwr = 0 ;

	if (n != 0)
		{
		for (;;)
			{
			if (n == 1) return pwr ;
			if ((n & 1) != 0)  break ;
			n >>= 1 ;
			pwr++ ;
			}
		}
	return 0 ;
	}


BOOLEAN Shl_Op()
	{
	if (stmt2!=NULL && stmt2->opcode==SHR && stmt2->dst.reg==dreg1 &&
	    Equal(sorig1, stmt2->src.orig))
	    	{
		/*
			----------------
			SHL r,n  =>  ###
			SHR r,n
			----------------
		*/
		stmt2->label = deleted ;
		stmt1->label = deleted ;
		stats.cseg_del += 2 ;
		return TRUE ;
		}
	return FALSE ;
	}


PRIVATE BOOLEAN Shift8(stmt, r, imm)
PARTS *stmt ;
REG *r ;
unsigned imm ;
	{
	if (imm != 8 || r->name[1]!='X') return FALSE ;
	if (stmt->opcode == SHL) Shl8(stmt, r) ;
	else if (stmt->opcode == SHR) Shr8(stmt, r) ;
	else if (stmt->opcode==SAR && r==AX) Sar8(stmt) ;
	else if (stmt->opcode==ROR || stmt->opcode==ROL) Rotate8(stmt, r) ;
	else return FALSE ;

	stmt->label = replaced ;
	stmt1->label = absorbed ;
	stats.cseg_del += 2 ;
	return TRUE ;
	}


PRIVATE VOID Sar8(stmt)
PARTS *stmt ;
	{
	Emit(stmt, null, "CBW", null, null) ;
	Emit(stmt, null, mov, "AL", "AH") ;
	stats.cseg_ins += 2 ;
	}


PRIVATE VOID Shr8(stmt, rx)
PARTS *stmt ;
REG *rx ;
	{
	CHAR *rl = RL(rx)->name ;
	CHAR *rh = RH(rx)->name ;

	Emit(stmt, null, xor, rh, rh) ;
	Emit(stmt, null, mov, rl, rh) ;
	stats.cseg_ins += 2 ;
	}


PRIVATE VOID Shl8(stmt, rx)
PARTS *stmt ;
REG *rx ;
	{
	CHAR *rl = RL(rx)->name ;
	CHAR *rh = RH(rx)->name ;

	Emit(stmt, null, xor, rl, rl) ;
	Emit(stmt, null, mov, rh, rl) ;
	stats.cseg_ins += 2 ;
	}

PRIVATE VOID Rotate8(stmt, rx)
PARTS *stmt ;
REG *rx ;
	{
	CHAR *rl = RL(rx)->name ;
	CHAR *rh = RH(rx)->name ;

	Emit(stmt, null, "XCHG", rh, rl) ;
	stats.cseg_ins++ ;
	}

BOOLEAN Collapse()
	{
	PARTS *stmt, *last, *mark ;
	unsigned count, i ;
	static CHAR bfr[] = "99" ;
	BOOLEAN changed ;


	/*
		------------------------
		shift r,1  =>  ###
		shift r,1      ###
		 ...           MOV CL,imm
		shift r,1      shift r,CL
		------------------------
	*/
	if (dreg1==NULL || sorig1[0]!='1' || sorig1[1]!='\0')
		{
		return FALSE ;
		}

	count = 1 ;
	stmt = stmt2 ;
	do
		{
		if (stmt->opcode==opc1 && stmt->dst.reg==dreg1 &&
		    stmt->src.orig[0]=='1' && stmt->src.orig[1]=='\0')
		    	{
			if (++count == 8) mark = stmt ;
			last = stmt ;
			}
		else break ;
		} while ((stmt = Get_Next(stmt)) != NULL) ;

	if (count <= 4) return FALSE ;

	stmt = stmt1 ;
	changed = FALSE ;
	if ((count >= 8) && Shift8(mark, dreg1, 8))
		{
		changed = TRUE ;
		for (i = 0; i < 8; i++)
			{
			stmt->label = combined ;
			stmt = Get_Next(stmt) ;
			}
		stats.cseg_del += 6 ;	/* 2 deleted by Shift8() */
		count -= 8 ;
		if (count <= 2) return TRUE ;
		stmt = Get_Next(Get_Next(stmt)) ;
		}

	if (option.superset.enabled) return changed ;

	if (CL_Is_Usable(count, last))
		{
		for (i = 0; i < count; i++)
			{
			stmt->label = combined ;
			stmt = Get_Next(stmt) ;
			}
		stats.cseg_del += count ;
		Emit(last, null, opc1->name, dorig1, "CL") ;
		Emit(last, null, mov, "CL", Convert(count, &bfr[1])) ;
		stats.cseg_ins += 2 ;
		changed = TRUE ;
		}

	return changed ;
	}


PRIVATE BOOLEAN CL_Is_Usable(n, stmt)
unsigned n ;
PARTS *stmt ;
	{
	if (AVAIL(CL) || EMPTY(CL) || Reg_Needed(CL, stmt)==NULL ||
            (CONSTANT(CL->attb) &&
	    (n == (unsigned) Value(CL->content, CL->len))))
		{
		return TRUE ;
		}
	return FALSE ;
	}


PRIVATE CHAR *Copy_Prefix(prefix, operand)
CHAR *prefix ;
CHAR *operand ;
	{
	static CHAR buffer[100] ;

	strcpy(buffer, prefix) ;
	strcpy(index(buffer, '['), rindex(operand, ' ') + 1) ;
	return buffer ;
	}


			/* End of file 8088-PT2.C  */
