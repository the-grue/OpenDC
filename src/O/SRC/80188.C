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
			/*  Start of file 80188.C  */

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


/* Functions PRIVATE to 80188.C...					*/
/* -------------------------------------------------------------------- */

#ifdef	_lint

VOID		Asmbl_IMULI(PARTS *) ;
unsigned	Mem_Field(FLAGS) ;
unsigned	Mode_Field(OPERAND *) ;
unsigned	Reg_Field(REG *) ;
VOID		Add_Disp(CHAR *, unsigned, CHAR *, FLAGS) ;
CHAR		*Disp(CHAR *) ;

#ifdef	OLD_ASM88
VOID		Asmbl_PUSH(PARTS *) ;
VOID		Asmbl_Shift(PARTS *, unsigned) ;
#endif

#else

VOID		Asmbl_IMULI() ;
unsigned	Mem_Field() ;
unsigned	Mode_Field() ;
unsigned	Reg_Field() ;
VOID		Add_Disp() ;
CHAR		*Disp() ;

#ifdef	OLD_ASM88
VOID		Asmbl_PUSH() ;
VOID		Asmbl_Shift() ;
#endif

#endif

BOOLEAN Mov_188()
	{
	CHAR newsrc[100] ;
	OPCODE *opc ;
	PARTS *stmt ;
	REG *t ;


	if (!option.superset.enabled || stmt2==NULL)
		{
		return FALSE ;
		}

	/*
		---------------------------
		MOV r,imm16  =>  ###
		PUSH r           PUSH imm16
		---------------------------
	*/
	if (stmt2->opcode==PUSH && sreg1==NULL &&
	    stmt2->dst.reg==dreg1 && (sattb1 & _CONST)!=0 &&
	    !EqualN(sorig1, "SEG ", 4) &&
	    Reg_Needed(dreg1, stmt2)==NULL)
		{
		Emit(stmt2, stmt1->label, push, sorig1, null) ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	/*
		--------------------
		MOV SP,BP  =>  ###
		POP BP         LEAVE
		--------------------
	*/
	if (stmt2->opcode==POP && dreg1==SP && sreg1==BP &&
	    stmt2->dst.reg==BP && !Unnecessary(stmt1))
		{
		Emit(stmt2, stmt1->label, "LEAVE", null, null) ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.cseg_del++ ;
		return TRUE ;
		}

	if (sreg1==NULL && CONSTANT(sattb1) &&
	    ((stmt = Reg_Needed(dreg1, stmt1)) != NULL) &&
	    Reg_Needed(dreg1, stmt)==NULL)
		{
		opc = stmt->opcode ;
		if (!option.time.enabled && (opc==IMUL || opc==MUL) &&
		    (stmt->dst.reg == dreg1) &&
		    (((unsigned) Value(sorig1, slen1)) > 256))
			{
			/*
				-------------------------------
				MOV r,imm	###
				....
				multiply r  =>  IMULI AX,AX,imm
				-------------------------------
			*/
			Emit(stmt, null, "IMULI", "AX,AX", sorig1) ;
			stmt->label = replaced ;
			stmt1->label = absorbed ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		else if ((dreg1==CL || dreg1==CX) && stmt->src.reg==dreg1 &&
                         opc->fnc==Shift)
		    	{
			/*
				--------------------------------
				MOV CL,imm8       ###
				...           =>  ...
				shift r16,CL      shift r16,imm8
				--------------------------------
			*/
			Emit(stmt, null, stmt->mnemonic, stmt->dst.orig,
				sorig1) ;
			stmt->label = replaced ;
			stmt1->label = absorbed ;
			stats.cseg_del++ ;
			return TRUE ;
			}
		}

	if (stmt4!=NULL && stmt3->opcode==IMULI && stmt4->opcode==MOV)
		{
		if (dreg1==AX && stmt3->dst.reg==dreg1 &&
		    stmt3->src.reg==dreg1 && stmt4->src.reg==dreg1 &&
		    ((t = stmt4->dst.reg) != NULL)
		    && !REFERENCED(dreg1, stmt2))
			{
			/*
				--------------------------------------
				MOV AX,src1           ###
				opcode dst2,src2  =>  opcode dst2,src2
				IMULI AX,AX,imm       IMULI r,src1,imm
				MOV r,AX              ###
				--------------------------------------
			*/
			_move(slen1, sorig1, newsrc) ;
			newsrc[slen1] = ',' ;
			strcpy(&newsrc[slen1+1], stmt3->third) ;
			Emit(stmt4, null, "IMULI", t->name, newsrc) ;
			stmt4->label = combined ;
			stmt3->label = combined ;
			stmt1->label = combined ;
			stats.cseg_del += 2 ;
			return TRUE ;
			}
		}

	return FALSE ;
	}


BOOLEAN Push_Op()
	{
	PARTS *last ;
	CHAR *locals = zero ;

	if (!option.superset.enabled)
		{
		return FALSE ;
		}

	/*
		-----------------------------------------
		label: PUSH BP     label:
		 MOV BP,SP      =>  ENTER (0|imm),0
	         [SUB SP,imm]       ###
	       -----------------------------------
	*/
	if (stmt2!=NULL && stmt2->opcode==MOV && dreg1==BP &&
	    stmt2->dst.reg==BP && stmt2->src.reg==SP)
		{
		last = stmt2 ;
		if (stmt3!=NULL && stmt3->opcode==SUB && stmt3->dst.reg==SP)
			{
			last = stmt3 ;
			stmt3->label = combined ;
			locals = stmt3->src.orig ;
			stats.cseg_del++ ;
			}
		Emit(last, null, "ENTER", locals, zero) ;
		Emit(last, stmt1->label, null, null, null) ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.cseg_del++ ;
		return TRUE ;
		}
	return FALSE ;
	}


BOOLEAN Shift_188()
	{
	PARTS *stmt, *next ;
	unsigned count, i ;
	static CHAR bfr[] = "99" ;


	if (!option.superset.enabled)
		{
		return FALSE ;
		}

	/*
		------------------------
		shift r,1  =>  ###
		shift r,1      ###
		 ...           ###
		shift r,1      shift r,imm
		------------------------
	*/
	if (dreg1==NULL || sorig1[0]!='1' || sorig1[1]!='\0')
		{
		return FALSE ;
		}

	count = 1 ;
	for (stmt = stmt1 ; (next = Get_Next(stmt)) != NULL ; stmt = next)
		{
		if (next->opcode!=opc1 || next->dst.reg!=dreg1 ||
		    next->src.orig[0]!='1' || next->src.orig[1]!='\0')
		    	{
			break ;
			}
		count++ ;
		}

	if (count < 3)
		{
		return FALSE ;
		}

	next = stmt1 ;
	for (i = 0; i < count; i++)
		{
		stmt = next ;
		stmt->label = combined ;
		next = Get_Next(stmt) ;
		}
	stats.cseg_del += count ;

	Emit(stmt, null, stmt->mnemonic, dorig1, Convert(count, &bfr[1])) ;
	stmt1->label = combined ;
	return TRUE ;
	}


VOID Assemble(stmt)
PARTS *stmt ;
	{
	OPCODE *opc = stmt->opcode ;
	CHAR *dst = stmt->dst.orig ;
	CHAR *src = stmt->src.orig ;
	CHAR *third = stmt->third ;
	PACKED n ;
	CHAR *ptr ;
	static CHAR comment[100] ;
	static CHAR new_label[100] ;
	static CHAR new_src[12] ;
	static CHAR bfr[] = "   " ;

	if (*stmt->label!=';' &&
	    (opc==IMULI
#ifdef	OLD_ASM88
	     || opc==ENTER || opc==LEAVE ||
	     (opc==PUSH && stmt->dst.reg==NULL &&
	      (OFFSET(stmt->dst.attb) || CONSTANT(stmt->dst.attb))) ||
	     (opc->fnc==Shift && stmt->src.reg==NULL &&
	      (src[0]!='1' || src[1]!='\0'))
#endif
	    ))
	    	{
		/* Superset opcode recognized - continue */
		}
	else
		{
		return ;
		}

	/* put label on separate line or else it won't assemble correctly */
	if (*stmt->label != '\0')
		{
		stmt->label = strcat(strcpy(new_label, stmt->label), "\r\n") ;
		}

	/* Make source code into a comment */

	_move(2, "  ", comment) ;
	ptr = Append(&comment[2], stmt->mnemonic) ;

	if (*dst != '\0')
		{
		*ptr++ = ' ' ;
		_move(stmt->dst.len, dst, ptr) ;
		ptr += stmt->dst.len ;
		}
	if (*src != '\0')
		{
		*ptr++ = ',' ;
		_move(stmt->src.len, src, ptr) ;
		ptr += stmt->src.len ;
		}
	if (*third != '\0')
		{
		*ptr = ',' ;
		ptr = Append(ptr + 1, third) ;
		}
	*ptr = '\0' ;
	stmt->comment = comment ;

#ifdef	OLD_ASM88

	if (opc == LEAVE)
		{
		stmt->mnemonic = "DB" ;
		stmt->dst.orig = "201" ;
		}

	else if (opc == ENTER)
		{
		stmt->mnemonic = "DB" ;
		stmt->dst.orig = "200" ;
		n.word = (unsigned) Value(dst, stmt->dst.len) ;
		*(ptr = Append(new_src,
			       Convert((unsigned) n.byte.lsb,
			       		&bfr[2]))
			      ) = ',' ;
		*Append(
			Append(ptr + 1,
			       Convert((unsigned) n.byte.msb,
			       		&bfr[2])
			      ),
			",0") = '\0' ;
		stmt->src.orig = new_src ;
		}

	else if (opc == PUSH)
		{
		Asmbl_PUSH(stmt) ;
		}

	else if (opc == SHL)
		{
		Asmbl_Shift(stmt, 0x04) ;
		}

	else if (opc == SHR)
		{
		Asmbl_Shift(stmt, 0x05) ;
		}

	else if (opc == SAR)
		{
		Asmbl_Shift(stmt, 0x07) ;
		}

	else if (opc == ROR)
		{
		Asmbl_Shift(stmt, 0x01) ;
		}

	else if (opc == ROL)
		{
		Asmbl_Shift(stmt, 0x00) ;
		}

	else
#endif

	if (opc == IMULI)
		{
		Asmbl_IMULI(stmt) ;
		}

	stmt->dst.len = strlen(stmt->dst.orig) ;
	stmt->src.len = strlen(stmt->src.orig) ;
	}


PRIVATE VOID Asmbl_IMULI(stmt)
PARTS *stmt ;
	{
	FLAGS sattb = stmt->src.attb ;
	CHAR *third = stmt->third ;
	CHAR *imm ;
	static CHAR new_src[100] ;
	static CHAR bfr[] = "   " ;
	unsigned byte2, mode ;
	int n ;

/*
	opc <- "DB"
	dst <- 0x6B (sign-extended imm8)  -or-  0x69 (imm16)
	src <- mode..dst..src[\r\n DB|DW disp]\r\n DB|DW imm"
	3rd <- ""
*/

	stmt->mnemonic = (stmt->src.attb & _DEPES) ?
			 "DB 38 ; ES:\r\n DB" : "DB" ;

	/* convert imm value to int */
	n = Value(third, strlen(third)) ;

	mode = Mode_Field(&stmt->src) ;
	byte2 = (mode << 6) | (Reg_Field(stmt->dst.reg) << 3) ;
	if (stmt->src.reg != NULL)
		{
		strcpy(new_src,
		       Convert(byte2 | Reg_Field(stmt->src.reg), &bfr[2])) ;
		}
	else
		{
		Add_Disp(strcpy(new_src,
		                Convert(byte2 | Mem_Field(sattb), &bfr[2])),
		         mode, stmt->src.orig, sattb) ;
		}

	/* set opcode byte and add immediate constant at the end */

	if (-128 <= n && n <= 127)
		{
		stmt->dst.orig = "107" ;
		imm = "\r\n DB " ;
		}
	else
		{
		stmt->dst.orig = "105" ;
		imm = "\r\n DW " ;
		}

	strcat(strcat(stmt->src.orig = new_src, imm), third) ;
	stmt->third = null ;
	}


PRIVATE unsigned Reg_Field(r)
REG *r ;
	{
	static int code[] =
		{
		4,	/* AH */
		0,	/* AL */
		0,	/* AX */
		6,	/* SI */
		7,	/* DI */
		5,	/* CH */
		1,	/* CL */
		1,	/* CX */
		7,	/* BH */
		3,	/* BL */
		3,	/* BX */
		6,	/* DH */
		2,	/* DL */
		2,	/* DX */
		-1,	/* ES - never referenced here */
		5,	/* BP */
		4,	/* SP */
		-1,	/* DS - never referenced here */
		-1	/* CS - never referenced here */
		} ;

	return code[r - reg] ;
	}


PRIVATE unsigned Mem_Field(attb)
FLAGS attb ;
	{
	int ndex = 0 ;
	static int code[] =
		{
		6,	/* Direct Address	*/
		7,	/* [BX]			*/
		6,	/* [BP+disp]		*/
		-1,	/* [BP+BX] (n/a)	*/
		4,	/* [SI]			*/
		0,	/* [BX+SI]		*/
		2,	/* [BP+SI]		*/
		-1,	/* [BP+BX+SI] (n/a)	*/
		5,	/* [DI]			*/
		1,	/* [BX+DI]		*/
		3,	/* [BP+DI]		*/
		-1,	/* [BP+BX+DI] (n/a)	*/
		-1,	/* [SI+DI] (n/a)	*/
		-1,	/* [BX+SI+DI] (n/a)	*/
		-1,	/* [BP+SI+DI] (n/a)	*/
		-1,	/* [BP+BX+SI+DI] (n/a)	*/
		} ;

	if ((attb & _DEPBX) != 0)
		{
		ndex += 1 ;
		}
	if ((attb & _DEPBP) != 0)
		{
		ndex += 2 ;
		}
	if ((attb & _DEPSI) != 0)
		{
		ndex += 4 ;
		}
	if ((attb & _DEPDI) != 0)
		{
		ndex += 8 ;
		}
	return code[ndex] ;
	}


PRIVATE unsigned Mode_Field(op)
OPERAND *op ;
	{
	CHAR *ptr, *str ;
	int n ;


	if (op->reg != NULL)
		{
		return 3 ;	/* register operand */
		}

	if (index(op->orig, '[') == NULL)
		{
		return 0 ;	/* direct address */
		}

	if (index(op->orig, ' ')[1] != '[')
		{
		return 2 ;	/* label (disp16) */
		}

	/* look for constant disp */

	for (ptr = op->orig ; *ptr!='\0' ; ptr++)
		{
		if (*ptr == '+' || *ptr == '-')
			{
			if (isdigit(ptr[1]))
				{
				break ;
				}
			}
		}
	if (*ptr == '\0')
		{
		return 0 ;	/* no disp */
		}

	str = ptr + 1 ;
	for (ptr = str + 1 ; *ptr!='\0' ; ptr++)
		{
		if (!isdigit(*ptr))
			{
			break ;
			}
		}

	*ptr = '\0' ;
	n = Value(str, (unsigned) (ptr - str)) ;

	if (-128 <= n && n <= 127)
		{
		return 1 ;	/* +disp8 */
		}
	else
		{
		return 2 ;	/* +disp16 */
		}
	}


PRIVATE VOID Add_Disp(bfr, mode, orig, attb)
CHAR *bfr ;
unsigned mode ;
CHAR *orig ;
FLAGS attb ;
	{
	if (mode == 1)		/* reg + disp8 */
		{
		strcat(strcat(bfr, "\r\n DB "), Disp(orig)) ;
		}

	else if (mode == 2)	/* reg + disp16 */
		{
		strcat(strcat(bfr, "\r\n DW "), Disp(orig)) ;
		}

	else if (!VARSRC(attb))	/* direct address */
		{
		strcat(strcat(bfr, "\r\n DW "), Disp(orig)) ;
		}
	}


PRIVATE CHAR *Disp(orig)
CHAR *orig ;
	{
	static CHAR disp[80], str[] ="?" ;
	CHAR *ptr, insides[40] ;


	/* copy original string, eliminating "WORD" or "BYTE", then */
	/* remove everything starting with left-most '[' (if any)   */
	if ((ptr = index(strcpy(disp, Skip_Prefix(orig)), '[')) != NULL)
		{
		*ptr = '\0' ;
		}

	/* append everything to the right of right-most ']' (if any) */
	if ((ptr = rindex(orig, ']')) != NULL)
		{
		strcat(disp, ptr + 1) ;
		}

	/* get copy of everything between left-most '[' and right-most ']' */
	if ((ptr = index(orig, '[')) != NULL)
		{
		if ((ptr = rindex(strcpy(insides, ptr), ']')) != NULL)
			{
			*ptr = '\0' ;
			}

		/* add any constants to the string 'disp' */
		for (ptr = insides ; *ptr!='\0' ; ptr++)
			{
			if (isdigit(*ptr))
				{
				if (ptr[-1] == '-')
					{
					strcat(disp, "-") ;
					}
				else if (!isdigit(ptr[-1]))
					{
					strcat(disp, "+") ;
					}
				str[0] = *ptr ;
				strcat(disp, str) ;
				}
			}
		}

	return disp ;
	}

#ifdef	OLD_ASM88

PRIVATE VOID Asmbl_Shift(stmt, midl_bits)
PARTS *stmt ;
unsigned midl_bits ;
	{
	static CHAR new_dst[4], new_src[100], bfr[] = "   " ;
	unsigned mode, byte1, byte2 ;
	FLAGS dattb = stmt->dst.attb ;

/*
	opc <- "DB"
	dst <- 0xC0 (byte)  -or-  0xC1 (word)
	src <- mode..midl_bits..dst[\r\n DB|DW disp]\r\n DB imm8
	3rd <- ""
*/

	stmt->mnemonic = (stmt->dst.attb & _DEPES) ?
			 "DB 38 ; ES:\r\n DB" : "DB" ;

	byte1 = WORD(dattb) ? 0xC1 : 0xC0 ;
	strcpy(new_dst, Convert(byte1, &bfr[2])) ;

	mode = Mode_Field(&stmt->dst) ;
	byte2 = (mode << 6) | (midl_bits << 3) ;
	if (stmt->dst.reg != NULL)
		{
		strcpy(new_src,
		       Convert(byte2 | Reg_Field(stmt->dst.reg), &bfr[2])) ;
		}
	else
		{
		Add_Disp(strcpy(new_src,
		                Convert(byte2 | Mem_Field(dattb), &bfr[2])),
		         mode, stmt->dst.orig, dattb) ;
		}
	stmt->src.orig = strcat(strcat(new_src, "\r\n DB "), stmt->src.orig) ;
	stmt->dst.orig = new_dst ;
	stmt->third = null ;
	}


PRIVATE VOID Asmbl_PUSH(stmt)
PARTS *stmt ;
	{
	CHAR *dst = stmt->dst.orig ;
	int n ;


	if (OFFSET(stmt->dst.attb))
		{
		stmt->dst.orig += 7 ;
		n = 999 ;	/* (a flag) */
		}
	else
		{
		n = Value(dst, stmt->dst.len) ;
		}

	if (-128 <= n && n <= 127)
		{
		stmt->mnemonic = "DB" ;
		stmt->dst.orig = "106" ;
		stmt->src.orig = dst ;
		}
	else
		{
		stmt->mnemonic = "DB 104\r\n DW" ;
		}
	}

#endif
			 /*  End of file 80188.C  */
