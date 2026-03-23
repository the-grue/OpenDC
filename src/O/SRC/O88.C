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

			    (All rights erved)


*/

#include "inc\o88.h"

/* Variables PRIVATE to O88.C...					*/
/* -------------------------------------------------------------------- */
PRIVATE CHAR fpop[]	= "FSTP,ST(0)" ;
PRIVATE CHAR movah[]	= "MOV,AH,BYTE _FTEMP[1]" ;
PRIVATE CHAR pushax[]	= "PUSH,AX" ;
PRIVATE CHAR popax[]	= "POP,AX" ;
PRIVATE CHAR popcx[]	= "POP,CX" ;
PRIVATE CHAR popdx[]	= "POP,DX" ;
PRIVATE CHAR popsi[]	= "POP,SI" ;
PRIVATE CHAR popdi[]	= "POP,DI" ;
PRIVATE CHAR popes[]	= "POP,ES" ;
PRIVATE CHAR rolax[]	= "ROL,AX,1" ;
PRIVATE CHAR xorah[]	= "XOR,AH,AH" ;
PRIVATE CHAR movaxds[]	= "MOV,AX,DS" ;

PRIVATE CHAR *floadd[] =
	{
/*3*/	"FLD,QWORD [SI]",
	NULL
	} ;

PRIVATE CHAR *esfloadd[] =
	{
/*4*/	"FLD,ES:QWORD [SI]",
	NULL
	} ;

PRIVATE CHAR *floade[] =
	{
/*3*/	"FLD,DWORD [SI]",
	NULL
	} ;

PRIVATE CHAR *esfloade[] =
	{
/*4*/	"FLD,ES:DWORD [SI]",
	NULL
	} ;

PRIVATE CHAR *floadl[] =
	{
/*3*/	"MOV,WORD _FTEMP,AX",
/*4*/	"MOV,WORD _FTEMP[2],DX",
/*5*/	"FILD,DWORD _FTEMP",
	NULL
	} ;

PRIVATE CHAR *fadd[] =
	{
/*3*/	"FADD",
	NULL
	} ;

PRIVATE CHAR *fdec[] =
	{
/*3*/	"FLD1",
/*3*/	"FSUB",
	NULL
	} ;

PRIVATE CHAR *fdiv[] =
	{
/*3*/	"FDIV",
	NULL
	} ;

PRIVATE CHAR *finc[] =
	{
/*3*/	"FLD1",
/*3*/	"FADD",
	NULL
	} ;

PRIVATE CHAR *fmul[] =
	{
/*3*/	"FMUL",
	NULL
	} ;

PRIVATE CHAR *fneg[] =
	{
/*3*/	"FCHS",
	NULL
	} ;

PRIVATE CHAR *fstored[] =
	{
/*3*/	"FSTP,QWORD [SI]",
	NULL
	} ;

PRIVATE CHAR *esfstoree[] =
	{
/*4*/	"FSTP,ES:DWORD [SI]",
	NULL
	} ;

PRIVATE CHAR *esfstored[] =
	{
/*4*/	"FSTP,ES:QWORD [SI]",
	NULL
	} ;

PRIVATE CHAR *fstoree[] =
	{
/*3*/	"FSTP,DWORD [SI]",
	NULL
	} ;

PRIVATE CHAR *fstorel[] =
	{
/*5*/	"FSTCW,WORD _FTEMP[4]",		/* Get control word	*/
/*1*/	"FWAIT",			/* Wait for FSTCW	*/
/*5*/	"OR,BYTE _FTEMP[5],12",		/* Set round to chop	*/
/*5*/	"FLDCW,WORD _FTEMP[4]",		/*     (truncate)	*/
/*5*/	"FISTP,DWORD _FTEMP",		/* Store the long	*/
/*5*/	"AND,BYTE _FTEMP[5],243",	/* Restore round to	*/
/*5*/	"FLDCW,WORD _FTEMP[4]",		/*	even.		*/
/*3*/	"MOV,AX,WORD _FTEMP",		/* Load the long into	*/
/*4*/	"MOV,DX,WORD _FTEMP[2]",	/* 	AX and DX	*/
	NULL
	} ;

PRIVATE CHAR *fsub[] =
	{
/*3*/	"FSUB",
	NULL
	} ;

PRIVATE CHAR *fxch[] =
	{
/*3*/	"FXCH,ST(1)",
	NULL
	} ;

PRIVATE CHAR *fclear[] =
	{
/*3*/	fpop,
	NULL
	} ;

PRIVATE CHAR *fcmp[] =
	{
/*3*/	"FCOMPP",
/*5*/	"FSTSW,WORD _FTEMP",
/*1*/	pushax,
/*1*/	"FWAIT",
/*4*/	movah,
/*1*/	"SAHF",
/*1*/	popax,
	NULL
	} ;

PRIVATE CHAR *fcmpkeep[] =
	{
/*3*/	"FCOMP",
/*5*/	"FSTSW,WORD _FTEMP",
/*1*/	pushax,
/*1*/	"FWAIT",
/*4*/	movah,
/*1*/	"SAHF",
/*1*/	popax,
	NULL
	} ;

PRIVATE CHAR *fis[] = /* Return NZ if TOS != 0; POP */
	{
/*3*/	"FTST",				/* Set 8087 flags	*/
/*5*/	"FSTSW,WORD _FTEMP",		/* Put them in memory	*/
/*1*/	pushax,				/* Preserve AX		*/
/*3*/	fpop,				/* Pop the stack	*/
/*4*/	movah,				/* Get the flags	*/
/*1*/	"SAHF",				/* Transfer to 8088	*/
/*1*/	popax,				/* Restore AX		*/
	NULL
	} ;

PRIVATE CHAR *fnot[] = /* Return 1 if TOS == 0; POP */
	{
/*3*/	"FTST",				/* Set 8087 flags	*/
/*5*/	"FSTSW,WORD _FTEMP",		/* Put them in memory	*/
/*3*/	fpop,				/* Pop the stack	*/
/*4*/	movah,				/* Get the flags	*/
/*2*/	rolax,				/* Move the ZF flag	*/
/*2*/	rolax,				/*  into LSB of AX	*/
/*3*/	"AND,AX,1",			/* Return 0 or 1	*/
	NULL
	} ;

PRIVATE CHAR *fpush[] = /* Transfer 8087 TOS to 8088 stack */
	{
/*4*/	"SUB,SP,8",
/*1*/	"PUSH,BP",
/*2*/	"MOV,BP,SP",
/*5*/	"FSTP,QWORD [BP+2]",
/*1*/	"POP,BP",
	NULL
	} ;

PRIVATE CHAR *move[] =
	{
/*1*/	popcx,
/*1*/	popsi,
/*1*/	popdi,
/*2*/	movaxds,
/*2*/	"MOV,ES,AX",
/*1*/	"CLD",
/*2*/	"REP,MOVSB",
	NULL
	} ;

PRIVATE CHAR *lmove[] =
	{
/*1*/	popcx,
/*1*/	popsi,
/*1*/	popdx,		/* DX=>DS */
/*1*/	popdi,
/*1*/	popes,
/*1*/	"CLD",
/*2*/	movaxds,	/* Preserve DS */
/*2*/	"MOV,DS,DX",
/*2*/	"REP,MOVSB",
/*2*/	"MOV,DS,AX",	/* Restore DS */
	NULL
	} ;

PRIVATE CHAR *setmem[] =
	{
/*1*/	popdi,
/*1*/	popcx,
/*2*/	movaxds,
/*2*/	"MOV,ES,AX",
/*1*/	popax,
/*1*/	"CLD"
/*2*/	"REP,STOSB",
	NULL
	} ;

PRIVATE CHAR *showds[] =
	{
/*2*/	movaxds,
	NULL
	} ;

PRIVATE CHAR *showcs[] =
	{
/*2*/	"MOV,AX,CS",
	NULL
	} ;

PRIVATE CHAR *showsp[] =
	{
/*2*/	"MOV,AX,SP",
	NULL
	} ;

PRIVATE CHAR *inb[] =
	{
/*1*/	popdx,
/*1*/	"IN,AL,DX",
/*2*/	xorah,
	NULL
	} ;

PRIVATE CHAR *inw[] =
	{
/*1*/	popdx,
/*2*/	"IN,AX,DX",
	NULL
	} ;

PRIVATE CHAR *outb[] =
	{
/*1*/	popax,
/*1*/	popdx,
/*1*/	"OUT,DX,AL",
	NULL
	} ;

PRIVATE CHAR *outw[] =
	{
/*1*/	popax,
/*1*/	popdx,
/*1*/	"OUT,DX,AX",
	NULL
	} ;

PRIVATE CHAR *peek[] =
	{
/*1*/	popdi,
/*1*/	popes,
/*3*/	"MOV,AL,ES:BYTE [DI]",
/*2*/	xorah,
	NULL
	} ;

PRIVATE CHAR *poke[] =
	{
/*1*/	popax,
/*1*/	popdi,
/*1*/	popes,
/*3*/	"MOV,ES:BYTE [DI],AL",
	NULL
	} ;

PRIVATE CHAR *lset[] =
	{
/*1*/	popdi,
/*1*/	popes,
/*1*/	popax,
/*1*/	popcx,
/*1*/	"CLD",
/*2*/	"REP,STOSB",
	NULL
	} ;

PRIVATE CHAR *peekw[] =
	{
/*1*/	popdi,
/*1*/	popes,
/*3*/	"MOV,AX,ES:WORD [DI]",
	NULL
	} ;

PRIVATE CHAR *peekd[] =
	{
/*1*/	popdi,
/*1*/	popes,
/*3*/	"MOV,AX,ES:WORD [DI]",
/*5*/	"MOV,DX,ES:WORD [DI+2]",
	NULL
	} ;

PRIVATE CHAR *pokew[] =
	{
/*1*/	popax,
/*1*/	popdi,
/*1*/	popes,
/*3*/	"MOV,ES:WORD [DI],AX",
	NULL
	} ;

PRIVATE CHAR *poked[] =
	{
/*1*/	popax,
/*1*/	popdx,
/*1*/	popdi,
/*1*/	popes,
/*3*/	"MOV,ES:WORD [DI],AX",
/*5*/	"MOV,ES:WORD [DI+2],DX",
	NULL
	} ;

PRIVATE CHAR *rotl[] =
	{
/*1*/	popax,
/*1*/	popcx,
/*2*/	"ROL,AX,CL",
	NULL
	} ;

PRIVATE CHAR *rotr[] =
	{
/*1*/	popax,
/*1*/	popcx,
/*2*/	"ROR,AX,CL",
	NULL
	} ;

/* NOTE: All "LIB" tables are ordered by frequency of reference so that	*/
/*	 they are inserted into the hash table in such a manner that	*/
/*	 search time (after collision) is minimal.			*/

PRIVATE LIB mov88[] =	/* enabled by +M option */
	{
	{"_MOVE_",	{move,	   6,	10},	{lmove,	   10,	14}},
	{"_move_",	{move,	   6,	10},	{lmove,	   10,	14}},
	{"_lmove_",	{lmove,	  10,	14},	{NULL,	    0,	0}}
	} ;

#define MOVS88	(sizeof(mov88)/sizeof(LIB))

PRIVATE LIB lib88[] =
	{
	{"_setmem_",	{setmem,   6,	10},	{NULL,	    0,	10}},
	{"_showds_",	{showds,   0,	2},	{showds,    0,	2}},
	{"_showcs_",	{showcs,   0,	2},	{showcs,    0,	2}},
	{"_showsp_",	{showsp,   0,	2},	{NULL,	    0,	0}},
	{"_inb_",	{inb,	   2,	4},	{inb,	    2,	4}},
	{"_inw_",	{inw,	   2,	3},	{inw,	    2,	3}},
	{"_outb_",	{outb,	   4,	3},	{outb,	    4,	3}},
	{"_outw_",	{outw,	   4,	3},	{outw,	    4,	3}},
	{"_peek_",	{peek,	   4,	7},	{peek,	    4,	7}},
	{"_poke_",	{poke,	   6,	6},	{poke,	    6,	6}},
	{"_lset_",	{lset,	   8,   7},	{lset,      8,  7}},
	{"_peekw_",	{peekw,	   4,   5},	{peekw,	    4,  5}},
	{"_peekd_",	{peekd,	   4,   10},	{peekd,	    4,  10}},
	{"_pokew_",	{pokew,	   6,   6},	{pokew,	    6,  6}},
	{"_poked_",	{poked,	   8,   12},	{poked,	    8,  12}},
	{"_rotl_",	{rotl,	   4,   4},	{rotl,	    4,	4}},
	{"_rotr_",	{rotr,	   4,   4},	{rotr,	    4,	4}}
	} ;

#define	LIBS88	(sizeof(lib88)/sizeof(LIB))

PRIVATE LIB lib87[] =
	{
	{"_FADD",	{fadd,	   0,	3},	{fadd,	    0,	3}},
	{"_FDEC",	{fdec,	   0,	6},	{fdec,	    0,	6}},
	{"_FDIV",	{fdiv,	   0,	3},	{fdiv,	    0,	3}},
	{"_FINC",	{finc,	   0,	3},	{finc,	    0,	3}},
	{"_FLOADD",	{floadd,   0,	3},	{esfloadd,  0,	4}},
	{"_FLOADE",	{floade,   0,	3},	{esfloade,  0,	4}},
	{"_FLOADL",	{floadl,   0,	12},	{floadl,    0,	12}},
	{"_FMUL",	{fmul,	   0,	3},	{fmul,	    0,	3}},
	{"_FNEG",	{fneg,	   0,	3},	{fneg,	    0,	3}},
	{"_FSTORED",	{fstored,  0,	3},	{esfstored, 0,	4}},
	{"_FSTOREE",	{fstoree,  0,	3},	{esfstoree, 0,	4}},
	{"_FSTOREL",	{fstorel,  0,	38},	{fstorel,   0,	38}},
	{"_FSUB",	{fsub,	   0,	3},	{fsub,	    0,	3}},
	{"_FXCH",	{fxch,	   0,	3},	{fxch,	    0,	3}},
	{"_FCLEAR",	{fclear,   0,	3},	{fclear,    0,	3}},
	{"_FCMP",	{fcmp,	   0,	16},	{fcmp,	    0,	16}},
	{"_FCMPKEEP",	{fcmpkeep, 0,	16},	{fcmpkeep,  0,	16}},
	{"_FIS",	{fis,	   0,	18},	{fis,	    0,	18}},
	{"_FNOT",	{fnot,	   0,	22},	{fnot,	    0,	22}},
	{"_FPUSH",	{fpush,	   0,	13},	{fpush,	    0,	13}}
	} ;

#define	LIBS87	(sizeof(lib87)/sizeof(LIB))

/* Functions PRIVATE to O88.C						*/
/* -------------------------------------------------------------------- */

#ifdef	_lint

unsigned	Per_Cent(int, int) ;
VOID		Make_Entry(CHAR **) ;
unsigned	Show_Options(VOID) ;

#else

unsigned	Per_Cent() ;
VOID		Make_Entry() ;
unsigned	Show_Options() ;

#endif

VOID main(argc, argv)
unsigned argc ;
CHAR *argv[] ;
	{
	REG *r ;
	PARTS *next ;
	unsigned i, dos_kbytes ;

	Init_Heap(3000) ;
	Get_Args(argc, argv) ;

	/* Find size and base of DOS memory available for buffers */
	DOS_Memory() ;

	/* -------------------------------------------- */
	/* Reserve 10k bytes:				*/
	/*						*/
	/*	2k		COMMAND.COM loader	*/
	/*	1k (min)	inp_bfr			*/
	/*	1k		data_bfr		*/
	/*	1k		cs_pub_bfr		*/
	/*	1k (min)	code_bfr		*/
	/*	1k		temp_bfr		*/
	/*     ---					*/
	/* ttl: 7k					*/
	/* -------------------------------------------- */
	dos_kbytes = dos_size / 64 ;
	dos_kbytes -= 2 /* for loader */ ;
	if (dos_kbytes < 7)
		{
		Errs("\r\nInsufficient DOS memory!\r\n\7") ;
		exit(ERRORS) ;
		}
	/* leave the rest for inp_bfr & code_bfr */
	dos_kbytes -= 5 ; /* all but inp_bfr & code_bfr */

	/* -------------------------------------------- */
	/* If possible, reserve room for the rest of	*/
	/* the transient portion of COMMAND.COM.  This	*/
	/* guarantees that command line editing keys	*/
	/* (like F3) will continue to work when done.	*/
	/* -------------------------------------------- */
	if (!option.maximize.enabled)
		{
		if (dos_kbytes > 20) dos_kbytes -= 20 ;
		}

	Init_Buffer(&inp_bfr,	 src_file,		dos_kbytes / 2) ;
	Init_Buffer(&code_bfr,   Make_Filename(),	dos_kbytes / 2) ;
	Init_Buffer(&cs_pub_bfr, Make_Filename(),	1) ;	/* 1k */
	Init_Buffer(&temp_bfr,   Make_Filename(),	1) ;	/* 1k */
	Init_Buffer(&data_bfr,   Make_Filename(),	1) ;	/* 1k */

	if (Equal(src_file, "STDIN"))
		{
		inp_bfr.mode = OPENED ;
		inp_bfr.handle = STDIN ;
		}

	Put_Line(" CSEG", &cs_pub_bfr) ;

	if (option.big_model.enabled)
		{
		Put_Line("_CALL EQU LCALL", &cs_pub_bfr) ;
		Put_Line("_RET EQU LRET", &cs_pub_bfr) ;
		}
	else
		{
		Put_Line("_CALL EQU CALL", &cs_pub_bfr) ;
		Put_Line("_RET EQU RET", &cs_pub_bfr) ;
		}

	if (option.call_help.enabled)
		{
		Put_Line(" PUBLIC _HELP_", &cs_pub_bfr) ;
		}

	Clean_Setup() ;

	/* Enter registers into symbol table ... */
	for (r = AH; r <= CS; r++)
		{
		Make_Entry((CHAR **) r) ;
		}

	/* Enter opcodes into symbol table ... */
	for (i = 0 ; i < CODES88 ; i++)
		{
		Make_Entry((CHAR **) &opc88[i]) ;
		}
	if (option.superset.enabled)
		{
		for (i = 0 ; i < CODES188 ; i++)
			{
			Make_Entry((CHAR **) &opc188[i]) ;
			}
		}
	if (option.numeric.enabled)
		{
		for (i = 0 ; i < CODES87 ; i++)
			{
			Make_Entry((CHAR **) &opc87[i]) ;
			}
		}

	/* Enter functions into symbol table ... */
	if (option.numeric.enabled)
		{
		for (i = 0 ; i < LIBS87 ; i++)
			{
			Make_Entry((CHAR **) &lib87[i]) ;
			}
		}
	if (option.expand.enabled)
		{
		for (i = 0 ; i < LIBS88 ; i++)
			{
			Make_Entry((CHAR **) &lib88[i]) ;
			}
		}
	if (option.move.enabled)
		{
		for (i = 0 ; i < MOVS88 ; i++)
			{
			Make_Entry((CHAR **) &mov88[i]) ;
			}
		}

	stmt1 = Get_Next(NULL) ;
	while (stmt1 != NULL)
		{
		opc1 = stmt1->opcode ;
		if (*stmt1->label == ';') goto done ;

		opc1_attb = (opc1 != NULL) ? opc1->attb : 0 ;

		if (islower(stmt1->mnemonic[0]))
			{
			Free_Registers(_EMPTY, _EMPTY) ;
			flg_reg = NULL ;
			flg_sts = 0 ;
			goto done ;
			}

		dop1 = &stmt1->dst ;
		dorig1 = dop1->orig ;
		dlen1 = dop1->len ;
		if ((dreg1 = dop1->reg) != NULL)
			{
			/*
			------------------------------
			Update attributes to reflect
			current register status. Strip
			_DEPBX, etc. since those are
			used only to see if the memory
			operands reference the index
			registers.
			------------------------------
			*/
			dop1->attb = dreg1->attb & ~_DEPSRC ;
			}
		dattb1 = dop1->attb ;

		sop1 = &stmt1->src ;
		sorig1 = sop1->orig ;
		slen1 = sop1->len ;
		if ((sreg1 = sop1->reg) != NULL)
			{
			/*
			------------------------------
			Update attributes to reflect
			current register status. Strip
			_DEPBX, etc. since those are
			used only to see if the memory
			operands reference the index
			registers.
			------------------------------
			*/
			sop1->attb = sreg1->attb & ~_DEPSRC ;
			}
		sattb1 = sop1->attb ;

		jmp_reg = cmp_reg ;
		cmp_reg = NULL ;

		stmt2 = Find_Next(stmt1) ;
		if (Labels()) goto done ;
		stmt3 = Find_Next(stmt2) ;
		stmt4 = Find_Next(stmt3) ;
		stmt5 = Find_Next(stmt4) ;
		if (Dead_Code()) goto done ;
		if (opc1==NULL) goto done ;
		if (option.numeric.enabled) NDP_Init() ;
		if ((opc1==XOR || opc1==CWD) && Divide()) goto done ;
		if (Unnecessary(stmt1))
			{
			stmt1->label = redundant ;
			stats.cseg_del++ ;
			goto done ;
			}
		if (Simplify(sop1, RSWR) || Simplify(dop1, RDWR))
		    	{
			goto done ;
			}
		if ((*opc1->fnc)()) goto done ;
		if ((r = Bad_Ref(dop1))!=NULL || (r = Bad_Ref(sop1))!=NULL)
			{
			Undefined_Reg(r) ;
			}
		Update_Flg_Reg() ;
		Updates(BYTE(sattb1 | dattb1)) ;
	done:
		if (opc1==LINE)
			{
			for (r = AH; r <= ES; r++)
				{
				r->attb |= _AVAIL ;
				if (EMPTY(r) || !UNKNOWN(r))
					{
					continue ;
					}
				if (r->name[1]=='X')
					{
					if (UNKNOWN(RH(r)) && UNKNOWN(RL(r)))
						{
						LOAD_ATTB(r, _EMPTY) ;
						LOAD_ATTB(RH(r), _EMPTY) ;
						LOAD_ATTB(RL(r), _EMPTY) ;
						}
					}
				else if (WORD(r->attb))
					{
					LOAD_ATTB(r, _EMPTY) ;
					}
				}
			}
	      	Output(stmt1) ;
		next = Get_Next(stmt1) ;
		My_Free((CHAR *) stmt1) ;
		stmt1 = next ;
		}

	/* Clean-up left over memory allocated from last function */
	while (Remove_Entry()) ;
	Clean_Free() ;

	/* All done with the input file */
	inp_bfr.mode &= ~OPENED ;	/* (was using STDIN) */

	if (option.redundant.enabled) Squeeze() ;

	/* Set-up to reuse input buffer for final output file */
	Reuse_Buffer(&inp_bfr, dst_file) ;
	if (Equal(dst_file, "STDOUT"))
		{
		inp_bfr.mode = OPENED ;
		inp_bfr.handle = STDOUT ;
		}


	/* ----------------------------- */
	/* Now put the parts together... */
	/* ----------------------------- */

	if (option.numeric.enabled)
		{
		Put_Line("_FTEMP DQ 0", &data_bfr) ;
		stats.dseg_del -= 8 ;
		}

	Fold_Constants() ;

	Reverse_Buffer(&cs_pub_bfr) ;
	Copy_Buffer(&cs_pub_bfr, &inp_bfr) ;

	if (option.jump.enabled) Jump_Chains() ;
	else
		{
		Reverse_Buffer(&code_bfr) ;
		Copy_Buffer(&code_bfr, &inp_bfr) ;
		}

	Flush_Buffer(&inp_bfr) ;

	Report() ;
	exit(exit_code) ;
	}


PRIVATE VOID Make_Entry(data)
CHAR **data ;
	{
	CHAR first, i, *key ;
	static CHAR offset = (((' ' << 1) + ' ') << 1) + ' ' ;

	key = *data ;
	first = i = ((((key[0] << 1) + key[1]) << 1) + key[2]) - offset ; 
	do
		{
		if (table[i]==NULL)
			{
			table[i] = (CHAR *) data ;
			break ;
			}
		i += (first << 1) + 1 ;
		} while (i != first) ;
	}


REG *Bad_Ref(op)
OPERAND *op ;
	{
	REG *r ;
	FLAGS attb ;

	if (*op->orig=='\0')
		{
		return NULL ;
		}

	attb = op->attb ;
	if ((attb & _DEPSI) != 0)
		{
		if (EMPTY(SI))
			{
			return SI ;
			}
		}
	else if ((attb & _DEPDI) != 0)
		{
		if (EMPTY(DI))
			{
			return DI ;
			}
		}
	if ((attb & _DEPBX)!=0 && EMPTY(BX))
		{
		return BX ;
		}

	if (((r = op->reg) != NULL) && EMPTY(r))
		{
		if ((opc1==XOR && dreg1==sreg1) ||
		    (opc1==XCHG && dreg1!=NULL && sreg1!=NULL))
			{
			return NULL ;
			}
		else if (op == sop1)
			{
			return (opc1->attb & RSWR)!=0 ? r : NULL ;
			}
		else if ((opc1->attb & MDFYDST)!=0 &&
			 (opc1->attb & LOADS)==0)
			{
			return r ;
			}
		else
			{
			return (opc1->attb & RDWR)!=0 ? r : NULL ;
			}
		}
	return NULL ;
	}


PRIVATE unsigned Per_Cent(old, diff)
int old ;
int diff ;
	{
	int pcnt ;

	pcnt = old!=0 ? (int) ((100L * diff + old/2) / old) : 0 ;
	return (pcnt >= 0) ? pcnt : -pcnt ;
	}

VOID Report()
	{
	CHAR bfr[20], *ptr ;
	int cseg_del, dseg_del ;
	unsigned pcnt_del, pcnt_smp, len ;

	len = Show_Options() ;
	cseg_del = ((int) stats.cseg_del) - ((int) stats.cseg_ins) ;

	pcnt_del = Per_Cent((int) stats.cseg_ttl, cseg_del) ;
	pcnt_smp = Per_Cent((int) stats.cseg_ttl, (int) stats.cseg_smp) ;

	bfr[9] = '\0' ;
	bfr[8] = '%' ;
	ptr = Convert(pcnt_smp, &bfr[7]) ;
	*--ptr = ',' ;
	*--ptr = '%' ;
	ptr = Convert(pcnt_del, ptr - 1) ;
	if (cseg_del < 0) *--ptr = '-' ;
	for (len += strlen(ptr); len < 24; len++) Errc(' ') ;
	Errs(ptr) ;
	Errs(" code") ;

	dseg_del = (int) stats.dseg_del ;
	if (dseg_del < 0) dseg_del = -dseg_del ;
	ptr = Convert((unsigned) dseg_del, &bfr[8]) ;
	if (((int) stats.dseg_del) < 0) *--ptr = '-' ;
	for (len = strlen(ptr); len < 7; len++) Errc(' ') ;
	Errs(ptr) ;
	Errs(" data") ;

	ptr = Convert(stats.call_del, &bfr[8]) ;
	for (len = strlen(ptr); len < 11; len++) Errc(' ') ;
	Errs(ptr) ;
	Errs(" calls\r\n") ;
	}


PRIVATE unsigned Show_Options()
	{
	unsigned i, len ;
	SWITCH *opt ;

	len = 2 ;
	opt = (SWITCH *) &option ;
	Errc('+') ;
	for (i = 0 ; i < OPTIONS ; i++, opt++)
		{
		if (opt->enabled)
			{
			Errc(opt->selector) ;
			len++ ;
			}
		}
	Errc(':') ;
	return len ;
	}


VOID Undefined_Reg(r)
REG *r ;
	{
	undefined[8] = r->name[0] ;
	undefined[9] = r->name[1] ;
	stmt1->label = undefined ;
	stmt1->attb |= ERRMSG ;
	}


		      /*  End of file O88.C  */

