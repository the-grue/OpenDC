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
			/*  Start of file GATHER.C  */

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

BOOLEAN Mov_Op()
	{
	REG *r ;
	FLAGS attb ;
	CHAR temp[100] ;


	/* All MOV optimizations require dst == reg */
	if (dreg1!=NULL)
		{
		if (Preserve()  ||
		    Mov_87()    ||
		    Mov_188()   ||
		    (dreg1!=BP && dreg1!=SP &&
		     (Mov_Pt2()  ||
		      Mov_Pt1())))
		      {
		      return TRUE ;
		      }
		}

	if ((r = Bad_Ref(dop1)) != NULL || (r = Bad_Ref(sop1)) != NULL)
		{
		Undefined_Reg(r) ;
		}

	Update_Flg_Reg() ;
	Modified(dop1) ;

	if (dreg1!=NULL && sreg1!=NULL)
		{
		/*
		----------------------------------------
		Get attributes from register rather than
		from sattb1 since the latter does NOT
		include register dependencies (_DEPSRC).
		----------------------------------------
		*/
		Load_Reg(dreg1, sreg1->content, sreg1->attb) ;
		}

	else if (dreg1!=NULL)
		{
		attb = (sattb1 & DEPEND_FLAG(dreg1))!=0 ?
		       _UNKN : sattb1 ;
		if (sorig1[0] == '@')
			{
			Imm_Op(sorig1, temp) ;
			Load_Reg(dreg1, temp, attb) ;
			}
		else
			{
			Load_Reg(dreg1, sorig1, attb) ;
			}
		}

	else if (sreg1!=NULL && !ZREG(sreg1))
		{
		attb = (dattb1 & DEPEND_FLAG(sreg1))!=0 ?
			_UNKN : dattb1 ;
		Load_Reg(sreg1, dorig1, attb) ;

		if ((dattb1 & DEPEND_FLAG(sreg1)) != 0)
			{
			MODIFY(sreg1) ;
			}
		}

	return TRUE ;
	}


BOOLEAN Add_Op()
	{
	return	Add_Pt2()	||
		Add_Pt1() ;
	}


BOOLEAN Sub_Op()
	{
	return	Sub_Pt2()	||
		Sub_Pt1() ;
	}


BOOLEAN Lea_Op()
	{
	REG *r ;
	CHAR off[100] ;

	if (Lea_87() || Preserve() || Lea_Pt2() || Lea_Pt1())
		{
		return TRUE ;
		}

	if ((r = Bad_Ref(sop1)) != NULL)
		{
		Undefined_Reg(r) ;
		}

	Update_Flg_Reg() ;
	_move(7, "OFFSET ", off) ;
	strcpy(&off[7], Skip_Prefix(sorig1)) ;
	Load_Reg(dreg1, off, sattb1) ;
	return TRUE ;
	}


BOOLEAN Xor_Op()
	{
	REG *r ;

	if (Preserve() || Xor_87() || Xor_Pt1()) return TRUE ;

	if ((r = Bad_Ref(sop1)) != NULL)
		{
		Undefined_Reg(r) ;
		}

	Update_Flg_Reg() ;

	if (dreg1!=NULL && dreg1==sreg1)
		{
		Load_Reg(dreg1, zero, _NUMB) ;
		}
	else Updates(BYTE(sattb1|dattb1)) ;
	return TRUE ;
	}


BOOLEAN Shift()
	{
	REG *r ;
	CHAR *ptr ;

	if (Collapse() || Shift_188() || (opc1==SHL && Shl_Op()))
		{
		return TRUE ;
		}

	if ((r = Bad_Ref(dop1)) != NULL || (r = Bad_Ref(sop1)) != NULL)
		{
		Undefined_Reg(r) ;
		}

	Update_Flg_Reg() ;

	if (opc1 == SHL)
		{
		if (dreg1!=NULL && sreg1==NULL && CONSTANT(sattb1) &&
		    VALID(dattb1) && sorig1[1]=='\0' && (sorig1[0] < '3'))
		    	{
			Dependencies(dreg1) ;
			ptr = &dreg1->content[dreg1->len] ; 
			if (sorig1[0] == '1')
				{
				ptr[0] = '<' ;
				ptr[1] = '\0' ;
				dreg1->len++ ;
				}
			else if (sorig1[0] == '2')
				{
				ptr[0] = '<' ;
				ptr[1] = '<' ;
				ptr[2] = '\0' ;
				dreg1->len += 2 ;
				}
			return TRUE ;
			}
		}

	Updates(BYTE(dattb1)) ;
	return TRUE ;
	}


BOOLEAN Call_Op()
	{
	REG *r ;

	if (Clean_Call()) return TRUE ;
	if ((r = Bad_Ref(dop1)) != NULL)
		{
		Undefined_Reg(r) ;
		}

	Update_Flg_Reg() ;

	cld_set = FALSE ;
	if (Equal(dorig1, "_SWITCH"))
		{
		switch_stmt = 1 ;
		Free_Registers(_EMPTY, _EMPTY) ;
		}
	else if (*dorig1 == '_')
		{
		Free_Registers(_UNKN, _UNKN) ;
		}
	else
		{
		Free_Registers(_UNKN, _EMPTY) ;
		}
	return TRUE ;
	}

BOOLEAN Cwd_Op()
	{
	if (Cwd_87()) return TRUE ;

	if (EMPTY(AX))
		{
		Undefined_Reg(AX) ;
		}

	Update_Flg_Reg() ;

	if (CONSTANT(AX->attb) && AX->content[0]!='-' &&
	    (Value(AX->content, AX->len) & 0x8000)==0)
	    	{
		Load_Reg(DX, zero, _NUMB) ;
		}
	else Updates(FALSE) ;
	return TRUE ;
	}


BOOLEAN Unnecessary(parts)
PARTS *parts ;
	{
	CHAR off[100], seg[100] ;
	CHAR *src, *dst ;
	CHAR *sorig ;
	FLAGS dattb, sattb ;
	REG *dreg, *sreg ;
	OPCODE *opc ;
	unsigned slen ;


	dreg = parts->dst.reg ;
	if (dreg==NULL) return FALSE ;

	opc = parts->opcode ;
	sreg = parts->src.reg ;
	if (opc==MOV && sreg==dreg) return TRUE ;

	dattb = dreg->attb ;
	sattb = parts->src.attb ;
	if ((sattb & (_EMPTY|_UNKN)) != 0) return FALSE ;
	if (((dattb ^ sattb) & ~(_FXD|_AVAIL)) != 0) return FALSE ;

	sorig = parts->src.orig ;
	slen = parts->src.len ;
	if (opc==MOV)
		{
		dst = (dreg!=NULL) ? dreg->content : parts->dst.orig ;
		if (sreg!=NULL) src = sreg->content ;
		else if (sorig[0] == '@') Imm_Op(sorig1, src = seg) ;
		else src = sorig ;
		if (Equal(dst, src)) return TRUE ;
		}

	else if (opc==LEA)
		{
		_move(7, "OFFSET ", off) ;
		strcpy(&off[7], Skip_Prefix(sorig)) ;
		if (Equal(dreg->content, off)) return TRUE ;
		}

	else if (opc==LES)
		{
		if (((ES->attb ^ sattb) & ~(_FXD|_AVAIL))==0)
			{
			Seg_Off(sorig, slen, off, seg) ;
			if (Equal(dreg->content, off) &&
			    Equal(ES->content, seg))
			    	{
				return TRUE ;
				}
			}
		}

	else if (opc==XOR)
		{
		if (dreg==sreg && ZREG(dreg)) return TRUE ;
		}

	return FALSE ;
	}

VOID Seg_Off(orig, len, off, seg)
CHAR *orig, *off, *seg ;
unsigned len ;
	{
	if (orig[0] == '@')	/* 0123456789 */
		{		/* @DWORD ... */
		_move(7, "OFFSET ", off) ;
		_move(4, "SEG ", seg) ;
		_move(len - 6, &orig[7], &off[7]) ;
		_move(len - 6, &orig[7], &seg[4]) ;
		}
	else
		{
		if (orig[2] == ':')
			{
			_move(3, &orig[0], &off[0]) ;
			_move(len - 3, &orig[4], &off[3]) ;
			}
		else _move(len, &orig[1], &off[0]) ;
		strcpy(seg, Insert_String(off, len - 1, 2, NULL)) ;
		}
	}

BOOLEAN Inc_Op()
	{
	static CHAR bfr[] = "65535" ;
	REG *r ;
	unsigned n ;

	if ((r = Bad_Ref(dop1)) != NULL)
		{
		Undefined_Reg(r) ;
		}

	Update_Flg_Reg() ;
	if (dreg1!=NULL && CONSTANT(dattb1))
		{
		n = (unsigned) Value(dreg1->content, dreg1->len) + 1 ;
		Load_Reg(dreg1, Convert(n, &bfr[4]), _NUMB) ;
		}
	else Updates(BYTE(dattb1)) ;
	return TRUE ;
	}

BOOLEAN Dec_Op()
	{
	static CHAR bfr[] = "65535" ;
	REG *r ;
	unsigned n ;


	if ((r = Bad_Ref(dop1)) != NULL)
		{
		Undefined_Reg(r) ; 
		}

	Update_Flg_Reg() ;
	if (dreg1!=NULL && CONSTANT(dattb1))
		{
		n = (unsigned) Value(dreg1->content, dreg1->len) - 1 ;
		Load_Reg(dreg1, Convert(n, &bfr[4]), _NUMB) ;
		}
	else Updates(BYTE(dattb1)) ;
	return TRUE ;
	}

BOOLEAN Div_Op()
	{
	REG *r = NULL ;

	if (WORD(dattb1) && EMPTY(DX)) r = DX ;
	else if (EMPTY(AX)) r = AX ;
	if (((r = dreg1) != NULL) && !EMPTY(r)) r = NULL ;
	if (r != NULL)
		{
		Undefined_Reg(r) ;
		}

	Update_Flg_Reg() ;

	if (dreg1!=NULL && !BYTE(dattb1) && CONSTANT(dattb1) &&
	    ((unsigned) Value(dreg1->content, dreg1->len)) <= 256)
	    	{
		MODIFY(AX) ; MODIFY(AH) ; MODIFY(AL) ;
		MODIFY(DX) ; MODIFY(DL) ; Load_Reg(DH, zero, _NUMB) ;
		}
	else Updates(BYTE(dattb1)) ;
	return TRUE ;
	}


BOOLEAN Xchg_Op()
	{
	CHAR temp[80] ;
	REG *r ;

	if ((r = Bad_Ref(dop1)) != NULL || (r = Bad_Ref(sop1)) != NULL)
		{
		Undefined_Reg(r) ;
		}

	Update_Flg_Reg() ;

	if (dreg1!=NULL && sreg1!=NULL)
		{
		_move(sreg1->len + 1, sreg1->content, temp) ;
		_move(dreg1->len + 1, dreg1->content, sreg1->content);
		_move(sreg1->len + 1, temp, dreg1->content) ;
		Swap(&dreg1->len, &sreg1->len) ;
		Swap(&dreg1->attb, &sreg1->attb) ;
		Fix_Reg_Set(dreg1) ;
		Fix_Reg_Set(sreg1) ;
		}
	else
		{
		Modified(dop1) ;
		Modified(sop1) ;
		if (dreg1!=NULL) Load_Reg(dreg1, sorig1, sattb1) ;
		else Load_Reg(sreg1, dorig1, dattb1) ;
		}
	return TRUE ;
	}


BOOLEAN Rep_Op()
	{
	REG *r ;
	BOOLEAN movs ;
	static CHAR bfr[] = "00000:" ;
	CHAR lbl[20], str_op[6] ;
	
	movs = EqualN(dorig1, "MOVS", 4) ;
	if (option.time.enabled && dorig1[4]=='B')
		{
		/*
		----------------------------------------
		REP MOVSB/STOSB  =>      SHR CX,1
					 JAE @		; same as JNC
					 MOVSB/STOSB
				       @:
				       	[MOV AH,AL]
				       	 REP MOVSW/STOSW
		----------------------------------------
		*/
		_move(dlen1 + 1, dorig1, str_op) ;
		str_op[4] = 'W' ;
		_move(5, "_O88_", lbl) ;
		strcpy(&lbl[5], Convert(++o88_labels, &bfr[4])) ;
		Emit(stmt1, null, "REP", str_op, null) ;
		if (!movs)
			{
			Emit(stmt1, null, mov, "AH", "AL") ;
			stats.cseg_ins++ ;
			}
		Emit(stmt1, lbl, null, null, null) ;
		Emit(stmt1, null, dorig1, null, null) ;
		*index(lbl, ':') = '\0' ;
		Emit(stmt1, null, "JAE", lbl, null) ;
		Emit(stmt1, null, "SHR", "CX", one) ;
		stmt1->label = replaced ;
		stats.cseg_ins += 3 ;
		return TRUE ;
		}

	if (EMPTY(CX)) r = CX ;
	else if (EMPTY(DI)) r = DI ;
	else if (movs && EMPTY(SI)) r = SI;
	else r = NULL ;
	if (r != NULL)
		{
		Undefined_Reg(r) ;
		}

	Update_Flg_Reg() ;
	for (r = AH; r <= ES; r++)
		{
		if (!CONSTANT(r->attb)) MODIFY(r) ;
		}
	MODIFY(DI) ;
	if (movs) MODIFY(SI) ;
	Load_Reg(CX, zero, _NUMB) ;
	return TRUE ;
	}


BOOLEAN Movs_Op()
	{
	REG *r ;

	if (EMPTY(SI)) r = SI ;
	else if (EMPTY(DI)) r = DI ;
	else if (EMPTY(ES)) r = ES ;
	else r = NULL ;

	if (r != NULL) Undefined_Reg(r) ;

	for (r = AH; r <= ES; r++)
		{
		if (!CONSTANT(r->attb)) MODIFY(r) ;
		}
	MODIFY(SI) ; MODIFY(DI) ;
	if (flg_reg==SI || flg_reg==DI) flg_sts = 0 ;
	return TRUE ;
	}


BOOLEAN Stos_Op()
	{
	REG *r ;

	if (EMPTY(DI)) r = DI ;
	else if (EMPTY(ES)) r = ES ;
	else if (opc1==STOSB && EMPTY(AL)) r = AL ;
	else if (opc1==STOSW)
		{
		if (EMPTY(AX)) r = AX ;
		else if (EMPTY(AL)) r = AX ;
		else if (EMPTY(AH)) r = AX ;
		}
	else r = NULL ;

	if (r != NULL) Undefined_Reg(r) ;

	for (r = AH; r <= ES; r++)
		{
		if (!CONSTANT(r->attb)) MODIFY(r) ;
		}
	MODIFY(DI) ;
	if (flg_reg == DI) flg_sts = 0 ;
	return TRUE ;
	}

VOID Imm_Op(orig, op)
CHAR *orig ;
CHAR *op ;
	{
	unsigned len ;

	len = strlen(orig = Skip_Prefix(orig)) ;
	
	if (orig[len-2]=='+' && orig[len-1]=='2')
		{
		_move(4, "SEG ", op) ;
		_move(len - 2, orig, &op[4]) ;
		op[len + 2] = '\0' ;
		}
	else
		{
		_move(7, "OFFSET ", op) ;
		_move(len + 1, orig, &op[7]) ;
		}
	}

			 /*  End of file GATHER.C  */

