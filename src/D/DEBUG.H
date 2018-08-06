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
/*	D88.H  --	DEBUGGER FOR C88 PACKAGE	*/
#define OPTYPE		11	/* O		name of public, type. */
#define OLTYPE		12	/* O		name of local, type, offset from BP. */
#define OMTYPE		13	/* O		name of public, type, offset from start. */
#define ONAME		14	/* O		file name. file is now open.*/
#define OLNAME		15	/* O		name of function for locals, type. */
#define OLINE		16	/* PO		line number. also location in .CHK. */
#define OOV			17	/*			overley number. in .CHK only.	*/
#define OBIG		20	/*			large case.	*/

/*	for expressions, VTYPE describes a toked value. */

#define CONSTV	0
#define RMV		1
#define REGV	2
#define OPPV	3

/* vtype   offset   typep
CONSTV     value
RMV        offset
ILLV				*/

struct vstruct {int vtype;
		union {double vald; int vali; unsigned valu; long vall;} val;
		char *typep;};


/*	registers. must be in order.	*/
#define NUMREG 14
unsigned rax,rbx,rcx,rdx,rsi,rdi,rbp,rsp,rds,rss,res,rcs,rip,rfl;
extern char rname[14][3];
unsigned _pcb;
unsigned origds,origss,origcs,localss,highbp,int3off,int3seg,allocat;
char *curproc,*curname,listopen[65],lastcmd,col,lineno,flipped;
char line [120],*nextin,heir,dbuffer[40];
char *symbols,*lastsym,*listoff,*ovnum_at,ovnum,*needline;
unsigned top,curline,curoff,procloc,procend,procseg;
int  listnum,repeat,macronum;
char listbuf[2050],*strings,*nexts,mainflag,macro_line[4][80],*inmacro,*memat;
char flipok,goflip,needname,cmd,noinit,got_call,overley,*overley_at,is_big;
int scr_rows;
extern int listfil;
extern char int3,listname[65],golist;

#define MAXLOCAL	100
int numlocal;
char buffer[1024],*localat[MAXLOCAL];


extern struct {unsigned goff,gseg; char gbyte;}
	 brk[4];

struct { unsigned loc; char type [1];};
struct { unsigned zzloc,loc_seg; char big_type [1];};
struct { char lchar,llen; unsigned lline,lloc; char lbyte; };
struct { char bjunk[6]; unsigned lloc_seg; char big_lbyte; };

/*	LTYPE VALUES	*/

#define LETTER 1
#define DIGIT 2
#define SPACE 3
#define OTHER 4
#define EOL   5

extern char ltype[128];

char operator[5];

union  { char byte; int word; } ;
struct { unsigned loword,hiword; } ;

#define CCHAR		1
#define CINT		2
#define CUNSG		3
#define CLONG		4
#define CFLOAT		5
#define CDOUBLE		6
#define CLABEL		7
#define CSTRUCT		8

/*	THE FOLLOWING ARE THE TYPE MODIFIERS	*/
#define FUNCTION	253
#define ARRAY		254	/* FOLLOWED BY DIMENSION */
#define PTRTO		255

extern char i_type[];
extern char u_type[];
extern char l_type[];
extern char d_type[];
extern char c_type[];
extern char p_type[];
