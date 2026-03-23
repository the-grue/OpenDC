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
/*	BBIND.C	--	First part of BBIND, large case binder.	*/

#include "OBJ.H"
#include "BBIND.H"


	char libname[65]="BCSTDIO.S";
	int  pfile=-1,numfile=1;
	char initobj[52]={OBIG,OCSEG,1,0x5f,0x63,0x73,0x65,0x74,0x75,0x70,0,1,0
			0x85,0xea,0,0,0,0,OLNAMEREL,1,0,
			11,'M','A','I','N','_',0,2,0,2,253,2,
			1,'M','A','I','N','_',0,2,0,4,
			0x86,0,0,0,0,0,0,0};
	struct header ihead={0x5a4d,0,1,0,32,0,0xffff,0,0,0,0,0,0x1c,0};

main(argc,argv)
	int  argc;
	char *argv[]; {
	int  i;

	i=2;
	while (i < argc) if (*(argv[i++]+1) == '@') see_exit=1;
	if (see_exit == 0)
	{
		puts("OpenBBind v0.8    Based on");
		puts("Large Binder for C88 and ASM88     V1.1    (c) Mark DeSmet, 1986-87\n");
	}
	init(argc,argv);
	nextpass(1);
	between();
	nextpass(2);
	endup();
	exit(nerr ? 2:0);
	}

init(argc,argv)
	int  argc;
	char *argv[]; {
	char *argat,i;
	int  nin,i,ffile;


	inext=memory=_memory();
	memlast=_showsp()-512;

	util=argc*3;
	pname="CON:";

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
							strcat(libname,"BCSTDIO.S");
							break;
				case 'S':	while (i=toupper(*++argat)) {
								if (i >= '0' && i <= '9') i-='0';
								else if (i >= 'A' && i <= 'Z') i-='A'-10;
								else ferror("bad Stack option","");
								stacklen<<=4;
								stacklen+=i;
								}
							break;
				case 'N':	if (toupper(*(argat+1)) == 'M') {
								no_member=1;
								argat++;
								}
							break;
				case 'O':	cmdname(argat+1);
							break;
				case 'M':
				case 'V':	ferror("overlays not supported with large case","");
							break;
				case '_':	under_opt=1;
							break;
				default:	puts("bad argument ");
							puts(argat-1);
							ocrlf();
							nerr++;
				}
			}
		else {
			isopen[filen]=FCLOSED;
			if (outname[0] == 0) cmdname(argat);
			}
		}
	if (copt) {
		if ((chkfile=creat(chkname)) == -1)
			ferror("cannot create",chkname);
		chkat=chkbuf;
		chkb(OBIG);
		outchk();
		}

/*	if MS-DOS V2.0 and no -L option, use path to find BCSTDIO.S */

	extern char _msdos2;

	if (_msdos2 && libname[7] == '.') findfile("BCSTDIO.S",libname);
	}

cmdname(name)
	char *name; {
	char i;

	i=0;
	while (*name && *name != '.') {
		chkname[i]=*name;
		outname[i++]=*name++;
		}
	strcpy(&outname[i],".EXE");
	strcpy(&chkname[i],".CHK");
	}



/*	return a 1 if character is a blank, CR or LF	*/

iswhite(ch)
	char ch; {

	return ch == ' ' || ch == '\r' || ch == '\n';
	}



nextpass(npass)
	char npass; {
	char *argat,gotdot;
	int  i;
	long tot;

	pass=npass;
	nummod=0;


	/*	file zero is initobj if A option is off	*/
	/*	numfile file is BCSTDIO.S	*/

	for (filen=aopt; filen <= numfile; filen++) {
		gotdot=0;
		i=0;
		curseg=INDSEG;
		curseg_res=INDSEG_RES;
		curseg_alloc=INDSEG_ALLOC;

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
			if (lseek(infile[filen],0,0,0) == -1)
				ferror("cannot seek ",filen ? fileat[filen] : "STDIO.O");
			inin=endin=&inbuf[4096];
			}
		else continue;

		i=0;
		while (inname[i]) i++;
		islib=0;
		if (toupper(inname[i-1]) == 'S' && inname[i-2] == '.') {
			islib=1;
			}
		do {
			_setmem(labis,NUMLAB,LOTHER);
			_setmem(lab_ptr4,sizeof(lab_ptr4),0);
			if (pass == 1) pass1();
			else pass2();
			if (((unsigned) offs[INCSEG]) > 32000) up_addr(INCSEG);
			roundup(INESEG);
			up_addr(INESEG);
			roundup(INDSEG);
			if (++nummod > MAXMOD) error("over 500 modules","");
			if (inin >= endin) refill();
			if (*inin == CONTZ) inin=endin;
			}
		while (inin < endin);
		if (pass == 1 && offs[INDSEG] > 64536) ferror("over 64K data","");
		}
	}

pass1() {
	char clen,*oldext,needed;
	unsigned i,*next,old_relo,old_byte4;
	long oldoffs[INESEG_RES+1];

	oldext=inext;
	_move(sizeof(offs),offs,oldoffs);
	old_relo=ihead.relo;
	old_byte4=num_byte4;
	needed=1-islib;

	num_public_patch=0;		/* number of times found->edefn is incorrectly
								changed by a module that is not used.	*/

	if (inin+128 > endin) refill();
	if (*inin != OBIG) ferror("small case object ",inname);
	while (1) {
		if (inin+128 > endin) refill();
		switch (clen=*inin++) {
			case OEOF:		if (needed == 0) {
								inext=oldext;
								_move(sizeof(offs),oldoffs,offs);
								for (i=0; i < 32; i++) {
									next=&hash[i];
									while (*next && *next < oldext)
										next=*next;
									*next=0;
									}
								wantmod[nummod]=0;
								while (num_public_patch)
									*public_patch[--num_public_patch]=LOTHER;
								ihead.relo=old_relo;
								num_byte4=old_byte4;
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
								found=(unsigned)lab_off[num];
								switch (found->edefn) {
									case LOTHER:	found->edefn=LRESERVE;
													found->eaddr=len;
													if (found < oldext && found->eused)
														needed=1;
													break;
									case LRESERVE:	if (found->eaddr < len)
														found->eaddr=len;
									case LPUBLIC:	;
									}
								}
							else {
								if (labis[num] == LTYPE) {
									found=(unsigned) lab_off[num];
									found->edefn=LTYPERES;
									found->eaddr=offs[curseg_res];
									}
								offs[curseg_res]+=len;
								if (curseg == INESEG) {
									roundup(INESEG_RES);
									up_addr(INESEG_RES);
									}
								else if (offs[INDSEG_RES]+len > 65535)
									ferror("over 64K of local data","");

								}
							break;
			case OLOCAL:	num=inin->word;
							if (labis[num] == LPUBLIC) {
								inin+=2;
								len=inin->word;
								inin+=2;
								if (num >= NUMLAB)
									ferror("too many labels in ",inname);
								found=(unsigned) lab_off[num];
								if (found->edefn == LPUBLIC) {
									if (needed) error("multiply defined",
										found-found->enlen);
									}
								else {
									found->edefn=LPUBLIC;
									public_patch[num_public_patch++]=&found->edefn;
									found->eaddr=len+offs[curseg];
									if (found < oldext && found->eused)	needed=1;
									}
								}
							else if (labis[num] == LTYPE) {
								inin+=2;
								len=inin->word;
								inin+=2;
								found=(unsigned) lab_off[num];
								found->eaddr=len+offs[curseg];
								}								
							else inin+=4;
							break;
			case ODSEG:		curseg=INDSEG;
							curseg_res=INDSEG_RES;
							curseg_alloc=INDSEG_ALLOC;
							break;
			case OCSEG:		curseg=INCSEG;
							curseg_res=INESEG_RES;
							curseg_alloc=INESEG_ALLOC;
							break;
			case OESEG:		curseg=INESEG;
							curseg_res=INESEG_RES;
							curseg_alloc=INESEG_ALLOC;
							break;
			case ONAMEREL:
			case OJUMPREL:	num=inin->word;
							inin+=2;
							if (needed && labis[num] == LPUBLIC)
								((char *) lab_off[num])->eused=1;
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
			case OBIG:		break;
			case OPTR:		num=inin->word;
							inin+=2;
							if (lab_ptr4[num] == 0) {
								if (labis[num] == LPUBLIC) {
									found=(unsigned) lab_off[num];
									if (needed)	((char *) lab_off[num])->eused=1;
									if (found->eptr4_at == 0) {
										found->eptr4_at=1;
										}
									else break;
									}
								num_byte4++;
								lab_ptr4[num]=1;
								ihead.relo++;
								}
							break;
			case OSEGPTR:
			case OLNAMEREL:	ihead.relo++;
							num=inin->word;
							inin+=2;
							if (needed && labis[num] == LPUBLIC)
								((char *) lab_off[num])->eused=1;
							break;
			default:		if (clen <= 128)
								ferror("bad object file ",inname);
							clen-=128;
							offs[curseg]+=clen;
							inin+=clen;
			}
		}
	}
