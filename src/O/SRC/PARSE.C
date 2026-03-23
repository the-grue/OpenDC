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
			 /*  Start of file O88.C  */

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

#define SMALL_FILE "\r\n*** Wrong option: +B, Input is SMALL case ***\7\r\n"
#define LARGE_FILE "\r\n*** Wrong option: -B, Input is LARGE case ***\7\r\n"

/* Functions PRIVATE to PARSE.C						*/
/* -------------------------------------------------------------------- */

#ifdef	_lint

VOID		Classify(PARTS *, OPERAND *) ;
VOID		Redefines(PARTS *) ;
VOID		References(PARTS *) ;

#else

VOID		Classify() ;
VOID		Redefines() ;
VOID		References() ;

#endif


PARTS *Emit(parts, label, mnemonic, dst, src)
PARTS *parts ;
CHAR *label, *mnemonic, *dst, *src ;
	{
	PARTS *new ;
	CHAR bfr[100] ;
	CHAR *ptr ;
	static CHAR comment[] = " ; {O88}" ;
	unsigned len ;

	*(ptr = bfr) = '\0' ;
	if (*label!='\0')
		{
		ptr = Append(ptr, label) ;
		}
	if (*mnemonic!='\0')
		{
		*ptr = ' ' ;
		ptr = Append(ptr + 1, mnemonic) ;
		}
	if (*dst!='\0')
		{
		*ptr = ' ' ;
		ptr = Append(ptr + 1, dst) ;
		}
	if (*src!='\0')
		{
		*ptr = ',' ;
		ptr = Append(ptr + 1, src) ;
		}

	_move(sizeof(comment), comment, ptr) ;
	len = &ptr[sizeof(comment)] - bfr ;
	new = (PARTS *) Allocate(sizeof(PARTS) + len) ;
	new->id = id++ ;
	_move(len, bfr, new->stmt) ;

	Parse(new) ;
	if (new->opcode==LINE) new->label = deleted ;

	new->next = parts->next ;
	return (parts->next = new) ;
	}


CHAR *Make_String(str, size)
CHAR *str ;
unsigned size ;
	{
	CHAR *ptr ;

	_move(size - 1, str, ptr = Allocate(size)) ;
	ptr[size - 1] = '\0' ;
	return ptr ;
	}


VOID Parse(parts)
PARTS *parts ;
	{
	unsigned dst_len, src_len, end_len ;
	CHAR *ptr, *line ;
	BOOLEAN string, text ;


	/* Set-up defaults */
	/* --------------- */
	line = parts->stmt ;
	parts->dst.len = 0 ;
	parts->src.len = 0 ;
	parts->opcode = (OPCODE *) NULL ;
	parts->dst.reg  = (REG *) NULL ;
	parts->src.reg  = (REG *) NULL ;
	parts->attb = 0 ;
	parts->dst.attb = 0 ;
	parts->src.attb = 0 ;
	parts->label = null ;
	parts->mnemonic = null ;
	parts->dst.orig = null ;
	parts->src.orig = null ;
	parts->third = null ;
	parts->comment = null;

	/* Extract comment first */
	/* --------------------- */
	string = FALSE ;
	text = FALSE ;
	for (ptr = line; *ptr != '\0'; ptr++)
		{
		if (*ptr == '\'') string = !string ;
		else if (!string && *ptr == ';')
			{
			if (text)
				{
				*ptr = '\0' ;
				parts->comment = ptr + 1 ;
				break ;
				}
			else
				{
				parts->label = ptr ;
				return ;
				}
			}
		if (*ptr != ' ') text = TRUE ;
		}

	/* Extract label */
	/* ------------- */
	if (*line != ' ')
		{
		/* Look for separator */
		/* ------------------ */
		for (parts->label = line; *line != '\0'; line++)
			{
			if (*line == ' ')
				{
				*line++ = '\0' ;
				break ;
				}
			}
		}
	line = Leading(line) ;

	/* Extract the mnemonic */
	/* -------------------- */
	for (parts->mnemonic = line; *line != '\0'; line++)
		{
		if (*line == ' ')
			{
			*line++ = '\0' ;
			break ;
			}
		}
	line = Leading(line) ;
	if (*(ptr = parts->mnemonic) == '\0') return ;

	/* Check mnemonic to see if it's an opcode. */
	/* ---------------------------------------- */
	if (Equal(ptr, "CALL") || Equal(ptr, "LCALL"))
		{
		parts->mnemonic = call ;
		parts->opcode = CALL ;
		}
	else if (Equal(ptr, "RET"))
		{
		if (option.big_model.enabled)
			{
			Errs(SMALL_FILE) ;
			Usage(ERRORS) ;
			}
		parts->mnemonic = ret ;
		parts->opcode = RET ;
		}
	else if (Equal(ptr, "LRET"))
		{
		if (!option.big_model.enabled)
			{
			Errs(LARGE_FILE) ;
			Usage(ERRORS) ;
			}
		parts->mnemonic = ret ;
		parts->opcode = RET ;
		}
	else
		{
		parts->opcode = (OPCODE *) Find(parts->mnemonic) ;
		}
	if (parts->opcode == NULL)
		{
		parts->dst.len  = strlen(parts->dst.orig = line) ;
		return ;
		}

	/* Extract the operands */
	/* -------------------- */
	if ((dst_len = Trailing(line, strlen(line))) != 0)
		{
		if ((ptr = index(parts->dst.orig = line, ',')) != NULL)
			{
			*ptr++ = '\0' ;
			src_len = dst_len - (ptr - line) ;
			dst_len -= src_len + 1 ;
			while (*ptr == ' ')
				{
				src_len-- ;
				ptr++ ;
				}
			parts->src.orig = ptr ;
			if ((ptr = index(line = ptr, ',')) != NULL) /* 3rd? */
				{
				*ptr++ = '\0' ;
				end_len = src_len - (ptr - line) ;
				src_len -= end_len + 1 ;
				Trailing(parts->third = ptr, end_len) ;
				}
			parts->src.len = Trailing(parts->src.orig, src_len) ;
			}
		parts->dst.len = Trailing(parts->dst.orig, dst_len) ;
		}

	parts->dst.reg = (parts->dst.len==2) ? Find(parts->dst.orig) : NULL ;
	parts->src.reg = (parts->src.len==2) ? Find(parts->src.orig) : NULL ;

	Classify(parts, &parts->dst) ;
	Classify(parts, &parts->src) ;
	References(parts) ;
	Redefines(parts) ;
	}


PRIVATE VOID Classify(parts, operand)
PARTS *parts ;
OPERAND *operand ;
	{
	REG *r ;
	static CHAR name[] = "??" ;
	CHAR *ptr, *orig, ch, temp[100] ;
	unsigned len ;

/* Take care of default first... */

	operand->attb = 0 ;
	if (parts->opcode == REP) return ;

/* No operand at all ...  */
	
	if (*(orig = operand->orig) == '\0')
		{
		operand->attb = _UNKN ;
		return ;
		}


/* Operand is a register ...  */

	if (operand->reg!=NULL)
		{
		/*
		-------------------------------------------
		Return ONLY the fixed attributes here since
		they may be tested somewhere other than
		when they are valid (e.g., UTILS.C)
		-------------------------------------------
		*/
		operand->attb = operand->reg->attb & _FXD ;
		return ;
		}

/*  Operand is memory data or constant ...  */

	if ((ptr = index(orig, ':')) != NULL)
		{
		/* ----------------------------------------- */
		/* Code Cleanup:  WORD ES:XYZ => ES:WORD XYZ */
		/* ----------------------------------------- */
		len = operand->len ;
		_move(3, &ptr[-2], temp) ;
		_move(len - (ptr - orig), &ptr[1], &ptr[-2]) ;
		_move(len - 2, orig, &temp[3]) ;
		_move(len + 1, temp, orig) ;
		if (orig[0]=='E' && orig[1]=='S') operand->attb |= _DEPES ;
		orig += 3 ;
		}

/* Now classify the operand ... */

	for (ptr = orig ; (ch = *ptr) != '\0' ; ptr++)
		{
		if (ch=='[' || ch=='+' || ch=='-')
			{
			name[0] = ptr[1] ;
			name[1] = ptr[2] ;
			if ((r = (REG *) Find(name)) != NULL)
				{
				operand->attb |= DEPEND_FLAG(r) ;
				ptr += 2 ;
				}
			}
		}

	if (parts->opcode==LEA || orig[0]=='@')
		{
		operand->attb |= _ADDR ;
		if (!VARSRC(operand->attb)) operand->attb |= _CONST ;
		return ;
		}

	switch (*orig)
		{
		case 'B':	/* BYTE */
		operand->attb |= _BYTE ;
		case 'W':	/* WORD */
		operand->attb |= _DATA ;
		break ;

		case 'O':	/* OFFSET */
		case 'S':	/* SEG */
		operand->attb |= (_ADDR|_CONST) ;
		break ;

		case 'Q':	/* QWORD */
		case 'D':	/* DWORD */
		operand->attb |= _DATA ;
		break ;

		default:
		if (!isdigit(*orig) && *orig!='-' && *orig!='+')
			{
			operand->attb |= _DATA ; /* WORD */
			break ;
			}

		operand->attb |= _NUMB ;

		if (operand==&parts->src && parts->opcode!=PUSH)
			{
			operand->attb |= parts->dst.attb & _BYTE ;
			}
		if (operand->len==5 && Equal(orig, "0000H"))
			{
			operand->orig = "0" ;
			operand->len = 1 ;
			}
		if (operand->orig[0]=='0' && operand->orig[1]=='\0')
			{
			operand->attb |= _ZERO ;
			}

		} /* end of switch */
	}


PRIVATE VOID Redefines(stmt)
PARTS *stmt ;
	{
	OPCODE *opc ;
	REG *dreg ;
	REG *sreg ;
	FLAGS def ;


	def = 0 ;
	if (stmt->opcode==NULL)
		{
		goto done ;
		}

	opc = stmt->opcode ;
	dreg = stmt->dst.reg ;
	sreg = stmt->src.reg ;

	if ((opc->attb & LOADS) != 0)
		{
		if (dreg != NULL) def = defflgs[dreg - reg] ;
		if (opc == LES) def |= _DEFES ;
		}

	else if (opc==CALL)
		{
		if (*stmt->dst.orig != '_') def = _DEFALL ;
		}

	else if (opc==RET)
		{
		def = _DEFALL & ~(_DEFAX|_DEFDX) ;
		if (option.big_model.enabled) def &= ~(_DEFSI|_DEFES) ;
		}

	else if (opc==IMUL || opc==MUL)
		{
		if (WORD(stmt->dst.attb)) def = _DEFDX ;
		}

	else if (opc==IMULI)
		{
		if (dreg != stmt->src.reg) def = defflgs[dreg - reg] ;
		}

	else if (opc==CWD)
		{
		def = _DEFDX ;
		}

	else if (opc==XOR)
		{
		if (dreg == sreg) def = defflgs[dreg - reg] ;
		}

done:
	stmt->attb |= def ;
	}

PRIVATE VOID References(stmt)
PARTS *stmt ;
	{
	OPCODE *opc ;
	FLAGS opc_attb ;
	FLAGS attb ;
	REG *dreg ;
	REG *sreg ;
	FLAGS ref, dref ;


	/* ---------------------------------------------- */
	/* PART 1: Implicit references due to opcode type */
	/* ---------------------------------------------- */
	ref = 0 ;
	if (stmt->opcode == NULL)
		{
		goto done ;
		}

	opc = stmt->opcode ;
	opc_attb = opc->attb ;
	attb = stmt->dst.attb | stmt->src.attb ;
	dreg = stmt->dst.reg ;
	sreg = stmt->src.reg ;

	if (opc==XOR && dreg==sreg)
		{
		goto done ;
		}

	else if (opc==CALL)
		{
		if (*stmt->dst.orig == '_') ref = _REFALL ;
		}

	else if (opc==RET)
		{
		ref = _REFAX|_REFDX ;
		if (option.big_model.enabled) ref |= _REFSI|_REFES ;
		goto done ;
		}

	else if (opc==DIV || opc==IDIV)
		{
		ref = WORD(stmt->dst.attb) ? (_REFAX|_REFDX) : _REFAX ;
		}

	else if (opc==MUL || opc==IMUL || opc==CWD)
		{
		ref = _REFAX ;
		}

	else if (opc==REP)
		{
		ref = _REFCX | _REFDI | _REFES ;
		if (EqualN(stmt->dst.orig, "MOVS", 4)) ref |= _REFSI ;
		else ref |= _REFAX ;
		}

	else if (opc==MOVSB || opc==MOVSW)
		{
		ref = _REFDI | _REFSI | _REFES ;
		}

	else if (opc==STOSB || opc==STOSW)
		{
		ref = _REFDI | _REFAX | _REFES ;
		}

	/* -------------------------------------------- */
	/* PART 2 - Explicit references due to operands */
	/* -------------------------------------------- */
	if ((attb & _DEPSI) != 0) ref |= _REFSI ;
	if ((attb & _DEPDI) != 0) ref |= _REFDI ;
	if ((attb & _DEPBX) != 0) ref |= _REFBX ;
	if ((attb & _DEPES) != 0) ref |= _REFES ;

	if (sreg != NULL) ref |= refflgs[sreg - reg] ;
	if (dreg != NULL)
		{
		dref = refflgs[dreg - reg] ;
		if ((opc_attb & RDWR) != 0) ref |= dref ;
		else if ((opc_attb & MDFYDST) != 0)
			{
			if ((opc->attb & LOADS) == 0) ref |= dref ;
			}
		else if (opc == OUT) ref |= _REFDX ;
		}

done:
	stmt->attb |= ref ;
	}


CHAR *Skip_Prefix(op)
CHAR *op ;
	{
	CHAR *p ;

	if ((p = rindex(op, ' ')) != NULL) return p + 1 ;
	if ((p = index(op, ':')) != NULL) return p + 1 ;
	return op ;
	}


		      /* End of file PARSE.C */
