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
/*      the-grue - 20180809
 *      Changed lines 116-117 to add OpenASM88 header
 *      and changed lines 470 and 472 to conform to new compiler.
 */
/*	ASM88	ASSEMBLER FOR 8088					ASM88.C	*/

#include "ASM88.H"
#include "OBJ.H"
unsigned maxcol=80,maxrow=66,currow=9999,line_ordinal=1000;
char nextpgm[77]="Z:TOOBJ ",got_list;

main(argc,argv)
	int  argc;
	char *argv[]; {
	init(argc,argv);
	while (not_done) {
		doline();
		endline();
		}
	endit();
	}

init(argc,argv)
	int  argc;
	char *argv[]; {
	char opt,*optptr;
	int  nopt;

/*	set the debug options	*/

	if (argc < 2) ferror("no input file");

/*	set temporary name to Z drive.	*/

	tempname="Z:CTEMP3";
	tempname6="Z:CTEMP6";

/*	if assembling a CTEMP4 file, set kill flag for eventual deletion.	*/

	optptr=argv[1];
	if (optptr[1] == ':') {
		optptr+=2;
		}
	if (strcmp(optptr,"CTEMP4") == 0) {
		killopt=1;
		if (optptr != argv[1]) tempname[0]=tempname6[0]=*argv[1];
		}

	lstname[0]=0;
	for (nopt=2; nopt < argc; nopt++) {
		cur=argv[nopt];
		while (opt=toupper(*cur++)) {
#if LIMITED == 0
			if (opt == 'B') 		/* B means BIG model */
				is_big=1;
			if (opt == 'C') copt=1; /* C means checkout */
									/* Dd is drive of toobj option */
			if (opt == 'D') nextpgm[0]=toupper(*cur++);
			if (opt == 'E') eopt=1;	/* E prints Every token*/
			if (opt == 'L') {		/* L is followed by list name */
				strcpy(outname,cur);
				lopt=1;
				while (*cur) cur++;
				break;
				}
			if (opt == 'M') mopt=1;	/* M is for.OBJ format.	*/
#endif
			if (opt == 'O') {		/* O is followed by object name */
				strcpy(objname,cur);
				while (*cur) cur++;
				break;
				}
#if LIMITED == 0
			if (opt == 'P') {		/* Wnn is page length option */
				maxrow=atoi(cur);
				if (maxrow < 20 || maxrow > 500) maxrow=66;
				break;
				}
			if (opt == 'S') sopt=1;	/* S prints Symbols */
#endif
			if (opt == 'T')			/*	Td specifies drive for temporaries.*/
				tempname[0]=tempname6[0]=toupper(*cur++);
			if (opt == 'U') {		/* Unn is utilization from C */
				uopt=1;
				if (*cur >= '0' && *cur <= '9') util=(*cur++-'0')*10;
				if (*cur >= '0' && *cur <= '9') util+=*cur++-'0';
				}
#if LIMITED == 0
			if (opt == 'W') {		/* Wnn is width option	*/
				maxcol=atoi(cur);
				if (maxcol < 40 || maxcol > 132) maxcol=80;
				break;
				}
			if (opt == 'Z') zflag=1;
			if (opt == '.')			/* 186/v20 flag */
				switch(toupper(*cur)) {
					case 'V':	Xflag |= XV20;
					case '1':	Xflag |= X186;
					}
#endif
			}
		}

	if (uopt == 0)
		os("OpenASM88 8088 Assembler  V0.1    Based on\n");
		os("ASM88     8088 Assembler  V1.5    (c) Mark DeSmet, 1982-86\n");

	/*	if no temp drive, use default.	*/

	if (tempname[0] == 'Z') {
		tempname+=2;
		tempname6+=2;
		}
	not_done=1;
	errline=-1;
	freem=_memory();
	memend=_showsp()-512;
	curseg=ODSEG;				/* first stuff is in DSEG */
	inpipe=pipe;				/* pipe is empty */
	codeat=0;					/* points to code fragment (if last)in pipe*/
								/* offs[0]=offs[1]=offs[2]=offs[3]=offs[4]=0;*/
								/* total size of dseg ,cseg, reserved, eseg */
								/* and eseg reserved.	*/


/*	ltype contains the letter types for all ascii letters	*/
	_setmem(ltype,128,34);
	_setmem(&ltype['A'],26,LETTER);
	_setmem(&ltype['a'],26,LETTER);
	ltype['_']=LETTER;
	_setmem(&ltype['0'],10,DIGIT);
	ltype[9]=ltype[CR]=SPACE;
	initch(&ltype[32],"345XX678XX9AXBXD");
	initch(&ltype[60],"EFG");
	ltype['^']=17;
	ltype['|']=18;
	ltype[LF]=19;

/*	set jump codes. last is jmp short	*/

	for (i=0; i < 16; i++)
		shortj[i]=0x70+i;
	shortj[16]=0xe0;shortj[17]=0xe1;shortj[18]=0xe2;shortj[19]=0xe3;
	shortj[20]=0xeb;

/*	add reserved words to symbol table	*/

	initres(1,"AAA,2,37,AAD,13,1,AAM,13,0,AAS,2,3F,ADC,0,2,ADD,0,0,");
	initres(1,"AND,0,4,CALL,7,0,CBW,2,98,CLC,2,F8,CLD,2,FC,CLI,2,FA,");
	initres(1,"CMC,2,F5,CMP,0,7,CMPSB,2,A6,CMPSW,2,A7,CWD,2,99,DAA,2,27,");
	initres(1,"DAS,2,2F,DEC,3,1,DIV,18,6,ESC,14,0,FWAIT,2,9B,HLT,2,F4,IDIV,18,7,");
	initres(1,"IMUL,18,5,IN,15,0,INC,3,0,INT,11,0,INTO,2,CE,IRET,2,CF,");
	initres(1,"JA,4,7,JAE,4,3,JB,4,2,JBE,4,6,JC,4,2,JCXZ,4,13,JE,4,4,");
	initres(1,"JG,4,F,JGE,4,D,JL,4,C,JLE,4,E,JMP,17,0,JNA,4,6,JNAE,4,2,");
	initres(1,"JNB,4,3,JNBE,4,7,JNC,4,3,JNE,4,5,JNG,4,E,JNGE,4,C,JNL,4,D,");
	initres(1,"JNLE,4,F,JNO,4,1,JNP,4,B,");
	initres(1,"JNS,4,9,JNZ,4,5,JO,4,0,JP,4,A,JPE,4,A,JPO,4,B,JS,4,8,");
	initres(1,"JZ,4,4,LAHF,2,9F,LDS,10,C5,LCALL,21,3,LEA,19,0,LES,10,C4,");
	initres(1,"LJMP,21,5,LODSB,2,AC,LODSW,2,AD,LOOP,4,12,LOOPE,4,11,");
	initres(1,"LOOPNE,4,10,LOOPNZ,4,10,LOOPZ,4,11,");
	initres(1,"LRET,8,1,MOV,9,0,MOVSB,2,A4,MOVSW,2,A5,MUL,18,4,");
	initres(1,"NEG,18,3,NOP,2,90,");
	initres(1,"NOT,18,2,OR,0,1,OUT,16,0,POP,1,1,POPF,2,9D,PUSH,1,0,");
	initres(1,"PUSHF,2,9C,RCL,12,2,RCR,12,3,RET,8,0,ROL,12,0,ROR,12,1,");
	initres(1,"SAHF,2,9E,SAL,12,4,SAR,12,7,SBB,0,3,SCASB,2,AE,SCASW,2,AF,");
	initres(1,"SHL,12,4,SHR,12,5,STC,2,F9,STD,2,FD,STI,2,FB,STOSB,2,AA,");
	initres(1,"STOSW,2,AB,SUB,0,5,TEST,5,0,WAIT,2,9B,XCHG,6,0,XLAT,20,0,");
	initres(1,"XOR,0,6,");

	if(Xflag & X186) {
		initres(1,"BOUND,10,62,ENTER,22,0,INSB,2,6C,INSW,2,6D,LEAVE,2,C9,");
		initres(1,"OUTSB,2,6E,OUTSW,2,6F,PUSHA,2,60,POPA,2,61,");
		}
	if(Xflag & XV20) {
		initres(1,"ADD4S,24,20,CLR1,25,12,CMP4S,24,26,CVTBD,24,D4,CVTDB,24,D5,");
		initres(1,"INSRT,23,31,NOT1,25,16,REPC,2,65,REPNC,2,64,ROL4,26,28,");
		initres(1,"ROR4,26,2A,SET1,25,14,SUB4S,24,22,TEST1,25,10,XTRCT,23,33,");
		}
#if	X8087
	initres(2,"F2XM1,0,0,FABS,0,1,FCHS,0,2,FCLEX,0,3,FCOMPP,0,4,");
	initres(2,"FDECSTP,0,5,FDISI,0,6,FENI,0,7,FINCSTP,0,8,FINIT,0,9,");
	initres(2,"FLDLG2,0,A,FLDLN2,0,B,FLDL2E,0,C,FLDL2T,0,D,FLDPI,0,E,");
	initres(2,"FLDZ,0,F,FLD1,0,10,FNOP,0,11,FPATAN,0,12,FPREM,0,13,");
	initres(2,"FPTAN,0,14,FRNDINT,0,15,FSCALE,0,16,FSQRT,0,17,FTST,0,18,");
	initres(2,"FXAM,0,19,FXTRACT,0,1A,FYL2X,0,1B,FYL2XP1,0,1C,");
	initres(2,"FNCLEX,1,0,FNDISI,1,1,FNENI,1,2,FNINIT,1,3,");
	initres(2,"FLD,2,0,FST,2,1,FSTP,2,2,");
	initres(2,"FFREE,3,0,FXCH,3,1,");
	initres(2,"FADDP,4,0,FDIVP,4,1,FDIVRP,4,2,FMULP,4,3,FSUBP,4,4,");
	initres(2,"FSUBRP,4,5,");
	initres(2,"FADD,5,0,FDIV,5,1,FDIVR,5,2,FMUL,5,3,FSUB,5,4,FSUBR,5,5,");
	initres(2,"FBLD,6,0,FBSTP,6,1,FLDCW,6,2,FLDENV,6,3,FRSTOR,6,4,");
	initres(2,"FSAVE,6,5,FSTCW,6,6,FSTENV,6,7,FSTSW,6,8,");
	initres(2,"FNSAVE,7,0,FNSTCW,7,1,FNSTENV,7,2,FNSTSW,7,3,");
	initres(2,"FIADD,8,0,FICOM,8,1,FICOMP,8,2,FIDIV,8,3,FIDIVR,8,4,");
	initres(2,"FIMUL,8,5,FISUB,8,6,FISUBR,8,7,");
	initres(2,"FILD,9,0,FIST,9,1,FISTP,9,2,");
	initres(2,"FCOM,10,0,FCOMP,10,1,");
#endif

	initres(3,"LOCK,0,F0,REP,0,F3,REPNZ,0,F2,REPNE,0,F2,REPZ,0,F3,REPE,0,F3,");

	initres(4,"INCLUDE,1,PUBLIC,2,EVEN,3,EQU,4,RB,5,RW,6,DB,7,DW,8,DD,9,");
	initres(4,"DQ,10,BYTE,11,WORD,12,DWORD,13,QWORD,14,TBYTE,15,OFFSET,16,");
	initres(4,"DSEG,17,CSEG,18,ST,19,TITLE,20,EJECT,21,LINE,22,END,23,SPLIT,24,");
	initres(4,"ESEG,25,IF,26,ELSE,27,ENDIF,28,SEG,29,");
	initres(5,"ES,0,CS,1,SS,2,DS,3,");

	initres(6,"AX,2,0,CX,2,1,DX,2,2,BX,2,3,SP,2,4,BP,2,5,SI,2,6,DI,2,7,");
	initres(6,"AL,1,0,CL,1,1,DL,1,2,BL,1,3,AH,1,4,CH,1,5,DH,1,6,BH,1,7,");
	initthis(7,"LARGE_CASE,1,");		/* create an equ LARGE_CASE */
	*(freem-1)=is_big ? '1': '0';
	*freem++=LF;
	memover=freem;
	ordinal=0;

/*	rmto shows how to change the rm type with the addition of a register */
/*	fields are: new register, old rm, new rm.	*/

	initch(rmto,"387340351586542553684670662785771763");


/*	open files	*/

	strcpy(name,argv[1]);
	i=0;
	while (name[i] && name[i] != '.') {
		i++;
		}
	if (name[i] == 0 && killopt == 0) strcat(name,".A");
	if ((infile=open(name,0)) == -1) {
		os("cannot open ");
		ferror(name);
		}

	if (objname[0] == 0) {
		i=0;
		while (name[i] && name[i] != '.') {
			objname[i]=name[i];
			i++;
			}
		}
	else {
		i=0;
		while (objname[i] && objname[i] != '.') {
			i++;
			}
		}

	objname[i]='.';
	objname[i+1]='O';
	objname[i+2]=0;

	if (lopt) {
		if (outname[0] == 0) {
			i=0;
			while (objname[i] && objname[i] != '.') {
				outname[i]=objname[i];
				i++;
				}
			outname[i]='.';
			outname[i+1]='L';
			outname[i+2]=0;
			}
		strcpy(lstname,tempname);
		i=0;
		while (lstname[++i]) ;
		lstname[i-1]='5';
		if ((lst=creat(lstname)) == -1) {
			os("cannot create ");
			ferror(lstname);
			}
		inlst=lstbuf;
		}

#if CHECK
	if (copt) {
		if ((obj=fopen(objname,"a")) == 0) {
			os("cannot create ");
			ferror(objname);
			}
		mopt=0;
		}
	else {
#endif
		
	if (mopt) {			/* want Intel object modules.	*/
		strcat(nextpgm,tempname6);
		strcat(nextpgm," O");
		strcat(nextpgm,objname);
		strcpy(objname,tempname6);
		}
	if ((obj=creat(objname)) == -1) {
		os("cannot create ");
		ferror(objname);
		}
#if CHECK
		}
#endif

	inobj=&objbuf;

	ndummy=0;
	indummy=&dbuf;
	if (is_big && copt == 0) objb(OBIG);

/*	next .O and dummy contents will be in DSEG */
	objb(ODSEG);
	dummyb(ODSEG);

	fill();
	}

initch(array,vals)
	char *array,*vals; {
	while(*vals)
		if (*vals < 'A') *array++=*vals++-'0';
		else *array++=*vals++-'A'+10;
	}

initres(rclass,rstr)
	char rclass,*rstr; {
	char val;

	initthis(rclass,rstr);	/* add upper case reserved to symbol table */
							/* add lower case reserved words	*/
	cur=rstr;
	do {
		while (*cur != ',') {
			*cur=tolower(*cur);
			cur++;
			}
		cur++;
		while (*cur++ != ',') ;
		if (rclass == REG || rclass <= PREFIX) 	while (*cur++ != ',') ;
		}
	while (*cur);
	initthis(rclass,rstr);
	}


initthis(rclass,rstr)
	char rclass,*rstr; {
	char val;

	cur=rstr;
	do {
		cur++;
		find();
		freem-=8;
		*freem++=rclass;
		val=0;
		while (*++cur != ',')
			val=val*10+*cur-'0';
		*freem++=val;
		if (rclass == REG || rclass <= PREFIX) {
			cur++;
			*freem++=inhex();
			}
		cur++;
		}
	while (*cur);
	}

inhex() {
	char val;

	val=0;
	while (*cur != ',') {
		val<<=4;
		if (*cur <= '9') val+=*cur-'0';
		else val+=*cur-55;
		cur++;
		}
	return val;
	}

endit() {
	char *isize;
	unsigned util2;

	dumpsym();
	if (nerr == 0) {

		flush(1);
		if (ndummy) {
			if (lseek(dummy,0,0,0) == -1)
				ferror("cannot seek CTEMP3");
			while (ndummy--) {
				if(read(dummy,pipe,2048) == -1)
					ferror("cannot read CTEMP3");
				isize=pipe;
				while (isize < &pipe[2048])
					objb(*isize++);
				}
			if (close(dummy) == -1)
				ferror("cannot close CTEMP3");
			if (unlink(tempname) == -1)
				ferror("cannot unlink CTEMP3");
			}
		isize=dbuf;
		while (isize < indummy)
			objb(*isize++);
		objb(OEOF);
		isize=inobj;
		isize=isize-objbuf;
		if (write(obj,objbuf,isize) == -1) {
			os("cannot write ");
			ferror(objname);
			}
		if(close(obj) == -1) {
			os("cannot close ");
			ferror(objname);
			}

		if (lopt) {
			if (inlst > lstbuf) {
				if (write(lst,lstbuf,inlst-lstbuf) == -1) {
					os("cannot write ");
					ferror(lstname);
					}
				}
			listing();
			}

		if (killopt) {
			if (close(infile) == -1) {
				os("cannot close ");
				ferror(name);
				}
			if (unlink(name) == -1) {
				os("cannot delete ");
				ferror(name);
				}
			}

		if (uopt) os("end of C88        ");
		else os("end of ASM88        ");
		}
	else {
		if (ndummy)	{
			if (close(dummy) == -1)
				ferror("cannot close CTEMP3");
			if (unlink(tempname) == -1)
				ferror("cannot unlink CTEMP3");
			}
		if (lopt) {
			if (close(lst) == -1) ferror("cannot close CTEMP5");
			if (unlink(lstname) == -1) {
				os("cannot delete ");
				ferror(lstname);
				}
			}
		os("Number of Errors = ");
		onum(nerr);
		puts("     ");
		}
	ohw(offs[1]);
	printf("code   ");
	ohw(offs[0]+offs[2]);
	printf("data   ");
	if (is_big) {
		ohw(offs[3]+offs[4]);
		printf("extra   ");
		}
	util2=(freem-memover)/((memend-memover)/100)+1;
	ounum(util2 > util ? util2: util);
	puts("% utilization");
#if LIMITED
	puts("\n");
#endif
	if (nerr == 0 && mopt) {
		if (nextpgm[0] != 'Z') _chain(nextpgm);	/* execute toobj	*/
		else _chain(&nextpgm[2]);
		}
	exit(nerr ? 2:zflag ? 1:0);
	}

doline() {
	char *labat;

	needline();
	if (!not_done) return;
	if (*codeat > 178) codeat=0;
	if (inpipe > &pipe[940]) flush(0);
	tokit();
	labat=0;
	while (heir == NAME) {
		labat=wvalue;
		if ((labat->sclass)&1) error("duplicate label");
		if (copt && (wvalue->sclass & 0x80) == 0) {
			if (string[0] != '_' || (string[1] != 'L' && string[1] != '_')) {
				objb(OSTATIC);
				objs(string);
				objw(labat->snum);
				}
			}
		labat->sclass|=1;
		if (labat->svalue != curseg)
			error("label in wrong segment");
		tokit();
		if (curch == ':') {
			tokit();
			if (heir != RESERVED || (wvalue->sclass < REQU ||
			wvalue->sclass > RDQ)) 	label(labat->snum);
			}
		else if (heir != RESERVED || (wvalue->sclass < REQU ||
			wvalue->sclass > RDQ)) 	error("missing ':'");
		}

	if (curch == LF || curch == CONTZ || curch == ';') {
		return;
		}

got_pre:
	if (heir == RESERVED) dores(labat);
	else {
		if (heir == MNUM) doinst();
#if X8087
		else if (heir == FMNUM) dofinst();
#endif
		else if (heir == PREFIX) {
			codeb(wvalue->svalue);
			tokit();
			goto got_pre;
			}
		else error("unknown mnemonic");
		}

	if (curch != ';' && curch != LF)
		error("too many arguments");
	}

endline() {

	if (lopt) listline();
	while (cur > lastch) {
		cur=nestdef[--nest_cur];
		curch=*cur++;
		}
	while (curch != LF && curch != CONTZ)
		curch=*cur++;
	}

needline() {

	if (curch == CONTZ) {
		if (incfile == 0) {
			not_done=0;
			return;
			}
		close(incfile);
		incfile=0;
		line=lastline;
		inbuf[0]=LF;
		cur=lastlf=&inbuf[1];
		lastch=savein;
		while (lastch < lastinc)
			*lastlf++=*lastch++;
		lastch=lastlf;
		lastlf--;
		while (*lastlf != LF && *lastlf != CONTZ)
			lastlf--;
		}

	if (cur > lastlf) refill();
	line++;
	last=cur;
	}

union {double dbl; float flt; unsigned words[4];} floater;

dores(labat)
	char *labat; {
	char *extat,saveseg,*ptr,ch;
	int  vtype[6],i,want;

	switch (wvalue->sclass) {
		case RINCLUDE:	if (lopt) isinclude();
						doinc();
						break;
		case REQU:		if (labat == 0) {
							error("missing EQU name");
							break;
							}
						ordinal--;
						*labat++=EQU;
						while (ltype[*cur] == SPACE) cur++;
						while (*cur != LF && *cur != ';' && *cur != CONTZ)
							*labat++=*cur++;
						while (ltype[*--labat] == SPACE) ;
						*++labat=LF;
						freem=labat+1;
						tokit();
						break;
		case RPUBLIC:	if (labat) error("cannot label PUBLIC");
						tokit();
						do {
							if (heir != NAME) error("not a label");
							else {
								objb(OPUBLIC);
								objs(string);
								objw(wvalue->snum);
								wvalue->sclass|=0x80;
								extat=wvalue;
								tokit();
								if (curch == ':') {
									tokit();
									if (heir == RESERVED && ( wvalue->sclass
										>= RBYTE && wvalue->sclass <=RTBYTE))
										extat->stype=wvalue->sclass-RBYTE+
												CCHAR;
									else error("type not correct");
									tokit();
									}
								}
							}
						while (ifch(','));
						break;
		case REVEN:		evenb();
						tokit();
						break;
		case RRB:		tokit();
						one(vtype);
						if (vtype[VIS] != CONSTV)error("bad RB value");
						if (curseg == OCSEG) error("RB must be in DSEG or ESEG");
						if (labat) {
							labat->stype=CCHAR;
							objb(ORESERVE);
							objw(labat->snum);
							objw(vtype[VVAL]);
							offs[curseg == ODSEG ? 2: 4]+=vtype[VVAL];
							}
						else error("RB must have label");
						break;
		case RRW:		tokit();
						one(vtype);
						if (vtype[VIS] != CONSTV)error("bad RW value");
						if (curseg == OCSEG) error("RW must be in DSEG or ESEG");
						if (labat) {
							labat->stype=CINT;
							objb(ORESERVE);
							objw(labat->snum);
							vtype[VVAL]+=vtype[VVAL];
							objw(vtype[VVAL]);
							offs[curseg == ODSEG ? 2: 4]+=vtype[VVAL];
							}
						else error("RW must have label");
						break;
		case RDB:		if (labat) label(labat->snum);
						do {
							tokit();
							if (heir == SSTRING) {
								i=0;
								while (string[i]) {
									codeb(string[i++]);
									}
								tokit();
								}
							else {
								one(vtype);
								if (vtype[VIS] != CONSTV)error("bad DB value");
								codeb(vtype[VVAL]);
								}
							}
						while (curch == ',');
						if (labat) {
							labat->stype=CCHAR;
							}
						break;
		case RDW:		if (labat) label(labat->snum);
						saveseg=curseg; /* make labels default to CSEG */
						curseg=OCSEG;
						tokit();
						do {
							one(vtype);
							if (vtype[VIS] == CONSTV) {
								vtype[VT]=CINT;
								oconst(vtype);
								}
							else if (vtype[VIS] == VARV && vtype[VVAL] ==8) {
								vtype[VIS]=OFFV;
								oconst(vtype);
								}
							else error("invalid DW constant");
							}
						while(ifch(','));
						curseg=saveseg;
						if (labat) {
							labat->stype=CINT;
							}
						break;
		case RDD:		if (labat) label(labat->snum);
						saveseg=curseg; /* make labels default to CSEG */
						curseg=OCSEG;
						do {
							while (ltype[*cur] == SPACE) cur++;
#if X8087
							ptr=cur;
							while (ltype[ch=*ptr] == DIGIT || (toupper(ch) >= 'A' &&
								toupper(ch) <= 'F')) ptr++;
							if (toupper(*ptr) == 'H') goto ishex;
							ptr=cur;
 
							while (*ptr == '-' || ltype[*ptr] == DIGIT)ptr++;
							if (*ptr == '.' ||
								(toupper(*ptr) == 'E' && ptr != cur)) {
								cur+=_finput(cur,&floater,30);
								floater.flt=floater.dbl;
								codew(floater.words[0]);
								codew(floater.words[1]);
								while (ltype[*cur] == SPACE) cur++;
								tokit();
								}
							else {
#endif
ishex:							tokit();
								one(vtype);
								if (vtype[VIS] == CONSTV) {
									codew(vtype[VVAL]);
									codew(vtype[VNAME]);
									}
								else if (is_big && vtype[VIS]==VARV && vtype[VVAL]==8) {
									codew(vtype[VOFF]);
									codew(0);
									fixup(OLNAMEREL, vtype[VNAME]);
									}
								else error("invalid DD constant");
								}
#if X8087
							}
#endif
						while (curch == ',');
						curseg=saveseg;

						if (labat) {
							labat->stype=CLONG;
							}
						break;
#if X8087
		case RDQ:		if (labat) label(labat->snum);
						do {
							cur+=_finput(cur,&floater,30);
							for (i=0; i < 4; i++)
								codew(floater.words[i]);
							while (ltype[*cur] == SPACE) cur++;
							tokit();
							}
						while (curch == ',');

						if (labat) {
							labat->stype=CQWORD;
							}
						break;
#endif
		case RDSEG:		flush(1);
						curseg=ODSEG;
						objb(ODSEG);
						dummyb(ODSEG);
						dummyat=0;
						tokit();
						break;
		case RCSEG:		flush(1);
						curseg=OCSEG;
						objb(OCSEG);
						dummyb(OCSEG);
						dummyat=0;
						tokit();
						break;
		case RTITLE:
		case REJECT:	while (curch != LF && curch != ';' && curch != CONTZ)
							tokit();
						break;
		case RLINE:		tokit();
						one(vtype);
						if (vtype[VIS] != CONSTV)error("bad LINE value");
						linenum(vtype[VVAL]);
						break;
		case REND:		tokit();
						break;
		case RSPLIT:	flush(1);
						ptr=dbuf;
						while (ptr < indummy)
							objb(*ptr++);
						indummy=dbuf;
						dummyat=0;
						objb(OEOF);
						offs[0]=offs[1]=offs[2]=0;	/* total size of dseg ,cseg and reserved */
						offs[3]=offs[4]=0;			/* eseg, eseg reserved */
						tokit();
						if (is_big) objb(OBIG);
						break;
		case RESEG:		flush(1);
						if (is_big == 0) error("ESEG illegal in small case");
						curseg=ODSEG+3;
						objb(OESEG);
						dummyb(OESEG);
						dummyat=0;
						tokit();
						break;
		case RIF:		tokit();
						one(vtype);
						if (vtype[VIS] != CONSTV) badexp();
						if (!vtype[VVAL]) skipsome();
						break;
		case RELSE:		skipsome();
						break;
		case RENDIF:	tokit();
						break;
		default:		error("illegal reserved word");
		}
	}



refill() {
	int  i;

	if (cur == lastlf+1) {
		if (lastch-cur > 110) {
			error("line too long");
			cur=lastch;
			}
		lastlf=&inbuf;
		while (cur < lastch) *lastlf++=*cur++;
		cur=&inbuf;
		bptr=incfile ? incfile: infile;
		i=read(bptr,lastlf,512);
		if (i == -1) {
			os("cannot read ");
			ferror(incfile ? incname: name);
			}
		if (i) {
			lastch=lastlf=lastlf+i;
			while ((*--lastlf != LF) && (*lastlf != CONTZ));
			}
		else {
			*lastlf=26;
			lastch=++lastlf;
			}
		}
	}

fill() {
	lastlf=0;
	lastch=cur=1;
	curch=LF;
	refill();
	}

doinc() {
	if (incfile) { error("nested include"); return;}
	tokit();
	if (heir != DSTRING) { error("bad include"); return; }
	lastinc=&savein;
	endline();
	while (cur < lastch) *lastinc++=*cur++;
	if ((incfile=open(string,0)) == -1) {
		os("cannot open ");
		ferror(string);
		}
	lastline=line+1;
	line=0;
	i=0;
	do
		incname[i]=string[i];
	while (string[i++]);
	fill();
	}

dumpsym() {
	int  val,name;
	char found;

	bptr=_memory();
	while (bptr < freem) {
		val=bptr->word;
		bptr+=2;
		if (sopt) {
			ohw(bptr-2);
			ohw(val);
			os(bptr);
			oc(' ');
			}
		name=bptr;
		while (*bptr++) ;
		switch (*bptr++) {
			case MNUM:
			case FMNUM:
			case PREFIX:	if (sopt) {
								os("MNU  ");
								obnum(*bptr++);
								obnum(*bptr++);
								}
							else bptr+=2;
							break;
			case RESERVED:	if (sopt) {
								os("RSRV ");
								obnum(*bptr++);
								}
							else bptr++;
							break;
			case SEGREG:	if (sopt) {
								os("SEGREG ");
								obnum(*bptr++);
								}
							else bptr++;
							break;
			case REG:		if (sopt) {
								os("REG ");
								obnum(*bptr++);
								obnum(*bptr++);
								}
							else bptr+=2;
							break;
			case EQU:		if (sopt) {
								os("EQU ");
								while (*bptr != LF)
									oc(*bptr++);
								}
							else {
								while (*bptr != LF) bptr++;
								}
							bptr++;
							break;
			case NAME:		bptr--;
							if (bptr->sclass == 0) {
								os("undefined variable - ");
								os(name);
								nerr++;
								ocrlf();
								}
							if (sopt) {
								obnum(bptr->sclass);
								obnum(bptr->svalue);
								obnum(bptr->stype);
								onum(bptr->snum);
								}
							bptr+=8;
							break;
			default:		os("Mystery ");
							bptr--;
							obnum(*bptr);
							bptr++;
			}
		if (sopt) ocrlf();
		}
	}

ferror(str)
	char *str; {

	os(str);
	ocrlf();
	if (obj) {
		close(obj);
		unlink(objname);
		}
	exit(2);
	}

skipsome() {
	char ifnest,templopt;

	ifnest = 1;
	while(ifnest) {
		endline();
		if (lopt && inpipe > &pipe[940]) flush(0);
		if (*cur == CONTZ) {
			curch = *cur;
			break;
			}
		needline();
		while (ltype[*cur] == SPACE) cur++;
		if (match(cur, "IF"))
			ifnest++;
		else if (match(cur,"ENDIF")) 
			ifnest--;
		else if (match(cur, "ELSE") && ifnest == 1)
			ifnest = 0;
		curch = *cur == CONTZ ? CONTZ: 0;
		}
	templopt=lopt;
	lopt=0;
	endline();
	lopt=templopt;
	}


match(str1,str2)
	char *str1, *str2; {

	while(*str2) {
		if (toupper(*str1++) != *str2++) return 0;
		}
	if (ltype[*str1] == SPACE || *str1 == CR || *str1 == CONTZ) return 1;
	else return 0;
	}
