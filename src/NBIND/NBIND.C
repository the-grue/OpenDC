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
/*	BIND.C		binder for ASM88 and C88	*/

#define IBM		1			/*	true if creating BIND for MS-DOS	*/
#define OVERLAY	1
#define LIMITED	0

#include "OBJ.H"

#define NUMARG	100
#define ABSIZE	2000
#define NUMLAB	2000
#define MAXOVER	40

#define LOTHER		0
#define LPUBLIC		1
#define LRESERVE	2
#define LTYPE		3
#define LTYPERES	4

#define CINT	2
#define CLAB	7
#define CFUN	253

#define INDSEG	0
#define INCSEG	1

#define FOPT	0
#define FOPEN	1
#define FCLOSED	2

#define CONTZ	26
	char inbuf[4234],*inin,*endin,codebuf[650],*incode;
	char databuf[4234],*indata,filen;
	char inname[65],outname[65],chkname[65],ovname[65];
	int  infile[NUMARG],outfile,thisout,*fixat,numab,nerr,chkfile,ovfile;
	char isopen[NUMARG],curseg,*inext,numopen,labis[NUMLAB];
	char *thisname,no_member;

	/*	index 0 for data, 1 for code, 2 for reserved len, 3 for res at */
	unsigned over_offs[MAXOVER][4],offs[4],base[2],maxover;
	int  num_public_patch;
	char *labat[NUMLAB],*public_patch[NUMLAB/2];
	long  dataout,datatot,codeout,codetot,ovbase;
	long  dataout_root,datatot_root,codeout_root,codetot_root;
	unsigned util,stacklen;
	unsigned len,num,nummod;
	char copt,aopt,hopt,mopt,under_opt,*found,islib,libname[65]="CSTDIO.S";
	char *pname,numover[NUMARG],gotover,*fileat[NUMARG],ovnum,lastov;
	char see_exit;
	int  pfile=-1,numfile=1;
	char argbuf[ABSIZE];		/* -f arguments go here	*/
	long objat,objat_root,objat_ov;

#define CHKSIZE	512
	char chkbuf[CHKSIZE],*chkat;

	char initobj[]={5,1,0x5f,0x63,0x73,0x65,0x74,0x75,0x70,0,1,0,0x83,
			0xe9,0,0,7,1,0,11,'M','A','I','N','_',0,2,0,2,253,2,
			1,'M','A','I','N','_',0,2,0,4,0x86,0,0,0,0,0,0,0};

typedef	struct sym{
		struct sym *next;
		unsigned name;
		char eseg,edefn,enlen,eov,eused;
		int elen;
		char typlen, etype[1];
		} Sym;

Sym * hash[32];

	union {int word; char byte; };

	/*	header for overlays. at start of .OV file. followed by code,data
		for each overlay except 0 (root).	*/
	/*	ovlen is length of data, code and reserved for each overlay.	*/

	unsigned ovlen[MAXOVER][3];

#if	IBM
	struct {unsigned sigture,padb,pages,relo,hsize,padc,
			hilo,regss,regsp,csum,regip,regcs,firstr,zero[243];}
			ihead={0x5a4d,0,1,0,32,0,0xffff,0,0,0,0,0,0};

#else
	struct {char cform; int clen,cbase,cmin,cmax;
			char dform; int dlen,dbase,dmin,dmax; char rest[110]; };
#endif

#define MAXMOD 700
	char wantmod[MAXMOD];
	char *memory,*memlast;

unsigned nseg, nfree;
char opt2;

extern char _msdos2;


main(argc,argv)
	int  argc;
	char *argv[]; {
	int  i;

#if LIMITED == 0
	i=2;
	while (i < argc) if (*(argv[i++]+1) == '@') see_exit=1;
	if (see_exit == 0)
		fputs("Object Module Linker X3.0b  Copyright Mark DeSmet, 1988\n",2);
#endif
	init(argc,argv);
	nextpass(1);
	between();
	nextpass(2);
	endup();
	_exit(nerr ? 2:0);
	}

init(argc,argv)
	int  argc;
	char *argv[]; {
	char *argat,*_memory();
	int  nin,i,ffile;
	char * _showds();

#if	IBM==0
	incode=&codebuf[128];
	indata=&databuf[256];
#endif

	inext=memory=_memory();
	memlast=_showsp()-512;
	nseg = _showds() + 0x1000;
	util=argc*3;
	pname="CON:";
	ovnum=0;

/*	create a list fileat of pointers to arguments including arguments in
	-f files.	*/

	for (filen=1; filen < argc; filen++) {
		argat=argv[filen];

/*	-ffile options means arguments are also in the file.	*/
		if (*argat == '-' && toupper(*(argat+1)) == 'F') {
			if ((ffile=open(argat+2,0)) == -1) ferror("cannot open ",argat+2);
			if ((nin=read(ffile,&argbuf[numab],ABSIZE-numab)) == ABSIZE-numab)
				ferror("too many filenames","");
			close(ffile);

			for (i=numab; i < numab+nin;i++) 	/* take care of control Z	*/
				if (argbuf[i] == CONTZ) nin=i-numab;
			for (i=numab; i < numab+nin;) {
				while (iswhite(argbuf[i]))
					argbuf[i++]=0;	/* turn white space into zeros	*/
				if (i < numab+nin) {
					fileat[numfile++]=&argbuf[i];
					while (iswhite(argbuf[++i]) == 0) ;
					}
				}
			numab+=nin;
			}
		else fileat[numfile++]=argat;
		}
	if (numfile > NUMARG) ferror("over 100 arguments","");


	isopen[0]=FCLOSED;
	isopen[numfile]=FCLOSED;
	for (filen=1; filen < numfile; filen++) {
		argat=fileat[filen];
		if (*argat == '-') {
			isopen[filen]=FOPT;
			i=toupper(*++argat);
			switch (i) {
				case '@':	see_exit=1;
							break;
				case 'P':	if (*++argat) pname=argat;
							if ((pfile=creat(pname)) == -1)
								ferror("cannot open ",pname);
							break;
				case 'A':	aopt=1;
							break;
				case 'C':	copt=1;
							break;
				case 'H':	hopt=1;
							break;
				case 'L':	strcpy(libname,argat+1);
							if (strlen(libname) == 1) strcat(libname,":");
							strcat(libname,"CSTDIO.S");
							break;
				case 'N':	if (toupper(*(argat+1)) == 'M') {
								no_member=1;
								argat++;
								}
							break;
				case 'O':	cmdname(argat+1);
							break;
				case 'S':	while (i=toupper(*++argat)) {
								if (i >= '0' && i <= '9') i-='0';
								else if (i >= 'A' && i <= 'Z') i-='A'-10;
								else ferror("bad Stack option","");
								stacklen<<=4;
								stacklen+=i;
								}
							break;
				case 'M':	mopt=1;
				case 'V':	lastov=ovnum;
							ovnum=0;
							while (i=*++argat)
								ovnum=ovnum*10+i-'0';
							if (ovnum <= lastov || ovnum >= MAXOVER)
								ferror("illegal overlay number","");
							maxover=ovnum;
							break;
				case '_':	under_opt=1;
							break;
				case '2':	opt2=1;
							break;
				default:	fputs("bad argument ", 2);
							fputs(argat-1, 2);
							ocrlf();
							nerr++;
				}
			}
		else {
			isopen[filen]=FCLOSED;
			if (outname[0] == 0) cmdname(argat);
			numover[filen]=ovnum;
			}
		}
	if (copt) {
		if ((chkfile=creat(chkname)) == -1)
			ferror("cannot create",chkname);
		chkat=chkbuf;
		}
/*	if MS-DOS V2.0 and no -L option, use path to find CSTDIO.S */
#if IBM

	if (_msdos2 && libname[6] == '.') findfile("CSTDIO.S",libname);
#endif

	}

cmdname(name)
	char *name; {
	char i;

	i=0;
	while (*name && *name != '.') {
		ovname[i]=*name;
		chkname[i]=*name;
		outname[i++]=*name++;
		}
#if	IBM
	strcpy(&outname[i],".EXE");
#else
	strcpy(&outname[i],".CMD");
#endif
	strcpy(&ovname[i],".OV");
	strcpy(&chkname[i],".CHK");
	}



/*	return a 1 if character is a blank, CR or LF	*/

iswhite(ch)
	char ch; {

	return ch == ' ' || ch == '\r' || ch == '\n';
	}



nextpass(pass)
	char pass; {
	char *argat,gotdot;
	int  i;

	nummod=0;

/*	clear all offsets	*/

	if (pass == 2) {
		for (i=1; i <= maxover; i++) {
			over_offs[i][0]=base[0];
			over_offs[i][1]=base[1];
			}
		}
#if	IBM
	over_offs[0][0]=0;
#else
	over_offs[0][0]=256;
#endif
	over_offs[0][1]=0;


	/*	file zero is initobj if A option is off	*/
	/*	numfile file is CSTDIO.S	*/

	for (filen=aopt; filen <= numfile; filen++) {
		gotdot=0;
		i=0;
		curseg=INDSEG;

		if (filen == 0) argat="CSETUP";
		else if (filen == numfile) argat=libname;
		else argat=fileat[filen];
		while (*argat) {
			if (*argat == '.') gotdot=1;
			inname[i++]=*argat++;
			}
		if (gotdot) {
			inname[i]=0;
			}
		else strcpy(&inname[i],".O");

		if (filen == 0) {
			inin=inbuf;
			_move(sizeof(initobj),initobj,inbuf);
			endin=&inbuf[sizeof(initobj)];
			}

		else if (isopen[filen] == FCLOSED) {
			if (numopen == 4) {
				i=numfile-1;
				while (isopen[i] != FOPEN) i--;
				if (close(infile[i]) == -1)
					ferror("cannot close",fileat[i]);
				isopen[i]=FCLOSED;
				}
			else numopen++;
			if ((infile[filen]=open(inname,0)) == -1)
				ferror("cannot open ",inname);
			isopen[filen]=FOPEN;
			inin=endin=&inbuf[4096];
			}
		else if (isopen[filen] == FOPEN) {
			if (lseek(infile[filen],0L,0) == -1)
				ferror("cannot seek ",filen ? fileat[filen] : "STDIO.O");
			inin=endin=&inbuf[4096];
			}
		else continue;

		i=0;
		while (inname[i]) i++;
		islib=0;
		lastov=ovnum;
		if (toupper(inname[i-1]) == 'S' && inname[i-2] == '.') {
			islib=1;
			ovnum=0;
			}
		else ovnum=numover[filen];

	/*	if pass2 must set up to write overlays	*/
		if (ovnum != lastov && pass == 2) set_location();

		for (i=0; i < 4; i++)
			offs[i]=over_offs[ovnum][i];
		do {
			_setmem(labis,NUMLAB,LOTHER);
			if (pass == 1) pass1();
			else pass2();
			if (++nummod > MAXMOD) error("over 700 modules","");
			if (inin >= endin) refill();
			if (*inin == CONTZ) inin=endin;
			}
		while (inin < endin);
		if (offs[0] < over_offs[ovnum][0])
			ferror("over 64K data","");
		if (offs[1] < over_offs[ovnum][1])
			ferror("over 64K code","");
		for (i=0; i < 4; i++)
			over_offs[ovnum][i]=offs[i];
		}
	}

pass1() {
	char clen,*oldext,needed;
	unsigned i,oldoffs[4];
	int  num;
	Sym **ssp, *sp;

	oldext=inext;
	for (i=0; i < 4; i++)
		oldoffs[i]=offs[i];
	needed=1-islib;
	num_public_patch=0;		/* number of times found->edefn is incorrectly
								changed by a module that is not used.	*/
	while (1) {
		if (inin+128 > endin) refill();
		switch (clen=*inin++) {
			case OEOF:		if (needed == 0) {
								inext=oldext;
								for (i=0; i < 4; i++)
									offs[i]=oldoffs[i];
								for (i=0; i < 32; i++) {
									ssp=&hash[i];
									while (*ssp && (char *)(*ssp) < oldext)
										ssp=*ssp;
									*ssp=0;
									}
								wantmod[nummod]=0;
								/* fix the references that were erroniously
									flagged as resolved	*/
								while (num_public_patch)
									*public_patch[--num_public_patch]=LOTHER;
								}
							else wantmod[nummod]=1;
							return;
			case OPUBLIC:	find(1);
							break;
			case OSTATIC:	find(3);
							break;
			case ORESERVE:	num=inin->word;
							inin+=2;
							len=inin->word;
							inin+=2;
							if (labis[num] == LPUBLIC) {
								sp=labat[num];
								switch (sp->edefn) {
									case LOTHER:	sp->edefn=LRESERVE;
													sp->elen=len;
													sp->eov=ovnum;
													if (sp < oldext && sp->eused)
														needed=1;
													break;
									case LRESERVE:	if (sp->elen < len)
														sp->elen=len;
													if (ovnum == 0)
														sp->eov=0;
									case LPUBLIC:	;
									}
								}
							else {
								if (labis[num] == LTYPE) {
									sp=labat[num];
									sp->edefn=LTYPERES;
									sp->eov=ovnum;
									sp->elen=offs[2];
									}
								if (offs[2]+len < offs[2])
									ferror("over 64K of data","");
								offs[2]+=len;
								}
							break;
			case OLOCAL:	num=inin->word;
							if (labis[num] == LPUBLIC) {
								inin+=2;
								len=inin->word;
								inin+=2;
								if (num >= NUMLAB)
									ferror("too many labels in ",inname);
								sp=labat[num];
								if (sp->edefn == LPUBLIC) {
									if (needed) {
										_lmov(sp->enlen, sp->name, nseg, inext, _showds());
										error("multiply defined", inext);
										}
									}
								else {
									sp->edefn=LPUBLIC;
									public_patch[num_public_patch++]=&sp->edefn;
									sp->elen=len+offs[curseg];
									sp->eov=ovnum;
									if (sp < oldext && sp->eused)	needed=1;
									}
								}
							else if (labis[num] == LTYPE) {
								inin+=2;
								len=inin->word;
								inin+=2;
								sp=labat[num];
								sp->eov=ovnum;
								sp->elen=len+offs[curseg];
								}								
							else inin+=4;
							break;
			case ODSEG:		curseg=INDSEG;
							break;
			case OCSEG:		curseg=INCSEG;
							break;
			case ONAMEREL:
			case OJUMPREL:	num=inin->word;
							inin+=2;
							if (needed && labis[num] == LPUBLIC)
								((Sym*)labat[num])->eused=1;
							break;

			case OPTYPE:	if (copt) {
								find(0);	/* remember public type */
								break;
								}
			case OMTYPE:
			case OLTYPE:	while (*inin++) ;
							inin+=2;
							inin+=*inin+1;
							break;
			case OLINE:		inin+=2;
							break;
			case ONAME:
			case OLNAME:	while (*inin++) ;
							break;
			case OBIG:		ferror("small and large case mixed ",inname);
							break;
			case ODFIX:
			case OCFIX:
							break;

			default:		if (clen <= 128)
								ferror("bad object file ",inname);
							clen-=128;
							offs[curseg]+=clen;
							inin+=clen;
			}
		}
	}


pass2() {
	char clen,tlen,want;
	int fix,fixer,add;
	unsigned codebase,database;

	want=wantmod[nummod];
	codebase=offs[INCSEG];
	database=offs[INDSEG];

	while (1) {
		if (inin+128 > endin) refill();
		switch(clen=*inin++) {
			case OEOF:		return;
			case OPUBLIC:	if (want) find(2);
							else {
								while (*inin++) ;
								inin+=2;
								}
							break;
			case OSTATIC:	if (want) find(3);
							else {
								while (*inin++) ;
								inin+=2;
								}
							break;
			case ORESERVE:	if (want) {
								num=inin->word;
								inin+=2;
								len=inin->word;
								inin+=2;
								if (labis[num] == LOTHER) {
									labat[num]=offs[3];
									offs[3]+=len;
									}
								else if (labis[num] == LTYPE) {
									labis[num]=LPUBLIC;
									((Sym*)labat[num])->elen=offs[3];
									offs[3]+=len;
									}
								}
							else inin+=4;
							break;
			case OLOCAL:	num=inin->word;
							if (want) {
								inin+=2;
								len=inin->word;
								inin+=2;
								if (labis[num] == LOTHER || labis[num] == LTYPE) {
									labis[num]=LOTHER;
									labat[num]=len+offs[curseg];
									}
								}
							else inin+=4;
							break;
			case ODSEG:		curseg=INDSEG;
							break;
			case OCSEG:		curseg=INCSEG;
							break;
			case ONAMEREL:	fix=inin->word;
							inin+=2;
							if (want)
							*fixat+=labis[fix] == LPUBLIC ? ((Sym*)labat[fix])->elen:
								labat[fix];
							break;
			case OJUMPREL:	fix=inin->word;
							inin+=2;
							if (want)
							*fixat+=(labis[fix] == LPUBLIC? ((Sym*)labat[fix])->elen:
								labat[fix])-offs[curseg];
							break;
			case OPTYPE:	while (*inin++) ;
							inin+=2;
							inin+=*inin+1;
							break;

			case OMTYPE:	if (no_member) {
								while (*inin++) ;
								inin+=2;
								inin+=*inin+1;
								break;
								}
			case OLTYPE:	if (copt && want) {
								chkb(clen);
								do {
									chkb(toupper(*inin));
									}
								while (*inin++);
								chkw(inin->word);
								inin+=2;
								tlen=*inin;
								do {
									chkb(*inin++);
									}
								while (tlen--);
								outchk();
								break;
								}
							while (*inin++) ;
							inin+=2;
							inin+=*inin+1;
							break;
			case OLINE:		if (copt && want) {
								chkb(OLINE);
								chkw(inin->word);
								chkw(offs[curseg]);
								chkb(0xcc);
								outchk();
								}
							inin+=2;
							break;
			case ONAME:
			case OLNAME:	if (copt && want) {
								chkb(clen);
								do {
									chkb(toupper(*inin));
									}
								while (*inin++);
								outchk();
								break;
								}
							while (*inin++) ;
							break;
			case ODFIX:		if (want) *fixat+=database;
							break;
			case OCFIX:		if (want) *fixat+=codebase;
							break;

			default:		clen-=128;
							if (want == 0) {
								inin+=clen;
								break;
								}
							offs[curseg]+=clen;
							if (curseg == INDSEG) {
								while (clen--)
									*indata++=*inin++;
								if (indata >&databuf[4097]) odata(4096);
								fixat=indata-2;
								}
							else {
								while (clen--)
									*incode++=*inin++;
								if (incode > &codebuf[513]) ocode(512);
								fixat=incode-2;
								}
			}
		}
	}

unsigned maxof();

between() {
	char *hptr,*nameval,*nameptr,*pfrom,ov;
	int  i,*next,nout;
	unsigned *syms,nsym,add,*resptr,alldata,allcode;
	long pad;
	Sym *sp;

	objat=0;
	dataout=codeout=0;
	ovnum=lastov=0;

#if	IBM
	datatot=(((long)over_offs[0][0])+511)&0xffe00;
	ovbase=codetot=(((long)over_offs[0][1])+maxof(1)+511)&0xffe00;
	if (mopt) {
		for (i=1; i <= maxover; i++) {
			codetot+=over_offs[i][1];
			codetot+=over_offs[i][0];
			}
		codetot=(codetot+sizeof(ovlen)+511) & 0xffe00;
		}
#else
	datatot=((long)over_offs[0][0]+127)&0xfff80;
	codetot=((long)over_offs[0][1]+maxof(1)+127)&0xfff80;
	if (mopt) {
		for (i=1; i <= maxover; i++) {
			codetot+=over_offs[i][1];
			}
		codetot=(codetot+127) & 0xfff80;
		}
	hptr=codebuf;
	for (i=0; i< 110; i++)
		hptr->rest[i]=0;
#endif

	/*	allocate reserved in order so public reserved's are in order */
	/*	must do the root first	*/

	over_offs[0][3]=over_offs[0][0];
	sp=memory;
	syms=inext;			/* put array of symbol name pointers at end
						 of symbols	*/
	nsym=0;
 	while (sp < inext) {
		if (sp->edefn == LRESERVE && sp->eov == 0) {
			add=sp->elen;
			sp->elen=over_offs[0][3];
			over_offs[0][3]+=add;
			resptr=&over_offs[0][2];
			if (*resptr+add < *resptr)
				ferror("over 64K data","");
			*resptr+=add;
			}
		else if (sp->edefn == LOTHER && sp->eused) {
			_lmov(sp->enlen, sp->name, nseg, &syms[nsym], _showds());
			fputs("undefined PUBLIC - ", 2);
			fputs(&syms[nsym], 2);
			ocrlf();
			nerr++;
			}
		if (&syms[nsym] < memlast && (sp->eused || sp->typlen >1))
			syms[nsym++]=sp;

		sp = sp->etype + sp->typlen;		/* area of type info	*/
		}

	base[0]=over_offs[0][0]+over_offs[0][2];
	if (base[0] < over_offs[0][0]) ferror("over 64K data","");
	base[1]=over_offs[0][1];

	/*	must do the overlays next	*/

	if (maxover) {
		/*	set resat for each overlay	*/

		for (i=1; i <= maxover; i++)	/*	set resat for every overlay to end of data*/
			over_offs[i][3]=base[0]+over_offs[i][0];

		sp=memory;
	 	while (sp < inext) {
			if (_peek(sp->name, nseg) != 255 && sp->edefn == LRESERVE && sp->eov != 0) {
				add=sp->elen;
				sp->elen=over_offs[sp->eov][3];
				over_offs[sp->eov][3]+=add;
				resptr=&over_offs[sp->eov][2];
				if (*resptr+add < *resptr)
					ferror("over 64K data","");
				*resptr+=add;
				}

			sp = sp->etype + sp->typlen;		/* area of type info	*/
			}
		}

	/* must find address of static reserved vars	*/
	/*	must also add to all overlay addresses	*/

	sp=memory;
 	while (sp < inext) {
		if (_peek(sp->name, nseg) != 255 ) {
			if (sp->edefn != LRESERVE && sp->eov)
				sp->elen+=base[sp->eseg];
			if (sp->edefn == LTYPERES) sp->elen+=over_offs[sp->eov][3];
			}
		sp = sp->etype + sp->typlen;		/* area of type info	*/
		}

	if (pfile != -1 || copt) printsym(nsym,syms);

	alldata=base[0];
	alldata+=maxof(0);
	if (alldata < base[0]) ferror("over 64K data","");
	if (alldata+maxof(2) < alldata) ferror("over 64K data","");
	alldata+=maxof(2);
	allcode=over_offs[0][1]+maxof(1);
	if (allcode < over_offs[0][1]) ferror("over 64K code","");

#if	IBM
/*	generate and output the header sector	*/

	ihead.pages=(codetot>>9)+(datatot >> 9) +1;
	ihead.regss=(codetot)>>4;
	ihead.regsp=alldata+(stacklen ? stacklen : 127);
	if (ihead.regsp < alldata) ferror("over 64K data plus stack","");
	if (stacklen == 0xffff) ihead.regss=ihead.regsp=0;

	codetot+=512;

	pfrom=&ihead;
	incode=codebuf;
	indata=databuf;
	while (incode < &codebuf[26])
		*incode++=*pfrom++;
	while (incode < &codebuf[512])
		*incode++=0;

#else
	hptr->cform=1;
	hptr->cbase=hptr->dbase=0;
	hptr->clen=hptr->cmin=hptr->cmax=codetot>>4;
	codetot+=128;
	hptr->dform=2;
	hptr->dlen=datatot>>4;
#endif


#if IBM==0
	hptr->dmin=(alldata+16)>>4;

	/*	if stack not specified, make it the maximum	*/

	if (stacklen) hptr->dmax=hptr->dmin+(stacklen>>4);
	else hptr->dmax=0xfff;
	if (hopt) hptr->dmax+=0x4000;
#endif

#if	IBM==0
	for (i=0; i < 256; i++)
		databuf[i]=0;
#endif

	next=&initobj[42];
	*next++=over_offs[0][0];
	*next++=alldata-over_offs[0][0];
	*next=ovbase;	
	if ((outfile=creat(outname)) == -1)
		ferror("cannot create",outname);
	thisout=outfile;
	thisname=outname;
	ovbase+=IBM ? 512: 128;
	if (maxover) {
		for (i=0; i < MAXOVER; i++) {
			ovlen[i][0]=over_offs[i][0];
			ovlen[i][1]=over_offs[i][1];
			ovlen[i][2]=over_offs[i][2];
			}
		if (mopt == 0) {
			if ((ovfile=creat(ovname)) == -1)
				ferror("cannot create",ovname);
			if (write(ovfile,ovlen,sizeof(ovlen)) == -1)
				ferror("cannot write",ovname);
			objat_ov=sizeof(ovlen);
			}
		else {
			pad=codetot;
			while (pad) {
				nout=pad > 4096 ? 4096:pad;
				oany(databuf,nout);
				pad-=nout;
				}
			if (lseek(outfile,ovbase,0) == -1)
				ferror("cannot seek ",outname);
			if (write(outfile,ovlen,sizeof(ovlen)) == -1)
				ferror("cannot write",outname);
			if (lseek(outfile,0L,0) == -1)
				ferror("cannot seek ",outname);
			}

		if (pfile != -1) {
			fputs("\noverlay    code    data",pfile);
			for (i=0; i < MAXOVER; i++) {
				if (ovlen[i][0]+ovlen[i][1]+ovlen[i][2]) {
					fputs("\n   ",pfile);
					ohn(i/10); ohn(i % 10);
					fputs("      ",pfile);
					oh(ovlen[i][1]);
					fputs("H   ",pfile);
					oh(ovlen[i][0]+ovlen[i][2]);
					fputs("H",pfile);
					}
				}
			}
		}

/*	print the CODE= message	*/
	if (pfile != -1) {
		fputs("\ncode=",pfile);
		oh(allcode);
		fputs("H  data=",pfile);
		oh(alldata);
		fputs("H  stack and locals=",pfile);
		if (stacklen) oh(stacklen);
		else oh(0-alldata);
		fputs("H\n",pfile);
		}
	}


acmp(Sym **left, Sym **right) {

	switch(((*left)->eseg << 1) | (*right)->eseg){
		case 0:	/* DSEG DSEG */
		case 3: /* CSEG CSEG */
				if((*left)->eov == (*right)->eov) {
					if((*left)->elen == (*right)->elen) return 0;
					return (unsigned)((*left)->elen) > (unsigned)((*right)->elen) ? 1 : -1;
					}
				else
					return (int)((*left)->eov) - (int)((*right)->eov);
		case 1: /* DSEG CSEG */
				return 1;
		case 2: /* CSEG DSEG */
				return -1;
		default: ferror("Bad eseg value", "");
		}
	}

ncmp(Sym **left, Sym **right) {
	unsigned lnm, rnm, lsz, rsz;
	lnm = (*left)->name;
	rnm = (*right)->name;
	lsz = (*left)->enlen;
	rsz = (*right)->enlen;
	while(_peek(lnm, nseg) == '_') {lnm++; lsz--;}
	while(_peek(rnm, nseg) == '_') {rnm++; rsz--;}
	return _lcmp(lnm, nseg, rnm, nseg, lsz > rsz ? lsz : rsz);
	}

/*	print the symbols for 'P' option	*/

printsym(nsym,syms)
	unsigned nsym,syms[]; {

	int i,n;
	char nlen,lastov, ch, fds=0;
	Sym *sp;

/*	sort the symbols	*/

	qsort(syms, nsym, sizeof(Sym *), ncmp);

/*	output public sysbols for checkout option	*/

	if (copt) {
		lastov=0;
		for (i=0; i < nsym; i++) {
			sp=syms[i];
			n = sp->name;
			if (maxover) {
				if (sp->eov != lastov) {
					chkov(lastov=sp->eov);
					}
				}
			if ((ch=_peek(n, nseg)) == 255 || (ch == '_' && under_opt == 0)) continue;
			chkb(OPTYPE);
			do {
				if (ch != '_' || _peek(n+1, nseg)) chkb(ch);
				}
			while (ch=_peek(++n, nseg));
			chkb(0);
			chkw(sp->edefn == LOTHER ? 0 :sp->elen);
			if (sp->etype[0] == 0) {
				if (sp->eseg == INDSEG) {
					sp->typlen=1;
					sp->etype[0]=CINT;
					}
				else { /* assume label */
					sp->typlen=1;
					sp->etype[0]=CLAB;
					}
				}
			found=&sp->typlen;
			nlen=*found;
			do
				chkb(*found++);
			while (nlen--);
			outchk();
			}
		if (lastov) {
			chkov(0);
			}
		if (pfile == -1) return;
		}


/*	print the symbols	*/

	lastov = 0;
	fputs("\tSymbols\n\n", pfile);
	for (i=0; i < nsym; i++) {
		sp=syms[i];
		n=sp->name;
		if ((ch=_peek(n, nseg)) == 255 || (ch == '_' && under_opt == 0)) continue;
		if(maxover) {
			if(sp->eov < 10) fputc( '0', pfile);
			onum(sp->eov, pfile);
			putc(' ',pfile);
			}
		if (sp->eseg == INDSEG) fputs("DS:",pfile);
			else fputs("CS:",pfile);
		oh(sp->elen);
		putc(' ',pfile);
		do
			if (ch != '_' || _peek(n+1, nseg)) fputc(ch, pfile);
		while (ch=_peek(++n, nseg));
		fputs("\n", pfile);
		}

/*	sort the addresses	*/

	qsort(syms, nsym, sizeof(Sym *), acmp);

/*	print the symbols	*/

	fputs("\n\n\tAddresses\n\n", pfile);
	for (i=0; i < nsym; i++) {
		sp=syms[i];
		n=sp->name;
		if ((ch=_peek(n, nseg)) == '_' && under_opt == 0) continue;
		if(maxover) {
			if(sp->eov != lastov) {
				fputs("\n", pfile);
				lastov = sp->eov;
				}
			if(sp->eov < 10) fputc( '0', pfile);
			onum(sp->eov, pfile);
			putc(' ',pfile);
			}
		if (sp->eseg == INDSEG) fputs("DS:",pfile);
			else fputs("CS:",pfile);
		oh(sp->elen);
		putc(' ',pfile);
		do 
			if (ch != '_' || _peek(n+1, nseg)) fputc(ch, pfile);
		while (ch=_peek(++n, nseg));
		fputs("\n", pfile);
		}
	}


/*	SET_LOCATION	--	set code and data locations for .EXE or .OV file */

set_location() {
	int  i;

	if (copt) {			/* say which overlay	*/
		chkov(ovnum);
		}

	if (incode-codebuf) ocode(incode-codebuf);
	if (indata-databuf) odata(indata-databuf);
	incode=codebuf;
	indata=databuf;
	if (lastov == 0) {		/* remember .EXE locations. */
		objat_root=objat;
		codeout_root=codeout;
		codetot_root=codetot;
		dataout_root=dataout;
		datatot_root=datatot;
		}
	else objat_ov=objat;

	if (ovnum == 0) {		/* restore .EXE locations.	*/
		if (mopt == 0) objat=objat_root;
		codeout=codeout_root;
		codetot=codetot_root;
		dataout=dataout_root;
		datatot=datatot_root;
		thisout=outfile;
		thisname=outname;
		}
	else {					/* set .OV locations	*/
		if (mopt == 0) {
			objat=objat_ov;
			thisout=ovfile;
			thisname=ovname;
			}
		codeout=sizeof(ovlen);
		if (mopt) codeout+=ovbase;
		dataout=0;
		for (i=1; i < ovnum; i++) { /* see where this overlay belongs */
			codeout+=ovlen[i][1];
			codeout+=ovlen[i][0];
			}
		codetot=codeout+ovlen[ovnum][1];
		datatot=dataout+ovlen[ovnum][0];
		}
	}





/*	MAXOV	--	return maximum of overlay sizes	*/

unsigned maxof(index)
	int index; {
	int i;
	unsigned max;

	max=0;
	for (i=1; i <= maxover; i++) {
		if (over_offs[i][index] > max) max=over_offs[i][index];
		}
	return max;
	}



find(pass)
	char pass; {
	char hashno,ch,mlen,nlen,*target,name[80];
	int  tlen,dif;
	Sym **ssp, *sp;

	hashno=nlen=0;
	do {
		ch=*inin++;
		if (!opt2 && ch >= 'a') ch-=32;
		name[nlen++]=ch;
		hashno+=ch;
		}
	while (ch);

	name[nlen]='!';

	hashno+=nlen;
	hashno&=31;
	num=0;
				/* get the variable number	*/
	num=inin->word;
	inin+=2;

	ssp=&hash[hashno];
	while (*ssp) {
		sp=*ssp;
		mlen=0;
		if (sp->enlen == nlen &&
				_lcmp(name, _showds(), sp->name, nseg, nlen) == 0) {
			if (ovnum && sp->eov && ovnum != sp->eov) goto next_find;
			if (pass == 0) {		/* pass 0 is debug information */
				tlen=*inin;
				target=sp->etype;
				if (*target == 0) {
					inin+=tlen+1;
					}
				else {
					dif=0;
					do {
						if (*target++ != *inin++) dif=1;
						}
					while (tlen--);
					if (dif && pass == 1) {
						fputs("conflicting types for ", 2);
						fputs(name, 2);
						fputs(" in ", 2);
						fputs(inname, 2);
						putchar('\n', 2);
						}
					}
				}
			else {
				if (pass == 3) {
					if (sp->edefn != LTYPE) return; /* ignore static if
															have a public.	*/
					if (labis[num] == LOTHER) {
						labis[num]=LTYPE;
						labat[num]=sp;
						return;
						}
					}
				if (sp->edefn >= LTYPE) {
					sp->eseg=curseg;
					sp->edefn=LOTHER;
					sp->enlen=nlen;
					sp->eov=ovnum;
					sp->elen=num;
					}
				if (sp->eseg != curseg && pass == 1)
					error("different segments for",name);
				/* kill multiple symbols where two overlays call puts etc.	*/
				if (pass == 4) {
					_poke(255, sp->name, nseg);
					sp->edefn=LTYPE;
					}
				else if (pass) {
					pass=4;
					labis[num]=LPUBLIC;
					labat[num]=sp;
					}
				}
			if (pass != 4 || ovnum) return;
			}
next_find:
		ssp=*ssp;
		}
	if (pass == 4) return;
	*ssp=sp=inext;
	sp->next=0;
	sp->name = nfree;
	if(nfree + nlen > nfree)
		nfree = _lcpy(nfree, nseg, name, _showds());
	else
		ferror("String space overflow", "");
	sp->eseg=curseg;
	sp->eused=0;
	sp->edefn=pass ? LOTHER: LTYPE;
	sp->enlen=nlen;
	sp->eov=ovnum;
	sp->elen=num;
	inext=sp->etype;
	if (pass == 0) {
		sp->typlen=tlen=*inin++;
		while (tlen--)
			*inext++=*inin++;
		}
	else if (copt) {
		sp->typlen=2;
		*inext++=0;
		*inext++=0;
		}
	else {
		sp->typlen=1;
		*inext++=0;
		}
	if (inext >= memlast)
		ferror("too many total PUBLICs in ",inname);
	if (pass) {
		labis[num]=pass == 3 ? LTYPE: LPUBLIC;
		labat[num]=sp;
		}
	}

refill() {
	char *temp;
	int  numbytes;

	if (endin < &inbuf[4096]) return;
	temp=inbuf;
	while (inin < endin) *temp++=*inin++;
	if ((numbytes=read(infile[filen],temp,4096)) == -1)
		ferror("cannot read ",inname);
	endin=temp+numbytes;
	inin=inbuf;
	}

/*	output data or code	*/

odata(bytes)
	int  bytes; {
	int  nout;
	int  i,num;
	long target,pad;

	target=codetot+dataout;
	if (objat != target) {
		if (dataout || mopt) {
			if (lseek(thisout,target,0) == -1)
				ferror("cannot seek ",thisname);
			}
		else {
			pad=target-codeout;
			while (pad) {
				nout=pad > 4096 ? 4096:pad;
				oany(databuf,nout);
				pad-=nout;
				}
			}
		objat=target;
		}
	oany(databuf,bytes);
	objat+=bytes;
	dataout+=bytes;
	if (dataout < datatot) {
		num=indata-&databuf[bytes];
		indata=&databuf[num];
		while (num > 0) {
			num--;
			databuf[num]=databuf[num+bytes];
			}
		}
	}

ocode(bytes)
	int  bytes; {
	int  num;

	if (objat != codeout) {
		if (lseek(thisout,codeout,0,0) == -1)
			ferror("cannot seek ",thisname);
		objat=codeout;
		}
	oany(codebuf,bytes);
	objat+=bytes;
	codeout+=bytes;
	if (codeout < codetot) {
		num=incode-&codebuf[bytes];
		incode=&codebuf[num];
		while (num > 0) {
			num--;
			codebuf[num]=codebuf[num+bytes];
			}
		}
	}

oany(from,num)
	char *from;
	int  num; {

	if (write(thisout,from,num) == -1)
		ferror("cannot write ",thisname);
	}

endup() {
	unsigned max;

	if (incode-codebuf) ocode(incode-codebuf);
	if (datatot-dataout) odata((int)(datatot-dataout));
	if (close(outfile) == -1)
		ferror("cannot close ",outname);
	if (maxover) {
		if (close(ovfile) == -1)
			ferror("cannot close ",ovname);
		}

	if (pfile != -1) {
		if (close(pfile) == -1)
			ferror("cannot close ",pname);
		}
	if (copt) {
		chkb(OEOF);
		outchk();
		if (chkat-chkbuf && write(chkfile,chkbuf,chkat-chkbuf) == -1)
			ferror("cannot write",chkname);
		if (close(chkfile) == -1) ferror("cannot close",chkname);
		}

	max=(inext-memory)/((memlast-memory)/100);
	if (max > util) util=max;
	nummod/=7;
#if LIMITED == 0
	if (see_exit == 0) {
		fputs("end of BIND        ", 2);
		onum(util > nummod ? util: nummod, 2);
		fputs("% utilization    ", 2);
		}
#endif
	if (nerr) {
		onum(nerr, 2);
		fputs(" errors", 2);
		}
	}

/*	output to .CHK file	*/

char chkline[80];
int  cindex;

chkb(ch)
	char ch; {
	chkline[cindex++]=ch;
	}

outchk() {
	int  i;

	chkbyte(chkline[0]);
	chkbyte(cindex+1);
	i=1;
	while (i < cindex)
		chkbyte(chkline[i++]);
	cindex=0;
	}


	
chkbyte(ch)
	char ch; {

	if (chkat == &chkbuf[CHKSIZE]) {
		if (copt && write(chkfile,chkbuf,CHKSIZE) == -1)
			ferror("cannot write",chkname);
		chkat=chkbuf;
		}
	*chkat++=ch;
	}

chkw(wrd)
	unsigned wrd; {

	chkb(wrd);
	chkb(wrd>>8);
	}


/*	output a check record to identify the overlay	*/

chkov(ovn)
	int  ovn; {

	chkb(OOV);
	chkb(ovn);
	outchk();
	}


ferror(str1,str2)
	char *str1,*str2; {

	ocrlf();
	fputs(str1, 2);
	fputs(str2, 2);
	fputs("     BIND abandoned\n", 2);
	exit(2);
	}

error(str1,str2)
	char *str1,*str2; {

	fputs(inname, 2);
	fputs(" - ", 2);
	fputs(str1, 2);
	putchar(' ', 2);
	fputs(str2, 2);
	ocrlf();
	nerr++;
	}

ocrlf() {

	putchar(10);
	}

ohn(ch)
	char ch; {

	ch=(ch&15)+'0';
	putc(ch > '9' ? ch+7: ch,pfile);
	}

oh(num)
	int  num; {

	ohn(num>>12);
	ohn(num>>8);
	ohn(num>>4);
	ohn(num);
	}

onum(nm, ffp)
int nm, ffp; {
	if (nm > 9) onum(nm/10, ffp);
	putc(nm%10+'0', ffp);
	}

#if IBM


/*	FINDFILE.C	*/
/*
	This file contains the routine to locate a file, utilizing the PATH
	environment variable for the directories to search.

	Interface:

		findfile(filename, target_buf)

		where filename is the name of the file to be searched for,
		      target_buf is the buffer to place the path name if found.

		if the file is found, findfile return 1 and the pathname,
		otherwise it returns 0.

	This program uses the environ routines to access the PATH variable.

	Stack requirements:  ~300 bytes

*/

findfile(filename, target_buf)
	char *filename, *target_buf; {
	int fid;
	char paths[256], *p_ptr, *t_ptr;

	/* first check in the local directory */
	strcpy(target_buf, filename);
	fid = open(target_buf, 0);
	if (fid >= 0)  {				/* got it */
		close(fid);
		return (1);
		}
	getenv("PATH", paths);
	p_ptr = paths;

	while (*p_ptr != 0) {
		/* copy the directory name */
		t_ptr = target_buf;
		while (*p_ptr != ';' && *p_ptr != 0) {
			*t_ptr++ = *p_ptr++;
			}
		if (*(t_ptr-1) != '/' && *(t_ptr-1) != '\\') *t_ptr++ = '\\';
		*t_ptr = 0;
		if (*p_ptr) p_ptr++;		/* beyond the ';' */
		strcat(target_buf, filename);
		fid = open(target_buf, 0);
		if (fid >= 0)  {				/* got it */
			close(fid);
			return (1);
			}
		}
	strcpy(target_buf, filename);
	return (0);							/* can't find one */
	}

#endif
