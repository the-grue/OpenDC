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
/*	TOOBJ.C		converter from .O to .OBJ format for C88	*/

#define IBM		1			/*	true if creating BIND for MS-DOS	*/
#include "OBJ.H"

#define ABSIZE	500
#define NUMLAB	2000

#define LOTHER		0
#define LPUBLIC		1
#define LRESERVE	2

#define CINT	2
#define CFUN	253

#define INDSEG	1
#define INCSEG	2
#define INUSEG	3
#define INESEG	4
#define INPTR4	5

#define CONTZ	26
	char inbuf[1024*10+200],*inin,*endin;
	char coderec[1000],*incode,codefix[1000],datafix[1000],esegfix[1000];
	char datarec[1000],esegrec[1000],*indata,inname[65],outname[65];
	char byte4rec[560],byte4fix[560];
	int  infile,outfile,*fixat,numab,nerr,chkfile;
	char curseg,*inext,name[33],labis[NUMLAB];
	unsigned offs[6],hash[32],labat[NUMLAB],lab_ptr4[NUMLAB];
	unsigned  stacklen;
	unsigned reslen,resat,len,num,nummod,num_byte4,off_byte4,byte4_at;
	char aopt,hopt,*found,deleteit,labseg[NUMLAB],is_large,progname[15];
	int  pfile=-1,numfile,numread;
	char argbuf[ABSIZE];		/* -f arguments go here	*/
	long objat;


	struct sym {int eseg; char edefn,enlen,eisext; int elen,eptr4_at; } *stable;
	typedef struct sym *sp;

	union {int word; char byte; };


	char *memory,*memlast;



main(argc,argv)
	int  argc;
	char *argv[]; {

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
	char *argat,i,gotdot;
	int  nin,i,ffile;

	inext=memory=_memory();
	memlast=_showsp()-512;

	argat=argv[1];
	if (*(argat+1) == ':') argat+=2;		/* see if input is CTEMP6 */
	if (strcmp(argat,"CTEMP6") == 0) {
		deleteit=1;
		}
	else 	{
		printf("OpenOBJ  v0.1  Based on\n");
		printf("OBJ Converter for C88 and ASM88     V1.4    (c) Mark DeSmet, 1984,85,86,88\n");
		}
	if (argc == 1) ferror("missing filename","");
	else if (argc == 2) cmdname(argv[1]);
	else if (argc == 3) {
		argat=argv[2];
		if (*argat == '-') argat++;
		if (toupper(*argat) == 'O') cmdname(argat+1);
		else ferror("illegal argument",argv[2]);
		}
	else ferror("too many arguments","");

/*	create the input name	*/
	strcpy(inname,argv[1]);
	gotdot=0;
	i=0;

	if (deleteit == 0) {
		argat=inname;
		while (*argat) {
			if (*argat == '.') gotdot=1;
			inname[i++]=*argat++;
			}
		if (gotdot) {
			inname[i]=0;
			}
		else strcpy(&inname[i],".O");
		}
	}


cmdname(name)
	char *name; {
	char i,*namefrom,namerec[40];

	i=0;
	namefrom=name;
	while (*name && *name != '.') {
		if (*name == '\\' || *name == '/' || *name == ':') namefrom=name+1;
		outname[i++]=*name++;
		}
	strcpy(&outname[i],".OBJ");
	if ((outfile=creat(outname)) == -1)
		ferror("cannot create",outname);

	/*	create the name record	*/
	newrec(namerec,0x80);
	addb(namerec,0);
	i=0;
	while (*namefrom && *namefrom != '.') {
		progname[i++]=*namefrom;
		add_string_b(namerec,&namerec[3],*namefrom++);
		}
	outrec(namerec);
	}



/*	return a 1 if character is a blank, CR or LF	*/

iswhite(ch)
	char ch; {

	return ch == ' ' || ch == '\r' || ch == '\n';
	}



nextpass(pass)
	char pass; {
	char *argat;
	int  i;

	if (numread == 0) {		/* file is still closed	*/
		if ((infile=open(inname,0)) == -1)
			ferror("cannot open ",inname);
		inin=endin=&inbuf[1024*10];
		}
	else if (numread == 1) {	/* file is entirely in buffer */
		inin=inbuf;
		}
	else {
		if (lseek(infile,0,0,0) == -1)
			ferror("cannot seek ",inname);
		inin=endin=&inbuf[1024*10];
		}

	do {
		_setmem(labis,NUMLAB,LOTHER);
		_setmem(lab_ptr4,sizeof(lab_ptr4),255);
		if (pass == 1) pass1();
		else pass2();
		if (inin >= endin) refill();
		if (*inin == CONTZ) inin=endin;
		}
	while (inin < endin);
	}

pass1() {
	char clen;
	unsigned i,*next;

	while (1) {
		if (inin+128 > endin) refill();
		switch (clen=*inin++) {
			case OEOF:		return;
			case OBIG:		is_large=1;
							break;
			case OPUBLIC:	find(1);
							break;
			case OSTATIC:	while(*inin++) ;
							inin+=2;
							break;
			case ORESERVE:	num=inin->word;
							inin+=2;
							len=inin->word;
							inin+=2;
							if (labis[num] == LPUBLIC) {
								found=labat[num];
								switch (((sp)found)->edefn) {
									case LOTHER:	((sp)found)->edefn=LRESERVE;
													((sp)found)->elen=len;
													break;
									case LRESERVE:	if (((sp)found)->elen < len)
														((sp)found)->elen=len;
									case LPUBLIC:	;
									}
								}
							else {
								if (reslen+len < reslen)
									ferror("over 64K of data","");
								reslen+=len;
								}
							break;
			case OLOCAL:	num=inin->word;
							if (labis[num] == LPUBLIC) {
								inin+=2;
								len=inin->word;
								inin+=2;
								if (num >= NUMLAB)
									ferror("too many labels in ",inname);
								found=labat[num];
								if (((sp)found)->edefn == LPUBLIC) {
									error("multiply defined",
										found-((sp)found)->enlen);
									}
								else {
									((sp)found)->edefn=LPUBLIC;
									((sp)found)->elen=len+offs[curseg];
									}
								}
							else inin+=4;
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

			case OPTR:		num=inin->word;
							inin+=2;
							if (lab_ptr4[num] == 65535) {
								if (labis[num] == LPUBLIC) {
									found=labat[num];
									if (((sp)found)->eptr4_at == 0) {
										((sp)found)->eptr4_at=1;
										}
									else break;
									}
								num_byte4++;
								lab_ptr4[num]=1;
								}
							break;
			case OSEGPTR:
			case OLNAMEREL:	inin+=2;
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
	char clen,tlen;
	int fix,fixer,add,len;

	while (1) {
		if (inin+128 > endin) refill();
		switch(clen=*inin++) {
			case OEOF:		return;
			case OBIG:		break;
			case OPUBLIC:	find(2);
							break;
			case OSTATIC:	while(*inin++) ;
							inin+=2;
							break;
			case ORESERVE:	num=inin->word;
							inin+=2;
							len=inin->word;
							inin+=2;
							if (labis[num] == LOTHER) {
								labat[num]=resat;
								labseg[num]=INUSEG;
								resat+=len;
								}
							break;
			case OLOCAL:	num=inin->word;
							if (labis[num] == LOTHER) {
								inin+=2;
								len=inin->word;
								inin+=2;
								labat[num]=len+offs[curseg];
								labseg[num]=curseg;
								}
							else inin+=4;
							break;
			case ODSEG:		curseg=INDSEG;
							break;
			case OCSEG:		curseg=INCSEG;
							break;
			case OESEG:		curseg=INESEG;
							break;
			case ONAMEREL:	fix=inin->word;
							inin+=2;
							fixup_off(fix);
							break;
			case OJUMPREL:	fix=inin->word;
							inin+=2;
							if (labis[fix] == LPUBLIC) {
								if (((sp)labat[fix])->eisext) {
									add_fix(0x84,(is_large ? 0x26:0x16),
										((sp)labat[fix])->eseg,((sp)labat[fix])->elen);
									break;
									}
								*fixat+=((sp)labat[fix])->elen-offs[curseg];
								}
							else *fixat+=labat[fix]-offs[curseg];
							break;
			case OPTYPE:	while (*inin++) ;
							inin+=2;
							inin+=*inin+1;
							break;

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

			case OPTR:		num=inin->word;
							inin+=2;
							if (lab_ptr4[num] == 65535) {
								if (labis[num] == LPUBLIC) {
									found=labat[num];
									if (((sp)found)->eptr4_at == 0) {
										((sp)found)->eptr4_at=off_byte4;
										}
									else {
										lab_ptr4[num]=((sp)found)->eptr4_at;
										goto do_ptr4_fix;
										}
									}
								lab_ptr4[num]=off_byte4;
								create_byte4(num);
								}
do_ptr4_fix:
							*fixat+=lab_ptr4[num];
							add_fix(0xc4,4,INDSEG,0);
							break;
			case OSEGPTR:	fix=inin->word;
							inin+=2;
							fixup_seg(curseg,fix);
							break;
			case OLNAMEREL:	fix=inin->word;
							inin+=2;
							fixup_ptr(fix);
							break;
			default:		clen-=128;
							if (clen > 1) checkrecs(500);
							offs[curseg]+=clen;
							if (curseg == INDSEG) {
								while (clen--)
									addb(datarec,*inin++);
								len=*(int *) (datarec+1);
								fixat=&datarec[len+1];
								}
							else if (curseg == INCSEG) {
								while (clen--)
									addb(coderec,*inin++);
								len=*(int *) (coderec+1);
								fixat=&coderec[len+1];
								}
							else {
								while (clen--)
									addb(esegrec,*inin++);
								len=*(int *) (esegrec+1);
								fixat=&esegrec[len+1];
								}
			}
		}
	}

between() {
	char *room,*hptr,*nameptr,*pfrom,segrec[40],typerec[20],segname[40];
	char namerec[50];
	char zerorec[20],namebuf[40];
	int  i,*next,numres;
	unsigned add,next_seg,this_seg,puboff;

	objat=0;

/*	need space for 4 byte pointers	*/
	byte4_at=off_byte4=offs[INDSEG];
	offs[INDSEG]+=num_byte4*4;
	resat=0;
	numres=1;
	next_seg=offs[INESEG] ? 5: 4;
/*	create the lnames record	*/

	newrec(namerec,0x96);
	addb(namerec,0);
	add_string(namerec,"DATA");
	add_string(namerec,"DGROUP");
	if (is_large == 0) {
		add_string(namerec,"PGROUP");
		add_string(namerec,"PROG");
		add_string(namerec,"UDATA");
		}
	else {
		add_string(namerec,"PROG");
		strcpy(namebuf,progname);
		strcat(namebuf,"_PROG");
		add_string(namerec,namebuf);
		strcpy(namebuf,progname);
		strcat(namebuf,"_ESEG");
		add_string(namerec,namebuf);
		add_string(namerec,"UDATA");
		if (offs[INESEG]) add_string(namerec,"ESEG");
		}
	outrec(namerec);

	/*	allocate reserved in order so public reserved's are in order */
	found=memory;
 	while (found < inext) {
		found+=2;
		nameptr=found;
		while (*found++) ;
		if (((sp)found)->edefn == LRESERVE) {
			if (((sp)found)->eseg != INESEG) {
				add=((sp)found)->elen;
				((sp)found)->elen=resat;
				resat+=add;
				((sp)found)->eseg=INUSEG;
				if (reslen+add < reslen)
					ferror("over 64K data","");
				reslen+=add;
				}
			else {
				/*	output lname record for the reserved publics in eseg	*/
				newrec(namerec,0x96);
				strcpy(segname,nameptr);
				strcat(segname,"DATA");
				add_string(namerec,segname);
				((sp)found)->eseg=next_seg++;
				outrec(namerec);
				}
			}
		found+=sizeof(*stable);
		}

/*	Output SEGDEF records. DATA first then CODE then ESEG. */

	newrec(segrec,0x98);
	addb(segrec,0x48);
	addw(segrec,offs[INDSEG]);	
	addb(segrec,2);
	addb(segrec,2);
	addb(segrec,1);
	outrec(segrec);
	newrec(segrec,0x98);
	addb(segrec,is_large ? 0x20: 0x28);
	addw(segrec,offs[INCSEG]);	
	addb(segrec,5);
	addb(segrec,is_large ? 4: 5);
	addb(segrec,1);
	outrec(segrec);

/*	SEGDEF for UDATA	*/

	newrec(segrec,0x98);
	addb(segrec,0x48);
	addw(segrec,reslen);	
	addb(segrec,6+is_large);
	addb(segrec,2);
	addb(segrec,1);
	outrec(segrec);
	if (is_large && offs[INESEG]) {
		newrec(segrec,0x98);
		addb(segrec,0x20);
		addw(segrec,offs[INESEG]);	
		addb(segrec,6);
		addb(segrec,8);
		addb(segrec,1);
		outrec(segrec);
		}

/*	SEGDEF records for ESEG	*/

	if (is_large) {
		found=memory;
 		while (found < inext) {
			found+=2;
			nameptr=found;
			while (*found++) ;
			((sp)found)->eptr4_at=0;
			if (((sp)found)->edefn == LRESERVE && ((sp)found)->eseg >= INESEG) {
				newrec(segrec,0x98);
				addb(segrec,0x40);
				addw(segrec,((sp)found)->elen);	
				addb(segrec,((sp)found)->eseg+4);
				addb(segrec,2);
				addb(segrec,1);
				outrec(segrec);
				((sp)found)->elen=0;	/* no offset	*/
				}
			found+=sizeof(*stable);
			}
		}

/*	Output GRPDEF records.	*/

	newrec(segrec,0x9a);
	addb(segrec,3);
	addb(segrec,0xff);
	addb(segrec,1);
	addb(segrec,0xff);
	addb(segrec,3);
	outrec(segrec);
	if (is_large == 0) {
		newrec(segrec,0x9a);
		addb(segrec,4);
		addb(segrec,0xff);
		addb(segrec,2);
		outrec(segrec);
	}

/*	Output TYPDEF record with NULL type.	*/

	newrec(typerec,0x8e);
	addb(typerec,0);
	addb(typerec,0);
	addb(typerec,0x80);
	outrec(typerec);


/*	output PUBDEF and EXTDEF records	*/

	found=memory;
 	while (found < inext) {
		found+=2;
		nameptr=found;
		while (*found++) ;
		if (((sp)found)->edefn == LRESERVE || ((sp)found)->edefn == LPUBLIC) {
			((sp)found)->eisext=0;
			newrec(segrec,0x90);
			addb(segrec,0);
			this_seg=((sp)found)->eseg;
			if (this_seg >= 128) addb(segrec,128+(this_seg>>8));
			addb(segrec,this_seg);
			add_string(segrec,nameptr);
			puboff=((sp)found)->elen;
			if (((sp)found)->eseg >= INESEG && ((sp)found)->edefn == LRESERVE) puboff=0;
			addw(segrec,puboff);
			addb(segrec,1);
			outrec(segrec);
			}
		else {
			((sp)found)->eisext=1;
			((sp)found)->elen=numres++;
			newrec(segrec,0x8c);
			add_string(segrec,nameptr);
			addb(segrec,1);
			outrec(segrec);
			}
		found+=sizeof(*stable);
		}

/*	restart offset counts	*/

	offs[INDSEG]=offs[INCSEG]=offs[INESEG]=0;

/*	set up data, code and fixup records	*/

	newdata();
	newcode();
	neweseg();
	newbyte4();

	}



find(pass)
	char pass; {
	char hashno,ch,mlen,nlen,*target;
	int  *next,tlen,dif;

	hashno=nlen=0;
	do {
		ch=*inin++;
		if (ch >= 'a') ch-=32;
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

	next=&hash[hashno];
	while (*next) {
		found=*next+2;
		mlen=0;
		while (*found++ == name[mlen++]) ;
		if (mlen > nlen) {
			found--;
				if (((sp)found)->eseg != curseg && pass == 1)
					error("different segments for",name);
				labis[num]=LPUBLIC;
				labat[num]=found;
				labseg[num]=((sp)found)->eseg;
			return;
			}
		next=*next;
		}
	*next=inext;
	inext->word=0;
	found=inext+2;
	mlen=0;
	do
		*found++=name[mlen];
	while (name[mlen++]);
	((sp)found)->eseg=curseg;
	((sp)found)->edefn=LOTHER;
	((sp)found)->enlen=nlen;
	((sp)found)->elen=num;
	((sp)found)->eptr4_at=0;
	inext=found+sizeof(*stable);
	if (inext >= memlast)
		ferror("too many total PUBLICs in ",inname);
	labis[num]=LPUBLIC;
	labat[num]=found;
	}

refill() {
	char *temp;
	int  numbytes;

	if (endin < &inbuf[1024*10]) return;
	temp=inbuf;
	while (inin < endin) *temp++=*inin++;
	if ((numbytes=read(infile,temp,1024*10)) == -1)
		ferror("cannot read ",inname);
	numread++;
	endin=temp+numbytes;
	inin=inbuf;
	}

endup() {
	unsigned max;
	char endrec[20];

	checkrecs(0);
	checkbyte4(0);

/*	output the end record	*/

	newrec(endrec,0x8a);
	addb(endrec,0);
	outrec(endrec);

	if (close(infile) == -1)
		ferror("cannot close ",inname);
	if (close(outfile) == -1)
		ferror("cannot close ",outname);
	if (deleteit == 1) {
		if (unlink(inname) == -1)
			ferror("cannot delete",inname);
		}
	else puts("end of TOOBJ    ");
	if (nerr) {
		onum(nerr);
		puts(" errors");
		}
	}

/*	CREATE_byte4  --	create a four byte pointer.	*/

create_byte4(fix)
	int  fix; {
	unsigned *temp_fixat;
	char temp_seg;

	temp_fixat=fixat;
	temp_seg=curseg;
	curseg=INPTR4;
	addw(byte4rec,0);
	addw(byte4rec,0);
	len=*(int *) (byte4rec+1);
	fixat=&byte4rec[len+1];
	fixup_ptr(fix);
	fixat=temp_fixat;
	curseg=temp_seg;
	off_byte4+=4;
	checkbyte4(500);
	}
	

/*	FIXUP_PTR  --	add a four byte fixup at fixat.	*/

fixup_ptr(fix)
	int  fix; {
	int  segis;

	fixat--;
	segis=labseg[fix];
	if (labis[fix] == LPUBLIC) {
		if (((sp)labat[fix]) ->eisext) {
			add_fix(0xcc,0x26,((sp)labat[fix])->eseg,
				((sp)labat[fix])->elen);
			return;
			}
		*fixat+=((sp)labat[fix])->elen;
		segis=((sp)labat[fix])->eseg;
		}
	else *fixat+=labat[fix];
	add_fix(0xcc,4,segis,0);
	fixat++;
	}


/*	FIXUP_OFF  --	do an offset fixup at fixat.	*/

fixup_off(fix)
	int  fix;	{
	int  segis;
	segis=labseg[fix];
	if (labis[fix] == LPUBLIC) {
		if (((sp)labat[fix]) ->eisext) {
			add_fix(0xc4,(is_large ? 0x26: 0x16),((sp)labat[fix])->eseg,
				((sp)labat[fix])->elen);
			return;
			}
		*fixat+=((sp)labat[fix])->elen;
		segis=((sp)labat[fix])->eseg;
		}
	else *fixat+=labat[fix];
	add_fix(0xc4,4,segis,0);
	}


	

/*	FIXUP_SEG  --	do a segment fixup at fixat .	*/

fixup_seg(segin,fix)
	int  segin,fix; {

	int  segis;
	segis=labseg[fix];
	if (labis[fix] == LPUBLIC) {
		if (((sp)labat[fix]) ->eisext) {
			add_fix(0xc8,0x26,((sp)labat[fix])->eseg,
				((sp)labat[fix])->elen);
			return;
			}
		segis=((sp)labat[fix])->eseg;
		}
	add_fix(0xc8,4,segis,0);
	}


/*	form OBJ records	*/

checkrecs(max)
	int max; {
	if ((*(int *) &datarec[1]) > 3+max || (*(int *) &datafix[1]) > max) {
		outrec(datarec);
		if (*(int *) &datafix[1]) outrec(datafix);
		newdata();
		}
	if ((*(int *) &coderec[1]) > 3+max || (*(int *) &codefix[1]) > max) {
		outrec(coderec);
		if (*(int *) &codefix[1]) outrec(codefix);
		newcode();
		}
	if ((*(int *) &esegrec[1]) > 3+max || (*(int *) &esegfix[1]) > max) {
		outrec(esegrec);
		if (*(int *) &esegfix[1]) outrec(esegfix);
		neweseg();
		}
	}


checkbyte4(max)
	int max; {
	if ((*(int *) &byte4rec[1]) > max+3 || (*(int *) &byte4fix[1]) > max) {
		outrec(byte4rec);
		if (*(int *) &byte4fix[1]) outrec(byte4fix);
		byte4_at+=*((int *) &byte4rec[1])-3;
		newbyte4();
		}
	}

newdata() {

	newrec(datarec,0xa0);
	addb(datarec,1);
	addw(datarec,offs[INDSEG]);
	newrec(datafix,0x9c);
	}


newbyte4() {

	newrec(byte4rec,0xa0);
	addb(byte4rec,1);
	addw(byte4rec,byte4_at);
	newrec(byte4fix,0x9c);
	}


newcode() {

	newrec(coderec,0xa0);
	addb(coderec,2);
	addw(coderec,offs[INCSEG]);
	newrec(codefix,0x9c);
	}

neweseg() {

	newrec(esegrec,0xa0);
	addb(esegrec,4);
	addw(esegrec,offs[INESEG]);
	newrec(esegfix,0x9c);
	}


newrec(rec,type)
	char *rec,type; {

	rec[0]=type;
	rec[1]=rec[2]=0;
	}


addb(rec,ch)
	char *rec,ch; {
	int len;

	len=*(int *) (rec+1);
	rec[len+3]=ch;
	*((int *) (rec+1))=len+1;
	}


addw(rec,wrd)
	char *rec;
	unsigned wrd; {
	int len;

	len=*(int *) (rec+1);
	*((unsigned *) (rec+len+3))=wrd;
	*((int *) (rec+1))=len+2;
	}


add_string_b(rec,str,ch)
	char *rec,*str,ch; {

	addb(rec,ch);
	(*str)++;
	}

add_string(rec,str)
	char *rec,*str; {
 	char slen;
	int  sat;

	slen=0;
	sat=*(int *) (rec+1);
	addb(rec,0);

	/* remove the trailing _	*/
	while (*str) {
		if (*str == '_' && *(str+1) == 0) break;
		addb(rec,*str++);
		slen++;
		}
	rec[sat+3]=slen;
	}


add_fix(relbyte,ftype,toseg,toext)
	char relbyte,ftype,toseg;
	int toext; {
	char *target;
	unsigned dro;

	if (curseg == INCSEG) {
		target=codefix;
		dro=((int)fixat)-coderec-6;
		}
	else if (curseg == INDSEG) {
		target=datafix;
		dro=((int)fixat)-datarec-6;
		}
	else if (curseg == INESEG) {
		target=esegfix;
		dro=((int)fixat)-esegrec-6;
		}
	else {
		target=byte4fix;
		dro=((int)fixat)-byte4rec-6;
		}

	addb(target,relbyte+(dro>>8));
	addb(target,dro);
	if (ftype == 4) {
		if (toseg == INDSEG) {
			addb(target,0x14);
			addw(target,0x101);
			}
		else if (toseg == INCSEG) {
			if (is_large) {
				addb(target,0x4);
				addw(target,0x202);
				}
			else {
				addb(target,0x14);
				addw(target,0x202);
				}
			}
		else if (toseg == INUSEG) {
			addb(target,0x14);
			addw(target,0x301);
			}
		else {
			addb(target,0x4);
			if (toseg >= 128) addb(target,128+(toseg>>8));
			addb(target,toseg);
			if (toseg >= 128) addb(target,128+(toseg>>8));
			addb(target,toseg);
			}
		return;
		}
	addb(target,ftype);
	if (ftype == 0x26) {
		if (toext >= 128) addb(target,128+(toext>>8));
		addb(target,toext);
		if (toext >= 128) addb(target,128+(toext>>8));
		addb(target,toext);
		}
	else {
		addb(target,toseg == INDSEG ? 1: 2);
		if (toext >= 128) addb(target,128+(toext>>8));
		addb(target,toext);
		}
	}




outrec(rec)
	char *rec; {
	int len,i;
	char csum;

	csum=0;
	len=++*((int *) (rec+1));
	i=len+1;
	do {
		csum+=rec[i];
		}
	while (i--);
	rec[len+2]=-csum;
	if (write(outfile,rec,len+3) == -1)
		ferror("cannot write",outname);
	}


	

ferror(str1,str2)
	char *str1,*str2; {

	ocrlf();
	puts(str1);
	puts(str2);
	puts("     TOOBJ abandoned\n");
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
