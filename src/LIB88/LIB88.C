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
/*		Librarian for ASM88 and C88	*/

#include "OBJ.H"

#define NUMARG	100
#define ABSIZE	800
#define NUMLAB	2000

#define LOTHER		0
#define LPUBLIC		1
#define LRESERVE	2
#define LTYPE		3
#define LTYPERES	4

#define INDSEG	0
#define INCSEG	1
#define INESEG	2

#define FOPT	0
#define FOPEN	1
#define FCLOSED	2

#define CONTZ	26
#define MAXHASH	64

	char inbuf[4234],*inin,*endin;
	char databuf[4234],*indata,filen;
	char inname[65],outname[65];
	int  infile[NUMARG],outfile,thisout,*fixat,numab,nerr;
	char isopen[NUMARG],curseg,*inext,numopen,labis[NUMLAB];
	char *thisname,under_opt,need_opt,*oldext,newfile,is_big;

	unsigned hash[MAXHASH],labat[NUMLAB];
	unsigned util;
	unsigned len,num,nummod;
	char *found;
	char *pname,*fileat[NUMARG];

	int  pfile=-1,numfile=1;
	char argbuf[ABSIZE];		/* -f arguments go here	*/

	struct {char got,seg; int mod_number;};		/* follows name in symbol table	*/

	union {int word; char byte; };



#define MAXMOD 300
	char wantmod[MAXMOD];
	unsigned modsize[MAXMOD];

	char *memory,*memlast;
#define MAXDEPEN	1500
	struct {int need,have; } dependency[MAXDEPEN];
	int  depnum;

main(argc,argv)
	int  argc;
	char *argv[]; {
	int i;

	puts("Librarian for C88 and ASM88     V2.1A    (c) Mark DeSmet, 1982,83,84,2005\n");
	init(argc,argv);
	nextpass(1);
	/*	eliminate modules that depend upon others from list of wanted mods */
	for (i=0; i < depnum; i++) {
		wantmod[dependency[i].have]=0;
		}
	do {
		if (pfile != -1) printsym();
		nextpass(2);
		}
	while (set_wants());

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
				ferror("over 100 arguments #1","");
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
	if (numfile > NUMARG) ferror("over 100 arguments #2","");


	for (filen=1; filen < numfile; filen++) {
		argat=fileat[filen];
		if (*argat == '-') {
			isopen[filen]=FOPT;
			i=toupper(*++argat);
			switch (i) {
				case 'P':	if (*++argat) pname=argat;
							if ((pfile=creat(pname)) == -1)
								ferror("cannot open ",pname);
							break;
				case 'O':	cmdname(argat+1);
							break;
				case 'N':	need_opt=1;
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
			if (outname[0] == 0) {
				i=strlen(argat);
				if (argat[i-2] != '.' || toupper(argat[i-1]) != 'S') 
					cmdname(argat);
				else strcpy(outname,"NUL");
				}
			}
		}
	if ((outfile=creat(outname)) == -1)
		ferror("cannot create",outname);
	}

cmdname(name)
	char *name; {
	char i,needs;

	i=0;
	needs=1;
	while (*name) {
		if (*name == '.') needs=0;
		outname[i++]=*name++;
		}
	if (needs) strcpy(&outname[i],".S");
	else outname[i]=0;
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

	for (filen=1; filen < numfile; filen++) {
		gotdot=0;
		i=0;

		argat=fileat[filen];
		while (*argat) {
			if (*argat == '.') gotdot=1;
			inname[i++]=*argat++;
			}
		if (gotdot) {
			inname[i]=0;
			}
		else strcpy(&inname[i],".O");

		if (isopen[filen] == FCLOSED) {
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
				ferror("cannot seek ",fileat[filen]);
			inin=endin=&inbuf[4096];
			}
		else continue;

		newfile=1;
		do {
			if (pass == 1) {
				_setmem(labis,NUMLAB,LOTHER);
				pass1();
				}
			else pass2();
			if (++nummod > MAXMOD) error("over 300 modules","");
			if (inin >= endin) refill();
			if (*inin == CONTZ) inin=endin;
			}
		while (inin < endin);
		}
	}

pass1() {
	char clen,needed,got_big;
	unsigned i,*next;
	int  old_depnum,num;

	old_depnum=depnum;
	oldext=inext;
	modsize[nummod]-=inin;
	got_big=0;

	while (1) {
		if (inin+128 > endin && endin >= &inbuf[4096]) {
			modsize[nummod]+=inin;
			refill();
			modsize[nummod]-=inin;
			}

		switch (clen=*inin++) {
			/*	see if this module is needed and keep track of dependencies */

			case OEOF:		modsize[nummod]+=inin;
							needed=check_mod();
							if (needed == 0) {
								depnum=old_depnum;
								inext=oldext;
								for (i=0; i < MAXHASH; i++) {
									next=hash[i];
									while (next >= oldext) next=*next;
									hash[i]=next;
									}
								wantmod[nummod]=0;
								}
							else wantmod[nummod]=1;
							if (!nummod) is_big=got_big;
							else if (got_big != is_big)
								ferror("cannot mix LARGE and SMALL case","");
							return;
			case OPUBLIC:	add_name();
							break;
			case OSTATIC:	while (*inin++) ;
							inin+=2;
							break;
			case ORESERVE:	inin+=4;
							break;
			case OLOCAL:	num=inin->word;
							if (labis[num] == LPUBLIC) {
								found=labat[num];
								found->got=1;
								}
							inin+=4;
							break;
			case ODSEG:		curseg=INDSEG;
							break;
			case OCSEG:		curseg=INCSEG;
							break;
			case OESEG:		curseg=INESEG;
							break;
			case ONAMEREL:
			case OJUMPREL:	inin+=2;
							break;

			case OPTYPE:
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
			case OBIG:		got_big=1;
							break;
			case OLNAMEREL:
			case OSEGPTR:
			case OPTR:		inin+=2;
							break;
			default:		if (clen <= 128)
								ferror("bad object file ",inname);
							inin+=clen-128;
			}
		}
	}


pass2() {
	unsigned max,this,inbuf;

	max=modsize[nummod];
	while (max) {
		if (inin == endin) refill();
		inbuf=endin-inin;
		this=max > inbuf ? inbuf: max;
		if (wantmod[nummod]) {
			if (write(outfile,inin,this) != this)
				ferror("cannot write",outname);
			}
		inin+=this;
		max-=this;
		}
	}



/*	print the symbols for 'P' option	*/

printsym() {

	int column,lastmod;
	char *nameptr,*found,nlen;

/*	print the symbols	*/

	column=0;
	lastmod=-1;
	nameptr=found=memory+2;

	while (nameptr < inext) {
		while (*found++) ;
		if (found->got == 0 || found->seg != INCSEG) goto next_print;
		if (*nameptr == '_' && under_opt == 0) goto next_print;
		if (wantmod[found->mod_number] == 0) goto next_print;
		if (found->mod_number != lastmod) {
			fputs("\r\n-",pfile);
			lastmod=found->mod_number;
			column=0;
			}
		if (column >= 60) {
			fputs("\r\n ",pfile);
			column=0;
			}
		else if (column >= 40) while (column < 60) {
			column++;
			putc(' ',pfile);
			}
		else if (column >= 20) while (column < 40) {
			column++;
			putc(' ',pfile);
			}
		else if (column) while (column < 20) {
			column++;
			putc(' ',pfile);
			}
		if (column+strlen(nameptr) >= 79) {
			fputs("\r\n ",pfile);
			column=0;
			}
		while (*nameptr) {
			putc(*nameptr++,pfile);
			column++;
			}
next_print:
		nameptr=found=found+6;
		}
	}


/*	return true if first string is greater than second	*/

sover(stra,strb)
	char *stra,*strb; {

	do {
		if (*stra > *strb) return 1;
		if (*stra < *strb++) return 0;
		}
	while (*stra++);
	return 0;
	}




/*	CHECK_MOD	--  see if this module is needed and record all
					dependencies	*/

check_mod() {
	char *ptra,*ptrb;
	unsigned *next,*match;
	int  i;

	if (need_opt) return 1;
	for (i=0; i < MAXHASH; i++) {
		next=hash[i];
		while (next >= oldext) {
			match=*next;
			while (match) {
				if (match >= oldext) goto nextm;
				ptra=next+1;
				ptrb=match+1;
				if (*ptra == *ptrb) {
					do {
						if (*++ptra != *++ptrb) goto nextm;
						}
					while (*ptra);
					/*	have an older public def	*/
					ptra++;	/* point to 'have' flag */
					ptrb++;
					if (ptra->got) {
						if (ptrb->got) {
							return 0;
							}
						add_depen(ptrb->mod_number,ptra->mod_number);
						}
					else {
						if (ptrb->got)
							add_depen(ptra->mod_number,ptrb->mod_number);
						}
					}
nextm:			match=*match;
				}
			next=*next;
			}
		}
	return 1;
	}


/*	add a dependency to the dependency list	*/

add_depen(needed,haved)
	int  needed,haved; {

	dependency[depnum].need=needed;
	dependency[depnum].have=haved;
	if (++depnum >= MAXDEPEN) ferror("too many dependencies in ",inname);
	}




/*	SET_WANTS  --	set wantmod to list of modules without dependencies.
					fatal error if circular dependencies. return 0 when done. */

set_wants() {
	int i,j,numthis;
	char tempwant[MAXMOD];

	/*	see how many modules resolve no dependencies.	*/

	for (i=0; i < MAXMOD; i++)
		tempwant[i]=0;
	for (i=0; i < depnum; i++) {
		tempwant[dependency[i].have]=1;
		}

	/*	clean irrelevent dependencies from dependency list */

	j=0;
	for (i=0; i < depnum; i++) {
		if (wantmod[dependency[i].need] == 0) {
			dependency[j].have=dependency[i].have;
			dependency[j].need=dependency[i].need;
			j++;
			}
		}
	depnum=j;

	/*	remember modules without dependencies	*/
	for (i=0; i < MAXMOD; i++) {
		wantmod[i]=tempwant[i];
		}

	/*	remove modules that still have dependencies	*/

	for (i=0; i < depnum; i++) {
		tempwant[dependency[i].have]=0;
		}

	/*	if not done and cannot write any now then have circular dependency */

	numthis=0;
	for (i=0; i < MAXMOD; i++) {
		numthis+=tempwant[i];
		}
	if (depnum && numthis == 0) {
		puts("\nwarning: circular dependencies\n");
		for (i=0; i < MAXMOD; i++) {
			numthis+=wantmod[i];
			}
		depnum=0;
		}
	else {
		for (i=0; i < MAXMOD; i++) {
			wantmod[i]=tempwant[i];
			}
		}
	return numthis;
	}




/*	add the name of a public to the symbol table	*/

add_name(pass)
	char pass; {
	char hashno,ch,mlen,nlen,*target;
	int  *next,tlen,dif;
	unsigned *achain;

	hashno=nlen=0;
	achain=inext;
	inext+=2;
	do {
		nlen++;
		ch=*inin++;
		if (ch >= 'a') ch-=32;
		*inext++=ch;
		hashno+=ch;
		}
	while (ch);
	hashno+=nlen;
	hashno&=(MAXHASH-1);
				/* get the variable number	*/
	num=inin->word;
	inin+=2;

	/*	add to the hash chain	*/
	*achain=hash[hashno];
	hash[hashno]=achain;

	labat[num]=inext;		/*	remember where to patch got byte	*/
	inext->got=0;			/*	and leave room for it	*/
	inext->seg=curseg;		/*	remember segment for printing */
	inext->mod_number=nummod;
	inext+=4;

	labis[num]=LPUBLIC;
	if (inext >= memlast)
		ferror("too many total PUBLICs in ",inname);
	}

refill() {
	char *temp;
	int  numbytes;

	if (endin < &inbuf[4096]) return;
	temp=inbuf;
	while (inin < endin) *temp++=*inin++;
	if ((numbytes=read(infile[filen],temp,4096)) == -1)
		ferror("cannot read ",inname);
	if (newfile && numbytes == 0) ferror("null file ",inname);
	newfile=0;
	endin=temp+numbytes;
	inin=inbuf;
	}

endup() {
	unsigned max;

	if (close(outfile) == -1)
		ferror("cannot close ",outname);

	if (pfile != -1) {
		fputs("\r\n",pfile);
		if (close(pfile) == -1)
			ferror("cannot close ",pname);
		}
	max=(inext-memory)/((memlast-memory)/100);
	if (max > util) util=max;
	nummod/=3;
	puts("end of LIB88        ");
	onum(util > nummod ? util: nummod);
	puts("% utilization    ");
	if (nerr) {
		onum(nerr);
		puts(" errors");
		}
	}

ferror(str1,str2)
	char *str1,*str2; {

	ocrlf();
	puts(str1);
	puts(str2);
	puts("     LIB88 abandoned\n");
	exit(2);
	}

error(str1,str2)
	char *str1,*str2; {

	puts(inname);
	puts(" - ");
	puts(str1);
	putchar(' ');
	puts(str2);
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

onum(num)
	int  num; {
	if (num > 9) onum(num/10);
	putchar(num%10+'0');
	}

