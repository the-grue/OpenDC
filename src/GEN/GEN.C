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
/* GEN.C   - pass 2 for c88	*/

#include "pass2.h"
#include "nodes.h"
#include "inst86.h"

char nextpgm[77]="Z:ASM88 Z:CTEMP4     U00 O";
char perfect=1;

main(argc,argv)
	int  argc,argv[]; {
	int	ctype;
	if (argc < 2) {
		os("missing file name");
		real_exit(2);
		}
	initopt(argc,argv);
	init();
	if (zopt) asm_init();
	while(1) {
		ctype=ctlbyte();
#if DEBUG
		if (eopt) {
			os("control = ");
			odbyte(ctype);
			olf();
			}
#endif
		switch (ctype) {
			case 0:	endit();

			case 1:	addext(0);
					break;
			case 2:	addfun();
					break;
			case 3:	nodesin=0;
					break;
			default:error("Bad Control File");
			}
		}
	}

init() {
	char i,j,k,*intname;

	nextlab=1000;				/* count __n labels from 1000	*/
	lastlab=1969;				/* count _F  labels down from 1969 */
	intname="A:CTEMP2";
	if (topt) *intname=topt;
	else intname+=2;
	if ((treef=open(intname,0))==-1) 
		error("Cannot Open CTEMP2");
	intree=&treebuf[BUFSIZE];

	intname="A:CTEMP1";
	if (topt) *intname=topt;
	else intname+=2;
	if ((control=open(intname,0))==-1)
		error("Cannot Open CTEMP1");
	if (read(control,ctlbuf,BUFSIZE) == -1)
		error("Cannot Read CTEMP1");
	inctl=ctlbuf;

	if (zopt == 0) {
		if (eopt == 0) {
			if ((output=creat(asmname)) == -1) {
				os("Cannot Create ");
				error(asmname);
				}
			inout=outbuf;
			}
		else {
			inout=0;
			os(asmname);
			olf();
			}
		}
	nextext=ext;
	numswit=-1;
	k=1;
	for (i=0; i <= 11; i++) { regat[i]=0; regpat[i]=k; k+=k; }
	for (i=0; i < 9; i++) /* relationship between vtype[VVAR] and used reg*/
		regused[i]=8;
	regused[2]=regused[4]=SI;
	regused[3]=regused[5]=DI;
	regused[7]=BX;

	/*	initilize the names of the builtin functions.	*/

	initst(bltname,"_SWITCH _CMP4 _SHR4 _SHL4 _MUL4 _DIV4 _MOD4 ");
	initst(&bltname[7],"_FLOADD _FLOADE _FLOADL _FSTORED _FSTOREE ");
	initst(&bltname[12],"_FSTOREL _FADD _FSUB _FMUL _FDIV _FCMP _FNEG ");
	initst(&bltname[19],"_FIS _FNOT _FDEC _FINC _FPUSH _FXCH _FCLEAR ");
	initst(&bltname[26],"_MOVE_ ");

#if DEBUG
	initch(namet,"CHAR  INT   UNSG  LONG  LABEL STRUCTBITS  ");
	initch(&namen[0],"OPND  CONST LAB   CAST  TA    IND   IFE   WHIL  ");
	initch(&namen[8],"DOW   SWIT  FORS  RET   GOTOS CONT  BRK   LST2  ");
	initch(&namen[16],"LST3  LST4  LST5  LST6  CAS   DFLT  CALL  TEST  ");
	initch(&namen[24],"ADD   SUB   MUL   DIV   MOD   SHR   SHL   AND   ");
	initch(&namen[32],"OR    XOR   LOR   LAND  EQ    NE    GE    LT    ");
	initch(&namen[40],"GT    LE    NEG   NOT   COMP  PREI  PRED  POSTI ");
	initch(&namen[48],"POSTD ASGN  AADD  ASUB  AMUL  ADIV  AMOD  ASHR  ");
	initch(&namen[56],"ASHL  AAND  AOR   AXOR  NULL  ASM   STMT  MOVE  ");
#endif

	initst(jumps,"JNZ JZ JL JGE JLE JG JB JAE JBE JA JMP ");

	/* initialize register names */
	initst(regname,"AX CX DX BX SP BP SI DI ES DS SS CS ");
	initst(reglow,"AL CL DL BL ");
	initst(reghigh,"AH CH DH BH ");

/*	operator types

	0	operand,  CAST, TA, IND
	1	statement
	2	function call
	3	test
	4	arithmetic
	5	shift
	6	binary
	7	logical
	8	comparison
	9	unary
	10	pre ++ or --
	11	post ++ or --
	12	assignment
	13	null
	14	list
	15	line number node
	16	structure assignment
*/

	initnum(optype,"001000111111111EEEEE112344444");
	initnum(subn  ,"00111132224110023456112322222");
	initnum(commut,"00000000000000000000000010100");

	initnum(&optype[29],"5566677888888999AABBCCCCCCCCCCCD1FG");
	initnum(&subn[29],  "22222222222221111111222222222220012");
	initnum(&commut[29],"00111001111110000000000000000000000");
	tree[0]=60;

	i=60;
	while (--i) flipped[i]=i;
	flipped[38]=41;flipped[41]=38;flipped[40]=39;flipped[39]=40;

	wantcseg();
	}


initch(ptrs,chs)
	int *ptrs;
	char *chs; {
	while (*chs) {
		*ptrs++=chs;
		chs=chs+6;
		}
	}
initst(target,str)
	int  *target;
	char *str; {
	while (*str) {
		*target++=str;
		while(*str != ' ') str++; 
		*str++=0;
		}
	}



initnum(target,fromi)
	char	*target,*fromi; {
	while (*fromi) {
		*target=*fromi++-'0';
		if (*target > 9) *target=*target-7;
		target++;
		}
	}


/*	get the command options. set nextpgm which is string to invoke asm88. */

/*	A	- leave .a file. do not invoke assembler.
	B	- BIG model.
	C	- checkout option.
	Dd	- d is drive from which to invoke the assembler.
    M   - Intel OMF's.
	Td	- d is the drive for temporaries.
	VE	- asks for debug information.
	VS	- asks for dump of the tree.
	X	- do not delete temporaries. do not invoke assembler.
*/

initopt(argc,argv)
	int  argc;
	char *argv[]; {
	char ch,*optat;
	static char dopt[4]=" DZ";
	int  narg;

	copt=0;
	strcpy(objname,argv[1]);

#if LIMITED == 0
	for (narg=2; narg < argc; narg++) {
		optat=argv[narg];

		while (ch=toupper(*optat++)) {
			switch (ch) {
				case '@':	see_addr(optat);
							*optat=0;
							break;
				case 'A':	aopt=1;
							break;
				case 'B':	nextpgm[17] = 'B';
							is_big=1;
							break;
				case 'C':	copt=1;
							break;
				case 'D':	nextpgm[0]=dopt[2]=toupper(*optat++);
							break;
				case 'M':	nextpgm[19]='M';
							mopt=1;
							break;
				case 'O':	strcpy(objname,optat);
							while (*++optat) ;
							break;
				case 'P':	*optat=0;
							break;
				case 'T':	topt=toupper(*optat++);
							break;
				case 'V':	while(*optat) switch(toupper(*optat++)) {
								case 'E':	aopt=eopt=xopt=1;
											break;
								case 'S':	aopt=sopt=xopt=1;
											break;
								}
							break;
				case 'X':	aopt=xopt=1;
							break;
				case 'Z':	zopt=1;
				}
			}
		}
#endif

	/*	create invocation line for assembler.	*/

	if (topt) nextpgm[8]=topt;
	else nextpgm[8]=nextpgm[9]=' ';

	strcpy(asmname,objname);
	strcat(objname,".O");
	strcat(nextpgm,objname);
	if (copt) strcat(nextpgm," C");
	if (dopt[2] != 'Z') strcat(nextpgm,dopt);
	strcat(asmname,".A");

	/*	set asmname to name of output file	*/

	if (aopt == 0) {
		if (topt) {
			asmname[0]=topt;
			asmname[1]=':';
			asmname[2]=0;
			}
		else asmname[0]=0;
		strcat(asmname,"CTEMP4");
		}
	}


endit() {
	int  i;
	unsigned util1,warn;
	char chr,*intname;

	if (eopt) olf();
	warn=ctlword();
	util1=ctlword();
	if (util1 > util) util=util1;
	if (zopt) asm_endit();
	else os(" END\n");
	close(treef);
	close(control);

/*	get rid of temporaties CTEMP1 and CTEMP2	*/

	intname="A:CTEMP1";
	if (topt) *intname=topt;
	else intname+=2;

	if (eopt == 0) {
/*	get rid of temporaties CTEMP1 and CTEMP2	*/

		intname="A:CTEMP1";
		if (topt) *intname=topt;
		else intname+=2;
		if (unlink(intname) == -1) error("Cannot Unlink CTEMP1");

		intname="A:CTEMP2";
		if (topt) *intname=topt;
		else intname+=2;

		if (unlink(intname) == -1) error("Cannot Unlink CTEMP2");
		if (zopt == 0) {
			if (write(output,outbuf,inout-outbuf) == -1)
				error("Cannot Write .A");
			if (close(output) == -1) error("Cannot Close .A");
			}
		}
	if (nextlab >= lastlab) error("too many internal labels");
	util1=(nextlab-1000)/(1000/100);
	if (util1 > util) util=util1;
	util1=numext/(EXTATMAX/100);
	if (util1 > util) util=util1;
	util1=(nextext-ext)/(EXTMAX/100);
	if (util1 > util) util=util1;
	inout=0;
	if (warn && see_exit == 0) {
		os(" Number of Warnings = ");
		ounum(warn);
		os("\n");
		strcat(nextpgm," Z");
		}

/*	if a option is off, trigger assembler. the Cnn option supplies the
	current %utilization number.	*/


	if (aopt == 0 && zopt == 0) {
		nextpgm[22]+=util/10;
		nextpgm[23]+=util%10;
		if (nextpgm[0] != 'Z') _chain(nextpgm);	/* execute asm88	*/
		else _chain(&nextpgm[2]);
		}
	os("End of C88     ");
	if (zopt) asm_size();
	os("     ");
	ounum(util);
	os("% Utilization   \n");
	if (mopt && zopt) {
		if (nextpgm[0] != 'Z') _chain(nextpgm);	/* execute toobj	*/
		else _chain(&nextpgm[2]);
		}
	real_exit(warn ? 1:0);
	}

addfun() {
	unsigned tutil,fline;
	int  i,old_lab,old_last;

	funord=ctlword();
	treelen=ctlword();
	pstart=ctlword();
	pbytes=ctlword();
	curoff=locoff=-ctlword();
	fline=ctlword();
#if DEBUG
	if (eopt) {
		oc(' ');
		oname(funord);
		os(" size ");
		odword(treelen);
		os (" start ");
		odword(pstart);
		os(" parms ");
		odword(pbytes);
		os(" locals ");
		odword(locoff);
		olf();
		}
#endif
	if (treelen >= TREEMAX) error("function too big");
	tutil=treelen/(TREEMAX/100);
	if (tutil > util) util=tutil;
	while (nodesin < treelen)
		tree[++nodesin]=treeword();

#if OPT
	down=0;
	for (i=0; i < NUMREC; i++)
		recat[i]=recent[i]=0;
#endif

	reorder(pstart);
#if OPT
	candidate(pstart);
	down=0;
#endif

#if DEBUG
	if (sopt) {
		level=0;
		listnode(pstart);
		}
#endif
	wantcseg();
	if (fline) {
		asm_line(fline);
		}

#if OPT
	/*  keep trying code generation until no improvement results	*/
	better=perfect=0;
	do {
		better=0;
		old_lab=nextlab;
		old_last=lastlab;
		down=0;
		genfun();
		nextlab=old_lab;
		lastlab=old_last;
		}
	while (better);
	perfect=1;
	down=0;
#endif
	genfun();
	}

#if DEBUG
listnode(node)
	int  node; {
	char i,*name,type,ch;
	int  j,k;

	type=tree[node];
	if (level < 30) i=level+level+1; else i=61;
	while (--i) oc(' ');
	odword(node);
	oc(' ');
	i=7;
	name=namen[type];
	while (--i) oc(*name++);
	i=tree[node]>>8;
	if (i) {oc(' '); obyte(i); oc(' '); }
	else os("   ");
	if (type <= CONST) {
		onum(tree[node+1]);
		oc(' ');
		}
	if (type == OPND || type == LAB || type == CAS ||
		(type >= PREI && type <= POSTD)) 
			onum(tree[node+2]);
	if (type == OPND) {
		oc(' ');
		obyte(tree[node+3]>>8);
		oc(' ');
		i=tree[node+3];
		onum(i);
		}
	k=0;
	switch (type) {
		case WHIL:
		case DOW:	k=3;
					break;
		case FORS:	k=5;
		}
	if (k) {
		for (j=k; j < k+3; j++) {
			oc(' ');
			onum(tree[node+j]);
			}
		}

	olf();
	i=subn[type];
	if (i==0) return;
	level++;
	while (i--) listnode(tree[++node]);
	level--;
	}
#endif


addext(nested)
	char nested; {
	char extst,ch,typ,slen,i,*bptr,xstring;
	int	 thisext,vtype[7],oldnode;
	unsigned len,index;
	union {double dbl; float flt; unsigned flts[2]; long lng;} cvt;

	extst=ctlbyte();
	asmext=thisext=numext=ctlword();
	if (numext >= EXTATMAX-1 || nextext >= &ext[EXTMAX-40])
		error("too many externals");
	extat[numext]=nextext;
	*nextext++ = extst;	/* need the attribute byte too */
	do {
		ch=ctlbyte();
		*nextext++=ch;
		}
	while (ch);

	if ((extst & 0x7f) == SEXTONLY) {
		if (extst >= 128) wanteseg();
		else wantdseg();
		asm_public(numext,extat[numext]+1,1);
		ch=INITFUN;
		}
	else ch=ctlbyte();
	if ((extst & 0x7f) == SEXTERN) {
		if (ch == INITFUN) wantcseg();
		else {
			if (extst >= 128) wanteseg();
			else wantdseg();
			}
		asm_public(numext,extat[numext]+1,1);
		}
	if (zopt && copt && (extst & 0x7f) == SSTATIC)
		asm_static(numext,extat[numext]+1);
	if (ch != INITFUN) {
		findstr(ch);
		if (extst >= 128) wanteseg();
		else wantdseg();
		if (zopt == 0) oname(thisext);
		while (ch != INITEND) {
			if (zopt) test_flush(300);
			switch (ch) {
				case 1:			ctlbyte(); /* skip nested string init */
								ctlword();
								if (ctlbyte() != 0) ierror();
								if (ctlbyte() != INITSTR) ierror();
								do {
									slen=ctlbyte();
									while (slen--) ctlbyte();
									} while (ctlbyte()!=0xFF);
								if (ctlbyte() != INITEND) ierror();
								ch=ctlbyte();
								break;
				case INITDB:	len=ctlword();
								if (zopt) test_flush(len);
								for (index=0; index < len; index++) {
									if ((index % 10) == 0) {
										if (index && zopt == 0) olf();
										if (zopt) test_flush(300);
										asm_db();
										}
									else if (zopt == 0) oc(',');
									if (zopt) codeb(0); else oc('0');
									}
								if (zopt == 0) olf();
								ch=ctlbyte();
								break;
				case INITRB:	len=ctlword();
								asm_rb(len,thisext);
								ch=INITEND;
								break;
				case INITVAL:	typ=ctlbyte();
								oldnode=ctlword(); /* re-use space for values */
								index=ctlword();
								while (nodesin < index)
									tree[++nodesin]=treeword();
								index=ctlword();
#if DEBUG
								if (sopt) {
									level=0;
									listnode(index);
									}
#endif
								getval(index,vtype);
								nodesin=oldnode;	/* chop off value expression */

								switch (typ) {
									case CSCHAR:
									case CCHAR:	if (vtype[VIS] != CONSTV)
													ierror();
												else {
													forceint(vtype);
													asm_db();
													if (zopt) codeb(vtype[VVAL]);
													else {
														odbyte(vtype[VVAL] & 255);
														olf();
														}
													}
												break;
									case CINT:
									case CUNSG:	if (vtype[VIS] != CONSTV &&
													vtype[VIS] != OFFV)
														ierror();
												else {
													forceint(vtype);
													if (vtype[VIS] == OFFV)
														vtype[VIS]=VARV;
													asm_dw();
													vtype[VT]=typ;
													if (zopt) asm_const(vtype,CINT);
													else {
														orm(vtype);
														olf();
														}
													}
												break;
									case CLONG:	if (vtype[VIS] != CONSTV)
													ierror();
												else {
													forcel(vtype);
													asm_dd();
													if (zopt) {
														codew(vtype[VVAL]);
														codew(vtype[VVAL+1]);
														}
													else {
														olong(&vtype[VVAL]);
														olf();
														}
													}
												break;
									case CFLOAT:if (vtype[VIS] != CONSTV)
													ierror();
												if (vtype[VT] == CDOUBLE)
													cvt.flt=((char *)(&vtype[VVAL]))->dbl;
												else if (vtype[VT] == CLONG)
													cvt.flt=((char *)(&vtype[VVAL]))->lng;
												else cvt.flt=vtype[VVAL];
												asm_dw();
												if (zopt) {
													codew(cvt.flts[0]);
													codew(cvt.flts[1]);
													}
												else {
													oword(cvt.flts[0]);
													ocomma();
													oword(cvt.flts[1]);
													olf();
													}
												break;
									case CDOUBLE:
												if (vtype[VIS] != CONSTV)
													ierror();
												forcef(vtype);
												asm_dw();
												if (zopt) {
													codew(vtype[VVAL]);
													codew(vtype[VNAME]);
													codew(vtype[VOFF]);
													codew(vtype[VMORE]);
													}
												else {
													oword(vtype[VVAL]);
													ocomma();
													oword(vtype[VNAME]);
													ocomma();
													oword(vtype[VOFF]);
													ocomma();
													oword(vtype[VMORE]);
													olf();
													}
												break;
								case PTRTO:
												if (vtype[VIS] == CONSTV) {
													forcel(vtype);
													asm_dd();
													if (zopt) {
														codew(vtype[VVAL]);
														codew(vtype[VVAL+1]);
														}
													else {
														olong(&vtype[VVAL]);
														olf();
														}
													}
												else if (vtype[VIS] == OFFV) {
													vtype[VIS]=VARV;
													asm_dd();
													if (zopt) asm_ptr(vtype);
													else {
														vtype[VT]=typ;
														oddrm(vtype);
														olf();
														}
													}
												else ierror();
												break;
									default:	ierror();
									}
								ch=ctlbyte();
								break;
concatstring:
				case INITSTR:	asm_db();
								slen=ctlbyte();
								len=0;
	/*	i=0 no output yet. i=1 number output last. i=2 character last. */
								i=0;
								while (slen--) {
									ch=ctlbyte();
									if (zopt) codeb(ch);
									else {
										if (ch < ' '||ch == '\47' || ch == '\\'||
											ch >= 126) {
											if (i == 2) oc('\47');
											if (i) {
												if (len > 40) {
													olf();
													asm_db();
													len=i=0;
													}
												else oc(',');
												}
											onum(ch);
											len+=5;
											i=1;
											}
										else {
											if (i == 1) oc(',');
											if (i != 2) oc('\47');
											oc(ch);
											len++;
											if (len > 60) {
												os("'\n DB ");
												i=len=0;
												}
											else i=2;
											}
										}
									}
								if(ctlbyte()!=0xFF) {
									if(zopt==0) {
										if(i== 2)
											oc('\'');
										olf();
									}
									goto concatstring;
									}
								if (zopt) codeb(0);	/* end the string */
								else { switch (i) {
									case 0:	oc('0');	/* null string */
											break;
									case 1:	os(",0");	/* number last */
											break;
									case 2:	os("',0");	/* char last */
									}
									olf();
									}
								if (nested) return;
								ch=ctlbyte();
				}
			}
		}
#if DEBUG
	if (eopt) {
		oc(' ');
		oname(numext);
		olf();
		}
#endif
	}
findstr(itype)
	char itype; {
	char slen;
	int  oldblk,oldin,oldext;

	oldblk=ctlblk;
	oldin=inctl;
	while (itype != INITEND) {
		switch (itype) {
			case 1:			oldext=asmext;
							addext(1);	/* go back and look for strings */
							if (ctlbyte() != INITEND) ierror();
							itype=ctlbyte();
							asmext=oldext;
							break;
			case INITDB:	ctlword(); /* skip length */
							itype=ctlbyte();
							break;
			case INITRB:	ctlword(); /* skip length */
							itype=INITEND;
							break;
			case INITVAL:	ctlbyte();
							ctlword();
							ctlword();
							ctlword();
							itype=ctlbyte();
							break;
			case INITSTR:	do {
								slen=ctlbyte();
								while (slen--) ctlbyte();
								} while(ctlbyte()!=0xFF);
							itype=ctlbyte();
							break;
			default:		ierror();
			}
		}
	if (oldblk != ctlblk) {
		if (lseek(control,((long)oldblk)*BUFSIZE,0) == -1)
			error("Cannot Seek CTEMP1");
		if (read(control,ctlbuf,BUFSIZE) == -1)
			error("Cannot Read CTEMP1");
		ctlblk=oldblk;
		}
	inctl=oldin;
	}

ierror() {

	inout=0;
	if (line_num) {
		os("in line ");
		onum(line_num);
		os("  ");
		}
	os("illegal initialization for ");
	oname(numext);
	olf();
	real_exit(2);
	}

ctlbyte() {
	if (inctl == &ctlbuf[BUFSIZE]) {
		if (read(control,ctlbuf,BUFSIZE) == -1)
			error("Cannot Read CTEMP1");
		inctl=ctlbuf;
		ctlblk++;
		}
	return *inctl++;
	}

ctlword() {
	char ch;
	ch=ctlbyte();
	return (ch+(ctlbyte()<<8));
	}

treeword() {
	if (intree == &treebuf[BUFSIZE]) {
		if (read(treef,treebuf,BUFSIZE) == -1)
			error("Cannot Read CTEMP2");
		intree=treebuf;
		}
	return *intree++;
	}


/*	OC  --	output a .a character	*/

oc(ch)
	char	ch; {

#if OPT
	if (perfect || xopt) {
#endif
		if (ch == LF) oc(CR);
		if (inout) {
			if (inout == &outbuf[BUFSIZE]) {
				if (write(output,outbuf,BUFSIZE) == -1)
					error("Cannot Write .A");
				inout=outbuf;
				}
			*inout++=ch;
			}
		else {
			if (see_exit) {
				if (see_index < 78 && ch != '\n') see_msg[see_index++]=ch;
				}
			else putchar(ch);
			}
#if OPT
		}
#endif
	}

os(str)
	char	*str; {
	while (*str) oc(*str++);
	}

obyte(ch)
	char	ch; {
	char	hexb;
	hexb=ch>>4;
	if (hexb > 9) oc(hexb+55);
	else oc(hexb+'0');
	hexb=ch & 15;
	if (hexb > 9) oc(hexb+55);
	else oc(hexb+'0');
	}

oword(word)
	int	word; {
	if (word & 0x8000) oc('0');
	obyte(word >> 8);
	obyte(word);
	oc('H');
	}

error(estr)
	char *estr; {
	inout=0;
	olf();
	os("pass 2 error ");
	if (line_num) {
		os("in line ");
		onum(line_num);
		os("  ");
		}
	os(estr);
	olf();
	if (zopt) asm_error();
	real_exit(2);
	}

odbyte(ch)
	char ch; {
	char i,divisor,signif;
	signif=0;
	divisor=100;
	while (divisor) {
		if (divisor == 1) signif=1;
		if (i=ch/divisor) { oc(i+'0'); signif=1; }
		else if (signif) oc('0');
		else oc(' ');
		ch=ch%divisor;
		divisor=divisor/10;
		}
	}

odword(word)
	int	word; {
	int	i,divisor;
	char	signif;
	signif=0;
	if (word < 0) {
		word=-word;
		oc('-');
		}
	divisor=10000;
	while (divisor) {
		if (divisor == 1) signif=1;
		if (i=word/divisor) { oc(i+'0'); signif=1;}
		else if (signif) oc('0');
		else oc(' ');
		word=word%divisor;
		divisor=divisor/10;
		}
	}

onum(word)
	int  word; {

	if (word < 0) {
		if (word == 0x8000) {
			os("-32768");
			return;
			}
		oc('-');
		word=-word;
		}
	if (word >= 10) onum(word/10);
	oc(word % 10 +'0');
	}

ouunum(word)
	unsigned word; {

	if (word >= 10) ouunum(word/10);
	oc(word % 10 +'0');
	}

ounum(word)
	unsigned word; {

	if (word >= 10) ounum(word/10);
	oc(word % 10 +'0');
	}

olong(lnum)
	char *lnum; {

	oc('0');
	obyte(*(lnum+3));
	obyte(*(lnum+2));
	obyte(*(lnum+1));
	obyte(*lnum);
	oc('H');
	}


/*	stuff for SEE interface.	*/


see_addr(ptr)
	char *ptr; {

	see_exit=1;
	see_seg=see_hex(&ptr);
	see_off=see_hex(&ptr);
	}

see_hex(ptrptr)
	char **ptrptr; {
	char *ptr,ch;
	int  num;

	ptr=*ptrptr;
	num=0;
	while (1) {
		ch=*ptr++;
		if (ch >= '0' && ch <= '9') num=num*16+ch-'0';
		else if (ch >= 'a' && ch <= 'f') num=num*16+ch-'a'+10;
		else if (ch >= 'A' && ch <= 'F') num=num*16+ch-'A'+10;
		else {
			*ptrptr=ptr;
			return num;
			}
		}
	}

/*	SEE_CALL  --	SEE interface routine.	*/

see_call(p1,p2,p3)
	int p1,p2,p3; {
#asm
	pop		word see_bp_
	pop		word see_ret_
	lcall	dword see_off_
	push	word see_ret_
	push	word see_bp_
#
	}

/*	REAL_EXIT  --	do an exit or a return to SEE.	*/

real_exit(cc)
	int  cc; {

	if (see_exit) {
		see_call(2);
		if (cc) {
			if (copt) unlink(objname);
			see_call(-1,see_msg,line_num ? line_num: 1);
			}
		}
	exit(cc);
	}



getval(node,vtype)
	int  node,vtype[]; {
	int  lastnode;
	char type,otype;
	type=tree[node];
	otype=optype[type];
#if DEBUG
	if (eopt) {
		os("node "); onum(node);
		os(" type "); onum(type);
		os(" otype "); onum(otype);
		olf();
		}
#endif
	vtype[VMORE] = -1;
	switch(otype) {
		case 0:	genopnd(node,vtype);
				break;
		case 2:	genfcal(node,vtype);
				break;
		case 3:	gentest(node,vtype);
				break;
		case 4:	genarith(node,vtype);
				break;
		case 5:	genshift(node,vtype);
				break;
		case 6:	genbin(node,vtype);
				break;
		case 7:	genlog(node,vtype);
				break;
		case 8:	gencmp(node,vtype);
				break;
		case 9:	genunary(node,vtype);
				break;
		case 10:genpre(node,vtype);
				break;
		case 11:genpost(node,vtype);
				break;
		case 12:genasgn(node,vtype);
				break;
		case 14:lastnode=node+type-LST;
				while (++node < lastnode)
					getnoval(tree[node],vtype);
				getval(tree[node],vtype);
				break;
		case 16:genmove(node,vtype);
				/*	reload the source value as result	*/
				nopost=1;
				nopre=1;
				getval(tree[tree[node+2]+1],vtype);
				nopost=0;
				nopre=0;
				break;
		default:error("bad expression");
		}
	}

genopnd(node,vtype)
	int  node,vtype[]; {
	int  nnode,vright[7];

	char type,dtype,reg;
	dtype=tree[node]>>8;
	type=tree[node];
#if OPT
	vtype[VFROM]=0;
#endif
	switch (type) {

	  case OPND:
		vtype[VNAME]=tree[node+1];
		vtype[VOFF]=tree[node+2];
		vtype[VMORE] = -1;
		if (vtype[VNAME]) {
			vtype[VVAL]=8;
			if (is_big) {
				if (*extat[vtype[VNAME]] < 128) vtype[VMORE] = DS;
				else vtype[VMORE] = NEED_ES;
				}
			}
		else {
			vtype[VVAL]=6;
			if (is_big) vtype[VMORE] = SS;
			}
		vtype[VT]=dtype;
		if ((dtype == ARRAY) || (dtype == FUNCTION)) {
			vtype[VIS]=OFFV;
			vtype[VT]=is_big ? PTRTO: CUNSG;
			if (dtype == FUNCTION && is_big) {
				if (*extat[vtype[VNAME]] == SSTATIC) vtype[VMORE] = CS;
				else vtype[VMORE] = NEED_ES;
				}
			}
		else {
			vtype[VIS]=VARV;
#if OPT
			down++;
			/* remember the location of type word	*/
			if (tree[node+3]) {
				vtype[VFROM]=node;
				if (isasgn == 0 && is_reg(vtype) == 0) {
					if ((reg=want_reg(tree[node+3]+down+20,dtype == CCHAR))
							 != -1) {
						movreg(reg,vtype);
						reg_from[reg]=node;
						reg_dirty[reg]=0;
						}
					}
				}
#endif
			}
		break;

	  case CONST:
		vtype[VIS]=CONSTV;
		vtype[VVAL]=tree[node+1];
		vtype[VNAME]=0;
		if (dtype == CLONG) vtype[VNAME]=tree[node+2];
		else if (dtype == CDOUBLE) {
			vtype[VNAME]=tree[node+2];
			vtype[VOFF]=tree[node+3];
			vtype[VMORE]=tree[node+4];
			}
		vtype[VT]=dtype;
		break;

	  case CAST:
		if ((dtype == CINT || dtype == CUNSG) && (tree[tree[node+1]] == 
			(CLONG << 8)+OPND || tree[tree[node+1]] == (PTRTO << 8)+OPND))
				tree[tree[node+1]]=(CUNSG << 8)+OPND;
		getval(tree[node+1],vtype);
		switch (dtype) {
			case CSCHAR:
			case CCHAR:		forcebyt(vtype);
							break;
			case CLONG:		forcel(vtype);
							break;
			case CFLOAT:	{
								union {float flt; double dbl; long lng;};

								if (vtype[VIS] == CONSTV) {
									if (vtype[VT] == CDOUBLE)
										((char *)(&vtype[VVAL]))->flt=((char *)(&vtype[VVAL]))->dbl;
									else if (vtype[VT] == CLONG)
										((char *)(&vtype[VVAL]))->flt=((char *)(&vtype[VVAL]))->lng;
									else ((char *)(&vtype[VVAL]))->flt=vtype[VVAL];
									vtype[VT]=CFLOAT;
									}
								else loadf(vtype);
								}
							break;
			case CDOUBLE:	forcef(vtype);
							break;
			case CSTRUCT:	if (vtype[VT] == CLONG) forceint(vtype);
							vtype[VT]=CSTRUCT;
							break;
			case FUNCTION:
			case PTRTO:		if (vtype[VT] != PTRTO)
								forcel(vtype);	/* ?? vtype[VT] = PTRTO; ?? */
							break;
			default:		forceint(vtype);
							if (dtype != CINT) vtype[VT]=CUNSG;
							else vtype[VT]=CINT;
			}
		break;
	  case TA:
		getval(tree[node+1],vtype);
		if (vtype[VIS]==VARV) {
			vtype[VIS]=OFFV;

			if (vtype[VNAME] == 0 && vtype[VOFF] == 0) {
				if (vtype[VVAL] == 4) {
					vtype[VVAL]=SI;
					vtype[VIS]=REGV;
					}
				else if (vtype[VVAL] == 5) {
					vtype[VVAL]=DI;
					vtype[VIS]=REGV;
					}
				}
			}
		else if (vtype[VT] != CSTRUCT) error("cannot address");
		vtype[VT]=is_big ? PTRTO: CUNSG;
		break;

	  case IND:
		nnode=tree[node+1];
	
/*		if an add then use an addressing register.	*/

		prefer_si=1;			/* would rather use SI in case index	*/
		if (tree[nnode] == ADD) {
			getval(tree[nnode+2],vright);
			getval(tree[nnode+1],vtype);
			prefer_si=0;

/*	ADD has better code for offsets		*/

			if (vtype[VIS] == OFFV || vright[VIS] == OFFV) {
				genadd(vtype,vright);
				}
			else {
				if (vright[VT] == PTRTO) flip(vtype,vright);
				forceind(vtype);
				vtype[VNAME]=vtype[VOFF]=0;
				if (vright[VIS] == CONSTV)
					vtype[VOFF]=vright[VVAL];
				else {
					forceint(vright);
					forcerm(vright);
					if (is_big) forcees(vright);
					asm_add(ADD86,vtype,vright);
					freev(vright);
					forceind(vtype);
					}
				if (vtype[VVAL] == SI) vtype[VVAL]=4;
				else vtype[VVAL]=5;
				}
			}
		else {
			getval(tree[node+1],vtype);
			prefer_si=0;
			if (vtype[VIS] != OFFV) {
				forceind(vtype);
				if (vtype[VVAL] == SI) vtype[VVAL]=4;
				else vtype[VVAL]=5;
				vtype[VNAME]=vtype[VOFF]=0;
				}
			}
		vtype[VIS]=VARV;
		vtype[VT]=dtype;
	  }
	}
