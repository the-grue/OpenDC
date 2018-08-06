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
/*	ASM88.H		include file for ASM88	*/

#define DEBUG	0
#define	X8087	1
#define CHECK	1
#define LIMITED	0

/*	data types	*/

#define CCHAR	1
#define CINT	2
#define CLONG	3
#define	CQWORD	4
#define CTBYTE	5
#define CUNSG	6
#define CLABEL	7
#define CFOREIGN	8
#define CSEGMENT	9

/*	heir values for constant and double quoted string 	*/

#define CONSTANT	27
#define SSTRING		28
#define DSTRING		29

/*	vtype contains an operand	*/
#define CONSTV	0
#define OFFV	1
#define REGV	2
#define SEGRV	3
#define VARV	4
#define STV		5
#define	ILLV	6

/*	the fields of VTYPE	*/

#define VIS		0
#define VVAL	1
#define VNAME	2
#define VOFF	3
#define VT		4
#define VSEG	5


/*	vtype[VT] is CCHAR,CINT OR CLABEL.	*/
/*	vtype[VSEG] (VARV only) is ES,CS,SS,DS or -1 (default) prefix.	*/
/*

vtype[VIS]	[VVAL]		[VNAME]		[VOFF]

CONSTV		value		[vhigh]		constant.
OFFV								same as VARV except offset of.
REGV		value					byte or word register.
SEGRV		value					segment register.
VARV		8		name	value	name[value].
			0		 or				name[BX+SI+value].
			1		 0				name[BX+DI+value].
			2						[BP+SI+value].
			3						[BP+DI+value].
			4						name[SI+value].
			5						name[DI+value].
			6						[BP+value].
			7						name[BX+value].
STV			value					st or st(value).
ILLV								illegal.

/*	registers	*/

#define AX	0
#define CX	1
#define DX	2
#define BX	3
#define SP	4
#define BP	5
#define SI	6
#define DI	7

#define AL	0
#define CL	1
#define DL	2
#define BL	3
#define AH	4
#define CH	5
#define DH	6
#define BH	7

#define ES	0
#define CS	1
#define SS	2
#define DS	3

/*	reserved words	*/

#define RINCLUDE	1
#define RPUBLIC		2
#define REVEN		3
#define REQU		4
#define RRB			5
#define RRW			6
#define RDB			7
#define RDW			8
#define RDD			9
#define	RDQ			10
#define RBYTE		11
#define RWORD		12
#define RDWORD		13
#define RQWORD		14
#define RTBYTE		15
#define ROFFSET		16
#define RDSEG		17
#define RCSEG		18
#define RST			19
#define RTITLE		20
#define REJECT		21
#define RLINE		22
#define REND		23
#define RSPLIT		24
#define RESEG		25
#define RIF			26
#define RELSE		27
#define RENDIF		28
#define RSEG		29

/*	symbol table element classification	*/

struct	{char sheir; char sclass; char svalue; char stype; int snum;};

#define MNUM		1	/* mnumonic. sclass is type, svalue is element */
#define FMNUM		2	/* floating mnumonic. same as MNUM. */
#define PREFIX		3	/* prefix. svalue is byte for LOCK or REP or REPZ */
#define RESERVED	4	/* reserved word. sclass is number.	*/
#define SEGREG		5	/* segment register. sclass is number. */
#define REG			6	/* register. sclass is CCHAR,CINT.svalue is number.*/
#define EQU			7	/* equate. ends with LF. */
#define NAME		8	/* name. sclass is true if defined. svalue is segment.						stype is CCHAR or CINT or CLABEL. snum is ordinal.	*/

/*	character classes	*/

#define LETTER	1
#define DIGIT	2
#define SPACE	3

/*	character names	*/

#define	TAB		9
#define LF		10
#define CR		13
#define CONTZ	26

/*	operators	*/

#define ADD		0
#define SUB		1

#define MUL		0
#define DIV		1
#define MOD		2

#define SHR		0
#define SHL		1

#define EQ		0
#define NE		1
#define GE		2
#define LT		3
#define GT		4
#define LE		5

#define NEG		0
#define NOT		1

#define MODEL_PTRS	1
#define MODEL_DATA	2

/*	reserve some storage	*/

int  incfile,infile,obj,dummy,lst;
char *cur,*inobj,*inpipe,*indummy,*inlst;
char inbuf[600],savein[600],objbuf[512],pipe[2048],dbuf[2048],string[80];
char lstbuf[512],listbuf[8],title[80];

char i,j,eopt,mopt,sopt,copt,uopt,killopt,ltype[128],curseg,nest_cur,curch,bvalue;
int  ordinal,nerr,hash[128],nestdef[5],line,last,lastline,errline;
int  offs[5],loff[5],offsett,labels[160],laboff[160];
char *lastch,*lastlf,*lastinc,*wvalue,*hiword;
char heir,*codeat,*bptr,name[65],incname[65],objname[65],outname[65];
char contype[6],shortj[21],not_done,ndummy,nextlab,zflag;
char *tempname,*tempname6,*dummyat;
char lopt,lstname[20],is_big;
char *freem,*memend,*memover;
unsigned util,listnum,curcol;
extern unsigned maxcol,maxrow,currow,line_ordinal;

struct {char regis; char rmwas; char rmis;};
char rmto[36];

union {int word; char byte;};

char Xflag;		/* 186/v20 flag */
#define	X186	0x01	/* 186 opcodes allowed */
#define	XV20	0x80	/* v20 opcodes allowed */
