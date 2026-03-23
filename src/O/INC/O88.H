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
			 /*  Start of file: O88.H  */

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

/*lint	+fcu		char-is-unsigned				*/
/*lint	+fsu		string-is-unsigned				*/
/*lint	+fzu		sizeof-is-unsigned				*/
/*lint	-library	don't worry about unreferenced or undefined	*/
/*lint  -e730		bullshit message about passing bool as arg	*/
/*lint  -e701		   "       "       "   left shift of signed	*/


#define	PERFECT		0
#define	WARNING		1
#define	ERRORS		255

#ifdef	DATA		/* The following is a work around for LINT bug	*/
unsigned char _cc ;
#else
extern unsigned char _cc ;
#endif
#define	_c _cc
#include "INC\MACROS.H"	/* Use in-line macros, not library functions	*/

#define	VOID	void
#define	BOOLEAN	int
#define	FLAGS	unsigned int
#define	CHAR	unsigned char
#define	HANDLE	unsigned int

#define	STDIN	0
#define	STDOUT	1
#define	STDERR	2

#define	PRIVATE	static
#define	PUBLIC	extern

#ifdef	TRUE
#undef	TRUE
#endif
#define	TRUE	1

#ifdef	FALSE
#undef	FALSE
#endif
#define	FALSE	0

#ifdef	NULL
#undef	NULL
#endif
#define	NULL	((void *) 0)

#define	CR	0x0D
#define	LF	0x0A
#define	CTLZ	0x1A

/* status bits for 'mode' below... */

#define	OPENED		0x01
#define	RESIDENT	0x02
#define	INPUT		0x04
#define	OUTPUT		0x08

typedef	struct LINKLIST
	{
	struct LINKLIST *next ;
	} LINKLIST ;

typedef struct BUFFER
	{
	CHAR		filename[100] ;
	HANDLE		handle ;
	CHAR		mode ;
	unsigned	off ;
	unsigned	seg ;
	unsigned	count ;
	unsigned	size ;
	} BUFFER ;

typedef union PACKED
	{
	unsigned word ;
	struct
		{
		CHAR lsb ;
		CHAR msb ;
		} byte ;
	} PACKED ;

typedef struct FNC
	{
	CHAR	**code ;	/* pointer to replacement code strings	*/
	CHAR	bytes ;		/* bytes pushed on stack by fnc call	*/
	int	cost ;		/* Bytes of in-line code in replacement	*/
	} FNC ;

typedef struct LIB
	{
	CHAR	*name ;		/* name of library routine		*/
	FNC	small ;		/* For small memory model		*/
	FNC	large ;		/* For large memory model		*/
	} LIB ;

/* An array of structures of the following description is used to	*
 * hold and describe the effects of the various instructions.		*/

typedef struct OPCODE
	{
	CHAR	*name ;		/* ptr to opcode name			*/
	FLAGS	attb ;		/* things that may be replaced and	*/
	BOOLEAN	(*fnc)() ;	/* function to call for optimization	*/
	CHAR	dummy[2] ;	/* pad to a power of 2			*/
	} OPCODE ;		/* affect of opcode on flags.		*/

#define	LINE	(&opc88[0])
#define	MOV	(&opc88[1])
#define	PUSH	(&opc88[2])
#define	CMP	(&opc88[3])
#define	ADD	(&opc88[4])
#define	CALL	(&opc88[5])
#define	JZ	(&opc88[6])
#define	JNZ	(&opc88[7])
#define	POP	(&opc88[8])
#define	RET	(&opc88[9])
#define	ADC	(&opc88[10])
#define	TEST	(&opc88[11])
#define	JMP	(&opc88[12])
#define	OR	(&opc88[13])
#define	AND	(&opc88[14])
#define	XOR	(&opc88[15])
#define	LEA	(&opc88[16])
#define	INC	(&opc88[17])
#define	SUB	(&opc88[18])
#define	SHL	(&opc88[19])
#define	IMUL	(&opc88[20])
#define	MUL	(&opc88[21])
#define	JAE	(&opc88[22])
#define	DEC	(&opc88[23])
#define	DIV	(&opc88[24])
#define	IDIV	(&opc88[25])
#define	CWD	(&opc88[26])
#define	JA	(&opc88[27])
#define	JB	(&opc88[28])
#define	JBE	(&opc88[29])
#define	JG	(&opc88[30])
#define	JGE	(&opc88[31])
#define	JL	(&opc88[32])
#define	JLE	(&opc88[33])
#define	IN	(&opc88[34])
#define	NEG	(&opc88[35])
#define	NOT	(&opc88[36])
#define	OUT	(&opc88[37])
#define	SAR	(&opc88[38])
#define	SBB	(&opc88[39])
#define	SHR	(&opc88[40])
#define	XCHG	(&opc88[41])
#define	CMC	(&opc88[42])
#define	SAHF	(&opc88[43])
#define	REP	(&opc88[44])
#define	CLD	(&opc88[45])
#define	MOVSB	(&opc88[46])
#define	MOVSW	(&opc88[47])
#define	STOSB	(&opc88[48])
#define	STOSW	(&opc88[49])
#define	LES	(&opc88[50])
#define	ROL	(&opc88[51])
#define	ROR	(&opc88[52])

#define	CODES88		53

#define	ENTER	(&opc188[0])
#define	LEAVE	(&opc188[1])
#define	IMULI	(&opc188[2])

#define	CODES188	3

#define	FLD	(&opc87[0])
#define	FSTP	(&opc87[1])
#define	FST	(&opc87[2])
#define	FADD	(&opc87[3])
#define	FSUB	(&opc87[4])
#define	FMUL	(&opc87[5])
#define	FDIV	(&opc87[6])
#define	FCHS	(&opc87[7])
#define	FLDZ	(&opc87[8])
#define	FLD1	(&opc87[9])
#define	FWAIT	(&opc87[10])
#define	FCOMPP	(&opc87[11])
#define	FCOMP	(&opc87[12])
#define	FCOM	(&opc87[13])
#define	FICOMP	(&opc87[14])
#define	FICOM	(&opc87[15])
#define	FSTSW	(&opc87[16])
#define	FXCH	(&opc87[17])
#define	FILD	(&opc87[18])
#define	FABS	(&opc87[19])
#define	FISTP	(&opc87[20])
#define	FIADD	(&opc87[21])
#define	FISUB	(&opc87[22])
#define	FIMUL	(&opc87[23])
#define	FIDIV	(&opc87[24])
#define	FTST	(&opc87[25])
#define	FLDCW	(&opc87[26])
#define	FSTCW	(&opc87[27])
#define	FINIT	(&opc87[28])
#define	FLDPI	(&opc87[29])
#define	FLDL2T	(&opc87[30])
#define	FLDL2E	(&opc87[31])
#define	FLDLG2	(&opc87[32])
#define	FLDLN2	(&opc87[33])
#define	FRNDINT	(&opc87[34])
#define	FSQRT	(&opc87[35])
#define	FYL2X	(&opc87[36])
#define	F2XM1	(&opc87[37])

#define	CODES87		38

/* Bits in OPCODE.attb... */

#define	MDFYSRC	0x0001	/* NZ if opcode modifies source operand		*/
#define	MDFYDST	0x0002	/* NZ if opcode modifies destination operand	*/
#define	MDFY_AX	0x0004	/* NZ if opcode modifies AX			*/
#define	MDFY_DX	0x0008	/* NZ if opcode modifies DX			*/
#define	DEF_FLG	0x0010	/* NZ if opcode defines ZF, SF, CF, and OF	*/
#define	BAD_FLG	0x0020	/* NZ if opcode destroys ZF, SF, CF, or OF	*/
#define	RSWR	0x0040	/* NZ if ok to Replace Src With Register	*/
#define	RDWR	0x0080	/* NZ if ok to Replace Dst With Register	*/
#define	CONDJMP	0x0100	/* NZ if JZ, JNZ, etc.				*/
#define	ZEROTST	0x0200	/* NZ if JZ or JNZ (need ZF only)		*/
#define	SGNDTST	0x0400	/* NZ if JG, JGE, JL, or JLE (need SF,ZF,OF)	*/
#define	COMMUTE	0x0800	/* NZ if commutative such as addition		*/
#define	LOADS	0x1000	/* NZ if opcode loads its destination		*/
#define	MEMOP87	0x2000	/* NZ if 87 opc can take int or real op fm mem	*/
#define	MOVABLE	0x4000	/* NZ if opcode may be postponed		*/
#define	ZRO_FLG	0x8000	/* NZ if opcode defines ZF (and SF) (CF,OF bad)	*/

#define	MDFYACC	(MDFY_AX | MDFY_DX)

/* Registers description */

typedef struct REG	/* ('content' is padded so size is power of 2)	*/
	{
	CHAR		*name ;		/* register name, e.g., "AX"	*/
	FLAGS		attb ;		/* most recently loaded data	*/
	unsigned	len ;		/* strlen(content)		*/
	CHAR		content[122] ;	/* most recent data		*/
	} REG ;

#define	AH	(&reg[0])
#define	AL	(&reg[1])
#define	AX	(&reg[2])
#define	SI	(&reg[3])
#define	DI	(&reg[4])
#define	CH	(&reg[5])
#define	CL	(&reg[6])
#define	CX	(&reg[7])
#define	BH	(&reg[8])
#define	BL	(&reg[9])
#define	BX	(&reg[10])
#define	DH	(&reg[11])
#define	DL	(&reg[12])
#define	DX	(&reg[13])
#define	ES	(&reg[14])
#define	SP	(&reg[15])
#define	BP	(&reg[16])
#define	DS	(&reg[17])
#define	SS	(&reg[18])
#define	CS	(&reg[19])

#define	RL(r)	((r) - 1)
#define	RH(r)	((r) - 2)
#define	RX(r)	(wregs[(r) - reg])

#define RELATED(r1, r2) (Contains(r1, r2) || Contains(r2, r1))

/* Operand description */

typedef struct OPERAND
	{
	CHAR		*orig ;	/* ptr to original operand string	*/
	FLAGS		attb ;	/* (see #defines below)			*/
	unsigned	len ;	/* strlen(orig)				*/
	REG		*reg ;	/* ptr to register, if op is one	*/
	} OPERAND ;

/* Bits in OPERAND.attb.... */

#define	_ACCUM	0x0001		/* AX, AL, AH, DX, DL, or DH		*/
#define	_INDEX	0x0002		/* BX, SI, DI				*/
#define	_SEGREG	0x0004		/* CS, DS, ES, SS			*/
#define	_BYTE	0x0008		/* AL, AH, BL, BH, CL, CH, DL, DH	*/
#define	_DEPSI	0x0010		/* operand depends on SI		*/
#define	_DEPDI	0x0020		/* operand depends on DI		*/
#define	_DEPBX	0x0040		/* operand depends on BX		*/
#define	_DEPBP	0x0080		/* operand is an argument		*/

#define	_DEPES	0x0100		/* operand depends on ES		*/
#define	_EMPTY	0x0200		/* register is not in use		*/
#define	_AVAIL	0x0400		/* in use, but available		*/
#define	_UNKN	0x0800		/* operand undefined			*/
#define	_ZERO	0x1000		/* operand is zero			*/
#define	_CONST	0x2000		/* operand is a constant		*/
#define	_ADDR	0x4000		/* operand is address of data		*/
#define	_DATA	0x8000		/* operand is data			*/

#define	_NUMB	(_CONST|_DATA)
#define	_DEPSRC	(_DEPSI|_DEPDI|_DEPBX|_DEPBP)
#define	_DEPDST	(_DEPSI|_DEPDI|_DEPBX)
#define	_FXD	(_BYTE|_ACCUM|_INDEX|_SEGREG)

#define	WORD(a)		(((a) & _BYTE) == 0)
#define	BYTE(a)		(((a) & _BYTE) != 0)
#define	ZERO(a)		(((a) & _ZERO) != 0)
#define	ADDRESS(a)	(((a) & _ADDR) != 0)
#define	OFFSET(a)	(((a) & (_ADDR|_CONST)) == (_ADDR|_CONST))
#define	MEMREF(a)	(((a) & _NUMB) == _DATA)
#define CONSTANT(a)	(((a) & _NUMB) == _NUMB)
#define VALID(a)	(((a) & (_EMPTY|_UNKN)) == 0)
#define	VARDST(a)	(((a) & _DEPDST) != 0)
#define	VARSRC(a)	(((a) & _DEPSRC) != 0)

#define	EMPTY(r)	(((r)->attb & _EMPTY) != 0)
#define	ACCUM(r)	(((r)->attb & _ACCUM) != 0)
#define	INDEX(r)	(((r)->attb & _INDEX) != 0)
#define	SEGREG(r)	(((r)->attb & _SEGREG) != 0)
#define	AVAIL(r)	(((r)->attb & _AVAIL) != 0)
#define	UNKNOWN(r)	(((r)->attb & _UNKN) != 0)

#define	DEPEND_FLAG(r)	(dpndflgs[(r) - reg])
#define	ZREG(r)		ZERO((r)->attb)
#define	MODIFY(r) 	((r)->attb &= _FXD), ((r)->attb |= _UNKN)
#define	LOAD_ATTB(r,a)	((r)->attb &= _FXD), ((r)->attb|=(a)&(~_FXD))

#define	_AH	0x0001
#define	_AL	0x0002
#define	_AX	0x0004
#define	_SI	0x0008
#define	_DI	0x0010
#define	_CH	0x0020
#define	_CL	0x0040
#define	_CX	0x0080
#define	_BH	0x0100
#define	_BL	0x0200
#define	_BX	0x0400
#define	_DH	0x0800
#define	_DL	0x1000
#define	_DX	0x2000
#define	_ES	0x4000

#define	REG_FLAG(r)	(regflgs[(r) - reg])

/* decomposed parts of assembly language statement */

typedef struct PARTS
	{
	struct PARTS  *next ;	/* ptr to next statement in segment	*/
	FLAGS	attb ;		/* (see below)				*/
	int	id ;		/* unique line number to identify with	*/
	CHAR	*label ;	/* ptr to label field of stmt		*/
	CHAR	*mnemonic ;	/* ptr to opcode field of stmt		*/
	OPCODE	*opcode ;	/* ptr to opcode (if recognized)	*/
	OPERAND	dst ;		/* ptrs to dst field of stmt		*/
	OPERAND	src ;		/* ptrs to src field of stmt		*/
	CHAR	*third ;	/* for 80188 IMUL <dst>,<src>,<imm8>	*/
	CHAR	*comment ;	/* ptr to comment field of stmt		*/
	CHAR	stmt[1] ;	/* expanded when structure is allocated	*/
	} PARTS ;


/* Bits in PARTS attb... */

#define	_REFAX	0x0001		/* statement references AH, AL, or AX	*/
#define	_REFBX	0x0002		/* statement references BH, BL, or BX	*/
#define	_REFCX	0x0004		/* statement references CH, CL, or CX	*/
#define	_REFDX	0x0008		/* statement references DH, DL, or DX	*/
#define	_REFSI	0x0010		/* statement references SI		*/
#define	_REFDI	0x0020		/* statement references DI		*/
#define	_REFES	0x0040		/* statement references ES		*/
#define	_DEFAX	0x0080		/* statement redefines AH, AL, and AX	*/
#define	_DEFBX	0x0100		/* statement redefines BH, BL, and BX	*/
#define	_DEFCX	0x0200		/* statement redefines CH, CL, and CX	*/
#define	_DEFDX	0x0400		/* statement redefines DH, DL, and DX	*/
#define	_DEFSI	0x0800		/* statement redefines SI		*/
#define	_DEFDI	0x1000		/* statement redefines DI		*/
#define	_DEFES	0x2000		/* statement redefines ES		*/
#define	NEWREG	0x4000		/* Destination register replaced	*/
#define	CLEAND	0x4000		/* ADD SP,n or MOV SP,BP was eliminated	*/
#define	POPXST	0x4000		/* POP exists below this expanded CALL	*/
#define	ERRMSG	0x8000		/* Echo this line to the console	*/

#define	_REFALL	(_REFAX|_REFBX|_REFCX|_REFDX|_REFSI|_REFDI|_REFES)
#define	_DEFALL	(_DEFAX|_DEFBX|_DEFCX|_DEFDX|_DEFSI|_DEFDI|_DEFES)

#define	REFERENCED(r, stmt)	(((stmt)->attb & refflgs[(r) - reg])!=0)
#define	REDEFINED(r, stmt)	(((stmt)->attb & defflgs[(r) - reg])!=0)

typedef struct STATS
	{
	unsigned cseg_ttl ;	/* total instructions in original file	*/
	unsigned cseg_ins ;	/* instructions inserted into file	*/
	unsigned cseg_del ;	/* instructions deleted	from file	*/
	unsigned cseg_smp ;	/* number simplified in optimized file	*/
	unsigned dseg_del ;	/* total bytes removed from DSEG	*/
	unsigned call_del ;	/* CALLs replaced by in-line code	*/
	} STATS ;

typedef struct SWITCH
	{
	CHAR	selector ;	/* identifying character, '\0' if end	*/
	BOOLEAN	enabled ;	/* TRUE if on (+), FALSE if not (-)	*/
	CHAR	*on_msg ;
	CHAR	*off_msg ;
	} SWITCH ;

typedef struct OPTION
	{
	SWITCH	maximize ;	/* (A) Allocate Maximum buffer size	*/
	SWITCH	big_model ;	/* (B) big (large) memory model		*/
	SWITCH	comments ;	/* (C) output to be commented		*/
	SWITCH	expand ;	/* (E) Expand library functions	in-line */
	SWITCH	fold ;		/* (F) Fold duplicate constants		*/
	SWITCH	call_help ;	/* (H) Add CALL _HELP_ in funcitons	*/
	SWITCH	init8087 ;	/* (I) 8087 initialization		*/
	SWITCH	jump ;		/* (J) Remove JMP's to JMP's.		*/
	SWITCH	move ;		/* (M) Replace MOVE() with REP MOVSB	*/
	SWITCH	optimize ;	/* (O) optimize the code		*/
	SWITCH	pointers ;	/* (P) ptrs to themselves allowed	*/
	SWITCH	redundant ;	/* (R) Remove redundant code 		*/
	SWITCH	stack ;		/* (S) Clean stack at function return	*/
	SWITCH	time ;		/* (T) optimize for time		*/
	SWITCH	superset ;	/* (1) use 80188 instruction superset	*/
	SWITCH	numeric ;	/* (7) optimize for 8087/80287		*/
	} OPTION ;

#define	OPTIONS	(sizeof(OPTION)/sizeof(SWITCH))

#define	DEFINED		0x0001
#define	REFERENCE	0x0002
#define	DELETED		0x0004

typedef struct REG_STATE
	{	
	FLAGS		attb ;
	unsigned	len ;
	CHAR		content[1] ;	/* Expanded when allocated	*/
	} REG_STATE ;

typedef struct PRESERVE	/* For preserving register states */
	{
	struct PRESERVE	*next ;
	FLAGS		status ;
	REG_STATE	*regs[15] ;
	CHAR		label[1] ;	/* expanded when allocated	*/
	} PRESERVE ;

#define	OFF_LBL		34	/* Move this to O88.H */
#define	FIND_LABEL(l)	((PRESERVE*)LL_Search((LINKLIST**)&states,OFF_LBL,l))


/* PUBLIC Library functions (arranged alphabetically)			*/
/* -------------------------------------------------------------------- */

#ifdef _lint	/* These function prototypes are only for lint...	*/

/*lint +fvr	The return value of these functions may be ignored	*/

VOID		exit(unsigned) ;
int		free(CHAR *) ;
VOID		freeall(unsigned) ;
CHAR		*strcat(CHAR *, CHAR *) ;
CHAR		*strcpy(CHAR *, CHAR *) ;
CHAR		*strncat(CHAR *, CHAR *, unsigned) ;
CHAR		*strncpy(CHAR *, CHAR *, unsigned) ;
VOID		_doint(unsigned) ;
VOID		_setmem(CHAR *, unsigned, unsigned) ;

/*lint -fvr	The return value of these functions must be used	*/

CHAR		*index(CHAR *, CHAR) ;
CHAR		*malloc(unsigned) ;
CHAR		*rindex(CHAR *, CHAR) ;
unsigned	strlen(CHAR *) ;
VOID		_lmove(unsigned, unsigned, unsigned, unsigned, unsigned) ;
CHAR		_peek(unsigned, unsigned) ;
VOID		_poke(unsigned, unsigned, unsigned) ;
int		_showds(VOID) ;
VOID		_move(unsigned, VOID *, VOID *) ;


/* Non-PRIVATE functions (arranged by file)...				*/
/* -------------------------------------------------------------------- */

/*lint +fvr	The return value of these functions may be ignored	*/

/* PARSE.C	*/
PARTS		*Emit(PARTS *, CHAR *, CHAR *, CHAR *, CHAR *) ;

/* 8087.C	*/
BOOLEAN		Fst(VOID) ;

/* LABELS.C	*/
BOOLEAN		Remove_Entry(VOID) ;

/* ASMLIB.C */
CHAR		*Append(CHAR *, CHAR *) ;
BOOLEAN		Close_File(HANDLE) ;
CHAR		*Convert(unsigned, CHAR *) ;
CHAR		*Leading(CHAR *) ;
int		Trailing(CHAR *, unsigned) ;

/*lint -fvr	The return value of these functions must be used	*/

/* GATHER.C	*/
BOOLEAN		Add_Op(VOID) ;
BOOLEAN		Call_Op(VOID) ;
BOOLEAN		Cwd_Op(VOID) ;
BOOLEAN		Dec_Op(VOID) ;
BOOLEAN		Div_Op(VOID) ;
BOOLEAN		Inc_Op(VOID) ;
BOOLEAN		Lea_Op(VOID) ;
BOOLEAN		Mov_Op(VOID) ;
BOOLEAN		Movs_Op(VOID) ;
BOOLEAN		Push_Op(VOID) ;
BOOLEAN		Rep_Op(VOID) ;
BOOLEAN		Shift(VOID) ;
BOOLEAN		Stos_Op(VOID) ;
BOOLEAN		Sub_Op(VOID) ;
BOOLEAN		Unnecessary(PARTS *) ;
BOOLEAN		Xchg_Op(VOID) ;
BOOLEAN		Xor_Op(VOID) ;
VOID		Imm_Op(CHAR *, CHAR *) ;
VOID		Seg_Off(CHAR *, unsigned, CHAR *, CHAR  *) ;

/* ASMLIB.C	*/
VOID		Delete_File(CHAR *) ;
VOID		DOS_Memory(VOID) ;
int		DOS_Version(VOID) ;
BOOLEAN		Equal(CHAR *, CHAR *) ;
BOOLEAN		EqualN(CHAR *, CHAR *, unsigned) ;
VOID		Errc(CHAR) ;
VOID		Errs(CHAR *) ;
BOOLEAN		False(VOID) ;
CHAR		*Find(CHAR *) ;
BOOLEAN		Found(CHAR *) ;
BOOLEAN		No_Input_File(VOID) ;
HANDLE		Open_File(CHAR *, unsigned) ;
unsigned	Read_File(HANDLE, unsigned, unsigned) ;
VOID		Rename_File(CHAR *, CHAR *) ;
BOOLEAN		True(VOID) ;
int		Value(CHAR *, unsigned) ;
unsigned	Write_File(HANDLE, unsigned, unsigned) ;

/* PARSE.C	*/
CHAR		*Make_String(CHAR *, unsigned) ;
VOID		Parse(PARTS *) ;
CHAR		*Skip_Prefix(CHAR *) ;

/* O88.C	*/
VOID		Report(VOID) ;
REG		*Bad_Ref(OPERAND *) ;
VOID		Undefined_Reg(REG *) ;

/* 8088-PT1.C	*/
BOOLEAN		Add_Pt1(VOID) ;
BOOLEAN		And_Op(VOID) ;
BOOLEAN		Cld(VOID) ;
BOOLEAN		Cmp_Op(VOID) ;
VOID		Dependencies(REG *) ;
REG		*Find_Val(CHAR *, FLAGS) ;
VOID		Fix_Reg_Set(REG *) ;
BOOLEAN		Flags_Needed(PARTS *) ;
VOID		Free_Registers(FLAGS, FLAGS) ;
BOOLEAN		IMul(VOID) ;
CHAR		*Insert_String(CHAR *, unsigned, int, REG *) ;
BOOLEAN		Lea_Pt1(VOID) ;
BOOLEAN		Les(VOID) ;
VOID		Load_Reg(REG *, CHAR *, FLAGS) ;
VOID		Modified(OPERAND *) ;
BOOLEAN		Mov_Pt1(VOID) ;
BOOLEAN		Or_Op(VOID) ;
BOOLEAN		Simplify(OPERAND *, FLAGS) ;
BOOLEAN		Sub_Pt1(VOID) ;
VOID		Swap(unsigned *, unsigned *) ;
BOOLEAN		Test_Op(VOID) ;
VOID		Updates(BOOLEAN) ;
VOID		Update_Flg_Reg(VOID) ;
CHAR		*Word_To_Byte(CHAR *) ;
BOOLEAN		Xor_Pt1(VOID) ;

/* 8088-PT2.C	*/
BOOLEAN		Add_Pt2(VOID) ;
BOOLEAN		Collapse(VOID) ;
BOOLEAN		Contains(REG *, REG *) ;
BOOLEAN		Divide(VOID) ;
BOOLEAN		Lea_Pt2(VOID) ;
BOOLEAN		Mov_Pt2(VOID) ;
BOOLEAN		Mul_Op(VOID) ;
BOOLEAN		Pop(VOID) ;
PARTS		*Reg_Needed(REG *, PARTS *) ;
BOOLEAN		Shl_Op(VOID) ;
BOOLEAN		Sub_Pt2(VOID) ;

/* ALLOCATE.C	*/
CHAR		*Allocate(unsigned) ;
VOID		Init_Heap(unsigned) ;
VOID		My_Free(CHAR *) ;

/* BUFFERS.C	*/
VOID		Init_Buffer(BUFFER *, CHAR *, unsigned) ;
VOID		Close_Input(BUFFER *) ;
VOID		Close_Output(BUFFER *) ;
VOID		Copy_Buffer(BUFFER *, BUFFER *) ;
VOID		Flush_Buffer(BUFFER *) ;
unsigned	Get_Line(CHAR [], BUFFER *) ;
VOID		Open_Input(BUFFER *) ;
VOID		Open_Output(BUFFER *) ;
VOID		Put_Line(CHAR *, BUFFER *) ;
VOID		Put_Str(CHAR *, BUFFER *) ;
VOID		Reuse_Buffer(BUFFER *, CHAR *) ;
VOID		Reverse_Buffer(BUFFER *) ;
VOID		Rewind_Buffer(BUFFER *) ;

/* 80188.C	*/
VOID		Assemble(PARTS *) ;
BOOLEAN		Mov_188(VOID) ;
BOOLEAN		Shift_188(VOID) ;

/* 8087.C	*/
BOOLEAN		Cwd_87(VOID) ;
BOOLEAN		Fild(VOID) ;
BOOLEAN		Fld(VOID) ;
BOOLEAN		Fstp(VOID) ;
BOOLEAN		Fistp(VOID) ;
BOOLEAN		Fstsw(VOID) ;
BOOLEAN		Lea_87(VOID) ;
BOOLEAN		Mov_87(VOID) ;
VOID		NDP_Init(VOID) ;
VOID		NDP_Replace(PARTS *) ;
BOOLEAN		Sahf(VOID) ;
BOOLEAN		Xor_87(VOID) ;
VOID		Zero_One(CHAR *) ;

/* 8088-PT3.C	*/
REG		*Find_Free(CHAR *, REG *) ;
BOOLEAN		Preserve(VOID) ;

/* CLEANUP.C	*/
BOOLEAN		Clean_Call(VOID) ;
VOID		Clean_Free(VOID) ;
VOID		Clean_Function(CHAR *) ;
BOOLEAN		Clean_Ret(VOID) ;
VOID		Clean_Setup(VOID) ;

/* LABELS.C	*/
BOOLEAN		Dead_Code(VOID) ;
BOOLEAN		Jump(VOID) ;
BOOLEAN		Labels(VOID) ;

/* UTILS.C	*/
VOID		Usage(VOID) ;
VOID		Error_Msg(CHAR *, CHAR *) ;
PARTS		*Find_Next(PARTS *) ;
VOID		Get_Args(unsigned, CHAR *[]) ;
CHAR		*Get_Env_Str(CHAR *) ;
PARTS		*Get_Next(PARTS *) ;
CHAR		*Make_Filename(VOID) ;
VOID		Output(PARTS *) ;
BOOLEAN		Invalidates(PARTS *, OPERAND *) ;

/* FOLDING.C	*/
VOID		Fold_Constants(VOID) ;

/* JUMPS.C	*/
VOID		Jump_Chains(VOID) ;

/* LINKLIST.C	*/
LINKLIST	*LL_Free(CHAR *, unsigned, unsigned) ;
LINKLIST	*LL_Search(LINKLIST **, unsigned, CHAR *) ;

/* SQUEEZE.C	*/
VOID		Squeeze(VOID) ;

#else	/* _lint (C88 doesn't like function prototypes!) */

#ifdef	NOT_TOO_MANY_PUBLICS
VOID		exit() ;
VOID		freeall() ;
VOID		_doint() ;
VOID		_lmove() ;
VOID		_poke() ;
VOID		_setmem() ;
VOID		_move() ;
int		free() ;
int		_showds() ;
unsigned	strlen() ;
CHAR		_peek() ;
CHAR		*index() ;
CHAR		*malloc() ;
CHAR		*rindex() ;
CHAR		*strcat() ;
CHAR		*strcpy() ;
CHAR		*strncat() ;
CHAR		*strncpy() ;

/* Non-PRIVATE functions (arranged by return type)...			*/
/* -------------------------------------------------------------------- */
VOID		Assemble() ;		/* 80188.C	*/
VOID		Clean_Free() ;		/* CLEANUP.C	*/
VOID		Clean_Function() ;	/* CLEANUP.C	*/
VOID		Clean_Setup() ;		/* CLEANUP.C	*/
VOID		Close_Input() ;		/* BUFFERS.C	*/
VOID		Close_Output() ;	/* BUFFERS.C	*/
VOID		Copy_Buffer() ;		/* BUFFERS.C	*/
VOID		Delete_File() ;		/* ASMLIB.C	*/
VOID		Dependencies() ;	/* 8088-PT1.C	*/
VOID		DOS_Memory() ;		/* ASMLIB.C	*/
VOID		Errc() ;		/* ASMLIB.C	*/
VOID		Usage() ;		/* UTILS.C	*/
VOID		Error_Msg() ;		/* UTILS.C	*/
VOID		Errs() ;		/* ASMLIB.C	*/
VOID		Fix_Reg_Set() ;		/* 8088-PT1.C	*/
VOID		Flush_Buffer() ;	/* BUFFERS.C	*/
VOID		Fold_Constants() ;	/* FOLDING.C	*/
VOID		Free_Registers() ;	/* 8088-PT1.C	*/
VOID		Get_Args() ;		/* UTILS.C	*/
VOID		Imm_Op() ;		/* GATHER.C	*/
VOID		Init_Buffer() ;		/* BUFFERS.C	*/
VOID		Init_Heap() ;		/* ALLOCATE.C	*/
VOID		Jump_Chains() ;		/* JUMPS.C	*/
VOID		Modified() ;		/* 8088-PT1.C	*/
VOID		NDP_Init() ;		/* 8087.C	*/
VOID		NDP_Replace() ;		/* 8087.C	*/
VOID		Open_Input() ;		/* BUFFERS.C	*/
VOID		Open_Output() ;		/* BUFFERS.C	*/
VOID		Output() ;		/* UTILS.C	*/
VOID		Parse() ;		/* PARSE.C	*/
VOID		Put_Line() ;		/* BUFFERS.C	*/
VOID		Put_Str() ;		/* BUFFERS.C	*/
VOID		Rename_File() ;		/* ASMLIB.C	*/
VOID		Report() ;		/* O88.C	*/
VOID		Reuse_Buffer() ;	/* BUFFERS.C	*/
VOID		Reverse_Buffer() ;	/* BUFFERS.C	*/
VOID		Rewind_Buffer() ;	/* BUFFERS.C	*/
VOID		Seg_Off() ;		/* GATHER.C	*/
VOID		Squeeze() ;		/* SQUEEZE.C	*/
VOID		Swap() ;		/* 8088-PT1.C	*/
VOID		Undefined_Reg() ;	/* O88.C	*/
VOID		Updates() ;		/* 8088-PT1.C	*/
VOID		Update_Flg_Reg() ;	/* 8088-PT1.C	*/
int		DOS_Version() ;		/* ASMLIB.C	*/
int		Trailing() ;		/* ASMLIB.C	*/
int		Value() ;		/* ASMLIB.C	*/
unsigned	Get_Line() ;		/* BUFFERS.C	*/
unsigned	Read_File() ;		/* ASMLIB.C	*/
unsigned	Write_File() ;		/* ASMLIB.C	*/
HANDLE		Open_File() ;		/* ASMLIB.C	*/
BOOLEAN		Add_Pt1() ;		/* 8088-PT1.C	*/
BOOLEAN		Add_Pt2() ;		/* 8088-PT2.C	*/
BOOLEAN		Clean_Call() ;		/* CLEANUP.C	*/
BOOLEAN		Close_File() ;		/* ASMLIB.C	*/
BOOLEAN		Collapse() ;		/* 8088-PT2.C	*/
BOOLEAN		Contains() ;		/* 8088-PT2.C	*/
BOOLEAN		Cwd_87() ;		/* 8087.C	*/
BOOLEAN		Dead_Code() ;		/* LABELS.C	*/
BOOLEAN		Divide() ;		/* 8088-PT2.C	*/
BOOLEAN		Equal() ;		/* ASMLIB.C	*/
BOOLEAN		EqualN() ;		/* ASMLIB.C	*/
BOOLEAN		Flags_Needed() ;	/* 8088-PT1.C	*/
BOOLEAN		Found() ;		/* ASMLIB.C	*/
BOOLEAN		No_Input_File() ;	/* ASMLIB.C	*/
BOOLEAN		Labels() ;		/* LABELS.C	*/
BOOLEAN		Lea_Pt1() ;		/* 8088-PT1.C	*/
BOOLEAN		Lea_Pt2() ;		/* 8088-PT2.C	*/
BOOLEAN		Lea_87() ;		/* 8087.C	*/
BOOLEAN		Mov_Pt1() ;		/* 8088-PT1.C	*/
BOOLEAN		Mov_Pt2() ;		/* 8088-PT2.C	*/
BOOLEAN		Mov_188() ;		/* 80188.C	*/
BOOLEAN		Mov_87() ;		/* 8087.C	*/
BOOLEAN		Preserve() ;		/* 8088-PT3.C	*/
BOOLEAN		Remove_Entry() ;	/* LABELS.C	*/
BOOLEAN		Shift_188() ;		/* 80188.C	*/
BOOLEAN		Shl_Op() ;		/* 8088-PT2.C	*/
BOOLEAN		Simplify();		/* 8088-PT1.C	*/
BOOLEAN		Sub_Pt1() ;		/* 8088-PT1.C	*/
BOOLEAN		Sub_Pt2() ;		/* 8088-PT2.C	*/
BOOLEAN		Unnecessary() ;		/* GATHER.C	*/
BOOLEAN		Xor_Pt1() ;		/* 8088-PT1.C	*/
BOOLEAN		Xor_87() ;		/* 8087.C	*/
VOID		Zero_One() ;		/* 8087.C	*/
BOOLEAN		Invalidates() ;		/* UTILS.C	*/

#endif	/* NOT_TOO_MANY_PUBLICS */

BOOLEAN		Add_Op() ;		/* GATHER.C	*/
BOOLEAN		And_Op() ;		/* 8088-PT1.C	*/
REG		*Bad_Ref() ;		/* O88.C	*/
BOOLEAN		Call_Op() ;		/* GATHER.C	*/
BOOLEAN		Cld() ;			/* 8088-PT1.C	*/
BOOLEAN		Clean_Ret() ;		/* CLEANUP.C	*/
BOOLEAN		Cmp_Op() ;		/* 8088-PT1.C	*/
BOOLEAN		Cwd_Op() ;		/* GATHER.C	*/
BOOLEAN		Dec_Op() ;		/* GATHER.C	*/
BOOLEAN		Div_Op() ;		/* GATHER.C	*/
BOOLEAN		False() ;		/* ASMLIB.C	*/
BOOLEAN		Fild() ;		/* 8087.C	*/
BOOLEAN		Fld() ;			/* 8087.C	*/
BOOLEAN		Fst() ;			/* 8087.C	*/
BOOLEAN		Fstp() ;		/* 8087.C	*/
BOOLEAN		Fistp() ;		/* 8087.C	*/
BOOLEAN		Fstsw() ;		/* 8087.C	*/
BOOLEAN		IMul() ;		/* 8088-PT1.C	*/
BOOLEAN		Inc_Op() ;		/* GATHER.C	*/
BOOLEAN		Jump() ;		/* LABELS.C	*/
BOOLEAN		Lea_Op() ;		/* GATHER.C	*/
BOOLEAN		Les() ;			/* 8088-PT1.C	*/
BOOLEAN		Mov_Op() ;		/* GATHER.C	*/
BOOLEAN		Movs_Op() ;		/* GATHER.C	*/
BOOLEAN		Mul_Op() ;		/* 8088-PT2.C	*/
BOOLEAN		Or_Op() ;		/* 8088-PT1.C	*/
BOOLEAN		Pop() ;			/* 8088-PT2.C	*/
BOOLEAN		Push_Op() ;		/* 80188.C	*/
BOOLEAN		Rep_Op() ;		/* GATHER.C	*/
BOOLEAN		Sahf() ;		/* 8087.C	*/
BOOLEAN		Shift() ;		/* GATHER.C	*/
BOOLEAN		Stos_Op() ;		/* GATHER.C	*/
BOOLEAN		Sub_Op() ;		/* GATHER.C	*/
BOOLEAN		Test_Op() ;		/* 8088-PT1.C	*/
BOOLEAN		True() ;		/* ASMLIB.C	*/
BOOLEAN		Xchg_Op() ;		/* GATHER.C	*/
BOOLEAN		Xor_Op() ;		/* GATHER.C	*/
LINKLIST	*LL_Free() ;		/* LINKLIST.C	*/
LINKLIST	*LL_Search() ;		/* LINKLIST.C	*/
PARTS		*Emit() ;		/* PARSE.C	*/
PARTS		*Find_Next() ;		/* UTILS.C	*/
PARTS		*Get_Next() ;		/* UTILS.C	*/
PARTS		*Reg_Needed() ;		/* 8088-PT2.C	*/
REG		*Find_Free() ;		/* 8088-PT3.C	*/
REG		*Find_Val() ;		/* 8088-PT1.C	*/
CHAR		*Allocate() ;		/* ALLOCATE.C	*/
CHAR		*Append() ;		/* ASMLIB.C	*/
CHAR		*Convert() ;		/* ASMLIB.C	*/
CHAR		*Find() ;		/* ASMLIB.C	*/
CHAR		*Get_Env_Str() ;	/* UTILS.C	*/
CHAR		*Insert_String() ;	/* 8088-PT1.C	*/
CHAR		*Leading() ;		/* ASMLIB.C	*/
CHAR		*Make_Filename() ;	/* UTILS.C	*/
CHAR		*Make_String() ;	/* PARSE.C	*/
CHAR		*Skip_Prefix() ;	/* PARSE.C	*/
CHAR		*Word_To_Byte() ;	/* 8088-PT1.C	*/

#endif	/* _lint */

#ifdef DATA

unsigned exit_code = PERFECT ;

unsigned o88_labels = 0 ;

CHAR	ax[] = "AX" ;
CHAR	dx[] = "DX" ;

CHAR	mov[] = "MOV" ;
CHAR	lea[] = "LEA" ;
CHAR	add[] = "ADD" ;
CHAR	and[] = "AND" ;
CHAR	or[] = "OR" ;
CHAR	xor[] = "XOR" ;
CHAR	sub[] = "SUB" ;
CHAR	test[] = "TEST" ;
CHAR	inc[] = "INC" ;
CHAR	dec[] = "DEC" ;
CHAR	cmp[] = "CMP" ;
CHAR	shl[] = "SHL" ;
CHAR	cwd[] = "CWD" ;
CHAR	ret[] = "_RET" ;
CHAR	call[] = "_CALL" ;
CHAR	push[] = "PUSH" ;

/* NOTE: All "OPCODE" tables are ordered by frequency of reference so	*/
/*	 that they are inserted into the hash table in such a manner	*/
/*	 that search time (after collision) is minimal.			*/

OPCODE opc88[] =
	{
	{ "LINE",	0,					True}
	{ mov,		MDFYDST|RSWR|LOADS|MOVABLE,		Mov_Op},
	{ "PUSH",	RDWR,					Push_Op},
	{ cmp,		RDWR|RSWR|DEF_FLG,			Cmp_Op},
	{ add,		COMMUTE|MDFYDST|RSWR|DEF_FLG|MOVABLE,	Add_Op},
	{ call,		BAD_FLG,				Call_Op},
	{ "JZ",		CONDJMP|ZEROTST,			Jump},
	{ "JNZ",	CONDJMP|ZEROTST,			Jump},
	{ "POP",	MDFYDST|LOADS,				Pop},
	{ ret,		0,					Clean_Ret},
	{ "ADC",	COMMUTE|MDFYDST|RSWR|DEF_FLG,		False},
	{ test,		COMMUTE|RDWR|RSWR|ZRO_FLG,		Test_Op},
	{ "JMP",	0,					Jump},
	{ or,		COMMUTE|MDFYDST|RSWR|ZRO_FLG|MOVABLE,	Or_Op},
	{ and,		COMMUTE|MDFYDST|RSWR|ZRO_FLG|MOVABLE,	And_Op},
	{ xor,		COMMUTE|MDFYDST|RSWR|ZRO_FLG|MOVABLE,	Xor_Op},
	{ lea,		MDFYDST|LOADS,				Lea_Op},
	{ inc,		MDFYDST|ZRO_FLG|MOVABLE,		Inc_Op},
	{ sub,		MDFYDST|RSWR|DEF_FLG|MOVABLE,		Sub_Op},
	{ shl,		MDFYDST|ZRO_FLG|MOVABLE,		Shift},
	{ "IMUL",	MDFYACC|RDWR|BAD_FLG,			IMul},
	{ "MUL",	MDFYACC|RDWR|BAD_FLG,			Mul_Op},
	{ "JAE",	CONDJMP,				Jump},
	{ dec,		MDFYDST|ZRO_FLG|MOVABLE,		Dec_Op},
	{ "DIV",	MDFYACC|RDWR|BAD_FLG,			Div_Op},
	{ "IDIV",	MDFYACC|RDWR|BAD_FLG,			Div_Op},
	{ cwd,		MDFY_DX,				Cwd_Op},
	{ "JA",		CONDJMP,				Jump},
	{ "JB",		CONDJMP,				Jump},
	{ "JBE",	CONDJMP,				Jump},
	{ "JG",		CONDJMP|SGNDTST,			Jump},
	{ "JGE",	CONDJMP|SGNDTST,			Jump},
	{ "JL",		CONDJMP|SGNDTST,			Jump},
	{ "JLE",	CONDJMP|SGNDTST,			Jump},
	{ "IN",		MDFYDST|LOADS,				False},
	{ "NEG",	MDFYDST|DEF_FLG|MOVABLE,		False},
	{ "NOT",	MDFYDST|MOVABLE,			False},
	{ "OUT",	0,					True},
	{ "SAR",	MDFYDST|ZRO_FLG|MOVABLE,		Shift},
	{ "SBB",	MDFYDST|RSWR|DEF_FLG,			False},
	{ "SHR",	MDFYDST|ZRO_FLG|MOVABLE,		Shift},
	{ "XCHG",	COMMUTE|MDFYSRC|MDFYDST,		Xchg_Op},
	{ "CMC",	0,					True},
	{ "SAHF",	DEF_FLG,				Sahf},
	{ "REP",	BAD_FLG,				Rep_Op},
	{ "CLD",	0,					Cld},
	{ "MOVSB",	0,					Movs_Op},
	{ "MOVSW",	0,					Movs_Op},
	{ "STOSB",	0,					Stos_Op},
	{ "STOSW",	0,					Stos_Op},
	{ "LES",	MDFYDST|LOADS|MOVABLE,			Les},
	{ "ROL",	MDFYDST|MOVABLE,			Shift},
	{ "ROR",	MDFYDST|MOVABLE,			Shift}
	} ;

OPCODE opc188[] =
	{
	{ "ENTER",	0,					True},
	{ "LEAVE",	0,					True},
	{ "IMULI",	MDFYDST|RSWR|BAD_FLG,			False}
	} ;

OPCODE opc87[] =
	{
	{ "FLD",	0,					Fld},
	{ "FSTP",	MDFYDST,				Fstp},
	{ "FST",	MDFYDST,				Fst},
	{ "FADD",	MEMOP87,				True},
	{ "FSUB",	MEMOP87,				True},
	{ "FMUL",	MEMOP87,				True},
	{ "FDIV",	MEMOP87,				True},
	{ "FCHS",	0,					True},
	{ "FLDZ",	0,					True},
	{ "FLD1",	0,					True},
	{ "FWAIT",	0,					True},
	{ "FCOMPP",	0,					True},
	{ "FCOMP",	0,					True},
	{ "FCOM",	0,					True},
	{ "FICOMP",	0,					True},
	{ "FICOM",	0,					True},
	{ "FSTSW",	MDFYDST,				Fstsw},
	{ "FXCH",	0,					True},
	{ "FILD",	0,					Fild},
	{ "FABS",	0,					True},
	{ "FISTP",	MDFYDST,				Fistp},
	{ "FIADD",	0,					True},
	{ "FISUB",	0,					True},
	{ "FIMUL",	0,					True},
	{ "FIDIV",	0,					True},
	{ "FTST",	0,					True},
	{ "FLDCW",	0,					True},
	{ "FSTCW",	MDFYDST,				True},
	{ "FINIT",	0,					True},
	{ "FLDPI",	0,					True},
	{ "FLDL2T",	0,					True},
	{ "FLDL2E",	0,					True},
	{ "FLDLG2",	0,					True},
	{ "FLDLN2",	0,					True},
	{ "FRNDINT",	0,					True},
	{ "FSQRT",	0,					True},
	{ "FYL2X",	0,					True},
	{ "F2XM1",	0,					True}
	} ;

REG reg[] =
	{
	{"AH",		_BYTE|_ACCUM|_EMPTY,	0,	0},
	{"AL",		_BYTE|_ACCUM|_EMPTY,	0,	0},
	{ax,		_ACCUM|_EMPTY,		0,	0},
	{"SI",		_INDEX|_EMPTY,		0,	0},
	{"DI",		_INDEX|_EMPTY,		0,	0},
	{"CH",		_BYTE|_EMPTY,		0,	0},
	{"CL",		_BYTE|_EMPTY,		0,	0},
	{"CX",		_EMPTY,			0,	0},
	{"BH",		_BYTE|_EMPTY,		0,	0},
	{"BL",		_BYTE|_EMPTY,		0,	0},
	{"BX",		_INDEX|_EMPTY,		0,	0},
	{"DH",		_BYTE|_ACCUM|_EMPTY,	0,	0},
	{"DL",		_BYTE|_ACCUM|_EMPTY,	0,	0},
	{dx,		_ACCUM|_EMPTY,		0,	0},
	{"ES",		_SEGREG|_UNKN,		0,	0},
	{"SP",		_UNKN,			0,	0},
	{"BP",		_UNKN,			0,	0},
	{"DS",		_SEGREG|_UNKN,		0,	0},
	{"SS",		_SEGREG|_UNKN,		0,	0},
	{"CS",		_SEGREG|_UNKN,		0,	0}
	} ;

CHAR *src_file = "STDIN" ;
CHAR *dst_file = "STDOUT" ;
BUFFER inp_bfr ;
BUFFER code_bfr ;
BUFFER cs_pub_bfr ;
BUFFER data_bfr ;
BUFFER temp_bfr ;
int switch_stmt = 0 ;
CHAR cmp_val[80] ;
FLAGS cmp_att ;
unsigned cmp_len ;
PARTS *stmt1, *stmt2, *stmt3, *stmt4, *stmt5 ;
OPCODE *opc1 ;
OPERAND *dop1, *sop1 ;
CHAR *dorig1, *sorig1, *table[256] ;
unsigned dlen1, slen1, err_code ;
FLAGS dattb1, sattb1, opc1_attb ;
REG *jmp_reg, *dreg1, *sreg1, *cmp_reg = 0 ;
STATS stats ;
CHAR *clean_stack = NULL ;

#define	ENABLED		TRUE
#define	DISABLED	FALSE

OPTION option =
	{	/* defaults are ...		*/

	{'A',	DISABLED,	/* maximize */
	"ALLOCATE maximum buffers (may overlay COMMAND.COM)",
	"Do NOT allocate maximum buffers (preserve COMMAND.COM)"
	},

	{'B',	DISABLED,	/* big_model */
	"Optimize code for the BIG (large) memory model",
	"Do NOT optimize for the big (large) memory model"
	},

	{'C',	DISABLED,	/* comments */
	"Add COMMENTS where optimizations are made",
	"Do NOT add optimization comments to the output"
	},

	{'E',	DISABLED,	/* expand */
	"EXPAND non-f.p. library function calls in-line",
	"Do NOT expand (non-f.p.) library function calls"
	},

	{'F',	DISABLED,	/* fold */
	"FOLD duplicate constants in data segment",
	"Do NOT fold duplicate constants in dseg"
	},

	{'H',	DISABLED,	/* call_help */
	"Add CALL _HELP_ at entrance of every function",
	"Do NOT add CALL _HELP_ at function entrance"
	},

	{'I',	ENABLED,	/* init8087 */
	"INITIALIZE the 8087/80287 with FINIT at main()",
	"Do NOT initialize the 8087/80287 with FINIT"
	},

	{'J',	DISABLED,	/* jump */
	"Collapse JMP chains (Remove JMPs to JMPs)",
	"Do NOT collapse JMP chains (JMPs to JMPs)"
	},

	{'M',	DISABLED,	/* move */
	"Replace CALL _MOVE/_LMOVE by inline REP MOVS",
	"Do NOT replace CALL _MOVE/_LMOVE by REP MOVS"
	},

	{'O',	ENABLED,	/* optimize */
	"OPTIMIZE! - the code will be modified",
	"Do NOT optimize - code will NOT be modified"
	},

	{'P',	DISABLED,	/* pointers */
	"Assume that POINTERS never point to themselves",
	"Do NOT assume that pointers never point to themselves"
	},

	{'R',	DISABLED,	/* redundant */
	"Remove REDUNDANT code (O88 runs very slowly!)",
	"Do NOT remove redundant code (O88 runs faster)"
	},

	{'S',	DISABLED,	/* stack */
	"Clean STACK before function return",
	"Do NOT clean stack at function return"
	},

	{'T',	DISABLED,	/* time */
	"Optimize for TIME (as opposed to space)",
	"Do NOT optimize for time (optimize for space)"
	},

	{'1',	DISABLED,	/* superset */
	"Replace with 80188/186 instructions where possible",
	"Do NOT use 80188/186 instructions - only 8088/8086"
	},

	{'7',	DISABLED,	/* numeric */
	"Replace f.p. CALLs by in-line 8087/80287 code",
	"Do NOT replace floating-point library CALLs"
	}

	} ;

CHAR reduced[]		=	";   Strength reduction applied -->" ;
CHAR smaller[]		=	";       Size reduction applied -->" ;
CHAR memory[]		=	";  Memory reference eliminated -->" ;
CHAR deadcode[]		=	";         Dead code eliminated -->" ;
CHAR redundant[]	=	";        Redundant, eliminated -->" ;
CHAR deleted[] 		=	";         Unnecessary, deleted -->" ;
CHAR absorbed[]		=	";         Instruction absorbed -->" ;
CHAR combined[] 	=	";        Instructions combined -->" ;
CHAR replaced[]		=	";         Instruction replaced -->" ;
CHAR simplified[] 	=	";       Instruction simplified -->" ;
CHAR inserted[]		=	"; Initialization code inserted -->" ;
CHAR adjusted[]		=	";      Address offset adjusted -->" ;
CHAR substitute[]	=	";         Register substituted -->" ;
CHAR prefetch[]		=	";  Prefetch queue optimization -->" ;
CHAR preserved[]	=	";     Index register preserved -->" ;
CHAR postponed[]	=	";        Instruction postponed -->" ;
CHAR new_jmp_dst[]	=	";    Jump destination replaced -->" ;
CHAR cleaned[]		=	";    Function now cleans stack -->" ;
CHAR conflict[]		=	";      Stack clean-up conflict -->" ;
CHAR undefined[]	=	";  Reg [??] contents undefined -->" ;
CHAR folded[]		=	";     Duplicate string or real -->" ;
CHAR arguments[]	=	"; Wrong number or size of args -->" ;

REG *wregs[] =
	{
	AX,	/* AH */
	AX,	/* AL */
	AX,	/* AX */
	SI,	/* SI */
	DI,	/* DI */
	CX,	/* CH */
	CX,	/* CL */
	CX,	/* CX */
	BX,	/* BH */
	BX,	/* BL */
	BX,	/* BX */
	DX,	/* DH */
	DX,	/* DL */
	DX,	/* DX */
	ES,	/* ES */
	SP,	/* SP */
	BP,	/* BP */
	DS,	/* DS */
	SS,	/* SS */
	CS	/* CS */
	} ;	

FLAGS refflgs[] =
	{
	_REFAX,	/* AH */
	_REFAX,	/* AL */
	_REFAX,
	_REFSI,
	_REFDI,
	_REFCX,	/* CH */
	_REFCX,	/* CL */
	_REFCX,
	_REFBX,	/* BH */
	_REFBX,	/* BL */
	_REFBX,
	_REFDX,	/* DH */
	_REFDX,	/* DL */
	_REFDX,
	_REFES,
	0,	/* SP */
	0,	/* BP */
	0,	/* DS */
	0,	/* SS */
	0,	/* CS */
	} ;

FLAGS defflgs[] =
	{
	_DEFAX,	/* AH */
	_DEFAX,	/* AL */
	_DEFAX,
	_DEFSI,
	_DEFDI,
	_DEFCX,	/* CH */
	_DEFCX,	/* CL */
	_DEFCX,
	_DEFBX,	/* BH */
	_DEFBX,	/* BL */
	_DEFBX,
	_DEFDX,	/* DH */
	_DEFDX,	/* DL */
	_DEFDX,
	_DEFES,
	0,	/* SP */
	0,	/* BP */
	0,	/* DS */
	0,	/* SS */
	0,	/* CS */
	} ;

FLAGS regflgs[] =
	{
	_AH,
	_AL,
	_AX,
	_SI,
	_DI,
	_CH,
	_CL,
	_CX,
	_BH,
	_BL,
	_BX,
	_DH,
	_DL,
	_DX,
	_ES,	/* ES */
	0,	/* SP */
	0,	/* BP */
	0,	/* DS */
	0,	/* SS */
	0	/* CS */
	} ;

FLAGS dpndflgs[] =
	{
	0,	/* AH */
	0,	/* AL */
	0,	/* AX */
	_DEPSI,	/* SI */
	_DEPDI,	/* DI */
	0,	/* CH */
	0,	/* CL */
	0,	/* CX */
	0,	/* BH */
	0,	/* BL */
	_DEPBX,	/* BX */
	0,	/* DH */
	0,	/* DL */
	0,	/* DX */
	_DEPES,	/* ES */
	0,	/* SP */
	_DEPBP,	/* BP */
	0,	/* DS */
	0,	/* SS */
	0	/* CS */
	} ;

int	id = 0 ;

CHAR	null[] = "" ;

CHAR	zero[] = "0" ;
CHAR	one[] = "1" ;

CHAR	offset[] = "OFFSET " ;
CHAR	byte[] = "BYTE " ;
CHAR	word[] = "WORD " ;

unsigned dos_size ;	/* Available DOS memory in paragraphs */
unsigned dos_base ;	/* Next available paragraph */

FLAGS flg_sts = 0 ;
REG *flg_reg = NULL ;

PRESERVE *states = NULL ;
BOOLEAN cld_set = FALSE ;

#else	/* DATA */

PUBLIC unsigned exit_code ;

PUBLIC unsigned o88_labels ;

PUBLIC	CHAR	null[] ;
PUBLIC	CHAR	*clean_stack ;
PUBLIC	CHAR	zero[] ;
PUBLIC	CHAR	one[] ;

PUBLIC	CHAR	offset[] ;
PUBLIC	CHAR	byte[] ;
PUBLIC	CHAR	word[] ;

PUBLIC	CHAR	mov[] ;
PUBLIC	CHAR	lea[] ;
PUBLIC	CHAR	add[] ;
PUBLIC	CHAR	and[] ;
PUBLIC	CHAR	or[] ;
PUBLIC	CHAR	xor[] ;
PUBLIC	CHAR	sub[] ;
PUBLIC	CHAR	test[] ;
PUBLIC	CHAR	inc[] ;
PUBLIC	CHAR	dec[] ;
PUBLIC	CHAR	cmp[] ;
PUBLIC	CHAR	shl[] ;
PUBLIC	CHAR	cwd[] ;
PUBLIC	CHAR	ret[] ;
PUBLIC	CHAR	call[] ;
PUBLIC	CHAR	push[] ;

PUBLIC	CHAR	ax[] ;
PUBLIC	CHAR	dx[] ;

PUBLIC int id ;
PUBLIC CHAR *src_file ;
PUBLIC CHAR *dst_file ;
PUBLIC BUFFER inp_bfr ;
PUBLIC BUFFER code_bfr ;
PUBLIC BUFFER cs_pub_bfr ;
PUBLIC BUFFER data_bfr ;
PUBLIC BUFFER temp_bfr ;
PUBLIC int switch_stmt ;
PUBLIC CHAR cmp_val[] ;
PUBLIC FLAGS cmp_att ;
PUBLIC unsigned cmp_len ;
PUBLIC OPCODE opc88[] ;
PUBLIC OPCODE opc87[] ;
PUBLIC OPCODE opc188[] ;
PUBLIC REG reg[] ;
PUBLIC PARTS *stmt1, *stmt2, *stmt3, *stmt4, *stmt5 ;
PUBLIC OPCODE *opc1 ;
PUBLIC OPERAND *dop1, *sop1 ;
PUBLIC CHAR *dorig1, *sorig1, *table[] ;
PUBLIC unsigned dlen1, slen1, err_code ;
PUBLIC FLAGS dattb1, sattb1, opc1_attb ;
PUBLIC REG *jmp_reg, *dreg1, *sreg1, *cmp_reg ;
PUBLIC STATS stats ;
PUBLIC OPTION option ;
PUBLIC CHAR reduced[] ;
PUBLIC CHAR smaller[] ;
PUBLIC CHAR memory[] ;
PUBLIC CHAR deadcode[] ;
PUBLIC CHAR redundant[] ;
PUBLIC CHAR deleted[] ;
PUBLIC CHAR absorbed[] ;
PUBLIC CHAR combined[] ;
PUBLIC CHAR replaced[] ;
PUBLIC CHAR simplified[] ;
PUBLIC CHAR inserted[] ;
PUBLIC CHAR adjusted[] ;
PUBLIC CHAR substitute[] ;
PUBLIC CHAR prefetch[] ;
PUBLIC CHAR preserved[] ;
PUBLIC CHAR postponed[] ;
PUBLIC CHAR new_jmp_dst[] ;
PUBLIC CHAR cleaned[] ;
PUBLIC CHAR conflict[] ;
PUBLIC CHAR undefined[] ;
PUBLIC CHAR folded[] ;
PUBLIC CHAR arguments[] ;
PUBLIC REG *wregs[] ;
PUBLIC FLAGS dpndflgs[] ;
PUBLIC FLAGS regflgs[] ;
PUBLIC FLAGS defflgs[] ;
PUBLIC FLAGS refflgs[] ;
PUBLIC unsigned dos_size ;
PUBLIC unsigned dos_base ;
PUBLIC FLAGS flg_sts ;
PUBLIC REG *flg_reg ;
PUBLIC PRESERVE *states ;
PUBLIC BOOLEAN cld_set ;

#endif	/* DATA */

		      /*  End of file O88.H  */
