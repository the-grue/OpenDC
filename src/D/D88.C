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
/*      the-grue - 20180830
 *      Changed several things to conform to new compiler.
 */
/*	D88.C  --	DEBUGGER FOR C88 PACKAGE	*/

#define IBM 1
char _ibm=IBM;

#include "debug.h"

char rname[14][3]={"AX","BX","CX","DX","SI","DI","BP","SP","DS","SS",
		"ES","CS","IP","FL"};
int listfil=-1;
char int3=0xcc,golist=1;
struct {unsigned goff,gseg; char gbyte;}
	 brk[4]={{-1,-1,0},{-1,-1,0},{-1,-1,0},{-1,-1,0}};
char ltype[128]=
	{5,4,4,4,4,4,4,4,4,2,5,4,4,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	 3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,
	 4,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,4,4,4,1,
	 4,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,4,4,4,4};

char i_type[]={CINT};
char u_type[]={CUNSG};
char l_type[]={CLONG};
char d_type[]={CDOUBLE};
char c_type[]={CCHAR};
char p_type[]={PTRTO,CCHAR};
char listname[65]="xyzzy";

char p0[]="Again  Breakpoint  Collection  Display  Expression  Flip  Go    --space--";
char p1[]="List  Macro  Options  Proc-step  Quit  Register  Step           --space--";
char p2[]="Unassemble  Variables  Where                                    --space--";
char scr_window_top;

char *pline,*tempargv[16],*pfrom;
int  i,uppara,tempargc;
unsigned oldsp;


main(argc,argv)
	int  argc;
	char *argv[]; {

	scr_window_top=5;
	pline=argv[1];
	if (*pline == '-' && toupper(*++pline) == 'M') {
		argc--;
		argv++;
		flipok=0;
		}

/*	save the argc and argv.	*/

	tempargc=argc;
	pline=&buffer[800];
	for (i=0; i < argc; i++) {
		tempargv[i]=pline;
		pfrom=argv[i];
		do {
			*pline++=*pfrom;
			}
		while (*pfrom++);
		}


	oldsp=_showsp();
	i=argv[argc-1]+strlen(argv[argc-1])+1-oldsp;
	_setsp(_memory()+0xc00);
	_setbp(_memory()+0xc02);
	_move(i,oldsp,_showsp());
	symbols=_showsp()+130;
	if (flipok) {
		uppara=flipinit();
		move_ds(uppara);
		}
	scr_setup();
#if IBM == 0
	_os(51,_showds());
#endif

	cmd=0;
	pline=p0;
	if (flipok) scr_save();
	scr_clr();
	lineno=5;
	cursor();
	dputs("OpenD88 v0.1   Based on ");
	dputs("D88 Debugger V1.6 (c) Mark DeSmet 1984,85,86");
	lf();
	setjmp(0);
	cmd++;
	nextin=line;
	if (cmd == 1) init(tempargc,tempargv);
	if (i=setjmp(0)) error(i);
	lastcmd='.';
	while (1) {
		skipbl();
		where();
		header();		/*	print page heading	*/

		cmd=incmd(pline);
		if (cmd != ' ' && cmd != ';' && cmd != 'F') lf();
		do_command(cmd);
		if (cmd != 'A') lastcmd=cmd;
		}
	}



/*	HEADER	--	print where line and F10 macro stuff if any	*/

header() {
	int  oldline;
	char cmd;

	scr_rowcol(2,0);
	dputs("-----");
	scr_rowcol(3,0);
	whereline();
	scr_rowcol(4,0);
	scr_window_top=5;

	/*	add the F10 macro header	*/
	if (macro_line[3][0] && inmacro == 0 && macronum != 4) {
		oldline=lineno;
		lineno=4;
		inmacro=macro_line[3];
		do {
			cmd=incmd(pline);
			cursor();
			do_command(cmd);
			}
		while (*inmacro);
		inmacro=0;
		dputs("-----");
		scr_clrl();
		scr_window_top=lineno < 16 ? lineno+1: 16;
		if (lineno < oldline) lineno=oldline;
		}
	else {
		dputs("-----");
		scr_clrl();
		}
	}




/*	DO_COMMAND	--	do a command.	*/

do_command(cmd)
	char cmd; {

	nexts=strings;
	switch (cmd) {
		case ' ':	if (pline == p0) pline=p1; else if (pline == p1)
						pline=p2; else pline=p0;
		case 0:
		case ';':	break;
		case 'A':	cmd=lastcmd;
					switch (lastcmd) {
						case 'D':	display(0);
									break;
						case 'L':	list(0);
									break;
						case 'U':	unassemble(0);
									break;
						default:	error("cannot repeat");
						}
					break;
		case 'B':	breakp();
					break;
		case 'C':	collection();
					break;
		case 'D':	display(1);
					break;
		case 'E':	expr();
					break;
		case 'F':	flip();
					break;
		case 'G':	go();
					break;
		case 'L':	list(1);
					break;
		case 'M':	macro();
					break;
		case 'O':	options();
					break;
		case 'P':	step(1);
					break;
		case 'Q':	quit();
					break;
		case 'R':	registers();
					break;
		case 'S':	step(0);
					break;
		case 'U':	unassemble(1);
					break;
		case 'V':	variables();
					break;
		case 'W':	showhere();
					break;
		case 'Z':	status_8087();
					break;
		default:	error("illegal command");
		}
	}




/*	QUIT  --		stop or start over.	*/

quit() {
	int  argc;
	char *argv[20],want,cmd;

	cmd=incmd("Quit:  Exit  Initialize");
	if (cmd == 'E') {
		if (flipok) scr_rest();
		else lf();
		scr_term();
		exit(0);
		}
	if (cmd != 'I') return;
#if IBM
	if (noinit) error("cannot Quit Initialize after exit() or break");
#endif
	skipbl();
	argc=1;
	argv[0]="\0";
	want=1;
	prompt("input command line");
	glineno();
	while (*nextin) {
		if (ltype[*nextin] == SPACE) {
			*nextin++=0;
			want=1;
			}
		else {
			if (want) argv[argc++]=nextin;
			want=0;
			nextin++;
			}
		}
	if (argc == 1) return;
	init(argc,argv);
	}



/*	INIT  --	restart the world */

init(argc,argv)
	int  argc;
	char *argv[]; {

	int  chkfile,exefile,i,*reg;
	char *endsym,exename[65],chkname[65],*name,len,fixbuf[128],*lastsym,*ptr;
	unsigned bottom,top,max,numin,datalen,fixup,fixword;
	long *lp,image;

	struct {char xx[2]; unsigned hiram; char xxx[6];
		unsigned termip,termcs,breakip,breakcs;
		char yy[110],tail[128];} *pcb;
	struct {unsigned magic,aa,size,nrelo,nhead,addmin,addmax,newss,
			newsp,ff,newip,newcs,relo_off,ovnum;} exe;
	struct {char cform; int clen,cbase,cmin,cmax;
			char dform; int dlen,dbase,dmin,dmax; char rest[110];} *cmdhead;
	struct {int xxy[3],cpmsp,yyy[60]; char cpmtail[128];} *cpmhead;

	struct {unsigned offset,segment;} *next_fixup;

	brk[0].goff=brk[1].goff=brk[2].goff=0;

	endsym=symbols;
	*endsym++=0;
	localss=_showds();

	if (listfil != -1) {
		close(listfil);
		listfil=-1;
		}

	for (reg=&rax, i=0; i < NUMREG; i++)
		*reg++=0;
	rcs=rds=res=rss=localss+(endsym>>4)+1;
	rfl=0x200;
	int3off=0xffff;
	origds=origss=origcs=rcs;
	rsp=0x1000;
	
	if (argc < 2) return;


/*	copy in command tail	*/
	pcb=&buffer[512];
	len=0;
	for (i=2; i < argc; i++) {
		(*pcb).tail[++len]=' ';
		name=argv[i];
		while (*name) {
			(*pcb).tail[++len]=*name++;
			}
		}
#if IBM
	(*pcb).tail[++len]='\r';
#else
	(*pcb).tail[++len]=0;
#endif
	(*pcb).tail[0]=len;

	name=argv[1];
	for (i=0; i < 64; i++) {
		if (*name == 0 || *name == '.') break;
		exename[i]=*name;
		chkname[i]=*name++;
		}
	exename[i]=chkname[i]=0;
#if IBM
	strcat(exename,".EXE");
#else
	strcat(exename,".CMD");
#endif
	strcat(chkname,".CHK");
	if ((exefile=open(exename,0)) == -1) {
		error2("cannot open",exename);
		}

	max=0xff00-symbols;
	if ((chkfile=open(chkname,0)) == -1) {
		dputs("no .CHK file\n");
		numin=0;
		}
	else {		
		if ((numin=read(chkfile,symbols,max)) == -1) {
			error2("cannot read",chkname);
			}
		close(chkfile);
		}
	if (numin == max) {
		dputs("some symbols lost");
		lf();
		lastsym=symbols;
		do {
			endsym=lastsym;
			lastsym+=*(lastsym+1);
			}
		while (lastsym < symbols+numin);
		*endsym++=0;
		}
	else endsym=symbols+numin;		

	if (*symbols == OBIG) is_big=1;
	else is_big=0;
/*	read in the executable file	*/

#if IBM
	if (read(exefile,&exe,sizeof(exe)) != sizeof(exe)) {
		error2("cannot read",exename);
		}
	lseek(exefile,((long) exe.nhead) << 4,0);
	_lmove(128,0,_pcb,pcb,localss);
	image=(((long)exe.size) << 9)-(((long)exe.nhead) << 4);
retest:
	if (memchop(endsym) == -1) {
		dputs(".EXE file is too large to load");
		lf();
		endsym=symbols;
		*endsym++=0;
		return;
		}

	if (allocat) {
		memfree(allocat);
		allocat=0;
		}
	max=(image>>4)+0x10;
	if (exe.addmax > 0x8000) exe.addmax=0x8000;
	if ((bottom=memalloc(max+exe.addmin,max+exe.addmax,&(*pcb).hiram))
		== -1) {
		if (endsym-symbols-1) {
			dputs("all symbols abandoned");
			lf();
			endsym=symbols;
			*endsym++=0;
			goto retest;
			}
		dputs(".EXE file is too large to load");
		lf();
		return;
		}
	allocat=origds=rds=res=bottom;
	fixup=rds+0x10;
	origss=rss=exe.newss+fixup;
	rsp=exe.newsp;
	rip=exe.newip;
	origcs=rcs=exe.newcs+fixup;
	int3off=((rss-rcs)<<4)-1;
	int3seg=rcs;
	if (is_big) {
		int3off=&int3;
		int3seg=localss;
		}

/*	move pcb up to new memory	*/

	_lmove(256,pcb,localss,0,bottom);
	bottom+=16;

	while (image) {
		numin=image > 1024 ? 1024: image;
		image-=numin;
		if (read(exefile,buffer,numin) == -1) {
			error2("cannot read",exename);
			lf();
			}
		_lmove(numin,buffer,localss,0,bottom);
		bottom+=numin >> 4;
		}

	lseek(exefile,(long)exe.relo_off,0);
	next_fixup=fixbuf+sizeof(fixbuf);
	while (exe.nrelo--) {
		if (next_fixup == fixbuf+sizeof(fixbuf)) {
			read(exefile,fixbuf,sizeof(fixbuf));
			next_fixup=fixbuf;
			}
		_lmove(2,next_fixup->offset,next_fixup->segment+fixup,&fixword,localss);
		fixword+=fixup;
		_lmove(2,&fixword,localss,next_fixup->offset,next_fixup->segment+fixup);
		next_fixup++;
		}

	if (is_big) {		/*	fixup symbols	*/
		lastsym=symbols;
		while (*lastsym) {
			if (*lastsym == OPTYPE) {
				ptr=lastsym;
				while (*ptr++) ;
				ptr->loc_seg+=fixup;
				}
			else if (*lastsym == OLINE) lastsym->lloc_seg+=fixup;
			lastsym=lastsym+=*(lastsym+1);
			}
		}

#else
	if (read(exefile,buffer,128) != 128) {
		error2("cannot read",exename);
		}
	cpmhead=buffer;
	_lmove(128,0,localss,pcb,localss);

cretest:
	rcs=origcs=bottom=localss+(endsym>>4)+1;
	origds=rds=res=(*cpmhead).clen+rcs;
	lp=6;
	datalen=(*lp)>>4;
	if (datalen+localss < (*cpmhead).dlen+rds+40) {
		if (endsym-symbols-1) {
			dputs("all symbols abandoned");
			lf();
			endsym=symbols;
			*endsym++=0;
			goto cretest;
			}
		dputs(".EXE file is too large to load");
		lf();
		return;
		}
	lp=&buffer[512]+6;
	*lp=((long)(datalen-rds+localss))<<4;
			
	rip=0;
	rss=localss;
	rsp=_showsp()-20;
	int3off=((rds-rcs)<<4)-1;
	int3seg=rcs;

/*	read in CPM file. */

	max=(*cpmhead).clen<<4;
	datalen=(*cpmhead).dlen<<4;
	while (max) {
		numin=max > 512 ? 512: max;
		if (read(exefile,buffer,numin) != numin) {
			error2("cannot read",exename);
			lf();
			}
		_lmove(numin,buffer,localss,0,bottom);
		bottom+=numin>>4;
		max-=numin;
		}
	while (datalen) {
		numin=datalen > 512 ? 512: datalen;
		if (read(exefile,buffer,numin) != numin) {
			error2("cannot read",exename);
			lf();
			}
		_lmove(numin,buffer,localss,0,bottom);
		bottom+=numin>>4;
		datalen-=numin;
		}
	_lmove(256,&buffer[512],localss,0,rds);


#endif


/*	go to main	*/

	close(exefile);
	if (is_big == 0 && rss-rcs < 0x1000) _lmove(1,&int3,localss,int3off,rcs);
	strcpy(line,"G AMAIN ;");
	nextin=line;
	mainflag=1;
	goflip=0;
	}


/*	DISPLAY  --  Print some bytes	*/

struct {unsigned off,seg;} dfrom;

display(needa)
	char needa; {
	int  i;
	char ch;

	if (needa) {
		dfrom.seg=rds;
		dfrom.off=0;
		inaddr(&dfrom);
		}
	if (repeat  == 0) repeat=3;
	while (repeat--) {
		dprintf("%04x:%04x  ",dfrom.seg,dfrom.off);
		for (i=0; i < 16; i++)
			dprintf("%02x ",_peek(dfrom.off+i,dfrom.seg));
		dputs("  *");
		for (i=0; i < 16; i++) {
			ch=_peek(dfrom.off+i,dfrom.seg);
			xscr_co((ch >= ' ' && ch < 0x7f) ? ch: ' ');
			}
		xscr_co('*');
		dfrom.off+=16;
		lf();
		}
	}

	


/*	COLLECTION  --	print an aggregate  */

collection() {
	struct vstruct vt,newvt;
	int numrpt,i,lastoff,len;
	char *typ;
	struct sloty {unsigned loc; char type [1];} *smemat;

	numrpt=10;
	if (repeat) numrpt=repeat;

	prompt("input an array name or structure.member");
	glineno();
	expression(&vt);
	typ=vt.typep;
	if ((*typ == ARRAY || *typ == PTRTO) && memat == 0) {
		print_val(&vt);
		lf();
		if (*typ == ARRAY) {
			len=*(int *)(typ+1);
			if (len < numrpt && len > 0) numrpt=len;
			typ+=2;						
			}
		vt.typep=typ+1;
		for (i=0; i < numrpt; i++) {
			if (col > 60) {
				lf();
				}
			dprintf("[%d]=",i);
			_move(12,&vt,&newvt);
			newvt.vtype=RMV;
			print_val(&newvt);
			dputs("   ");
			vt.val.valu+=longof(vt.typep);
			}
		}								

	else if (memat) {

		lastsym=memat;
		while (*memat++) ;
		smemat = (struct sloty *) &memat;
		vt.val.valu-=smemat->loc;
		lastoff=0;
		while (1) {
			memat=lastsym;
			while (*memat++) ;
			smemat = (struct sloty *) &memat;
			if (*lastsym != OMTYPE || smemat->loc < lastoff) break;
			if (col > 60) {
				lf();
				}
			xscr_co('.');
			dputs(lastsym+2);
			xscr_co('=');
			lastoff=smemat->loc;
			vt.val.valu+=lastoff;
			_move(12,&vt,&newvt);
			newvt.typep=&smemat->type[1];
			print_val(&newvt);
			dputs("   ");
			vt.val.valu-=lastoff;
			lastsym+=*(lastsym+1);
			}
		}
	else print_val(&vt);
	lf();
	}




/*	EXPR  --	print results of expressions  */

expr() {
	struct vstruct vt;

	prompt("input an expression");
	glineno();
	got_call=0;
	do {
		expression(&vt);
		if (got_call) lf();
		got_call=0;
		print_val(&vt);
		dputs("   ");
		}
	while (ifop(","));
	lf();
	}



/*	PRINT_VAL  -- print a vt value	*/

print_val(vt)
	struct vstruct *vt; {
	struct vstruct newvt;
	char *typ,*ptr,len,ch;
	unsigned *wp;

	if (vt.vtype == OPPV) return;

	typ=vt.typep;
	toconst(vt);
	switch(*typ) {
		case CCHAR:		dprintf("%3u ",vt.val.vali);
						if (vt.val.vali >= ' ' && vt.val.vali <= 0x7f) 
							dprintf("'%c'",vt.val.vali);
						break;
		case CLABEL:	dprintf("label at "); addr(vt);
						break;				
		case FUNCTION:	dprintf("function at "); addr(vt);
						break;				
		case CUNSG:		dprintf("%5u %04x",vt.val.vali,vt.val.vali);
						break;
		case CSTRUCT:	dprintf("structure at "); addr(vt);
						break;
		case ARRAY:		dprintf("array at "); addr(vt);
						len=*((int *) (typ+1));
						if (len > 21) len=21;
						if (*(typ+3) == CCHAR) {
							dputs("->");
							toptr(vt);
							ptr=vt.val.vali;
							wp=&origds;
							if (is_big) {
								wp=&vt.val.vall;
								wp++;
								}
							len=21;
							while ((ch=_peek(ptr++,*wp)) && --len)
								xscr_co((ch >= ' ' && ch < 0x7f) ? ch: ' ');
							}
						break;
		case PTRTO:		wp=&vt.val.vall;
						wp++;
						if (vt.val.valu >= strings &&
							(is_big == 0 || *wp == origss)) ;
						else  {
							addr(vt);
							dputs("->");
							}
						if (*(typ+1) == CCHAR) {
							ptr=vt.val.vali;
							if (is_big == 0) wp=&origds;
							len=21;
							while ((ch=_peek(ptr++,*wp)) && --len)
								xscr_co((ch >= ' ' && ch < 0x7f) ? ch: ' ');
							}
						else {
							_move(12,vt,&newvt);
							newvt.vtype=RMV;
							newvt.typep=typ+1;
							print_val(&newvt);
							}
						break;

		default:		dprintf("%6d %04x",vt.val.vali,vt.val.vali);
						break;
		case CLONG:		dprintf("%11ld",vt.val.vall);
						break;
		case CFLOAT:
		case CDOUBLE:	dprintf("%13.6e",vt.val.vald);
		}
	}






/*	LIST  --	list a file.	*/

list(nlist)
	int nlist; {
	struct vstruct vt;
	char ch,xname[60],match,*ptr,lline[130],lastch,*oldneed;
	int i;
	unsigned listwant,oldcur;
	struct slocum { char lchar,llen; unsigned llline,lloc; char lbyte; } *sloc, *sloc2;
	
	match=0;
	listwant=listnum;
	if (nlist == 2) {
		listwant=curline;
		}

	else if (nlist == 3) {		/* list 9 lines after a go	*/
		if (curline >= 5) {
			listwant=curline-4;
			repeat=9;
			}
		else {
			listwant=1;
			repeat=curline+4;
			}
		}

	else if (nlist == 1 && lastcmd != 'L') {
		if (strcmp (listname,curname) == 0) listwant=curline;
		else listwant=1;
		}

	if (listfil != -1 && strcmp(listopen,listname) !=0) {
		close(listfil);
		listfil=-1;
		}
	if (listfil == -1) {
		if ((listfil=open(listname,0)) == -1) {
			cmd='.';
			error2("cannot open",listname);
			}
		strcpy(listopen,listname);
		listnum=1;
		listoff=&listbuf[2048];
		}
	if (nlist == 1) {
		sprintf(xname,"%d",listwant);
		prompt("enter list line number or search string");
		gline(xname);
		ptr=nextin;
		for (i=0; *ptr && *ptr != '\r' ; i++) {
			if (isdigit(*ptr) == 0 && *ptr != '-' && *ptr != '+') match=1;
			xname[i]=*ptr++;
			}
		if (match) {
			cmd='.';
			xname[i]=0;
			nextin=ptr;
			listwant=1;
			}
		else {
			expression(&vt);
			if (vt.vtype == CONSTV) listwant=vt.val.vali;
			}
		}
	if (listwant <= 0) listwant =1;
	if (listwant < listnum) {
		rewind(listfil);
		listnum=1;
		listoff=&listbuf[2048];
		}

	while (listnum < listwant) {
		list_skip();
		ch=listch();
		if (ch == '\n') listnum++;
		if (ch == 26) {
			dputs("end of file");
			lf();
			listoff--;
			return;
			}
		}
	if (repeat == 0) repeat=10;
	while (repeat--) {
		i=0;
		while ((lastch=listch()) != '\n' && lastch != 26) {
			lline[i++]=lastch;
			}
		lline[i]=0;
		if (match) {
			ch=xname[0];
			ptr=lline;
			while (*ptr) {
				if (ch == *ptr) {
					i=1;
					while (xname[i]) {
						if (ptr[i] != xname[i]) goto notm;
						i++;
						}
					goto need_line;
					}
notm:			ptr++;
				}
			repeat++;
			if (lastch == '\n') listnum++;
			goto no_match;
			}
					
need_line:
		cursor();
		dprintf("%5d  ",listnum);
		if (nlist == 3 && listnum == curline) dputs("->");
		else dputs("  ");
		col=0;
		ptr=lline;
		while (ch=*ptr++) {
			if (col == 70) {
				lf();
				dputs("       ");
				}
			if (ch == '\t') {
				do {
					xscr_co(' ');
					}
				while ((col) & 3);
				}
			else {
				if (ch != '\r') xscr_co(ch);						
				}
			}
		if (lastch == '\n') listnum++;
		else {
			dputs("end of file");
			listoff--;
			}

	/*	if listing for step, list another if at same IP.	*/

		if (nlist == 2) {
			ptr=needline+*(needline+1);
			sloc = (struct slocum *) needline;
			sloc2 = (struct slocum *) ptr;
			if (*ptr == OLINE && sloc->lloc == sloc2->lloc) {
				oldcur=curline;
				oldneed=needline;
				sloc2 = (struct slocum *) ptr;
				curline=sloc2->llline;
				needline=ptr;
				lf();
				list(2);
				curline=oldcur;
				needline=oldneed;
				}
			return;
			}
		lf();
no_match:
		if (lastch == 26) return;
		}
	}


/*	LISTCH  -- return a character for list  */

listch() {
	int  numin;

	if (listoff == &listbuf[2048]) {
		if ((numin=read(listfil,listbuf,2048)) == -1) {
			error2("cannot read",listname);
			}
		listbuf[numin]=26;
		listbuf[numin+1]='\n';
		listoff=listbuf;
		}
	return *listoff++;
	}

/*	LIST_SKIP  --	skip to next LF within list file.	*/

list_skip() {
#asm
		mov		di,ds
		mov		es,di
		mov		di,word listoff_
		cmp		di,offset listbuf_[2048]
		jz		got_listoff
		mov		al,10					;line feed
		mov		cx,-1
		cld
repnz	scasb
		dec		di
		cmp		byte [di-1],26			;past eof ?
		jnz		got_listoff
		dec		di
got_listoff:
		mov		word listoff_,di

#
	}

	


/*	OPTIONS -- set the options.	*/

options() {
	char ch;

	prompt("Flip-on-go  Go-list  List-name");
	ch=inch();
	if (ch == 'L') {
		prompt("input list file name");
		infname(listname);
		}
	else if (ch == 'G') {
		prompt("list after a Go (y or n)");
		if ((ch=inch()) == 'Y') golist=1;
		else if (ch == 'N') golist=0;
		}
	else {
		prompt("flip screen on Go (y or n)");
		if ((ch=inch()) == 'Y') goflip=1;
		else if (ch == 'N') goflip=0;
		}
	}




/*	REGISTER  --	display registers */

registers() {
	int  i;

	for (i=0; i < NUMREG; i++) {
		if (i == 8) lf();
		dputs(rname[i]);
		xscr_co('=');
		dprintf("%04x",*((int *) &rax+i));
		xscr_co(' ');
		}
	lf();
	}





/*	UNASSEMBLE --  Print some dissassembled instructions	*/

unassemble(needa)
	char needa; {
	unsigned afrom,i;

	if (needa) {
		dfrom.seg=rcs;
		dfrom.off=rip;
		if (repeat != 32767) inaddr(&dfrom);
		}
	if (repeat  == 0) repeat=10;
	if (repeat == 32767) {
		prompt("space to step. default=quit");
		cursor();
		}
	while (repeat--) {
		dprintf("%04x:%04x  ",dfrom.seg,dfrom.off);
		afrom=dfrom.off;
		call_dis(&dfrom);
		for (i=0; i < 6; i++) {
			if (afrom < dfrom.off) dprintf("%02x ",_peek(afrom++,dfrom.seg));
			else dputs("   ");
			}
		dbuffer[dbuffer[0]+1]=0;
		dputs(dbuffer+1);
		if (repeat == 32766) {		/* want to step an instruction. */
			repeat++;
			dputs("        ");
			scr_set_cursor();
			if (inch() != ' ') break;
			xstep();
			where();
			scr_rowcol(3,0);
			whereline();
			dfrom.off=rip;
			dfrom.seg=rcs;
			}
		lf();
		}
	}

	


/*	VARIABLES  --	list some variable names */

variables() {
	char name[32],def,*ptr;
	int i,j,len,pad,inproc,temp;
	unsigned *wp,nline;
	struct vstruct vt;
	struct sloty { unsigned loc; char type [1];} *psloty;
	struct zloty { unsigned zzloc,loc_seg; char big_type [1];} *pzloty;

	inproc=0;
	def=0;
	nline=0;
	skipbl();
	prompt("input variable name or pattern (a* means start with a)");
	name[0]=0;
	infname(name);
	if (name[0] == 0) {
		name[0]='*';
		name[1]=0;
		def=1;
		}
	col=0;

/*	print locals first	*/

	i=0;
	while (localat[i]) {
		j=i+1;
		while (localat[j]) {
			if (strcmp(localat[i],localat[j]) == 1) {
				temp=localat[i];
				localat[i]=localat[j];
				localat[j]=temp;
				}
			j++;
			}
		i++;
		}

	i=0;
	while (localat[i]) {
		if (match(name,localat[i])) {
			if (nline == 16) {
				nline=0;
				prompt("hit key for more");
				inch();
				}
			if (def) {
				pad=(80-col) %20;
				len=strlen(localat[i]);
				if (col+pad+len >= 80) {
					nline++;
					lf();
					}
				else {
					while (pad--) xscr_co(' ');
					}
				dputs(localat[i]);
				}
			else {
				dputs(localat[i]);
				dputs(" = ");
				ptr=localat[i];
				while (*ptr++) ;
				psloty = (struct sloty *) ptr;
				vt.val.valu=psloty->loc+rbp;
				vt.typep=&psloty->type[1];
				if (is_big) *(&vt.val.valu+1)=origss;
				vt.vtype=RMV;
				print_val(&vt);
				nline++;
				lf();
				}
			}
		i++;
		}
	if (col) lf();

	ovnum=0;
	lastsym=symbols;
	while (*lastsym) {
		if (*lastsym == OPTYPE) {
			if (match(name,lastsym+2) && (ovnum == 0 || ovnum == overley)) {
				if (nline == 16) {
					nline=0;
					prompt("hit key for more");
					inch();
					}
				if (def) {
					pad=(80-col) %20;
					len=strlen(lastsym+2);
					if (col+pad+len >= 80) {
						nline++;
						lf();
						}
					else {
						while (pad--) xscr_co(' ');
						}
					dputs(lastsym+2);
					}
				else {
					dputs(lastsym+2);
					dputs(" = ");
					ptr=lastsym;
					while (*ptr++) ;
					psloty = (struct sloty *) ptr;
					vt.val.valu=psloty->loc;
					vt.typep=&psloty->type[1];
					if (is_big) {
						wp=&vt.val.valu;
						pzloty = (struct zloty *) ptr;
						*(wp+1)=pzloty->loc_seg;
						vt.typep=&pzloty->big_type[1];
						}
					vt.vtype=RMV;
					print_val(&vt);
					nline++;
					lf();
					}
				}
			}
		else if (*lastsym == OOV) ovnum=*(lastsym+2);
		lastsym=lastsym+=*(lastsym+1);
		}
	lf();
	}



/*	STATUS_8087  --		display the status of the 8087.	*/

status_8087() {
	static int  status,cw,i;

	status=cw=0;
#asm
	fnstsw	status_8087_status_
	fnstcw	status_8087_cw_
#
	for (i=0; i < 1000; i++) ;	/* wait in case no 8087 */
	dputs("8087 status word = ");
	ohex(status & 0xc7ff);
	dputs("  control word = ");
	ohex(cw);
	dputs("   level = ");
	ounum((status >> 11) & 7);
	scr_clrl();
	}




/*	SHOWHERE  --	show calling list.	*/

showhere() {
	unsigned oldip,thisbp;

	oldip=rip;
	thisbp=rbp;

	while (thisbp >= rbp && thisbp <= highbp) {
		where();
		whereline();
		lf();
		if (thisbp == highbp) break;
		_lmove(2,thisbp+2,origss,&rip,localss);
		_lmove(2,thisbp,origss,&thisbp,localss);
		}
	rip=oldip;
	}


	
/*	WHERELINE  --	Tell where the user is in the program  */

whereline() {

	if (curproc[0]) {
		dputs("procedure ");
		dputs(curproc);
		}
	if (curname[0]) {
		if (needname) {
			needname=0;
			strcpy(listname,curname);
			}
		dputs("     file ");
		dputs(curname);
		if (curline) {
			dputs("  line ");
			ounum(curline);
			if (curoff) {
				dputs("+");
				ohex(curoff);
				}
			}
		}
	if (overley) {
		dputs("  overlay ");
		ounum(overley);
		}
	scr_clrl();
	}




/*	WHERE  --	Set curproc to name of current procedure and
				curname to current file name and curlin to current line	*/

where() {
	unsigned lineloc;
	char *locat,*thisname,inproc,proc_type;
	curproc=curname="";
	curline=0;
	procloc=lineloc=0;
	procend=0xffff;
	procseg=0;
	numlocal=0;
	inproc=0;
	struct slocum { char lchar,llen; unsigned lline,lloc; char lbyte; } *sloc;
	struct bslocum { char bjunk[6]; unsigned lloc_seg; char big_lbyte; } *bsloc;
	struct zlocum { unsigned zzloc,loc_seg; char big_type [1];} *zloc;
	struct sloty { unsigned loc; char type [1];} *pslot;

	if (overley_at) overley=_peek(overley_at,origds);
	ovnum=0;
	lastsym=symbols;
	if (*lastsym == 0) goto nowhere;
	do {
		if (*lastsym == OOV) ovnum=*(lastsym+2);
		if (ovnum != 0 && ovnum != overley) continue;
		if (*lastsym == OLINE) {
			sloc = (struct slocum *) lastsym;
			if (sloc->lloc <= rip && sloc->lloc >= lineloc) { 
				bsloc = (struct bslocum *) lastsym;
				if (is_big && bsloc->lloc_seg != rcs) continue;
				curname=thisname;
				curline=sloc->lline;
				lineloc=sloc->lloc;
				}
			}
		else switch (*lastsym) {
			case OPTYPE:	locat=lastsym;
							while (*locat++) ;
							pslot = (struct sloty *) locat;
							proc_type=pslot->type[1];
							if (is_big) {
								zloc = (struct zlocum *) locat;
								if (zloc->loc_seg != rcs) break;
								proc_type=zloc->big_type[1];
								}
							if (proc_type == FUNCTION) {
								pslot = (struct sloty *) locat;
								if (pslot->loc > procloc && pslot->loc <= rip) {
									curproc=lastsym+2;
									procloc=pslot->loc;
									procseg=rcs;
									}
								else if (pslot->loc > rip && pslot->loc < procend) {
									procend=pslot->loc;
									procseg=rcs;
									}
								}
							break;
			case OLTYPE:	if (inproc && numlocal < MAXLOCAL-1)
								localat[numlocal++]=lastsym+2;
							break;
			case ONAME:		thisname=lastsym+2;
							break;

			case OLNAME:	if (strcmp(lastsym+2,curproc) == 0) inproc=1;
							else inproc=0;
							break;
			}
		}
	while (*(lastsym+=*(lastsym+1)));

	curoff=rip-lineloc;
nowhere:
	localat[numlocal]=0;
	}





/*	MATCH  --	Match a name to a name pattern.	*/

match(namepat,name)
	char *namepat,*name; {

	while (*name) {
		if (*namepat == '*') return 1;
		if (toupper(*name) == *namepat || *namepat == '?') {
			name++;
			namepat++;
			}
		else return 0;
		}
	return *namepat == 0 || *namepat == '*';
	}

