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
#define	OPT		0		/*	on if want optimize code	*/
#define DEBUG	1		/* on if want debug code		*/
#define	CHECK	1		/* on if want checkout option	*/
#define LIMITED	0		/* on for limited version		*/

/*	vtype contains an operand	*/

#define CONSTV	0
#define OFFV	1
#define REGV	2
#define SEGRV	3
#define VARV	4
#define FLOATV	5

/*	the fields of vtype */

#define VIS		0
#define VVAL	1
#define VNAME	2
#define VOFF	3
#define VMORE	4
#define VT		5
#define VFROM	6

/*	vtype[VT] is CCHAR, CINT, CUNSG, CLONG, BITS+, CFLOAT, CDOUBLE, 
		OR CLABEL.	*/
/*	vtype[VSEG] (VARV only) is ES,CS,SS,DS or -1 (default) prefix.	*/
/*

vtype[VIS]	[VVAL]		[VNAME]		[VOFF]	   [VMORE]

CONSTV		value					constant. value follows in as many words
									as needed for the type.
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
FLOATV								the floating value is on the float stack.

for the BIG model and the REGV, VARV and OFFV VIS types, the VMORE field is
used to indicate the segment or segment register:
VMORE:
	-1		normal
	0-7		AX thru DI holds the ES value
	8		ES register
	9		DS register
	10		SS register
	11		CS register
	12		will need to use ES
*/
#define NEED_ES		12
#define CFOREIGN	251
#define CSEGMENT	252


/*	registers	*/

#define AX	0
#define CX	1
#define DX	2
#define BX	3
#define SP	4
#define BP	5
#define SI	6
#define DI	7
#define ES	8
#define DS	9
#define	SS	10
#define CS	11

#define AL	0
#define CL	1
#define DL	2
#define BL	3
#define	AH	4
#define	CH	5
#define	DH	6
#define	BH	7

/*	paterns for register clases */

#define ESPAT		256
#define ANYPAT		207
#define NOTCXPAT	205
#define INDPAT		192
#define SIPAT		64
#define DIPAT		128
#define BYTEPAT		15
#define BXPAT		8
#define DXPAT		4
#define AXCXPAT		3
#define CXPAT		2
#define AXPAT		1

/*	names of the builtin calls	*/

#define _SWITCH		0
#define _CMP4		1
#define _SHR4		2
#define _SHL4		3
#define _MUL4		4
#define _DIV4		5
#define _MOD4		6
#define _FLOADD		7
#define _FLOADE		8
#define _FLOADL		9
#define _FSTORED	10
#define _FSTOREE	11
#define _FSTOREL	12
#define _FADD		13
#define _FSUB		14
#define _FMUL		15
#define _FDIV		16
#define _FCMP		17
#define _FNEG		18
#define _FIS		19
#define _FNOT		20
#define _FDEC		21
#define _FINC		22
#define _FPUSH		23
#define _FXCH		24
#define _FCLEAR		25
#define _MOVE		26


/* globals for pass2 of Medium C Compiler  */

char aopt,copt,eopt,mopt,sopt,topt,xopt,zopt,subn[64],*nextext;
char optype[64],commut[64],flipped[64],prefer_si,in_post;
char regpat[12],numswit,asmname[65],objname[65],nopre,nopost,isasgn;
int  nodesin,nextlab,lastlab,numlit;
int  funord,treelen,pstart,pbytes,level;
#if DEBUG
int  namet[8],namen[64];
#endif
int jumps[11],regname[12],reglow[4],reghigh[4];
int  *regat[13],bltname[27],numfloat;
char regused[9],is_big,bltused[27];
unsigned util,locoff,curoff,line_num;

#define MAXFOR	16
int  numwhil,whilcont[MAXFOR],whilbrk[MAXFOR],switnext[MAXFOR];
#define MAXCASE	128
int  swits[MAXCASE],switto[MAXCASE];
#define	BUFSIZE 512
int  treef,control,output;
int  *intree;
char *inctl,*inout,see_exit;
unsigned see_off,see_seg,see_ret,see_bp;
char see_msg[80],see_index;

int  ctlblk;
char treebuf[BUFSIZE],ctlbuf[BUFSIZE],outbuf[BUFSIZE];
extern char nextpgm[77];

#if OPT
#define NUMREC	10
/*	bit mask for reg_dirty	*/
#define USED	1
#define DIRTY	2
extern char perfect;
char better,reg_dirty[9];
int  reg_from[9],reg_weight[9];
unsigned down,recat[NUMREC],recent[NUMREC],weight[NUMREC];
#endif

#define TREEMAX	8000
int  tree[TREEMAX];

#define EXTMAX	5000
char ext[EXTMAX];

#define EXTATMAX 500
int  numext,asmext;
char *extat[EXTATMAX];

union {int word; char byte; } ;

#define SEXTONLY 0
#define SSTATIC	1
#define SEXTERN	2

#define CR 13
#define LF 10
