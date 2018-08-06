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
/*	PASS1.H			INCLUDE FOR PASS1	*/

#define	CHECK	1			/*	true if generating checkout compiler */
#define LIMITED 0

/*	LTYPE VALUES	*/

#define LETTER 1
#define DIGIT 2
#define SPACE 3

#define	DEFCPY	251
#define	DEFPAS	252
#define	DEFSFY	253
#define DEFSTR	254
#define DEFEND	255

/*	symbol table types	*/

#define RESERVED	1		/* BYTE CONTAINING RESERVED VALUE */
	struct {char rescl,rvalue;};

#define STAG		2		/* forward chain, size of structure */
	struct {char stagcl; int schain,staglen;};

#define DEFINED		3		/* #args, (arg # or DEFSTR chars... LF)
								...DEFEND	*/
	struct {char defcl, *dchain, dnlen, dargs, dval;};

#define DEFPARM		4		/* number of argument	*/
	struct {char dpcl,dpnum;};

#define OPERAND		26		/* op class, forward chain of members,
							 length of name, storage, offset or ordinal */
	struct {char opcl, *nchain, nlen, nstor; int noff; char ntype[1];};


/*	named heir values */

#define CONSTANT	27
#define	LCONSTANT	28
#define FFCONSTANT	29
#define FDCONSTANT	30
#define STRNG		31
#define UNDEF		99
#define	STREF		100

/*	storage classes. in nstor field.	*/

#define SEXTONLY	0
#define SSTATIC		1
#define SEXTERN		2
#define SUNSTOR		3
#define SMEMBER		4
#define SAUTO		5
#define SPARM		6
#define STYPEDEF	7

#define LF			10
#define	CR			13
#define CONTZ		26

/*	the reserved words */
/*	v3.1 03/08/88 -	resorted reserved word in ANSI order */

#define RAUTO		1
#define RBREAK		2
#define RCASE		3
#define RCHAR		4
#define RCONST		5
#define RCONTINUE	6
#define RDEFAULT	7
#define RDO			8
#define	RDOUBLE		9
#define RELSE		10
#define RENUM		11
#define REXTERN		12
#define RFLOAT		13
#define RFOR		14
#define RGOTO		15
#define RIF			16
#define RINT		17
#define RLONG		18
#define	RNOALIAS	19
#define RREGISTER	20
#define RRETURN		21
#define RSHORT		22
#define RSIGNED		23
#define RSIZEOF		24
#define RSTATIC		25
#define RSTRUCT		26
#define RSWITCH		27
#define RTYPEDEF	28
#define RUNION		29
#define RUNSIGNED	30
#define RVOID		31
#define RVOLATILE	32
#define RWHILE		33

/* predefined macro names */

#define	R_LINE		34
#define	R_FILE		35
#define	R_DATE		36
#define	R_TIME		37
#define	R_STDC		38

/* extended keywords + MSC keywords */

#define RASM		39
#define	RCDECL		40
#define	RFAR		41
#define	RFORTRAN	42
#define	RHUGE		43
#define	RINTERRUPT	44
#define	RNEAR		45
#define	RPASCAL		46
#define	RDOTDOT		47
#define RELIPSE		48

struct savearea{
	char _name[65];
	char *_cur,*_lineBeg,*_lineEnd,*_bufEnd,*_eofptr;
	unsigned _cline, _lineinc,_file;
	char _lineBuf[512],_fileBuf[2048];
	};

#define	SAVSIZE	sizeof(struct savearea)

char name[65];
char *lineBeg,*lineEnd,*cur,*bufEnd,*eofptr;
unsigned cline, lineinc, file;
char lineBuf[512],fileBuf[2048],savEnd[2];

char macExp[2048],macEnd[1],*macxp;

char trigraphs;
unsigned lastline;

	unsigned tree,control,ntree,*intree;
	unsigned maxmem,minmem,hiwater;
	int  last,retnum;
	int  *wp,findover,nerr,nwarn,locoff,locplus;
	int  ordinal;
	int  pbytes,wvalue;
	char curch,*tokat,heir,bvalue,*funname,*original,*mfree,*hash[33];
	char c_type[1],int_type[1],u_type[1],l_type[1],ff_type[1],fd_type[1],p_type[2];
	char treebuf[512],ctlbuf[512];
	char string[257],nested,*bptr,i,j,ininit,was_ext;
	char *addat,*addtype,addstor,*addloc,*addparm,*addproto,hashno,defhash,laststmt;
	char *inctl,*initnode,*nameat,in_stru,funtype,longconst;
	char is_big,eopt,istring[40],sopt,soopt,topt,xopt,wopt,have_asm,see_exit;
	long dvalue, constexp();
	unsigned fvalue[4],dsize(),fun_ret_len;
	int  objnum;
	char *inobj,newline,aopt,copt,objname[65];
	extern char ltype[128];
#if	CHECK
	char objbuf[512];
#endif

#define MAXINC	20
	unsigned incPara[MAXINC];
	char incnext;

#define MAXNEST 60
	int  nestfrom[MAXNEST];

	struct stag {char cstype, *stat; int staglen;} stagat;
	struct {char sttype; struct stag *ststagat;} *struct_type;

	union  { char byte; int word; } ;
	struct { unsigned loword,hiword; } ;

char mdate[12], mtime[9], mactokn, *macname[MAXNEST], *macfrom[MAXNEST],
	*machash[32], macOp, blevel, *anon;
char *protohash[32], *labhash[32], xkwd, __stdc;

char *lastname, *macproto;

struct pargs { struct pargs * plink; char ptype[1]; };

#define skipl() cur=lineEnd-1

#ifdef MacHiWater

char * MacHiWaterP, maxNest;

#endif

