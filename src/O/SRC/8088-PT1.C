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
		       /*  Start of file 8088-PT1.C  */

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

/* Functions PRIVATE to 8088-PT1.C...					*/
/* -------------------------------------------------------------------- */

#ifdef	_lint

CHAR		*Signed(int) ;

#else

CHAR		*Signed() ;

#endif

BOOLEAN Lea_Pt1()
	{
	CHAR *ptr, cnst[80] ;
	static CHAR srcreg[] = "??" ;


	if (!VARSRC(sattb1))
		{
		/*
			-----------------------------------
			LEA r,WORD XYZ  => MOV r,OFFSET XYZ
			-----------------------------------
		*/
		_move(7, "OFFSET ", cnst) ;
		strcpy(&cnst[7], Skip_Prefix(sorig1)) ;
		Emit(stmt1, null, mov, dorig1, cnst) ;
		stmt1->label = reduced ;
		stats.cseg_smp++ ;
		return TRUE ;
		}

	else if ((ptr = index(sorig1, ' '))!=NULL && strlen(ptr)==5)
		{
		/*
			-------------------------------
			LEA r1,WORD [r2]  =>  MOV r1,r2
			-------------------------------
		*/
		srcreg[0] = ptr[2] ;
		srcreg[1] = ptr[3] ;
		Emit(stmt1, null, mov, dorig1, srcreg) ;
		stmt1->label = reduced ;
		stats.cseg_smp++ ;
		return TRUE ;
		}

	if (DEPEND_FLAG(dreg1) && ((sattb1 & _DEPSRC)==DEPEND_FLAG(dreg1)))
		{
		/*
			-------------------------------------------
			LEA r1,WORD [r1+imm]  =>  ADD r1,imm
					-or-
			LEA r1,WORD xyz[r1]   =>  ADD r1,OFFSET xyz
			-------------------------------------------
		*/
		if (index(sorig1, ' ')[1] == '[')
			{
			cnst[0] = '\0' ;
			}
		else
			{
			_move(7, offset, cnst) ;
			strcpy(&cnst[7], Skip_Prefix(sorig1)) ;
			*index(&cnst[7], '[') = '\0' ;
			}
		ptr = index(strcat(cnst, index(sorig1, '[') + 3), ']') ;
		strcpy(ptr, ptr + 1) ;
		if (*(ptr = cnst) == '+') ptr++ ;
		Emit(stmt1, null, add, dorig1, ptr) ;
		stmt1->label = reduced ;
		stats.cseg_smp++ ;
		return TRUE ;
		}

	return FALSE ;
	}


BOOLEAN IMul()
	{
	PARTS *stmt ;

	if (Mul_Op())	/* Can it be expanded ? */
		{
		return TRUE ;
		}

/*
	MUL is faster than IMUL, and the two are equivalent
	when the most significant half of the product is not
	needed.  Since IMUL is used for signed subscripts, this
	routine attempts to replace IMUL by MUL when possible.

	(The most significant half is NEVER used by C88!)
*/
	if (WORD(dattb1) && ((stmt = Reg_Needed(DX, stmt1)) != NULL) &&
            (stmt->opcode==CALL ||
	    (Contains(DX, stmt->dst.reg) || Contains(DX, stmt->src.reg))))
		{
		Emit(stmt1, null, cwd, null, null) ;
		stats.cseg_ins++ ;
		}
	Emit(stmt1, null, "MUL", dorig1, sorig1) ;
	stmt1->label = reduced ;
	stats.cseg_smp++ ;
	return TRUE ;
	}


VOID Updates(byte)
BOOLEAN byte ;
	{
	if ((opc1_attb & MDFY_AX) != 0)
		{
		MODIFY(AX) ; MODIFY(AH) ; MODIFY(AL) ;
		}

	if ((opc1_attb & MDFY_DX)!=0 && (!byte || (opc1 == CWD)))
		{
		MODIFY(DX) ; MODIFY(DH) ; MODIFY(DL) ;
		}

	if ((opc1_attb & MDFYSRC) != 0) Modified(sop1) ;
	if ((opc1_attb & MDFYDST) != 0) Modified(dop1) ;
	}


VOID Modified(operand)
OPERAND *operand ;
	{
	REG *r ;
	FLAGS all ;
	FLAGS attb ;
	CHAR *ptr, *rstr, *op ;
	unsigned len, rlen ;


	if ((r = operand->reg) != NULL)
		{

/* The modified operand is a REGISTER:					*
 * -----------------------------------					*
 * (1) Mark the contents of the modified register as UNKNOWN.		*
 *									*
 * (2) If the modified register is SI, DI, BX (or BL or BH), recursive-	*
 *     ly search all registers and mark as UNKNOWN any whose contents	*
 *     is dependent.							*
 *									*
 * (3) If the modified register is AX, BX, CX, or DX, mark its (byte) 	*
 *     component contents as UNKNOWN.					*
 *									*
 * (4) If the modified register is AL, AH, BL, BH, CL, CH, DL, or DH,	*
 *     mark the containing word register contents as UNKNOWN.		*/

		/* r and dependents <- UNKNOWN	*/
		Dependencies(r) ;

		/* fixes components		*/
		Fix_Reg_Set(r) ;
		}
	else
		{

/* The modified operand is in MEMORY:					*
 * ----------------------------------					*
 * (1) Registers which are empty, contain a constant, or are already	*
 *     marked UNKNOWN are not affected.					*
 *									*
 * (2) Any register whose contents matches the modified memory operand	*
 *     must be marked as UNKNOWN.					*
 *									*
 * (3) Registers whose contents are dependent on SI, DI, or BX must	*
 *     be marked as UNKNOWN since it is impossible to determine if	*
 *     their content corresponds to the modified memory data.		*
 *									*
 * (4A) Option -P: Pointers MIGHT point to themselves...		*
 *									*
 *     If the location of the operand is dependent on SI, DI, or BX,	*
 *     then its exact location cannot be determined, and all registers	*
 *     containing valid data must be marked as UNKNOWN.			*
 *									*
 * (4B) Option +P: Pointers NEVER point to themselves...		*
 *									*
 *     Same as above, except that if the operand is dependent on only	*
 *     one register, then THAT register (and any containing the same	*
 *     data) are NOT marked UNKNOWN.					*
 *									*/

		op = operand->orig ;
		len = operand->len ;
		all = operand->attb & (_DEPDST|_DEPES) ;

		for (r = AH; r <= ES; r++)
			{
			attb = r->attb ;
			if ((attb & (_EMPTY | _CONST | _UNKN)) != 0)
				{
				/*
				----------------------------------
				Skip registers whose contents are
				empty, a constant, or unknown.
				(These are not memory references.)
				----------------------------------
				*/
				continue ;
				}

			/*
			----------------------------------
			Any register whose content matches
			the operand is affected.
			----------------------------------
			*/
			rstr = r->content ;
			rlen = ((ptr = index(rstr, '<')) != NULL) ?
			       (ptr - rstr) : r->len ;

			if (((operand->attb ^ attb) & _BYTE) == 0)
				{
				/*
				---------------------------------------
				Operand and register are the same size.
				Check max length of both strings,
				---------------------------------------
				*/
				if (rlen > len) len = rlen ;
				}
			else if (WORD(attb))
				{
				/*
				--------------------------------------
				Word register and byte operand.  Check
				up to length of register string.
				--------------------------------------
				*/
				len = rlen ;
				}
			/*
			-------------------------------------------
			Else byte register and word operand.  Check
			up to the length of the operand string.
			-------------------------------------------
			*/

			if (EqualN(op, rstr, len))
				{
				Dependencies(r) ;
				Fix_Reg_Set(r) ;
				continue ;
				}

			if (option.pointers.enabled &&
			    (all & DEPEND_FLAG(RX(r)))!=0)
				{
				/*
				------------------------------
				If option.pointers is set and
				the operand depends on this
				register, then assume it isn't
				affected.
				------------------------------
				*/
				continue ;
				}

			if ((attb & (_DEPES|_DEPDST))!=0 ||
			    (all & _DEPDST)!=0)
				{
				/*
				--------------------------------
				Any register which is dependent
				on another register is affected;
				If the operand is dependent on
				a register, then all registers
				are affected.
				--------------------------------
				*/
				Dependencies(r) ;
				Fix_Reg_Set(r) ;
				}
			}
		}
	}


VOID Free_Registers(acc, not)
FLAGS acc, not ;
	{
	AH->attb &= _FXD ;	DH->attb &= _FXD ;
	AL->attb &= _FXD ;	DL->attb &= _FXD ;
	AX->attb &= _FXD ;	DX->attb &= _FXD ;

	BH->attb &= _FXD ;	CH->attb &= _FXD ;
	BL->attb &= _FXD ;	CL->attb &= _FXD ;
	BX->attb &= _FXD ;	CX->attb &= _FXD ;
	
	SI->attb &= _FXD ;	DI->attb &= _FXD ;
	ES->attb &= _FXD ;

	SP->attb = _UNKN ;	BP->attb = _UNKN ;

	AH->attb |= acc ;	DH->attb |= acc ;
	AL->attb |= acc ;	DL->attb |= acc ;
	AX->attb |= acc ;	DX->attb |= acc ;

	BH->attb |= not ;	CH->attb |= not ;
	BL->attb |= not ;	CL->attb |= not ;
	BX->attb |= not ;	CX->attb |= not ;

	DI->attb |= not ;

	if (option.big_model.enabled)
		{
		SI->attb |= acc ;
		ES->attb |= acc ;
		}
	else
		{
		SI->attb |= not ;
		ES->attb |= not ;
		}

	/* Restore (fixed) segment registers */
	CS->attb = _ADDR|_CONST|_SEGREG ;
	CS->len = 6 ;
	_move(7, "(CODE)", CS->content) ;

	DS->attb = _ADDR|_CONST|_SEGREG ;
	DS->len = 6 ;
	_move(7, "(DATA)", DS->content) ;

	SS->attb = _ADDR|_CONST|_SEGREG ;
	SS->len = 6 ;
	_move(7, option.big_model.enabled ? "(STAK)" : "(DATA)", SS->content);
	}


VOID Fix_Reg_Set(r)
REG *r ;
	{
	REG *rx, *rl, *rh ;

	if (BYTE(r->attb))
		{
		rx = RX(r) ;
		MODIFY(rx) ;
		}
	else if (r->name[1] == 'X')
		{
		rl = RL(r) ;
		MODIFY(rl) ;
		rh = RH(r) ;
		MODIFY(rh) ;
		}
	}


VOID Dependencies(r)
REG *r ;
	{
	FLAGS depends ;

	if (r == DS) return ;

	MODIFY(r) ;

	if (r==BL || r==BH)
		{
		r = BX ;
		MODIFY(r) ;
		}

	if (INDEX(r))
		{
		depends = DEPEND_FLAG(r) ;
 		for (r = AH; r <= ES; r++)
			{
			if ((r->attb & depends) != 0)
				{
				Dependencies(r) ;
				}
			}
		}
	}


BOOLEAN Simplify(operand, use_reg)
OPERAND *operand ;
FLAGS use_reg ;
	{
	REG *r ;

	if (operand->reg==NULL /* constant or memory reference */ &&
	    (opc1_attb & use_reg)!=0 /* replace with reg if possible */ &&
	    ((r = Find_Val(operand->orig, operand->attb)) != NULL))
	    	{
		if (operand==dop1)
			{
			if (r==ES && opc1!=PUSH) return FALSE ;
			Emit(stmt1, null, stmt1->mnemonic, r->name, sorig1) ;
			}
		else
			{
			if (r==ES)
				{
				if (opc1!=MOV ||
				    (dreg1!=NULL && SEGREG(dreg1)))
					{
					return FALSE;
					}
				}
			Emit(stmt1, null, stmt1->mnemonic, dorig1, r->name) ;
			}
		stmt1->label = memory ;
		stats.cseg_smp++ ;
		return TRUE ;
		}

	return FALSE ;
	}


VOID Swap(ptr1, ptr2)
unsigned *ptr1, *ptr2 ;
	{
	unsigned int temp ;

	temp = *ptr1 ;
	*ptr1 = *ptr2 ;
	*ptr2 = temp ;
	}


VOID Load_Reg(r, data, attb)
REG *r ;
CHAR *data ;
FLAGS attb ;
	{
	REG *rx, *rl, *rh ;
	PACKED n ;
	static CHAR bfr[] = "99999" ;
	unsigned len ;

	Dependencies(r) ;

	_move((r->len = len = strlen(data)) + 1, data, r->content) ;
	if (CONSTANT(attb) && data[0]=='0' && data[1]=='\0')
		{
		attb |= _ZERO ;
		}
	if ((attb & _UNKN) != 0)
		{
		MODIFY(r) ;
		}
	else
		{
		LOAD_ATTB(r, attb) ;
		}

	if (BYTE(r->attb))
		{
		rx = RX(r) ;	rl = RL(rx) ;	rh = RH(rx) ;
		if (r==rl && CONSTANT(attb & rh->attb))
			{
			n.byte.msb = (CHAR) Value(rh->content, rh->len) ;
			n.byte.lsb = (CHAR) Value(data, len) ;
			}
		else if (r==rh && CONSTANT(attb & rl->attb))
			{
			n.byte.msb = (CHAR) Value(data, len) ;
			n.byte.lsb = (CHAR) Value(rl->content, rl->len) ;
			}
		else
			{
			MODIFY(rx) ;
			return ;
			}
		data = Convert(n.word, &bfr[4]) ;
		_move((rx->len = &bfr[5] - data) + 1, data, rx->content) ;
		LOAD_ATTB(rx, n.word!=0 ? _NUMB : (_NUMB|_ZERO)) ;
		}

	else if (CONSTANT(attb) && r->name[1]=='X')
		{
		n.word = (unsigned) Value(data, len) ;

		rl = RL(r) ;
		data = Convert((unsigned) n.byte.lsb, &bfr[4]) ;
		_move((rl->len = &bfr[5] - data) + 1, data, rl->content) ;
		LOAD_ATTB(rl, n.byte.lsb!=0 ? _NUMB : (_NUMB|_ZERO)) ;

		rh = RH(r) ;
		data = Convert((unsigned) n.byte.msb, &bfr[4]) ;
		_move((rh->len = &bfr[5] - data) + 1, data, rh->content) ;
		LOAD_ATTB(rh, n.byte.msb!=0 ? _NUMB : (_NUMB|_ZERO)) ;
		}

	else if (MEMREF(attb) && r->name[1]=='X')
		{
		data = Word_To_Byte(data) ;
		rl = RL(r) ;
		_move((rl->len = len) + 1, data, rl->content) ;
		LOAD_ATTB(rl, attb) ;

		rh = RH(r) ;
		strcpy(rh->content, Insert_String(data, len, 1, NULL)) ;
		rh->len = strlen(rh->content) ;
		LOAD_ATTB(rh, attb) ;
		}

	else
		{
		Fix_Reg_Set(r) ;
		}
	}


REG *Find_Val(content, attb)
CHAR *content ;
FLAGS attb ;
	{
	unsigned i ;
	REG *r ;
	static REG *w_reg[] = {SI, DI, AX, CX, BX, DX, ES} ;
	static REG *b_reg[] = {AL, AH, CL, CH, BL, BH, DL, DH} ;

	if (WORD(attb))
		{
		for (i = 0 ; i < 7 ; i++)
			{
			r = w_reg[i] ;
			if (((r->attb ^ attb) & ~(_FXD|_AVAIL))==0 &&
			    Equal(r->content, content))
				{
				return r ;
				}
			}
		}
	else
		{
		for (i = 0 ; i < 8 ; i++)
			{
			r = b_reg[i] ;
			if (((r->attb ^ attb) & ~(_FXD|_AVAIL))==0 &&
			    Equal(r->content, content))
				{
				return r ;
				}
			}
		}
	return NULL ;
	}


BOOLEAN Flags_Needed(stmt)
PARTS *stmt ;
	{
	static BOOLEAN result = TRUE ;
	static int last_id = -1 ;
	OPCODE *opc ;

	if (stmt->id == last_id)
		{
		return result ;
		}
	last_id = stmt->id ;

	for (stmt = Get_Next(stmt); stmt!=NULL ; stmt = Get_Next(stmt))
		{
		opc = stmt->opcode ;
		if (opc==LINE) break ;
		if (*stmt->label==';') continue ;
		if (*stmt->label!='\0') break ;
		if (islower(*stmt->mnemonic)) break ;
		if (opc==NULL) continue ;
		if (opc==ADC || opc==SBB || (opc->attb & CONDJMP)!=0)
			{
			return (result = TRUE) ;
			}

		if ((opc->attb & (DEF_FLG|ZRO_FLG|BAD_FLG)) != 0 ||
		    opc==RET || opc==JMP)
		    	{
			break ;
			}
		}

	return (result = FALSE) ;
	}


BOOLEAN Mov_Pt1()
	{
	REG *rl, *rh, *r ;
	PACKED n ;
	static CHAR bfr[] = "   " ;
	CHAR op[100] ;


	if (sreg1 != NULL) return FALSE ;

	if (CONSTANT(sattb1))
		{
		if ((n.word = (unsigned) Value(sorig1, slen1)) == 0)
			{
			if (!Flags_Needed(stmt1))
				{
				/*
					--------------------
					MOV r,0  =>  XOR r,r
					--------------------
				*/
				Emit(stmt1, null, xor, dorig1, dorig1) ;
				stmt1->label = reduced ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}

		else if (n.word==1 && ZREG(dreg1) && !Flags_Needed(stmt1))
			{
			/*
				-----------------
				MOV r,1  => INC r
				-----------------
			*/
			Emit(stmt1, null, inc, dorig1, null) ;
			stmt1->label = replaced ;
			stats.cseg_smp++ ;
			return TRUE ;
			}

		if (dorig1[1]=='X')
			{
			rh = RH(dreg1) ; 
			rl = RL(dreg1) ;
			if (n.byte.msb==0 && ZREG(rh))
				{
				/*
					-----------------------------
					MOV rX,imm16  =>  MOV rL,imm8
					-----------------------------
				*/
				Emit(stmt1, null, mov, rl->name, sorig1) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			if (n.byte.lsb==0 && ZREG(rl))
				{
				/*
					-----------------------------
					MOV rX,imm16  =>  MOV rH,imm8
					-----------------------------
				*/
				Emit(stmt1, null, mov, rh->name,
					Convert((unsigned) n.byte.msb,
						&bfr[2])) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}
		}

	if (sorig1[0]=='@')
		{
		Imm_Op(sorig1, op) ;
		if (!SEGREG(dreg1))
			{
			Emit(stmt1, null, mov, dorig1, op) ;
			stmt1->label = simplified ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		else if ((r = Find_Free(op, NULL)) != NULL)
			{
			Emit(stmt1, null, mov, dorig1, r->name) ;
			Emit(stmt1, null, mov, r->name, op) ;
			stmt1->label = replaced ;
			stats.cseg_ins++ ;
			return TRUE ;
			}
		}

	return FALSE ;
	}


BOOLEAN Cmp_Op()
	{
	REG *rl, *rh, *r ;
	PACKED n ;

	if (dreg1==NULL || sreg1!=NULL || !CONSTANT(sattb1))
		{
		goto done ;
		}

	if (ZERO(sattb1))
		{
		if (dreg1==flg_reg &&
		    (((flg_sts & DEF_FLG) != 0) ||
		     ((flg_sts & ZRO_FLG) != 0 &&
		     (stmt2->opcode->attb & ZEROTST) != 0)))
			{
			/*
				----------------
				CMP r,0  =>  ###
				----------------
			*/
			stmt1->label = deleted ;
			stats.cseg_del++ ;
			return TRUE ;
			}

		if ((stmt2->opcode->attb & ZEROTST) != 0)
			{
			/*
				-------------------
				CMP r,0  =>  OR r,r
				-------------------
			*/
			Emit(stmt1, null, or, dorig1, dorig1) ;
			stmt1->label = reduced ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}

	n.word = (unsigned) Value(sorig1, slen1) ;
	if (dorig1[1]=='X')
		{
		rl = RL(dreg1) ;
		rh = RH(dreg1) ;
		/*
			-----------------------------
			CMP rX,imm16  =>  CMP rH,imm8
			-----------------------------
		*/
		if (n.byte.lsb==0 && ZREG(rl))
			{
			Emit(stmt1, null, cmp, rh->name, sorig1);
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}

		/*
			-----------------------------
			CMP rX,imm16  =>  CMP rL,imm8
			-----------------------------
		*/
		if (n.byte.msb==0 && ZREG(rh) &&
		    (stmt2->opcode->attb & SGNDTST)==0)
			{
			Emit(stmt1, null, cmp, rl->name, sorig1) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}

done:	if ((r = Bad_Ref(dop1)) != NULL || (r = Bad_Ref(sop1)) != NULL)
		{
		Undefined_Reg(r) ;
		}
	Update_Flg_Reg() ;

	if (dreg1!=NULL)
		{
		cmp_reg = dreg1 ;
		cmp_att = sattb1 ;
		if (sreg1!=NULL)
			{
			_move((cmp_len = sreg1->len) + 1,
				sreg1->content, cmp_val) ;
			}
		else _move((cmp_len = slen1) + 1, sorig1, cmp_val) ;
		}

	return TRUE ;
	}


BOOLEAN And_Op()
	{
	static CHAR bfr[] = "   " ;
	PACKED n ;
	REG *rh, *rl, *r ;
	BOOLEAN flgs_needed ;

	if (dreg1!=NULL && sreg1==NULL && MEMREF(sattb1) && (dorig1[1]=='X'))
		{
		rh = RH(dreg1) ;
		rl = RL(dreg1) ;
		if (ZREG(rh))
			{
			/*
				-----------------------------
				AND rX,mem16  =>  AND rL,mem8
				-----------------------------
			*/
			Emit(stmt1, null, and, rl->name,
				Word_To_Byte(sorig1)) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		else if (ZREG(rl))
			{
			/*
				--------------------------------
				AND rX,mem16  =>  AND rH,mem8[1]
				--------------------------------
			*/
			Emit(stmt1, null, and, rh->name,
				Insert_String(Word_To_Byte(sorig1),
					slen1, 1, NULL)) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		goto done ;
		}

	if (sreg1!=NULL || !CONSTANT(sattb1))
		{
		goto done ;
		}

	flgs_needed = Flags_Needed(stmt1) ;
	if (ZERO(sattb1) && !flgs_needed)
		{
		/*
			------------------------
			AND dst,0  =>  MOV dst,0
			------------------------
		*/
		Emit(stmt1, null, mov, dorig1, sorig1) ;
		stmt1->label = reduced ;
		stats.cseg_smp++ ;
		return TRUE ;
		}

	n.word = (unsigned) Value(sorig1, slen1) ;
	if (n.word==0x00FF && dreg1!=NULL &&
	    (BYTE(dattb1) || (CONSTANT(dreg1->attb) &&
	     (Value(dreg1->content, dreg1->len) & 0xFF00)==0)))
		{
		if (BYTE(dattb1))
			{
			if (flgs_needed)
				{
				/*
				-----------------------------------
				AND r,255 => OR r,r  ; r = byte reg
				-----------------------------------
				*/
				Emit(stmt1, null, or, dorig1, dorig1) ;
				stmt1->label = simplified ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}

		if (!flgs_needed)
			{
			/*
			------------------
			AND r,255  =>  ###
			------------------
			*/
			stmt1->label = deleted ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}

	if (flgs_needed)
		{
		goto done ;
		}

	if (n.byte.lsb == 0xFF)
		{
		if (dreg1!=NULL)
			{
			if (dorig1[1]=='X')
				{
				/*
					-----------------------------
					AND rX,imm16  =>  AND rH,imm8
					-----------------------------
				*/
				Emit(stmt1, null, and, RH(dreg1)->name,
					Convert((unsigned) n.byte.msb,
						&bfr[2])) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}
		else if (WORD(dattb1))
			{
			/*
				-------------------------------------
				AND mem16,imm16  =>  AND mem8[1],imm8
				-------------------------------------
			*/
			Emit(stmt1, null, and,
				Insert_String(Word_To_Byte(dorig1),
					dlen1, 1, NULL),
				Convert((unsigned) n.byte.msb, &bfr[2])) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}

	if (n.byte.msb == 0xFF)
		{
		if (dreg1!=NULL)
			{
			if (dorig1[1]=='X')
				{
				/*
					-----------------------------
					AND rX,imm16  =>  AND rL,imm8
					-----------------------------
				*/
				Emit(stmt1, null, and, RL(dreg1)->name,
					Convert((unsigned) n.byte.lsb,
						&bfr[2])) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}
		else if (WORD(dattb1))
			{
			/*
				-------------------------------
				AND mem,imm16  =>  AND mem,imm8
				-------------------------------
			*/
			Emit(stmt1, null, and, Word_To_Byte(dorig1),
				Convert((unsigned) n.byte.lsb, &bfr[2])) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}

done:	if ((r = Bad_Ref(dop1)) != NULL || (r = Bad_Ref(sop1)) != NULL)
		{
		Undefined_Reg(r) ;
		}
	Update_Flg_Reg() ;

	if (dreg1!=NULL)
		{
		cmp_reg = dreg1 ;
		cmp_val[0] = '0' ;
		cmp_val[1] = '\0' ;
		cmp_len = 1 ;
		cmp_att = _NUMB|_ZERO ;
		if (dorig1[1]=='X')
			{
			rl = RL(dreg1) ;
			rh = RH(dreg1) ;
			if (ZREG(rl))
				{
				Dependencies(dreg1) ;
				Load_Reg(rl, zero, _NUMB) ;
				MODIFY(rh) ;
				}
			else if (ZREG(rh))
				{
				Dependencies(dreg1) ;
				Load_Reg(rh, zero, _NUMB) ;
				MODIFY(rl) ;
				}
			else
				{
				Dependencies(dreg1) ;
				MODIFY(rl) ;
				MODIFY(rh) ;
				}
			}
		else Modified(dop1) ;
		}
	else Updates(BYTE(sattb1|dattb1)) ;
	return TRUE ;
	}


BOOLEAN Or_Op()
	{
	static CHAR bfr[] = "   " ;
	REG *r ;
	PACKED n ;
	CHAR *src ;
	BOOLEAN flgs_needed ;

	if (dreg1!=NULL && dreg1==flg_reg && dreg1==sreg1 &&
	    ((flg_sts & DEF_FLG)!=0 ||
	     (flg_sts & ZRO_FLG)!=0 && (stmt2->opcode->attb & ZEROTST)!=0))
		{
		/*
			----------------
			OR r,r  =>  ###
			----------------
		*/
		stmt1->label = deleted ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	if (sreg1!=NULL || !CONSTANT(sattb1))
		{
		goto done ;
		}

	flgs_needed = Flags_Needed(stmt1) ;
	if (ZERO(sattb1))
		{
		if (flgs_needed)
			{
			if (dreg1!=NULL)
				{
				Emit(stmt1, null, or, dorig1, dorig1) ;
				stmt1->label = simplified ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			goto done ;
			}
		else
			{
			/*
			-----------------
			OR dst,0  =>  ###
			-----------------
			*/
			stmt1->label = deleted ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}

	if (flgs_needed)
		{
		goto done ;
		}

	n.word = (unsigned) Value(sorig1, slen1) ;
	if (n.byte.lsb==0)
		{
		src = Convert((unsigned) n.byte.msb, &bfr[2]) ;
		if (dreg1!=NULL)
			{
			if (dorig1[1]=='X')
				{
				/*
					--------------------------
					OR rX,imm16  => OR rH,imm8
					--------------------------
				*/
				Emit(stmt1, null, or, RH(dreg1)->name, src) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}
		else if (WORD(dattb1))
			{
			/*
				-----------------------------------
				OR mem16,imm16  =>  OR mem8[1],imm8
				-----------------------------------
			*/
			Emit(stmt1, null, or,
				Insert_String(Word_To_Byte(dorig1),
					dlen1, 1, NULL), src) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}
	else if (n.byte.msb==0)
		{
		if (dreg1!=NULL)
			{
			if (dorig1[1]=='X')
				{
				/*
					--------------------------
					OR rX,imm16  => OR rL,imm8
					--------------------------
				*/
				Emit(stmt1, null, or, RL(dreg1)->name, 
					sorig1) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}
		else if (WORD(dattb1))
			{
			/*
				-----------------------------
				OR mem,imm16  =>  OR mem,imm8
				-----------------------------
			*/
			Emit(stmt1, null, or, Word_To_Byte(dorig1),
				sorig1) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}

done:	if ((r = Bad_Ref(dop1)) != NULL || (r = Bad_Ref(sop1)) != NULL)
		{
		Undefined_Reg(r) ;
		}
	Update_Flg_Reg() ;

	if (dreg1!=NULL && dreg1==sreg1)
		{
		cmp_reg = dreg1 ;
		cmp_val[0] = '0' ;
		cmp_val[1] = '\0' ;
		cmp_len = 1 ;
		cmp_att = _NUMB|_ZERO ;
		}
	else Updates(BYTE(sattb1|dattb1)) ;
	return TRUE ;
	}


BOOLEAN Xor_Pt1()
	{
	static CHAR bfr[] = "   " ;
	PACKED n ;
	PARTS *stmt ;
	CHAR *src ;
	BOOLEAN flgs_needed ;


	if (dreg1!=NULL && ((stmt = Reg_Needed(dreg1, stmt1)) != NULL) &&
	    (stmt->src.reg == dreg1) && (stmt->opcode->attb & RSWR)!=0 &&
	    Reg_Needed(dreg1, stmt)==NULL && sreg1==dreg1)
		{
		/*
			------------------------
			XOR r,r        ...
			...        =>  ...
			opc dst,r      opc dst,0
			------------------------
		*/
		Emit(stmt, null, stmt->mnemonic, stmt->dst.orig, zero) ;
		stmt->label = replaced ;
		stmt1->label = absorbed ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	if (sreg1!=NULL || !CONSTANT(sattb1))
		{
		return FALSE ;
		}

	flgs_needed = Flags_Needed(stmt1) ;
	if (ZERO(sattb1))
		{
		if (flgs_needed)
			{
			if (dreg1!=NULL)
				{
				/*
				-----------------
				XOR r,0 => OR r,r
				-----------------
				*/
				Emit(stmt1, null, or, dorig1, dorig1) ;
				stmt1->label = simplified ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			return FALSE ;
			}
		else
			{
			/*
			------------------
			XOR dst,0  =>  ###
			------------------
			*/
			stmt1->label = deleted ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}

	if (flgs_needed)
		{
		return FALSE ;
		}

	n.word = (unsigned) Value(sorig1, slen1) ;
	if (n.byte.lsb==0)
		{
		src = Convert((unsigned) n.byte.msb, &bfr[2]) ;
		if (dreg1!=NULL)
			{
			if (dorig1[1]=='X')
				{
				/*
					----------------------------
					XOR rX,imm16  => XOR rH,imm8
					----------------------------
				*/
				Emit(stmt1, null, xor, RH(dreg1)->name,
					src) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}
		else if (WORD(dattb1))
			{
			/*
				-------------------------------------
				XOR mem16,imm16  =>  XOR mem8[1],imm8
				-------------------------------------
			*/
			Emit(stmt1, null, xor, 
				Insert_String(Word_To_Byte(dorig1),
					dlen1, 1, NULL), src) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}
	else if (n.byte.msb==0)
		{
		if (dreg1!=NULL)
			{
			if (dorig1[1]=='X')
				{
				/*
					----------------------------
					XOR rX,imm16  => XOR rL,imm8
					----------------------------
				*/
				Emit(stmt1, null, xor, RL(dreg1)->name, 
					sorig1) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}
		else if (WORD(dattb1))
			{
			/*
				---------------------------------
				XOR mem,imm16  =>  XOR mem,imm8
				---------------------------------
			*/
			Emit(stmt1, null, xor, 
				Word_To_Byte(dorig1), sorig1) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}

	return FALSE ;
	}


BOOLEAN Add_Pt1()
	{
	static CHAR bfr[] = "   " ;
	CHAR *src ;
	PACKED n ;

	if (sreg1!=NULL || !CONSTANT(sattb1))
		{
		return FALSE ;
		}

	if (ZERO(sattb1))
		{
		/*
			------------------------------
			ADD lo,0          ###
			[ADC hi,src]  =>  [ADD hi,src]
			------------------------------
		*/
		if (stmt2!=NULL && stmt2->opcode==ADC)
			{
			Emit(stmt2, null, add, stmt2->dst.orig,
			     stmt2->src.orig) ;
			stmt2->label = replaced ;
			}
		stmt1->label = deleted ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	n.word = (unsigned) Value(sorig1, slen1) ;
	if (n.word==1 && !Flags_Needed(stmt1))
		{
		/*
			----------------------
			ADD dst,1  =>  INC dst
			----------------------
		*/
		Emit(stmt1, null, inc, dorig1, null) ;
		stmt1->label = reduced ;
		stats.cseg_smp++ ;
		return TRUE ;
		}

	if (n.byte.lsb==0)
		{
		src = Convert((unsigned) n.byte.msb, &bfr[2]) ;
		if (dreg1!=NULL)
			{
			if (dorig1[1]=='X')
				{
				/*
					-----------------------------
					ADD rX,imm16  =>  ADD rH,imm8
					-----------------------------
				*/
				Emit(stmt1, null, add, RH(dreg1)->name,
					src) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}
		else if (WORD(dattb1))
			{
			/*
				-------------------------------------
				ADD mem16,imm16  =>  ADD mem8[1],imm8
				-------------------------------------
			*/
			Emit(stmt1, null, add, 
				Insert_String(Word_To_Byte(dorig1),
					dlen1, 1, NULL), src) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}

	return FALSE ;
	}


BOOLEAN Sub_Pt1()
	{
	static CHAR bfr[] = "   " ;
	PACKED n ;
	CHAR *src ;

	if (sreg1!=NULL || !CONSTANT(sattb1))
		{
		return FALSE ;
		}

	if (ZERO(sattb1))
		{
		/*
			------------------------------
			SUB lo,0          ###
			[SBC hi,src]  =>  [SUB hi,src]
			------------------------------
		*/
		if (stmt2!=NULL && stmt2->opcode==SBB)
			{
			Emit(stmt2, null, sub, stmt2->dst.orig,
			     stmt2->src.orig) ;
			stmt2->label = replaced ;
			}
		stmt1->label = deleted ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	n.word = (unsigned) Value(sorig1, slen1) ;
	if (n.word==1 && !Flags_Needed(stmt1))
		{
		/*
			----------------------
			SUB dst,1  =>  DEC dst
			----------------------
		*/
		Emit(stmt1, null, dec, dorig1, null) ;
		stmt1->label = reduced ;
		stats.cseg_smp++ ;
		return TRUE ;
		}

	if (n.byte.lsb==0)
		{
		src = Convert((unsigned) n.byte.msb, &bfr[2]) ;
		if (dreg1!=NULL)
			{
			if (dorig1[1]=='X')
				{
				/*
					-----------------------------
					SUB rX,imm16  =>  SUB rH,imm8
					-----------------------------
				*/
				Emit(stmt1, null, sub, RH(dreg1)->name,
					src) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}
		else if (WORD(dattb1))
			{
			/*
				-------------------------------------
				SUB mem16,imm16  =>  SUB mem8[1],imm8
				-------------------------------------
			*/
			Emit(stmt1, null, sub, 
				Insert_String(Word_To_Byte(dorig1),
					dlen1, 1, NULL), src) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}

	return FALSE ;
	}


BOOLEAN Test_Op()
	{
	PACKED n ;
	static CHAR bfr[] = "   " ;
	CHAR *src ;

	if (sreg1!=NULL || !CONSTANT(sattb1))
		{
		return FALSE ;
		}

	n.word = (unsigned) Value(sorig1, slen1) ;
	if (n.byte.lsb==0)
		{
		src = Convert((unsigned) n.byte.msb, &bfr[2]) ;
		if (dreg1!=NULL)
			{
			if (dorig1[1]=='X')
				{
				/*
					------------------------------
					TEST rX,imm16  => TEST rH,imm8
					------------------------------
				*/
				Emit(stmt1, null, test, RH(dreg1)->name,
					src) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}
		else if (WORD(dattb1))
			{
			/*
				---------------------------------------
				TEST mem16,imm16  =>  TEST mem8[1],imm8
				---------------------------------------
			*/
			Emit(stmt1, null, test, 
				Insert_String(Word_To_Byte(dorig1),
					dlen1, 1, NULL), src) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}
	else if (n.byte.msb==0)
		{
		if (dreg1!=NULL)
			{
			if (dorig1[1]=='X')
				{
				/*
					------------------------------
					TEST rX,imm16  => TEST rL,imm8
					------------------------------
				*/
				Emit(stmt1, null, test, RL(dreg1)->name,
					sorig1) ;
				stmt1->label = smaller ;
				stats.cseg_smp++ ;
				return TRUE ;
				}
			}
		else if (WORD(dattb1))
			{
			/*
				-----------------------------
				TEST x,imm16  =>  TEST x,imm8
				-----------------------------
			*/
			Emit(stmt1, null, test, 
				Word_To_Byte(dorig1), sorig1) ;
			stmt1->label = smaller ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		}
	return FALSE ;
	}

BOOLEAN Cld()
	{
	if (cld_set)
		{
		stmt1->label = redundant ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	cld_set = TRUE ;
	return FALSE ;
	}


CHAR *Word_To_Byte(op)
CHAR *op ;
	{
	static CHAR buffer[100] = "ES:BYTE " ;

	strcpy(&buffer[8], Skip_Prefix(op)) ;
	return (op[2] == ':') ? &buffer[0] : &buffer[3] ;
	}


BOOLEAN Les()
	{
	REG *r ;
	CHAR off[100], seg[100] ;
	FLAGS attb ;

	Seg_Off(sorig1, slen1, off, seg) ;

	if (option.time.enabled && sorig1[0]=='@' &&
	    stmt2->opcode==PUSH && stmt2->dst.reg==ES &&
	    stmt3->opcode==PUSH && stmt3->dst.reg==dreg1 &&
	    !Reg_Needed(ES, stmt2))
	    	{
		/*
			------------------------------
			LES r,src  => MOV r,SEG src
			PUSH ES	      PUSH r
			PUSH r        MOV r,OFFSET src
				      PUSH r
			------------------------------
		*/
		Emit(stmt3, null, push, dorig1, null) ;
		Emit(stmt3, null, mov, dorig1, off) ;
		Emit(stmt3, null, push, dorig1, null) ;
		Emit(stmt3, null, mov, dorig1, seg) ;
		stmt1->label = replaced ;
		stmt2->label = replaced ;
		stmt3->label = replaced ;
		stats.cseg_ins++ ;
		return TRUE ;
		}

	if (VALID(ES->attb) && Equal(ES->content, seg))
		{
		/*
			-------------------------------
			LES r,src  =>  MOV r,OFFSET src
			(ES already contains SEG src.)
			-------------------------------
		*/
		Emit(stmt1, null, mov, dorig1, off) ;
		stmt1->label = simplified ;
		stats.cseg_smp++ ;
		return TRUE ;
		}

	if (VALID(dattb1) && Equal(dreg1->content, off))
		{
		if (sorig1[0] != '@')
			{
			/*
				--------------------------------
				LES r,src  =>  MOV ES,src[2]
				(r already contains OFFSET src.)
				--------------------------------
			*/
			Emit(stmt1, null, mov, "ES", seg) ;
			stmt1->label = simplified ;
			stats.cseg_smp++ ;
			return TRUE ;
			}
		else if ((r = Find_Free(seg, NULL)) != NULL)
			{
			/*
				--------------------------------
				LES r,src  =>  MOV r,SEG src
					       MOV ES,r
				(r already contains OFFSET src.)
				--------------------------------
			*/
			Emit(stmt1, null, mov, "ES", r->name) ;
			Emit(stmt1, null, mov, r->name, seg) ;
			stmt1->label = simplified ;
			stats.cseg_ins++ ;
			return TRUE ;
			}
		}

	if (sorig1[0]=='@' && option.time.enabled)
		{
		/*
			------------------------------
			LES r,src  => MOV r,SEG src
				      MOV ES,r
				      MOV r,OFFSET src
			------------------------------
		*/
		Emit(stmt1, null, mov, dorig1, off) ;
		Emit(stmt1, null, mov, "ES", dorig1) ;
		Emit(stmt1, null, mov, dorig1, seg) ;
		stmt1->label = memory ;
		stats.cseg_ins += 2 ;
		return TRUE ;
		}

	if ((r = Bad_Ref(sop1)) != NULL)
		{
		Undefined_Reg(r) ;
		}

	Update_Flg_Reg() ;

	attb = (sattb1 & DEPEND_FLAG(dreg1))!=0 ? _UNKN : sattb1 ;

	Load_Reg(ES, seg, attb) ;
	Load_Reg(dreg1, off, attb) ;

	if (flg_reg == ES) flg_sts = 0 ;

	return TRUE ;
	}


CHAR *Insert_String(op, len, imm, reg)
CHAR *op ;
unsigned len ;
int imm ;
REG *reg ;
	{
	static CHAR bfr[100] ;
	CHAR *at, *ins ;
	int old ;

	/* ------------------------------------------------------------- */
	/* Assume the following syntax:					 */
	/* 								 */
	/* <op> := [<type-modifier> <space>] [<label>] [<bracket-exp>] ; */
	/* <type-modifier> := 'BYTE' | 'WORD' | 'DWORD' | 'QWORD' ;      */
	/* <bracket-exp> := '[' (<imm> | <reg> | <reg-exp>) ']' ;	 */
	/* <reg-exp> := (<reg> | <pair>) [ <sign> <imm>]) ;		 */
	/* <pair> := <reg> '+' <reg> ;					 */
	/*								 */
	/* NOTE: 'str' will always be either an immediate constant or	 */
	/* an index or base register.  In all cases, the final result	 */
	/* will become (if not already) a bracketed expression.		 */
	/* ------------------------------------------------------------- */

	_move(len, op, bfr) ;	/* Get our local copy to modify */
	bfr[len] = '\0' ;	/* just in case! */

	/* ------------------------------------------------------------	*/
	/* If the original operand does not contain a bracketed		*/
	/* expresion, then the new string is bracketed and appended.	*/
	/* ------------------------------------------------------------	*/
	if (bfr[len-1] != ']')
		{
		bfr[len] = '[' ;
		at = &bfr[len+1] ;
		if (reg != NULL)
			{
			ins = reg->name ;
			}
		else
			{
			ins = Signed(imm) ;
			if (*ins == '+') ins++ ;
			}
		}

	/* ----------------------------------------------------- */
	/* The original operand contains a bracketed expression. */
	/* If the insertion is a register, insert it at the end. */
	/* ----------------------------------------------------- */
	else if (reg!=NULL)
		{
		bfr[len-1] = '+' ;
		at = &bfr[len] ;
		ins = reg->name ;
		}

	/* ------------------------------------------------------------ */
	/* The string to be inserted is an immediate constant.		*/
	/* It must be numerically combined with any existing		*/
	/* constants in order to simplify string comparisons.		*/
	/*								*/
	/* NOTE: If an existing immediate constant exists, it will	*/
	/* always be at the end, just before the right bracket.		*/
	/* ------------------------------------------------------------ */
	else if (isdigit(bfr[len-2]))
		{
		/* Locate the start of the existing constant */
		for (at = bfr; *at != '\0'; at++)
			{
			if (isdigit(*at)) break ;
			}
		old = Value(at, (unsigned) (&bfr[len-1] - at)) ;
		if (*--at == '-') old = -old ;
		else if (*at != '+') at++ ;
		ins = Signed(old + imm) ;
		if (at[-1]=='[' && *ins=='+') ins++ ;
		}
	
	/* ------------------------------------------------------------ */
	/* There is no existing constant.  Add the new constant at	*/
	/* the end, just before the right bracket.			*/
	/* ------------------------------------------------------------ */
	else
		{
		at = &bfr[len-1] ;
		ins = Signed(imm) ;
		}

	strcpy(at, ins) ;
	strcat(bfr, "]") ;
	return bfr ;
	}


PRIVATE CHAR *Signed(imm)
int imm ;
	{
	static CHAR num[] = "-00000" ;
	CHAR *ins ;

	if (imm >= 0)
		{
		ins = Convert((unsigned) imm, &num[5]) ;
		*--ins = '+' ;
		}
	else
		{
		ins = Convert((unsigned) -imm, &num[5]) ;
		*--ins = '-' ;
		}
	return ins ;
	}

VOID Update_Flg_Reg()
	{
	if ((opc1_attb & (DEF_FLG|ZRO_FLG|BAD_FLG)) != 0)
		{
		flg_sts = (opc1 == CMP) ? 0 : opc1_attb ;
		flg_reg = dreg1 ;
		}
	else
		{
		if (flg_reg==dreg1 && (opc1_attb & MDFYDST)!=0)
			{
			flg_sts = 0 ;
			}
		else if (flg_reg==sreg1 && (opc1_attb & MDFYSRC)!=0)
			{
			flg_sts = 0 ;
			}
		else if (flg_reg==AX)
			{
			if ((opc1_attb & MDFY_AX) != 0) flg_sts = 0 ;
			}
		else if (flg_reg==DX)
			{
			if ((opc1_attb & MDFY_DX) != 0) flg_sts = 0 ;
			}
		}
	}


			/*  End of file 8088-PT1.C  */

