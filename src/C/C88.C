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
/*      the-grue - 20180823
 *      Added lines to add OpenC88 header
 */
/*	 C88 COMPILER			C88.C	*/

#include "PASS1.H"
#include "NODES.H"
#if CHECK
#include "OBJ.H"
#endif
char nextpgm[70]="Z:GEN ", mmonths[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
char ltype[128]={34,34,34,34,34,34,34,34,34,SPACE,
				 21,34,SPACE,SPACE,34,34,34,34,34,34,
				 34,34,34,34,34,34,20,34,34,34,
				 34,34,3,4,5,34,34,6,7,8,
				 34,11,9,10,11,12,13,14,DIGIT,DIGIT,
				 DIGIT,DIGIT,DIGIT,DIGIT,DIGIT,DIGIT,DIGIT,DIGIT,34,34,
				 15,16,17,34,34,LETTER,LETTER,LETTER,LETTER,LETTER,
				 LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,
				 LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,
				 LETTER,34,34,34,18,LETTER,34,LETTER,LETTER,LETTER,
				 LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,
				 LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,LETTER,
				 LETTER,LETTER,LETTER,34,19,34,34,34};

static char pNest;

main(argc,argv)
	int  argc;
	char *argv[]; {
	int  i;

#if LIMITED
	os("OpenC88 Compiler  V0.1    Based on\n");
	os("C88 Compiler    Special Version   V3.01    (c) Mark DeSmet, 1986\n");
#else
	i=2;
	while (i < argc) if (*(argv[i++]+1) == '@') see_exit=1;
#endif
	init(argc,argv);
	while (curch != CONTZ) {
		ntree=1;
		global();
		if (ntree > 1) {
			ctlb(3);		/* flag flush of tree */
			ntree=1;
			}
		}
	endit();
	}

static showVersion(void)
{
	os("OpenC88 Compiler  V0.1    Based on\n");
	os("C88 Compiler V3.1h  Copyright Mark DeSmet, 1982-1989\n");
}

init(argc,argv)
	int  argc;
	char *argv[]; {
	char *intname,option,cppopt=0;
	int  i,nopt;

/*	options are decoded here. unrecognised ones are passed through

	@	- special SEE option. Followed by hhhh:hhhh.
	A	- produce Assembler file and stop.
	B	- BIG model.
	C	- add checkout information to object.
	Dd	- d is drive to use for overlays.
	E   - write pre=processor output to stdout
	Id	- drive for #include files.
    M   - create .obj instead of .o format output.
	Nname=value  - like a define.
	Oname - object name.
	Px	- Set pragma switch x.
	S   - Syntax check only
	Tn	- drive to use for temporaries.
	VE	- print every token in pass 1. print debug information in pass 2.
	VS	- print symbols in pass 1. print the tree in pass 2.
	W	- want warnings on structure passing and return.
	X	- do not invoke next pass ond do not delete temporaries. */

/*	add program name without extension to next invocation.	*/

	if (argc < 2) {
		os("no input file");
		real_exit(2);
		}

	if(argv[1][0] == '-' && toupper(argv[1][1]) == 'V') {
		showVersion();
		real_exit(0);
		}

	cur=argv[1];
	nopt=6;
	i=0;
	while (*cur && *cur != '.') {
		objname[i++]=*cur;
		nextpgm[nopt++]=toupper(*cur++);
		}
/*	install the reserved words	*/

	mfree=_memory();
	maxmem=_showsp()-1224;
	addres(0,"auto.break.case.char.const.continue.default.do.double.else.");
	addres(10,"enum.extern.float.for.goto.if.int.long.noalias.register.return.short.");
	addres(22,"signed.sizeof.static.struct.switch.typedef.union.unsigned.");
	addres(30,"void.");
	*nameat=RCHAR;	/* make VOID same as CHAR	*/
	addres(31,"volatile.while.__LINE__.__FILE__.__DATE__.__TIME__.__STDC__.");
	addres(38,"asm.cdecl.far.fortran.huge.interrupt.near.pascal.");

	add_define(cur="__DESMET__");

	xsetblock();

	macxp=macExp;

	times(mtime);
	mtime[2]=mtime[5]=':';
	dates(mdate);
	mdate[10]=mdate[7];
	mdate[9]=mdate[6];
	mdate[8]='9';
	mdate[7]='1';
	mdate[5]=mdate[4];
	mdate[4]=mdate[3]=='0'?' ':'1';
	i = mdate[1] - '0';
	if(mdate[0] != ' ')
		i += (mdate[0] - '0') * 10;
	strncpy(mdate, &mmonths[(i-1)*3], 3);
	mdate[6]=mdate[3]=' ';

#if LIMITED == 0
	for (nopt=2; nopt < argc; nopt++) {
		strcat(nextpgm," ");
		cur=argv[nopt];
		if (*cur == '-') cur++;		/* allow leading - on options	*/
		option=toupper(*cur);
		if (option != 'I' && option != 'N' && option != 'P')
			strcat(nextpgm,cur);
		do {
			cur++;
			switch (option) {
				case '@':	see_addr(cur);
							*cur=0;
							break;
				case 'A':	aopt=1;
							break;
				case 'B':	is_big=1;
							/*	add LARGE_CASE to symbol table	*/
							add_define(cur="LARGE_CASE");
							add_define(cur="__LARGE__");
							break;
				case 'C':	copt=1;
							break;
				case 'D':	nextpgm[0]=toupper(*cur++);
							break;
				case 'E':	cppopt=1;
							break;
				case 'I':	strcpy(istring,cur);
							if (istring[1] == 0) {
								istring[1]=':';
								istring[2]=0;
								}
							*cur=0;
							break;
				case 'N':	add_define();
							goto next_arg;
							break;
				case 'O':	strcpy(objname,cur);
							*cur=0;
							break;
				case 'P':	while(*cur)
								switch(toupper(*cur++)) {
									case 'T':	trigraphs++;
												break;
									case 'W':	wopt=1;	
												break;
									case 'X':	xkwd=1;
												__stdc=0;
												break;
									}
							break;
				case 'S':	soopt=1;
							break;
				case 'T':	topt=toupper(*cur++);
							break;
				case 'V':	while(*cur)
								switch(toupper(*cur++)) {
									case 'E':	eopt=1;
												break;
									case 'S':	sopt=1;
												break;
									}
							break;
				case 'X':	xopt=1;
				}
			}
		while (*cur);
next_arg:;
		}
#endif

	if(!is_big)	add_define(cur="__SMALL__");
	i=0;
	while (objname[i] && objname[i] != '.') i++;
	if (objname[i] == 0) strcat(objname,".O");
	c_type[0]=CCHAR;
	int_type[0]=CINT;
	u_type[0]=CUNSG;
	l_type[0]=CLONG;
	ff_type[0]=CFLOAT;
	fd_type[0]=CDOUBLE;
	p_type[0]=PTRTO; p_type[1]=CCHAR;

/*	open files	*/

	if(*argv[1] == '-') {
		if(!isatty(0)) {
			strcpy(name, "stdin");
			file=0;
			}
		else {
			error("stdin not a device");
			real_exit(2);
			}
		}
	else {
		strcpy(name,argv[1]);
		for (i=0; i < 63; i++) {
			if (name[i] == 0) {
				strcpy(&name[i],".c");
				break;
				}
			if (name[i] == '.' && name[i+1] != '.') break;
			}
		if ((file=open(name,0)) == -1) {
			os("cannot open ");
			os(argv[1]);
			real_exit(2);
			}
		}

	if(cppopt) {
		char fname[65], *ebuf, *cbuf, newfile;
		unsigned lineno;

		if(see_exit==0) {
			fname[0]=lineno=0;
			ebuf=cbuf=mfree;
			mfree+=512;
			minmem=hiwater=mfree;
			bufEnd=savEnd;
			cur=nestfrom[0]=bufEnd-1;
			*cur=LF;
			lineinc=1;
			dolf(0);
			tokit();
			while (curch != CONTZ) {
				newfile=0;
				if(strcmp(fname,name)) {
					strcpy(fname,name);
					newfile=1;
					}
				if(newfile || lineno != cline) {
					if(cbuf != ebuf) {
						*cbuf++=LF;
						*cbuf=0;
						os(ebuf);
						cbuf=ebuf;
						lineno++;
						}
					if(newfile || cline - lineno > 1) {
						char lbuf[10];
		
						os("# line ");
						itoa(cline, lbuf, 10);
						os(lbuf);
						os(" \"");
						os(name);
						os("\"\n");
						lineno=cline;
						}
					else while(lineno < cline) {
						os("\n");
						lineno++;
						}
					}
				if(*tokat == '$') tokat++;
				while(tokat<cur) {
					char ch;
					if(ltype[(ch=*tokat)] == SPACE)
						break;
					if(ch == '"' || ch == '\'') {
						*cbuf++=*tokat++;
						while(*tokat != ch || *(tokat-1) == '\\')
							*cbuf++=*tokat++;
						}
					*cbuf++=*tokat++;
					}
				*cbuf++=' ';
				tokit();
				}
			if(cbuf != ebuf) {
				*cbuf++=LF;
				*cbuf=0;
				os(ebuf);
				cbuf=ebuf;
				}
			}
		else error("E option not valid from see");
		real_exit(2);
		}

	if (see_exit == 0)
		showVersion();

	if(!soopt) {

#if CHECK
		if (copt) {
			if ((objnum=creat(objname)) == -1) {
				os("cannot create ");
				os(objname);
				real_exit(2);
				}
			inobj=&objbuf[0];
			if (is_big) objb(OBIG);
			objb(ONAME);
			objs(name);
			}
#endif

		/*	determine drive for temporaries.	*/

		intname="A:CTEMP2";
		if (topt) *intname=topt;
		else intname+=2;
		if ((tree=creat(intname)) == -1) {
			os("cannot create CTEMP2");
			real_exit(2);
			}
		intree=&treebuf;

		intname="A:CTEMP1";
		if (topt) *intname=topt;
		else intname+=2;
		if ((control=creat(intname)) == -1) {
			os("cannot create CTEMP1");
			real_exit(2);
			}
		inctl=&ctlbuf;
		}
	minmem=hiwater=mfree;
	bufEnd=savEnd;
	cur=nestfrom[0]=bufEnd-1;
	*cur=LF;
	lineinc=1;
	dolf(0);
	tokit();
	}


addres(nres,res)
	int  res;
	char nres; {
	cur=res;
	while(*cur) {
		tokit();
		newname();
		*nameat++=RESERVED;
		*nameat=++nres;
		mfree=nameat+1;
		cur++;
		}
	}

endit() {
	unsigned isize;
	int i;

	for (i=0; i < 32; i++) {
		char *p;
		for(p=hash[i]; p; p = p->word) {
			char *q = p+2;
			while(*q++) ;
			if(*q == STREF) {
				os("undefined structure ");
				*(p+2) &= 0x7F;
				os(p+2);
				ocrlf();
				nerr++;
				}
			}
		}
	if(!soopt) {
#if CHECK
		if (sopt) dumpsym();
		if (copt) {
			objb(ODSEG);	/* re-establish data default	*/
			if (inobj-objbuf && write(objnum,objbuf,inobj-objbuf) == -1) {
				os("cannot write ");
				os(objname);
				real_exit(2);
				}
			close(objnum);
			if (nerr) unlink(objname);
			}
#endif

		if (nerr == 0) {
			ctlb(0);
			ctlw(nwarn);
			ctlw((hiwater-minmem)/((maxmem-minmem)/100));
			if (write(control,ctlbuf,inctl-ctlbuf) == -1) {
				os("cannot write temporary");
				real_exit(2);
				}
			close(control);
			isize=intree;
			if (write(tree,treebuf,isize-treebuf) == -1) {
				os("cannot write temporary");
				real_exit(2);
				}
			close(tree);

/*	if x option is off, use _chain to trigger gen.	*/

			if (xopt == 0) {
				if (have_asm == 0 && aopt == 0) {
					strcat(nextpgm," Z");
					}
				if (nextpgm[0] == 'Z') _chain(&nextpgm[2]);
				else _chain(nextpgm);
				}
			real_exit(0);					/* otherwise just EXIT	*/
			}
		else {
			close(control);
			close(tree);
		}
		if (nwarn) {
			os("Number of Warnings = ");
			onum(nwarn);
			os("    ");
			}
		os("Number of Errors = ");
		onum(nerr);
		real_exit(2);
		}
	else real_exit(0);
	}



global() {
	char *oldfree, *oldcur=0, oldch, *fundef;
	int  funord,oldnerr,funline;
	char gotlname;

	if (curch == ';') {
		tokit();
		return;
		}
	specs(SEXTERN);

	if (addstor > SEXTERN && addstor != STYPEDEF) {
		error("only STATIC and EXTERN allowed at this level");
		return;
		}
	if (ifch(';')) return;
	do {
		if (addvar(0) == 0) {
			error("illegal external declaration");
			newfun(mfree,0,0);
			return;
			}
		was_ext=0;
		atype(addtype);
#if CHECK
		if (copt && addstor != STYPEDEF)
			objtype(OPTYPE,addat->noff);
#endif
		if (copt) funline=cline; else funline=0;
		if (addat->ntype[0] == FUNCTION) {
			fundef=addat;
			funtype=addat->ntype[1];		/* needed to force ret type */
			fun_ret_len=0;
			if (funtype == CSTRUCT) {
				struct_type=&addat->ntype[1];
				fun_ret_len=struct_type->ststagat->staglen;
				pbytes=is_big ? 4: sizeof(char *);
				}
			else pbytes=0;
			if (is_big && funtype == PTRTO) ;
			else if (funtype > CDOUBLE) funtype=CUNSG;
			oldfree=mfree;
			funord=addat->noff;
			funname=addat-addat->nlen;
			gotlname=0;
			if ((addparm || addproto) && nested > pNest) {
				macxp = macfrom[pNest + 1];
				nested = pNest;
				}
			if (addparm) {
				gotlname=1;
#if CHECK
				if (copt) {	/* supply name for locals */
					objb(OLNAME);
					objs(funname);
					}
#endif
				oldnerr=nerr;
				cur=addparm;
				lineinc=1;
				tokit();
				doparms();
				if (oldnerr != nerr)
					while (curch != '{' && curch != CONTZ) tokit();
				}
			if (addproto) {
				struct pargs *pp, **lp;

				oldcur=cur;
				oldch=curch;
				wp=mfree;
				*wp=protohash[addat&31];
				protohash[addat&31]=wp;
				*(wp+1)=addat;
				*(lp=wp+2)=pp=wp+3;
				if(macproto) {
					cur = macproto;
					while(cur != addproto) tokit();
					}
				else cur=addproto;
				tokit();
				if(heir == RESERVED && curch == 'v' && *cur == ')')
					goto noargs;
				do {
					if(heir == RESERVED && bvalue == RELIPSE) {
						*lp=-1;
						tokit();
						notch(')');
						goto eliparg;
						}
					addloc=pp->ptype;
					specs(SUNSTOR);
					getab(1);
					if (addtype > 255) {
						char typb;

						do {
							*addloc++=typb=*addtype++;
							if (typb == ARRAY) {
								*addloc++=*addtype++;
								*addloc++=*addtype++;
								}
							} while (typb >= FUNCTION);
						if (typb == CSTRUCT) {
							addloc->word=addtype->word;
							addloc+=2;
							}
						}
					else *addloc++=addtype;
					lp=pp;
					*lp=pp=addloc;
					} while(ifch(','));
				notch(')');
noargs:			*lp=0;
eliparg:		oldfree=mfree=pp;
				cur=oldcur;
				curch=oldch;
				}
			if (ifch('{')) {
				if(addproto) {
					int poff=0;

					findover=mfree-1;
					if(macproto) {
						cur = macproto;
						while(cur != addproto) tokit();
						}
					else cur=addproto;
					macproto=addproto=0;
					cline++;
					lineinc=0;
					tokit();
					if(heir == RESERVED && curch == 'v' && *cur == ')') {
						tokit();
						goto voidspec;
						}
					do {
						if (specs(SPARM) == 0 || addvar(0) == 0) {
							error("bad parameter declare");
							findover=0;
							if (addat > mfree) mfree=addat;
							break;
							}
						addat->noff=poff+4+(is_big+is_big); /* 4 or 6 bytes from bp */
						atype(addtype);
						poff+=2;
						if (addat->ntype[0] == CLONG) poff+=2;

/*	could be a structure	*/
						else if (addat->ntype[0] == CSTRUCT) {
							struct_type=addat->ntype;
							poff+=struct_type->ststagat->staglen-2;
							if (wopt) warning("structure argument");
							}
						else if (is_big && addat->ntype[0] == PTRTO) poff+=2;
/*	a FLOAT is really a DOUBLE	*/
						else {
							if (addat->ntype[0] == CFLOAT) addat->ntype[0]=CDOUBLE;
							if (addat->ntype[0] == CDOUBLE) poff+=6;
							}
						} while (ifch(','));
voidspec:			notch(')');
					notch('{');
					}

#if CHECK
				if (copt && gotlname == 0) {	/* supply name for locals */
					objb(OLNAME);
					objs(funname);
					}
#endif
				/* funname is to separate statics	*/
				newfun(oldfree,funord,funline);
				funname=0;
				return;
				}
			funname=0;
			addproto=0;
			}
		else doinit();
		}
	while (ifch(','));
	notch(';');
	}

doparms() {
	char *next;
	int  *pchain,bigger,*fchain;

	pchain=0;
	cur=addparm;
	addparm=0;
	findover=mfree-1;
	tokit();
	do {
		if (heir == OPERAND && nameat < findover) heir=UNDEF;
		if (heir != UNDEF) {
			error("duplicate argument");
			findover=0;
			return;
			}
		newname();
		if (pchain) *pchain=nameat;
		else fchain=nameat;
		pchain=&nameat->nchain;
		nameat->opcl=OPERAND;
		nameat->nstor=SAUTO;
		nameat->noff=pbytes+4+(is_big+is_big); /* 4 or 6 bytes from bp */
		pbytes+=2;					/* assume parms are two bytes each */
		nameat->ntype[0]=CINT;		/* assume parm is integer */
		for (i=1; i <= 10; i++)		/* but leave room to grow */
			nameat->ntype[i]=0xff;
		mfree=&nameat->ntype[11];
		tokit();
		}
	while (ifch(','));

	if (notch(')')) return;
	while (specs(SPARM)) {
		do {
			if (curch == ';') break;	/* allow struct decls without parms*/
			if (addvar(0) == 0 || addat > mfree) {
				error("bad parameter declare");
				findover=0;
				if (addat > mfree) mfree=addat;
				return;
				}
			addparm=0;
			atype(addtype);

/*	if parm is not 2 bytes long, must adjust offsets	*/

			bigger=0;
			if (addat->ntype[0] == CLONG) bigger=2;

/*	could be a structure	*/
			else if (addat->ntype[0] == CSTRUCT) {
				struct_type=addat->ntype;
				bigger=struct_type->ststagat->staglen-2;
				if (wopt) warning("structure argument");
				}
			else if (is_big && addat->ntype[0] == PTRTO) bigger=2;

/*	a FLOAT is really a DOUBLE	*/
			else {
				if (addat->ntype[0] == CFLOAT) addat->ntype[0]=CDOUBLE;
				if (addat->ntype[0] == CDOUBLE) bigger=6;
				}
			if (bigger) {
				pbytes+=bigger;
				next=addat->nchain;
				while (next) {
					next->noff+=bigger;
					next=next->nchain;
					}
				}
			}
		while (ifch(','));
		if (notch(';')) break;
		}
	findover=0;
#if CHECK
	if (copt) {
		while (fchain) {
			addat=fchain;
			objtype(OLTYPE,addat->noff);
			fchain=((void *)fchain)->nchain;
			}
		}
#endif
	}

newfun(oldfree,funord,funline)
	char *oldfree;
	int  funord,funline; {
	int  pstart,rnode[3];
	unsigned retval;
	
#if LIMITED == 0
	if (sopt && nerr > 0) dumpsym();
#endif
	locoff=0;
	laststmt=0;
	pstart=compound(oldfree,1);
	if (laststmt != RET) {
		retval=0;
		if (fun_ret_len) error("must return structure");
		else if (funtype == CFLOAT || funtype == CDOUBLE) {
			warning("must return float");
			rnode[1]=oconst(0);
			rnode[0]=CAST+(funtype<<8);
			retval=tree2(rnode);
			}
		rnode[0]=RET;
		rnode[1]=retval;
		rnode[2]=tree2(rnode);
#if CHECK
		if (copt) {
			rnode[0]=STMT;
			rnode[1]=rnode[2];
			rnode[2]=retnum;
			rnode[2]=tree3(rnode);
			}
#endif
		rnode[0]=LST+2;
		rnode[1]=pstart;
		pstart=tree3(rnode);
		}

	ctlb(2);
	ctlw(funord);
	ctlw(ntree-1);
	ctlw(pstart);
	ctlw(pbytes);
	if (locoff & 1) locoff--;
	ctlw(locoff);
	ctlw(funline);
	}



/*	put any storage types into addstor and a pointer to a type in addtype */

specs(defstor)
	char defstor; {
	char tempstor,got,tempstru;
	int  tempaddat;

	addstor=defstor;		/* default storage class */
	addtype=CINT;			/* integer is always default */
	got=0;					/* see if have anything */

	while (1) {
		if (heir == RESERVED) {
			got=1;
			switch (bvalue){
				case RCONST:
				case RNOALIAS:
				case RVOLATILE:
				case RCDECL:
				case RFAR:
				case RFORTRAN:
				case RHUGE:
				case RINTERRUPT:
				case RNEAR:
				case RPASCAL:
				case RELIPSE:
								break;
				case RAUTO:
				case RREGISTER:	if (addstor != SPARM) addstor=SAUTO;
								break;
				case RSTATIC:	addstor=SSTATIC;
								break;
				case REXTERN:	addstor=SEXTONLY;
								break;
				case RTYPEDEF:	addstor=STYPEDEF;
								break;
				case RCHAR:		if(addtype != CSCHAR) addtype=CCHAR;
								break;
				case RSHORT:
				case RINT:		if(addtype != CUNSG && addtype != CLONG) addtype = CINT;
								break;
				case RSIGNED:	addtype=CSCHAR;
								break;
				case RLONG:		if (addtype == CUNSG) error("illegal type");
								addtype=CLONG;
								break;
				case RUNSIGNED:	if (addtype == CLONG) error("illegal type");
								addtype=CUNSG;
								break;
				case RFLOAT:	addtype=addtype==CLONG ? CDOUBLE: CFLOAT;
								break;
				case RDOUBLE:	addtype=CDOUBLE;
								break;
				case RSTRUCT:
				case RUNION:	tempstor=addstor;
								tempstru=in_stru;
								tempaddat=addat;
								stagat.stat=newstru();
								in_stru=tempstru;
								addstor=tempstor;
								addat=tempaddat;
								stagat.cstype=CSTRUCT;
								addtype=&stagat;
								return 1;
				case RENUM:		newenum();
								return 1;
				default:		return 0;
				}
			tokit();
			}
		else if (heir == OPERAND && nameat->nstor==STYPEDEF) {
			got=1;
			addtype=&nameat->ntype[0];
			tokit();
			}
		else return got;
		}
	}


addvar(found)
	char found; {
	int  val;

	if (! found) {
		if (curch == '*') {
			tokit();
			found=addvar(0);
			if (found) *addloc++=PTRTO;
			return found;
			}
		if (ifch('(')) {
			found=addvar(0);
			notch(')');
			return (addvar(found));
			}
		while(heir==RESERVED && bvalue >= RCDECL && bvalue <= RPASCAL)
			tokit();
		if (heir == UNDEF) newname();
		if (heir == OPERAND) {
			if (addstor == SPARM) {
				if (nameat > findover) heir=UNDEF;
				}
			else if (in_stru || nameat < findover || addstor == SEXTONLY ||
				nameat->nstor == SEXTONLY || nameat->ntype[0] == FUNCTION) {
				if (addstor == SEXTERN || addstor == SEXTONLY || 
					(addstor == SSTATIC && funname == 0)) original=nameat;
				newname();
				heir=UNDEF;
				}
			}
		if (heir == UNDEF) {
			addat=nameat;
			*nameat=OPERAND;
			addat->nstor=(addstor == SPARM) ? SAUTO : addstor;
			addloc=&nameat->ntype[0];
			found=1;
			tokit();
			}
		}
	if (found) {
		while (curch == '(' || curch == '[') {
			if (curch == '(') {		/* skip past parms if any */
				pNest = nested;
				addparm=cur;
				macproto=addproto=0;
				tokit();
				if((heir==OPERAND && nameat->nstor==STYPEDEF)
					|| (heir!=UNDEF && heir!=OPERAND)) {
					addproto=addparm;
					addparm=0;
					if(cur > savEnd && cur < macEnd)
						macproto=lastname;
					}
				if (curch != ')') {
					int rplvl = 1;
					while(rplvl){
						if(curch == '(')
							rplvl++;
						if(curch == ')')
							rplvl--;
						tokit();
						}
					}
				else {
					addparm=addproto=0;
					tokit();
					}
				*addloc++=FUNCTION;
				}
			else {
				tokit();
				if (heir == CONSTANT || heir == LCONSTANT || curch == '('
				    || (heir == RESERVED && bvalue == RSIZEOF)) val=constexp();
				else val=-1;
				if (addstor == SPARM && addloc == &addat->ntype[0]) {
					*addloc++=PTRTO;
					}
				else {
					*addloc++=ARRAY;
					if (val != -1) {
						addloc->word=val;
						}
					else {
						if (addstor <= SEXTERN || addstor == STYPEDEF)
							addloc->word=-1;
						else {
							error("sorry, must have dimension for locals");
							addloc->word=1;
							}
						}
					addloc+=2;
					}
				notch(']');
				}
			}
		}
	return found;
	}

atype(addtype)			/* add type byte(s) to end of operand types */
	char *addtype; {
	char i,typb;

	if (addtype > 255) {
		if (addstor == SPARM && *addtype==ARRAY && addloc==&addat->ntype[0]){
			addtype+=3;
			*addloc++=PTRTO;
			}
		do {
			*addloc++=typb=*addtype++;
			if (typb == ARRAY) {
				*addloc++=*addtype++;
				*addloc++=*addtype++;
				}
			}
		while (typb >= FUNCTION);
		if (typb == CSTRUCT) {
			addloc->word=addtype->word;
			addloc+=2;
			}
		}
	else *addloc++=addtype;

	if (mfree < addloc) {
		if (addstor <= SEXTERN) {
			if (original) {
							/* have older definition */
				addat->noff=original->noff;
				i=0;			/* types must agree	*/
				while (1) {
					typb=original->ntype[i];
					if (typb != addat->ntype[i]) {
						warning("conflicting types");
						break;
						}
					if (typb == ARRAY) i+=2;
					else if (typb != FUNCTION && typb != PTRTO) break;
					i++;
					}
				if (addat->ntype[0] == FUNCTION) original=0;
				else {
					if (addstor != SEXTONLY) {
						original=0;
						addat->nstor=SSTATIC;
						was_ext=1;
						}
					}
				}
			else {
				addat->noff=++ordinal;

				if (addat->ntype[0] == FUNCTION) {
					while (addloc <= &addat->ntype[10])
						*addloc++=0xff;
					addat->nchain=hash[32];
					hash[32]=addat;
					ctlb(1);
					ctlb(addstor == SEXTONLY ? SEXTERN: addstor);
					ctlw(addat->noff);
					ctls(addat-addat->nlen);
					ctlb(INITFUN);
					}
				}
			}
		mfree=addloc;
		}
	}

newstru() {
	char is_stru,*struat,bitsin;
	int  sesize,*memchain;
	char *real_type;

	is_stru=bvalue == RSTRUCT;
	tokit();
	if (heir == OPERAND || heir == UNDEF) {
		string[0]|=0x80;
		bptr=hash[hashno];
		while (bptr) {
			j=-1;
			wp=bptr;
			bptr+=2;
			while (*bptr++ == string[++j]);
			if (j > i) {
				nameat=wp;
				nameat+=i+3;
				heir=*nameat;
				if (heir == STAG || heir == STREF) goto gotSTAG;
				}
			bptr=*wp;
			}
		heir=UNDEF;
		}
gotSTAG:
	in_stru=1;
	if (heir == STAG || heir == STREF) {
		memchain=struat=nameat;
		tokit();
		}
	else {
		if (heir != UNDEF) {	/* create dummy stag	*/
			mfree->word=anon;
			anon=mfree;
			(mfree+2)->byte=CSTRUCT;
			(mfree+3)->word=mfree+5;
			memchain=struat=nameat=mfree+5;
			}
		else {
			string[0]|=0x80;
			newname();
			*wp=hash[hashno];
			hash[hashno]=wp;
			memchain=struat=nameat;
			tokit();
			}
		nameat->stagcl=STREF;
		mfree=&nameat->staglen;
		mfree+=2;
		}

	if (ifch('{')) {			/* allow pointer to forward structure */
		if(*struat != STREF && wopt)
			warning("structure redefinition");
		*struat = STAG;
		struat->schain=struat->staglen=0;
		if (is_stru == 0) struat->staglen=2;
		bitsin=0;
		do {
			if (specs(SMEMBER) == 0) {
				error("bad member declare");
				return struat;
				}
			if (addstor != SMEMBER) {
				error("bad member storage");
				return struat;
				}
			real_type=addtype;
			do {
				if (ifch(':')) {
					if (heir != CONSTANT || wvalue > 16) {
						error("field needs constant");
						return struat;
						}
					if (wvalue == 0 || wvalue+bitsin > 16) {
						if (is_stru && bitsin) struat->staglen+=2;
						bitsin=wvalue;
						}
					else bitsin+=wvalue;
					tokit();
					continue;
					}
				if (addvar(0) == 0) {
					/* allow stags in unions	*/
					if (!is_stru && addtype == &stagat) {
						sesize=stagat.stat->staglen;
						if (sesize == 0) error("undefined structure");
						goto got_size;
						}
					error("bad STRUCT declare");
					return struat;
					}
				addparm=0;
				((void *)memchain)->nchain=addat;
				memchain=addat;
				if (ifch(':')) {
					if (heir != CONSTANT || wvalue > 16 || wvalue == 0) {
						error("field needs constant");
						return struat;
						}
					if (wvalue+bitsin > 16) {
						if (is_stru) struat->staglen+=2;
						bitsin=0;
						}
					addtype=BITS+(wvalue-1)*16+bitsin;
					bitsin+=wvalue;
					if (is_stru) addat->noff=struat->staglen;
					else addat->noff=0;
					addloc=&addat->ntype[0];	/* forget real type */
					atype(addtype);
					tokit();
					}
				else {
					if (bitsin) {
						if (is_stru) struat->staglen+=2;
						bitsin=0;
						}
					atype(addtype);
#if CHECK
					if (copt) {
						objtype(OMTYPE,is_stru ? struat->staglen: 0);
						}
#endif
					sesize=dsize(&addat->ntype[0]);
got_size:			if (is_stru) {
						addat->noff=struat->staglen;
						struat->staglen+=sesize;
						}
					else {
						if (sesize > struat->staglen) struat->staglen=sesize;
						addat->noff=0;
						}
					}
				addtype=real_type;
				}
			while (ifch(','));
			notch(';');
			}
		while (! ifch('}'));
		if (bitsin && is_stru) struat->staglen+=2;
		}
	return struat;
	}


/*	NEWENUM  --	add enumerated types. treat as defines.	*/

newenum() {
	char *defat;
	int enumnum,num;

	enumnum=0;
	tokit();
	/*	ignore the enum name if present	*/
	if (heir == OPERAND || heir == UNDEF) {
		tokit();
		}

	/* if list of values, treat as #define x 1, #define y 2 etc.	*/
	if (ifch('{')) {
		do {
			while (ifch(','));
			if(ifch('}'))return;
			if (heir != UNDEF) {error_l("duplicate enum"); return;}
			newname();
			defat=nameat;
			tokit();
			if (ifch('=')) enumnum=constexp();
			defat->defcl=DEFINED;
			defat->dargs=255;
			defat=&defat->dval;
			*defat++=DEFSTR;
			/*	add number of the enum	*/
			num=enumnum++;
			if (num > 9999) *defat++=num/10000+'0';
			if (num > 999) *defat++=num/1000+'0';
			if (num > 99) *defat++=num/100+'0';
			if (num > 9) *defat++=(num/10)%10+'0';
			*defat++=num%10+'0';
			*defat++=LF;
			*defat=DEFEND;
			mfree=defat+1;
			}
		while (! ifch('}'));
		}
	}
