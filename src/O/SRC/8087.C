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
			 /*  Start of file 8087.C  */

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

/* Variables PRIVATE to 8087.C...					*/
/* -------------------------------------------------------------------- */
typedef struct FP_ZERO_ONE
	{
	struct FP_ZERO_ONE	*next ;
	OPCODE			*opc ;
	CHAR			label[1] ;
	} FP_ZERO_ONE ;

typedef struct CONSTR_INSTR
	{
	CHAR			*constant ;
	OPCODE			*instruction ;
	} CONST_INSTR ;

PRIVATE FP_ZERO_ONE *fp01 = NULL ;

typedef struct LIBRARY
	{
	CHAR			*function ;
	CHAR			**code ;
	int			cost ;
	} LIBRARY ;

#ifdef	_lint

#define	PRODUCTION	1
PRIVATE BOOLEAN SI_Ref(CHAR *) ;
PRIVATE CHAR *Reverse_Jump(OPCODE *) ;
PRIVATE BOOLEAN NDP_Library(PARTS *) ;
PRIVATE LIBRARY *Find_Library(CHAR *) ;
PRIVATE PARTS *Fpush(VOID) ;

#else

PRIVATE BOOLEAN SI_Ref() ;
PRIVATE CHAR *Reverse_Jump() ;
PRIVATE BOOLEAN NDP_Library() ;
PRIVATE LIBRARY *Find_Library() ;
PRIVATE PARTS *Fpush() ;

#endif

PRIVATE CHAR *floor[] =
	{
/*5*/	"FSTCW,WORD _FTEMP",		/* Get control word	*/
/*1*/	"FWAIT",			/* Wait for FSTCW	*/
/*5*/	"OR,BYTE _FTEMP[1],4",		/* Set round to down	*/
/*5*/	"FLDCW,WORD _FTEMP",		/* (Truncate to -infin)	*/
/*3*/	"FRNDINT",			/* Convert to integer	*/
/*5*/	"AND,BYTE _FTEMP[1],243",	/* Restore round to	*/
/*5*/	"FLDCW,WORD _FTEMP",		/* 	even.		*/
	NULL
	} ;
#define	FLOOR_COST	29 /*BYTES*/

PRIVATE CHAR *ceil[] =
	{
/*5*/	"FSTCW,WORD _FTEMP",		/* Get control word	*/
/*1*/	"FWAIT",			/* Wait for FSTCW	*/
/*5*/	"OR,BYTE _FTEMP[1],8",		/* Set round to up	*/
/*5*/	"FLDCW,WORD _FTEMP",		/* (Truncate to +infin)	*/
/*3*/	"FRNDINT",			/* Convert to integer	*/
/*5*/	"AND,BYTE _FTEMP[1],243",	/* Restore round to	*/
/*5*/	"FLDCW,WORD _FTEMP",		/* 	even.		*/
	NULL
	} ;
#define	CEIL_COST	29 /*BYTES*/

PRIVATE CHAR *fabs[] =
	{
/*3*/	"FABS",
	NULL
	} ;
#define	FABS_COST	3 /*BYTES*/

PRIVATE CHAR *sqrt[] =
	{
/*3*/	"FSQRT",
	NULL
	} ;
#define	SQRT_COST	3 /*BYTES*/

PRIVATE CHAR *log[] =
	{
/*3*/	"FLDLN2",
/*3*/	"FXCH",
/*3*/	"FYL2X",
	NULL
	} ;
#define	LOG_COST	9 /*BYTES*/

PRIVATE CHAR *log10[] =
	{
/*3*/	"FLDLG2",
/*3*/	"FXCH",
/*3*/	"FYL2X",
	NULL
	} ;
#define	LOG10_COST	9 /*BYTES*/

#ifdef	EXP		/* Only works if 0.0 <= arg <= 0.5! */
PRIVATE CHAR *exp[] =
	{
/*3*/	"FLDL2E",
/*3*/	"FMUL",
/*3*/	"F2XM1",
/*3*/	"FLD1",
/*3*/	"FADD",
	NULL
	} ;
#define	EXP_COST	15 /*BYTES*/

PRIVATE CHAR *exp10[] =
	{
/*3*/	"FLDL2T",
/*3*/	"FMUL",
/*3*/	"F2XM1",
/*3*/	"FLD1",
/*3*/	"FADD",
	NULL
	} ;
#define	EXP10_COST	15 /*BYTES*/
#endif

PRIVATE LIBRARY library[] =
	{
#ifdef	EXP
	{"exp_",	exp,		EXP_COST},
	{"exp10_",	exp10,		EXP10_COST},
#endif
	{"floor_",	floor,		FLOOR_COST},
	{"ceil_",	ceil,		CEIL_COST},
	{"fabs_",	fabs,		FABS_COST},
	{"sqrt_",	sqrt,		SQRT_COST},
	{"log_",	log,		LOG_COST},
	{"log10_",	log10,		LOG10_COST}
	} ;

PRIVATE LIBRARY *Find_Library(name)
CHAR *name ;
	{
#ifdef	PRODUCTION
	unsigned i ;

	for (i = 0; i < sizeof(library)/sizeof(LIBRARY); i++)
		{
		if (Equal(name, library[i].function)) return &library[i] ;
		}
#endif
	return NULL ;
	}

PRIVATE BOOLEAN NDP_Library(part1)
PARTS *part1 ;
	{
	PARTS *part2, *part3 ;
	LIBRARY *lib ;
	CHAR **str ;
#	define	CALLCOST1	9	/* CALL,CALL,opc */

#ifdef	PRODUCTION
	if (!Equal(part1->dst.orig, "_FPUSH")) return FALSE ;
	if ((part2 = Find_Next(part1)) == NULL) return FALSE ;
	if ((part3 = Find_Next(part2)) == NULL) return FALSE ;

	if (part2->opcode==CALL &&
	    (lib = Find_Library(part2->dst.orig)) != NULL)
		{
		if (!option.time.enabled && (lib->cost > CALLCOST1))
			{
			return FALSE ;
			}
		/*
			----------------------------
			CALL _FPUSH	=>	... 8087 code ...
			CALL ???_
			opc SP,src
			----------------------------
		*/
		part3->label = deleted ;
		part2->label = replaced ;
		part1->label = replaced ;
		stats.call_del += 2 ;
		stats.cseg_del += 3 ;
		for (str = lib->code ; *str != NULL; str++)
			{
			Emit(part3, null, *str, null, null) ;
			part3 = part3->next ;
			stats.cseg_ins++ ;
			}

		return TRUE ;
		}
#endif
	return FALSE ;
	}

VOID NDP_Replace(stmt)
PARTS *stmt ;
	{
#ifdef	PRODUCTION
	PARTS *jump ;
	CHAR *dst = stmt->dst.orig ;
	CHAR mne[80], *ptr, **str ;
	LIB *lib ;
	FNC *fnc ;
#	define	CALLCOST2	9	/* LEA,[MOV,MOV],CALL _F??? */

	/* Replace CALL's to 8087 library functions by in-line code */
	if (NDP_Library(stmt)) return ;
	if (dst[0]=='_' && dst[1]=='F' && ((lib = (LIB *) Find(dst)) != NULL))
		{
		fnc = option.big_model.enabled ? &lib->large : &lib->small ;
		if ((str = fnc->code) == NULL) return ;
		if (!option.time.enabled && (fnc->cost > CALLCOST2)) return ;
		if (Equal(dst, "_FCMP") || Equal(dst, "_FCMPKEEP"))
			{
			jump = Find_Next(stmt) ;
			Emit(jump, null, Reverse_Jump(jump->opcode),
				jump->dst.orig, null) ;
			jump->label = replaced ;
			}
		stmt->label = replaced ;
		stats.call_del++ ;
		stats.cseg_del++ ;
		while (*str!=NULL)
			{
			dst = null ;
			if ((ptr = index(strcpy(mne, *str++), ',')) != NULL)
				{
				*ptr = '\0' ;
				dst = ptr + 1 ;
				}
			Emit(stmt, null, mne, dst, null) ;
			stmt = stmt->next ;
			stats.cseg_ins++ ;	/* in-line code */

			/* account for assembler-generated FWAIT's */
			if (*mne == 'F')
				{
				stats.cseg_ins++ ;
				}
			}
		}
#endif
	}


VOID NDP_Init()
	{
#ifdef	PRODUCTION
	PARTS *last ;


	if (*stmt1->label!='\0' && opc1==PUSH && stmt2->opcode==MOV &&
	    dreg1==BP && stmt2->dst.reg==BP && stmt2->src.reg==SP)
	    	{
		if (option.init8087.enabled && Equal(stmt1->label, "main_:"))
			{
			last = stmt2 ;
			if (stmt3!=NULL && stmt3->opcode==SUB &&
			    stmt3->dst.reg==SP)
				{
				last = stmt3 ;
				}
			Emit(last, null, "FINIT", null, null) ;
			stats.cseg_ins++ ;
			}
		}
#endif
	}


BOOLEAN Fstp()
	{
#ifdef	PRODUCTION
	PARTS *middle, *stmt ;
	OPCODE *opc ;
	CHAR *src, *dst ;

	Fst() ;	/* Follow with FWAIT if needed */

	/* Now eliminate redundant pop/push sequence (if any) */
	if (stmt2!=NULL)
		{
		stmt = NULL ;
		dst = Skip_Prefix(dorig1) ;
		opc = stmt2->opcode ;
		if (opc==FLD)
			{
			middle = stmt = stmt2 ;
			}
		else if (opc==LEA && stmt3!=NULL && stmt3->opcode==FLD)
			{
			middle = stmt2 ;
			stmt = stmt3 ;
			}
		else if (opc==MOV && stmt2->dst.reg==SI &&
			 stmt2->src.reg==NULL &&
			 ADDRESS(stmt2->src.attb) && stmt3!=NULL &&
			 stmt3->opcode==FLD)
			{
			middle = stmt2 ;
			stmt = stmt3 ;
			}
		src = Skip_Prefix(stmt2->dst.orig) ;
		if (stmt!=NULL && Equal(src, dst))
			{
			/*
				---------------------
				(FWAIT)	      (FWAIT)
				FSTP mem      FST mem
				[LEA | MOV]
				(FWAIT)
				FLD mem   =>
				---------------------
			*/
			Emit(stmt1, null, "FST", dorig1, null) ;
			middle->label = deleted ;
			stmt->label = deleted ;
			stmt1->label = replaced ;
			stats.cseg_del += (middle == stmt) ? 2 : 3 ;
			return TRUE ;
			}
		}
#endif
	return FALSE ;
	}


BOOLEAN Fst()
	{
#ifdef	PRODUCTION
	PARTS *stmt, *after ;
	BOOLEAN needed ;
	OPCODE *opc ;
	CHAR *op ;

	
	needed = TRUE ;
	after = stmt1 ;
	op = Skip_Prefix(dorig1) ;
	for (stmt = stmt2 ; stmt!=NULL ; stmt = Get_Next(after = stmt))
		{
		if (*stmt->label == ';') continue ;
		if (*stmt->label != '\0') break ;
		if (*stmt->mnemonic=='F')
			{
			needed = FALSE ;
			break ;
			}
		if (islower(stmt->mnemonic[0])) break ;
		if ((opc = stmt->opcode) == NULL) break ;
		if (opc==CALL || opc==RET) break ;
		if (opc->name[0]=='J') break ;
		if (EqualN(opc->name, "MOVS", 4)) break ;
		if (opc==REP && EqualN(stmt->dst.orig, "MOVS", 4)) break ;
		if (MEMREF(stmt->src.attb))
			{
			if (VARSRC(dattb1 | stmt->src.attb) ||
			    Equal(op, Skip_Prefix(stmt->src.orig)))
				{
				break ;
				}
			}
		else if (MEMREF(stmt->dst.attb) && opc!=MOV)
			{
			if (VARSRC(dattb1 | stmt->dst.attb) ||
			    Equal(op, Skip_Prefix(stmt->dst.orig)))
				{
				break ;
				}
			}
		}

	if (needed)
		{
		Emit(after, null, "FWAIT", null, null) ;
		stats.cseg_ins++ ;
		}
#endif
	return FALSE ;
	}


VOID Zero_One(line)
CHAR *line ;
	{
#ifdef	PRODUCTION
	static CONST_INSTR c8087[] =
		{
		{" DW 0000H,0000H,0000H,0000H",		FLDZ},
		{" DW 0000H,0000H,0000H,3FF0H",		FLD1},
		{" DW 2D18H,5444H,21FBH,4009H",		FLDPI},
		{" DW 0A371H,0979H,0934FH,400AH",	FLDL2T},
		{" DW 082FEH,652BH,1547H,3FF7H",	FLDL2E},
		{" DW 79FFH,509FH,4413H,3FD3H",		FLDLG2},
		{" DW 39EFH,0FEFAH,2E42H,3FE6H",	FLDLN2}
		} ;
#	define NCONST	sizeof(c8087)/sizeof(CONST_INSTR)
	CHAR *data ;
	unsigned len, i ;
	FP_ZERO_ONE *entry ;

	if (option.numeric.enabled && line[0]=='_' && line[1]=='F' &&
	    isdigit(line[2]))
		{
		data = index(line, ' ') ;
		for (i = 0; i < NCONST; i++)
			{
			if (Equal(data, c8087[i].constant)) break ;
			}
		if (i == NCONST) return ;
		len = (unsigned) (data - line) ;
		entry = (FP_ZERO_ONE *) Allocate(sizeof(FP_ZERO_ONE) + len) ;
		entry->next = fp01 ;
		fp01 = entry ;
		_move(len, line, entry->label) ;
		entry->label[len] = '\0' ;
		entry->opc = c8087[i].instruction ;
		}
#endif
	}

BOOLEAN Fld()
	{
#ifdef	PRODUCTION
	PARTS *stmt9, *stmt6 ;
	FP_ZERO_ONE *entry, *prev ;

	prev = (FP_ZERO_ONE *) &fp01 ;
	for (entry = fp01; entry != NULL; entry = entry->next)
		{
		if (Equal(&dorig1[6], entry->label))
			{
			Emit(stmt1, null, entry->opc->name, null, null) ;
			stmt1->label = replaced ;
			stats.cseg_smp++ ;
			prev->next = entry->next ;
			My_Free((CHAR *) entry) ;
			return TRUE ;
			}
		prev = entry ;
		}

	if (stmt2!=NULL && (stmt2->opcode->attb & MEMOP87)!=0 &&
	    *stmt2->dst.orig=='\0')
		{
		/*
			---------------------
			(FWAIT)
			FLD mem
			(FWAIT)      (FWAIT)
			Fxxx     =>  Fxxx mem
			---------------------
		*/
		Emit(stmt2, null, stmt2->mnemonic, dorig1, null) ;
		stmt2->label = replaced ;
		stmt1->label = deleted ;
		stats.cseg_del += 2 ;
		return TRUE ;
		}

	if (stmt5!=NULL && (stmt2->opcode==FCOMPP || stmt2->opcode==FCOMP) &&
	    ((stmt9=Find_Next(Find_Next(Find_Next(Find_Next(stmt5)))))!=NULL)
	    && (stmt9->opcode->attb & CONDJMP)!=0)
		{
		/*
			----------------------
			(FWAIT)
			FLD mem            =>  
			(FWAIT)		       (FWAIT)
			FCOMP[P]               FCOM[P] mem
			FSTSW WORD _FTEMP      FSTSW WORD _FTEMP
			PUSH AX                PUSH AX
			FWAIT                  FWAIT
			MOV AH,BYTE _FTEMP+1   MOV AH,BYTE _FTEMP+1
			SAHF                   SAHF
			POP AX                 POP AX
			Jcond		       Jcond (reversed)
			----------------------
		*/
		Emit(stmt2, null, stmt2->opcode==FCOMPP ?
				"FCOMP" : "FCOM", dorig1, null);
		Emit(stmt9, null, Reverse_Jump(stmt9->opcode),
			stmt9->dst.orig, null) ;
		stmt9->label = replaced ;
		stmt2->label = replaced ;
		stmt1->label = deleted ;
		stats.cseg_del += 2 ;
		return TRUE ;
		}

	if (dorig1[0]=='Q' && (stmt6 = Fpush()) != NULL)
	    	{
		/*
			------------------------------------------------
			FLD QWORD xyz		=>	FWAIT
			SUB SP,8			PUSH WORD xyz[6]
			PUSH BP				PUSH WORD xyz[4]
			MOV BP,SP			PUSH WORD xyz[2]
			FSTP QWORD [BP+2]		PUSH WORD xyz
			POP BP
			------------------------------------------------
		*/
		Emit(stmt6, null, push, &dorig1[1], null) ;
		Emit(stmt6, null, push,
			Insert_String(&dorig1[1], dlen1-1, 2, NULL), null) ;
		Emit(stmt6, null, push,
			Insert_String(&dorig1[1], dlen1-1, 4, NULL), null) ;
		Emit(stmt6, null, push,
			Insert_String(&dorig1[1], dlen1-1, 6, NULL), null) ;
		Emit(stmt6, null, "FWAIT", null, null) ;
		stmt6->label = replaced ;
		stmt5->label = replaced ;
		stmt4->label = replaced ;
		stmt3->label = replaced ;
		stmt2->label = replaced ;
		stmt1->label = replaced ;
		stats.cseg_del += 2 ;
		return TRUE ;
		}

#endif
	return FALSE ;
	}


BOOLEAN Fild()
	{
#ifdef	PRODUCTION
	static CHAR mne[] = "FIxxxxxxxx" ;
	PARTS *stmt9 ;

	if (stmt2!=NULL && (stmt2->opcode->attb & MEMOP87)!=0 &&
	    *stmt2->dst.orig=='\0')
		{
		/*
			---------------------
			(FWAIT)
			FILD mem
			(FWAIT)      (FWAIT)
			Fxxx     =>  FIxxx mem
			---------------------
		*/
		strcpy(&mne[2], &stmt2->mnemonic[1]) ;
		Emit(stmt2, null, mne, dorig1, null) ;
		stmt2->label = replaced ;
		stmt1->label = deleted ;
		stats.cseg_del += 2 ;
		return TRUE ;
		}

	if (stmt5!=NULL && (stmt2->opcode==FCOMPP || stmt2->opcode==FCOMP) &&
	    ((stmt9=Find_Next(Find_Next(Find_Next(Find_Next(stmt5)))))!=NULL)
	    && (stmt9->opcode->attb & CONDJMP)!=0)
		{
		/*
			----------------------
			(FWAIT)	               
			FILD mem            =>  
			(FWAIT)                (FWAIT)
			FCOMP[P]               FICOM[P] mem
			FSTSW WORD _FTEMP      FSTSW WORD _FTEMP
			PUSH AX                PUSH AX
			FWAIT                  FWAIT
			MOV AH,BYTE _FTEMP+1   MOV AH,BYTE _FTEMP+1
			SAHF                   SAHF
			POP AX                 POP AX
			Jcond		       Jcond (Reversed)
			----------------------
		*/
		Emit(stmt2, null, stmt2->opcode==FCOMPP ?
				"FICOMP" : "FICOM", dorig1, null) ;
		Emit(stmt9, null, Reverse_Jump(stmt9->opcode),
			stmt9->dst.orig, null) ;
		stmt9->label = replaced ;
		stmt2->label = replaced ;
		stmt1->label = deleted ;
		stats.cseg_del += 2 ;
		return TRUE ;
		}
#endif
	return FALSE ;
	}


PRIVATE CHAR *Reverse_Jump(opcode)
OPCODE *opcode ;
	{
#ifdef	PRODUCTION
	static OPCODE *old_jumps[] = {JA, JAE, JB, JBE, JZ, JNZ} ;
	static OPCODE *new_jumps[] = {JB, JBE, JA, JAE, JZ, JNZ} ;
	int select ;

	for (select = 0; select < 6; select++)
		{
		if (opcode == old_jumps[select]) break ;
		}
#endif
	return new_jumps[select]->name ;
	}

BOOLEAN Fstsw()
	{
#ifdef	PRODUCTION
	PARTS *stmt6 ;

	if (stmt5!=NULL && EMPTY(AH) && stmt2->opcode==PUSH &&
	    stmt2->dst.reg==AX && ((stmt6 = Find_Next(stmt5)) != NULL) &&
	    stmt6->opcode==POP && stmt6->dst.reg==AX)
		{
		/*
			----------------------------------------
			(FWAIT)		       (FWAIT)
			FSTSW WORD _FTEMP      FSTSW WORD _FTEMP
			PUSH AX           =>   FWAIT
			FWAIT                  MOV AH,BYTE _FTEMP+1
			MOV AH,BYTE _FTEMP+1   SAHF
			SAHF                   
			POP AX
			----------------------------------------
		*/
		stmt6->label = deleted ;
		stmt2->label = deleted ;
		stats.cseg_del += 2 ;
		return TRUE ;
		}
#endif
	return FALSE ;
	}


BOOLEAN Sahf()
	{
#ifdef	PRODUCTION
	/*
	-----------------------------------------------
	The opcode SAHF is only used by _fcmp(), and AX
	is not needed nor of use afterwards - empty it!
	-----------------------------------------------
	*/
	LOAD_ATTB(AX, _EMPTY) ;
	LOAD_ATTB(AH, _EMPTY) ;
	LOAD_ATTB(AL, _EMPTY) ;
#endif
	return FALSE ;
	}

BOOLEAN Xor_87()
	{
#ifdef	PRODUCTION
	if (!option.numeric.enabled) return FALSE ;

	if (stmt5!=NULL && dreg1==AX && sreg1==AX &&
	    stmt2->opcode==CWD && stmt3->opcode==MOV &&
	    stmt3->src.reg==AX && stmt4->opcode==MOV &&
	    stmt4->src.reg==DX && stmt5->opcode==FILD &&
	    Equal(stmt3->dst.orig, "WORD _FTEMP") &&
	    Equal(stmt4->dst.orig, "WORD _FTEMP[2]") &&
	    Equal(stmt5->dst.orig, "DWORD _FTEMP"))
		{
		/*
			------------------------------
			XOR AX,AX
			CWD
			MOV WORD _FTEMP,AX
			MOV WORD _FTEMP[2],DX
			(FWAIT)                (FWAIT)
			FILD DWORD _FTEMP  =>  FLDZ
			------------------------------
		*/
		Emit(stmt5, null, "FLDZ", null, null) ;
		stmt5->label = combined ;
		stmt4->label = combined ;
		stmt3->label = combined ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.cseg_smp++ ;
		stats.cseg_del += 4 ;
		return TRUE ;
		}
#endif
	return FALSE ;
	}


BOOLEAN Cwd_87()
	{
#ifdef	PRODUCTION
	if (!option.numeric.enabled) return FALSE ;

	if (stmt4!=NULL && stmt2->opcode==MOV && stmt3->opcode==MOV &&
	    stmt4->opcode==FILD && stmt2->src.reg==AX && stmt3->src.reg==DX &&
	    Equal(stmt2->dst.orig, "WORD _FTEMP") &&
	    Equal(stmt3->dst.orig, "WORD _FTEMP[2]") &&
	    Equal(stmt4->dst.orig, "DWORD _FTEMP"))
	    	{
		/*
			-------------------------------------
			CWD                      MOV WORD _FTEMP,AX
			MOV WORD _FTEMP,AX   =>  (FWAIT)
			MOV WORD _FTEMP[2],DX     FILD WORD _FTEMP
			(FWAIT)
			FILD DWORD _FTEMP	      
			---------------------------------
		*/
		Emit(stmt4, null, "FILD", "WORD _FTEMP", null) ;
		Emit(stmt4, null, mov, "WORD _FTEMP", ax) ;
		stmt4->label = combined ;
		stmt3->label = combined ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.cseg_del += 2 ;
		return TRUE ;
		}
#endif
	return FALSE ;
	}


BOOLEAN Mov_87()
	{
#ifdef	PRODUCTION
	static CHAR qword[80] = "ES:QWORD " ;
	static CHAR dword[80] = "ES:DWORD " ;
	static OPCODE *implied[] =
		{CALL, REP, RET, MOVSW, MOVSB, STOSW, STOSB} ;
	PARTS *stmt6, *stmt7, *stmt ;
	BOOLEAN mdfd ;
	CHAR **str, *src, *dst, temp[100] ;
	unsigned i ;
	LIBRARY *lib ;

	if (sreg1!=NULL && SEGREG(sreg1) && stmt2!=NULL &&
	    stmt2->dst.reg==ES && stmt2->src.reg==dreg1)
		{
		/*
		------------------------------------
			MOV r,sreg  =>  ...
			MOV ES,r	...

		(and replace/eliminate prefix below)
		------------------------------------
		*/
		mdfd = FALSE ;
		stmt = stmt2 ;
		while ((stmt = Reg_Needed(ES, stmt)) != NULL)
			{
			for (i = 0; i < 7; i++)
				{
				if (stmt->opcode==implied[i]) break ;
				}
			if (i!=7) break ;
			if (stmt->mnemonic[0]=='J') break ;
			if ((stmt->dst.attb & _DEPES) != 0)
				{
				src = stmt->src.orig ;
				_move(stmt->dst.len+1, stmt->dst.orig, temp) ;
				if (sreg1 == DS) dst = &temp[3] ;
				else
					{
					temp[0] = sorig1[0] ;
					temp[1] = sorig1[1] ;
					dst = temp ;
					}
				}
			else if ((stmt->src.attb & _DEPES) != 0)
				{
				_move(stmt->src.len+1, stmt->src.orig, temp) ;
				if (sreg1 == DS) src = &temp[3] ;
				else
					{
					temp[0] = sorig1[0] ;
					temp[1] = sorig1[1] ;
					src = temp ;
					}
				dst = stmt->dst.orig ;
				}
			else continue ;
			Emit(stmt, null, stmt->mnemonic, dst, src) ;
			stmt->label = simplified ;
			stats.cseg_smp++ ;
			mdfd = TRUE ;
			}

		if (mdfd)
			{
			stmt1->label = deleted ;
			stmt2->label = deleted ;
			stats.cseg_del += 2 ;
			return TRUE ;
			}
		}

	if (!option.numeric.enabled || sreg1!=NULL)
		{
		return FALSE ;
		}

	if (stmt3!=NULL && dreg1==AX && MEMREF(sattb1) && 
	    stmt2->opcode==MOV && stmt2->src.reg==AX &&
	    stmt3->opcode==FILD && Equal(stmt2->dst.orig, stmt3->dst.orig))
		{
		/*
			----------------------------------
			MOV AX,WORD mem     =>  (FWAIT)
			MOV WORD _FTEMP,AX      FILD WORD mem
			(FWAIT)
			FILD WORD _FTEMP
			----------------------------------
		*/
		Emit(stmt3, null, "FILD", sorig1, null) ;
		stmt3->label = combined ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.cseg_del += 2 ;
		return TRUE ;
		}

	if (stmt5!=NULL && stmt2->opcode==MOV && stmt3->opcode==MOV && 
	    stmt4->opcode==MOV && stmt5->opcode==FILD &&
	    dreg1==DX && stmt2->dst.reg==AX &&
	    stmt3->src.reg==AX && stmt4->src.reg==DX &&
	    MEMREF(sattb1 & stmt2->src.attb) &&
	    Equal(stmt3->dst.orig, "WORD _FTEMP") &&
	    Equal(stmt4->dst.orig, "WORD _FTEMP[2]") &&
	    Equal(stmt5->dst.orig, "DWORD _FTEMP"))
		{
		/*
			---------------------------------------
			MOV DX,WORD xyz
			MOV AX,WORD xyz-2
			MOV WORD _FTEMP,AX
			MOV WORD _FTEMP[2],DX
			(FWAIT)                   (FWAIT)
			FILD DWORD _FTEMP     =>  FILD DWORD xyz-2
			---------------------------------------
		*/
		strcpy(&dword[9], Skip_Prefix(stmt2->src.orig)) ;
		Emit(stmt5, null, "FILD",
			(sattb1 & _DEPES) ? &dword[0] : &dword[3], null) ;
		stmt5->label = combined ;
		stmt4->label = combined ;
		stmt3->label = combined ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.cseg_del += 4 ;
		return TRUE ;
		}

	if (stmt5!=NULL && stmt2->opcode==CWD && stmt3->opcode==MOV && 
	    stmt4->opcode==MOV && stmt5->opcode==FILD &&
	    dreg1==AX &&
	    stmt3->src.reg==AX && stmt4->src.reg==DX &&
	    MEMREF(sattb1) &&
	    Equal(stmt3->dst.orig, "WORD _FTEMP") &&
	    Equal(stmt4->dst.orig, "WORD _FTEMP[2]") &&
	    Equal(stmt5->dst.orig, "DWORD _FTEMP"))
		{
		/*
			---------------------------------------
			MOV AX,WORD xyz
			CWD
			MOV WORD _FTEMP,AX
			MOV WORD _FTEMP[2],DX
			(FWAIT)                   (FWAIT)
			FILD DWORD _FTEMP     =>  FILD WORD xyz
			---------------------------------------
		*/
		Emit(stmt5, null, "FILD", sorig1, null) ;
		stmt5->label = combined ;
		stmt4->label = combined ;
		stmt3->label = combined ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.cseg_del += 4 ;
		return TRUE ;
		}

	if (stmt5!=NULL && dreg1==SI && ADDRESS(sattb1) &&
	    stmt2->opcode==PUSH && stmt3->opcode==PUSH &&
	    stmt4->opcode==PUSH && stmt5->opcode==PUSH &&
	    ((stmt7 = Find_Next(stmt6 = Find_Next(stmt5))) != NULL) &&
	    stmt6->opcode==CALL &&
	    (lib = Find_Library(stmt6->dst.orig)) != NULL)
	    	{
		/*
			------------------------------------
			MOV SI,OFFSET mem
			PUSH WORD [SI+6]
			PUSH WORD [SI+4]       (FWAIT)
			PUSH WORD [SI+2]       FLD QWORD mem
			PUSH WORD [SI]     =>  (FWAIT)
			CALL ???_              ...8087 code ...
			ADD SP,8 (or MOV SP,BP)
			------------------------------------
		*/
		stmt7->label = deleted ;
		stmt6->label = replaced ;
		for (str = lib->code ; *str != NULL; str++)
			{
			Emit(stmt7, null, *str, null, null) ;
			stmt7 = stmt7->next ;
			stats.cseg_ins++ ;
			}
		strcpy(&qword[9], Skip_Prefix(sorig1)) ;
		Emit(stmt5, null, "FLD",
			(stmt2->dst.attb & _DEPES) ? &qword[0] : &qword[3],
			null) ;
		stmt5->label = combined ;
		stmt4->label = combined ;
		stmt3->label = combined ;
		stmt2->label = combined ;
		stmt1->label = combined ;
		stats.call_del++ ;
		stats.cseg_del += 4 ;
		return TRUE ;
		}

	if (dreg1==SI && ADDRESS(sattb1))
		{
		return SI_Ref(sorig1) ;
		}
#endif
	return FALSE ;
	}


BOOLEAN Lea_87()
	{
#ifdef	PRODUCTION
	CHAR op[100] ;

	if (stmt2!=NULL && stmt2->opcode==MOV &&
	    stmt2->dst.reg==ES && stmt2->src.orig[0]=='@' &&
	    (sattb1 & DEPEND_FLAG(dreg1))==0)
	    	{
		/*
		------------------------------------------
		LEA r,WORD src[?]    =>  MOV r,SEG src
		MOV ES,@DWORD src+2      MOV ES,r
				         LEA r,WORD src[?]
		------------------------------------------
		*/
		Imm_Op(stmt2->src.orig, op) ;
		Emit(stmt2, null, lea, dorig1, sorig1) ;
		Emit(stmt2, null, mov, "ES", dorig1) ;
		Emit(stmt2, null, mov, dorig1, op) ;
		stmt1->label = replaced ;
		stmt2->label = replaced ;
		stats.cseg_ins++ ;
		return TRUE ;
		}

	if (option.numeric.enabled && dreg1==SI)
		{
		return SI_Ref(sorig1) ;
		}
#endif
	return FALSE ;
	}


PRIVATE BOOLEAN SI_Ref(dst)
CHAR *dst ;
	{
	BOOLEAN fixed = FALSE ;
#ifdef	PRODUCTION
	unsigned len ;
	PARTS *stmt ;
	CHAR bfr[80], *p ;

	stmt = stmt1 ;
	dst = Skip_Prefix(dst) ;
	while ((stmt = Get_Next(stmt)) != NULL)
		{
		if (stmt->opcode==LINE) break ;
		if (*stmt->label!='\0') continue ;
		if (REDEFINED(SI, stmt)) break ;
		if ((stmt->opcode->attb & MDFYDST)!=0 && stmt->dst.reg==SI)
			{
			return FALSE ;
			}
		if (stmt->mnemonic[0]=='F' &&
		    (stmt->dst.attb & _DEPSRC) == _DEPSI)
		    	{
			p = Skip_Prefix(stmt->dst.orig) ;
			len = p - stmt->dst.orig ;
			_move(len, stmt->dst.orig, bfr) ;
			strcpy(&bfr[len], dst) ;
			if (p[3] != ']') /* e.g., [SI+8] */
				{
				p = Insert_String(bfr, strlen(bfr),
					Value(p + 4, stmt->dst.len - len - 5),
					NULL) ;
				}
			else p = bfr ;
			Emit(stmt, null, stmt->mnemonic, p, stmt->src.orig);
			stmt->label = replaced ;
			if (!fixed)
				{
				stmt1->label = deleted ;
				stats.cseg_del++ ;
				fixed = TRUE ;
				}
			stmt = stmt->next ;
			}
		}
#endif
	return fixed ;
	}

BOOLEAN Fistp()
	{
	PARTS *stmt6, *stmt7 ;
	CHAR bfr[100] ;
	CHAR *newdst ;

	if (stmt2->opcode==AND && stmt3->opcode==FLDCW &&
	    Equal(dorig1, "DWORD _FTEMP") &&
	    (stmt6 = Find_Next(stmt5)) != NULL &&
	    stmt6->opcode==MOV && stmt6->src.reg==AX &&
	    stmt6->dst.reg==NULL && MEMREF(stmt6->dst.attb))
		{
		if ((stmt7 = Find_Next(stmt6)) == NULL) return FALSE ;
		if (stmt7->opcode==MOV && stmt7->src.reg==DX &&
		    stmt7->dst.reg==NULL && MEMREF(stmt7->dst.attb))
			{
			bfr[0] = 'D' ;
			_move(stmt6->dst.len + 1, stmt6->dst.orig, &bfr[1]) ;
			stmt7->label = deleted ;
			stats.cseg_del++ ;
			newdst = bfr ;
			}
		else if (!REFERENCED(DX, stmt7))
			{
			newdst = stmt6->dst.orig ;
			}
		else
			{
			return FALSE ;
			}
		Emit(stmt1, null, "FISTP", newdst, null) ;
		stmt1->label = replaced ;
		stmt4->label = deleted ;
		stmt5->label = deleted ;
		stmt6->label = deleted ;
		stats.cseg_del += 3 ;
		return TRUE ;
		}
	return FALSE ;
	}

PRIVATE PARTS *Fpush()
	{
	PARTS *stmt6 ;

	return (stmt2->opcode==SUB &&
		stmt3->opcode==PUSH &&
		stmt4->opcode==MOV &&
		stmt5->opcode==FSTP &&
		(stmt6 = Find_Next(stmt5)) != NULL &&
		stmt6->opcode==POP) ?
		stmt6 : NULL ;
	}

			  /*  End of file 8087.C  */
