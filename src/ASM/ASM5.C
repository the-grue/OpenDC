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
 *      Changed lines 227, 270, 714, and 718-720
 *	to conform to new compiler.
 */

/*	ASM5.C				part5 of 8088 assembler		*/

#include "ASM88.H"
#include "OBJ.H"

tokit() {
	do 
		while (ltype[*cur] == SPACE) cur++;
	while (tokone());
	}

tokone() {
	curch=*cur++;
	heir=0;

	switch (ltype[curch]) {
		case LETTER:find();
					if (heir == EQU) {
						if (nest_cur == 4) error("equate too deep");
						else {
							nestdef[nest_cur++]=cur;
							cur=wvalue+1;
							}
						return 1;
						}
					break;
		case DIGIT: number();
					break;
		case 4:		if (*cur == '=') {
						cur++;
						heir=19;
						bvalue=NE;
						}
					break;
		case 5:		heir=DSTRING;
					getstr('"');
					break;
		case 6:		heir=23;
					break;
		case 7:		heir=18;
					break;
		case 8:		heir=SSTRING;
					getstr('\'');
					break;
		case 9:		heir=23;
					break;
		case 10:	heir=22;
					break;
		case 11:	heir=22;
					break;
		case 13:	heir=23;
					break;
		case 14:	if (*cur == '<') {
						cur++;
						heir=21;
						break;
						}
					if (*cur == '=') {
						cur++;
						bvalue=LE;
						}
					else bvalue=LT;
					heir=20;
					break;
		case 15:	if (*cur == '=') {
						cur++;
						heir=19;
						bvalue=EQ;
						}
					break;
		case 16:	if (*cur == '>') {
						cur++;
						heir=21;
						break;
						}
					if (*cur == '=') {
						cur++;
						bvalue=GE;
						}
					else bvalue=GT;
					heir=20;
					break;
		case 17:	heir=17;
					break;
		case 18:	if (*cur == '|') {
						cur++;
						heir=14;
						}
					else heir=16;
					break;
		case 19:	if (cur > lastch) {
						cur=nestdef[--nest_cur];
						return 1;
						}
		default: ;
		}
	if (eopt) {
		obnum(heir);oc(curch);oc(' ');
		obnum(bvalue);ohw(wvalue);ocrlf();
		}
	return 0;
	}

/*	find a name. set wvalue to address of structure describing name type
	and set heir to symbol type. if name is new, assume it is a label
	that has not been defined yet	*/


find() {
	int  *wp,i,j,hashno;
	char *nextp;

	cur--;
	i=hashno=0;
	while ((ltype[*cur] <= DIGIT) && i < 32) {
		hashno+=string[i++]=*cur++;
		}
	hashno+=i;
	if (i == 32) while (ltype[*cur] <= DIGIT) cur++;
	string[i]=0;
	string[i+1]=0xfe;
	if (eopt) {
		os(string);
		oc(' ');
		}
	hashno&=127;
	nextp=hash[hashno];

/*	look for a match in symbol table	*/

	while (nextp) {
		j=0;
		wp=nextp;
		nextp+=2;
		if (*nextp == string[0]) {
			nextp++;
			while (*nextp == string[++j]) {
				if (*nextp++ == 0) {
					wvalue=nextp;
					heir=*wvalue;
					return;
					}
				}
			}
		nextp=*wp;
		}


	wp=freem;
	*wp=hash[hashno];
	hash[hashno]=wp;
	j=0;
	wvalue=wp+1;
	do
		*wvalue++=string[j++];
	while (j <= i);
	heir=wvalue->sheir=NAME;
	wvalue->sclass=0;
	wvalue->svalue=curseg;
	wvalue->stype=CLABEL;
	if (string[0] == '_' && string[1] == 'L' && string[2] >= '0' &&
		string[2] <= '9') {
		wvalue->snum=++line_ordinal;
		if (line_ordinal >= 2000) ferror("too many labels");
		}
	else {
		wvalue->snum=++ordinal;
		if (ordinal >= 1000) ferror("too many labels");
		}
	if ((freem=wvalue+8) >= memend) {
		error("too many symbols");
		exit(2);
		}
	}


getstr(quote)
	char quote; {
	char i;

	i=0;
	while (*cur != LF & *cur != CONTZ && i < 80) {
		if (*cur == quote) if (*(cur+1) == quote) cur++; else break;
		string[i++]=getach();
		}
	string[i]=0;
	if (*cur != quote) error("missing quote");
	cur++;
	}

getach() {
	char ch;
	ch=*cur++;
	if (ch == '\\') {
		ch=*cur++;
		if (ch >= 'a') ch-=32;
		if (ch == 'N') ch=LF;
		else if (ch == 'T') ch=9;
		else if (ch >= '0' && ch <= '7') {
			ch-='0';
			while (*cur >= '0' && *cur <= '7')
				ch=(ch<<3)+*cur++-'0';
			}
		}
	return ch;
	}

number() {
	char ch;
	int  fromp,i;
	long lval;
	struct vval{ unsigned lo,hi; } *ival = &lval;

	heir=CONSTANT;
	fromp=--cur;
	lval=0;

/*	if only 4 digits and not followed by a letter or period then integer. */

	i=wvalue=hiword=0;
	while (ltype[*cur] == DIGIT) {
		wvalue<<=1;
		wvalue+=(wvalue<<2)+*cur++-'0';
		i++;
		}
	if (ltype[*cur] != LETTER && i <= 4 && *cur !='.') return;

	cur=fromp;
	while (1) {
		ch=*cur++;
		if (ch >= 'a') ch-=32;
		if (ch >= '0' && ch <= '9') lval=(lval<<4)+ch-'0';
		else if (ch >= 'A' && ch <= 'F') lval=(lval<<4)+ch-55;
		else break;
		}
	if (ch != 'H' && ch != 'h') {
		cur=fromp;
		lval=0;
		while ((ch=*cur++) == '0' || ch == '1')
			lval=(lval<<1)+ch-'0';
		if (ch != 'b' && ch != 'B') {
			cur=fromp;
			lval=0;
			while ((ch=*cur++) >= '0' && ch <= '7')
				lval=(lval<<3)+ch-'0';
			if (ch != 'o' && ch != 'O' && ch != 'q' && ch != 'Q') {
				cur=fromp;
				lval=0;
				while ((ch=*cur++) >= '0' && ch <= '9')
					lval=lval*10+ch-'0';
				cur--;
				}
			}
		}
	hiword=ival->hi;
	wvalue=lval;
	}


/*	routines for console output	*/


error(str)
	char *str; {
	if (errline == line) return;
	errline=line;
	onum(line);
	oc(' ');
	bptr=last;
	do
		oc(*bptr);
	while (*bptr++ != LF);
	if (incfile) {
		os("file: ");
		os(incname);
		os("    ");
		}
	os(" error:");
	os(str);
	ocrlf();
	nerr++;
	}

oc(ch)
	char ch; {
	putchar(ch);
	}

os(str)
	char *str; {
	while (*str) oc(*str++);
	}

onum(num)
	int  num; {
	if (num < 0) {
		oc('-');
		num=-num;
		}
	if (num >= 10) onum(num/10);
	oc(num % 10 +'0');
	}

ounum(num)
	int  num; {
	if (num >= 10) ounum(num/10);
	oc(num % 10 +'0');
	}

obnum(bnum)
	char bnum; {
	int  num;
	onum(num=bnum);
	oc(' ');
	}

ohb(bnum)
	int  bnum; {
	char temp;

	temp=bnum;
	temp>>=4;
	oc(temp >= 10 ? temp+'7' : temp+'0');
	temp=bnum&15;
	oc(temp >= 10 ? temp+'7' : temp+'0');
	}

ohw(num)
	int  num; {
	ohb(num>>8);
	ohb(num);
	oc(' ');
	}

ocrlf() {
	oc(CR);
	oc(LF);
	}


/*	routines for object module output	*/

flush(eoseg)
	char eoseg; {
	int  lastoff,want,dif,saved;
	char mlab,slab,jtype,*epipe,*stoppipe;

	if (nerr > 0) inpipe=pipe;
	if (inpipe == pipe) return;

	epipe=inpipe;
	inpipe=pipe;
	offsett=0;

/*	remember how many labels left from last time */
	slab=nextlab;

/*	accumulate labels and worst case offsets	*/

	while (inpipe < epipe) {
		switch(*inpipe++) {
			case ONAMEREL:
			case OSEGPTR:
			case OPTR:
			case OLNAMEREL:	inpipe+=2;
							break;
			case OLOCAL:	if (nextlab < 160) {
								labels[nextlab]=inpipe->word;
								laboff[nextlab++]=offsett;
								}
							inpipe+=2;
							break;
			case OJUMP:		offsett+=5;
							inpipe+=3;
							break;
			case OEVEN:		offsett++;
			case OLIST:
			case OBIG:		break;
			case OLINE:		inpipe+=2;
							break;
			default:		i=*(inpipe-1)-128;
							offsett+=i;
							inpipe+=i;
			}
		}

/*	second pass over pipe data. flag long jumps and calculate relative
	offsets. a few jump downs are incorrectly flagged long. a fourth pass
	would solve this problem	*/

	inpipe=pipe;
	mlab=slab;
	saved=offsett=0;

	while (inpipe < epipe) {
		switch(*inpipe++) {
			case ONAMEREL:
			case OSEGPTR:
			case OPTR:
			case OLNAMEREL:	inpipe+=2;
							break;
			case OLOCAL:	if (mlab < 160) {
								laboff[mlab++]=offsett;
								}
							inpipe+=2;
							break;
			case OJUMP:		jtype=*inpipe;
							want=(inpipe+1)->word;
							if (jtype <= 20) {
								for (i=0; i < nextlab; i++) {
									if (want == labels[i]) {
										if (laboff[i] <= offsett)
											dif=offsett-laboff[i]+3;
										else dif=laboff[i]-offsett-saved;
										if (dif <= 129) {
											inpipe+=3;
											offsett+=2;
											saved+=3;
											goto endpipe2;
											}
										else break;
										}
									}
								}
							*inpipe|=0x80;	/* flag as a long jump */
							if ((jtype&0x7f) < 20 ) offsett+=5;
							else {
								offsett+=3;
								saved+=2;
								}
							inpipe+=3;
							break;
			case OEVEN:		if (offsett & 1) offsett++;
							else saved++;
			case OLIST:
			case OBIG:		break;
			case OLINE:		inpipe+=2;
							break;
			default:		i=*(inpipe-1)-128;
							offsett+=i;
							inpipe+=i;
endpipe2:;	}
		}

/*	write pipe bytes to dummy file	*/

	inpipe=pipe;
	stoppipe= eoseg ? epipe: epipe-256;
	offsett=0;
	mlab=slab;

	while (inpipe < stoppipe) {
		switch (*inpipe++) {
			case ONAMEREL:
			case OPTR:
			case OSEGPTR:
			case OLNAMEREL:
							dummyb(*(inpipe-1));
							dummyw(inpipe->word);
							dummyat=0;
							inpipe+=2;
							break;
			case OLOCAL:	if (laboff[mlab++] != offsett)
							  ferror("internal error in jump optimization");
							objb(OLOCAL);
							objw(inpipe->word);
							inpipe+=2;
							objw(offsett+offs[curseg-ODSEG]);
							break;
			case OJUMP:		jtype=*inpipe++;
							want=inpipe->word;
							inpipe+=2;
							if ((jtype & 0x80) == 0) {
								ldummyb(shortj[jtype]);
								i=-1;
								while (labels[++i] != want) ;
								offsett+=2;
								ldummyb(laboff[i]-offsett);
								}
							else {
								jtype&=0x7f;
								if (jtype == 18 || jtype == 19)
									ferror("JCXZ or LOOP can only jump 128 bytes");
								if (jtype < 20) {
									if (jtype < 18) jtype^=1;
									ldummyb(shortj[jtype]);
									ldummyb(3);
									offsett+=2;
									}
								if (jtype <= 20) ldummyb(0xe9);
								else ldummyb(0xe8);
								offsett+=3;
								ldummyb(0);
								ldummyb(0);
								dummyat=0;
								dummyb(OJUMPREL);
								dummyw(want);
								}
							break;
			case OEVEN:		if (offsett & 1) {
								ldummyb(0);
								offsett++;
								}
							break;
			case OLIST:		listb(listnum);
							if (listnum > 8) listnum=8;
							for (i=0; i < listnum; i++)
								listb(listbuf[i]);
							listnum=0;
							break;

			case OLINE:		dummyb(OLINE);
							dummyw(inpipe->word);
							inpipe+=2;
							dummyat=0;
							break;

			case OBIG:		listb(255);
							break;
			default:		i=*(inpipe-1)-128;
							offsett+=i;
							while (i--) {
								ldummyb(*inpipe++);
								}
			}
		}

	/*	save labels that may be needed by next flush	*/

	mlab=nextlab;
	nextlab=0;
	if (eoseg == 0) {
		lastoff=offsett-128;
		for (i=0; i < mlab; i++) {
			if (laboff[i] > lastoff) {
				if (laboff[i] > offsett) break;
				labels[nextlab]=labels[i];
				laboff[nextlab++]=laboff[i]-offsett;
				}
			}
		}

	if (offs[curseg-ODSEG]+offsett < offs[curseg-ODSEG])
		error("segment over 64K");
	else offs[curseg-ODSEG]+=offsett;

/*		move left over stuff to top of pipe	*/

	stoppipe=pipe;
	while (inpipe < epipe)
		*stoppipe++=*inpipe++;
	inpipe=stoppipe;
	codeat=0;
	dummyat=0;
	}


/*	routines to add to the pipe	*/

oconst(vtype)
	int  vtype[]; {

	if (vtype[VT] == CCHAR && vtype[VIS] != OFFV) {
		if (vtype[VIS] != CONSTV) error("invalid BYTE constant");
		codeb(vtype[VVAL]);
		}
	else {
		switch (vtype[VIS]) {
			case CONSTV:	codew(vtype[VVAL]);
							break;
			case OFFV:		if (vtype[VVAL] != 8 || vtype[VNAME] == 0)
								error("invalid OFFSET");
							codew(vtype[VOFF]);
							fixup((vtype[VT] == CSEGMENT)? OSEGPTR : ONAMEREL,
								vtype[VNAME]);
							break;
			default:		error("invalid constant");
			}
		}
	}

orm(reg,vtype)
	char reg;
	int  vtype[]; {
	char mod;

	switch (vtype[VIS]) {
		case REGV:	codeb(192+(reg<<3)+vtype[VVAL]);
					break;
		case VARV:	if (vtype[VT] == CLABEL) error("missing type");
					if (vtype[VSEG] >= 0) whatseg(vtype);
					if (vtype[VVAL] == 8) {
						codeb(6+(reg<<3));
						codew(vtype[VOFF]);
						fixup((vtype[VT] == CFOREIGN) ? OPTR : ONAMEREL,
							vtype[VNAME]);
						}
					else {
						mod=0;
						if (vtype[VVAL] == 6 || vtype[VOFF] != 0) mod=64;
						if (vtype[VNAME] ||vtype[VOFF]<-128||vtype[VOFF]>127)
							mod=128;
						codeb(mod+(reg<<3)+vtype[VVAL]);
						if (mod == 64) codeb(vtype[VOFF]);
						else if (mod == 128) {
							codew(vtype[VOFF]);
							fixup((vtype[VT] == CFOREIGN) ? OPTR : ONAMEREL,
								vtype[VNAME]);
							}
						}
					break;
		default:	error("invalid R/M");
		}
	}

whatseg(vtype)
	int  vtype[]; {
	char inst;

	if (vtype[VVAL] == 2 || vtype[VVAL] == 3 || vtype[VVAL] == 6){
		if (vtype[VSEG] == SS) return;
		}
	else if (vtype[VSEG] == DS) return;
	inpipe--;
	inst=*inpipe;
	*inpipe++=0x26+(vtype[VSEG]<<3);
	codeb(inst);
	}

fixup(type, num)
	int  type, num; {

	if (num) {
		codeat=0;
		*inpipe++=type;
		inpipe->word=num;
		inpipe+=2;
		}
	}

label(num)
	int  num; {

	codeat=0;
	*inpipe++=OLOCAL;
	inpipe->word=num;
	inpipe+=2;
	}

jump(jnum,num)
	char jnum;
	int  num; {

	codeat=0;
	*inpipe++=OJUMP;
	*inpipe++=jnum;
	inpipe->word=num;
	inpipe+=2;
	}

evenb() {

	codeat=0;
	*inpipe++=OEVEN;
	}

listline() {

	codeat=0;
	*inpipe++=OLIST;
	}

isinclude() {

	codeat=0;
	*inpipe++=OBIG;
	}

linenum(num)
	int  num; {

	codeat=0;
	*inpipe++=OLINE;
	inpipe->word=num;
	inpipe+=2;
	}

codeb(byt)
	char byt; {

	if (codeat == 0) {
		codeat=inpipe;
		*inpipe++=128;
		}
	(*codeat)++;
	*inpipe++=byt;
	}

union codebu{char bytes[2];};

revcodew(wrd)
	unsigned wrd; {
	union codebu * codeu = &wrd;

	codeb(codeu->bytes[1]);
	codeb(wrd);
	}

codew(wrd)
	int  wrd; {

	if (codeat == 0 || *codeat > 200) {
		codeat=inpipe;
		*inpipe++=128;
		}
	(*codeat)+=2;
	inpipe->word=wrd;
	inpipe+=2;
	}


/*	routine to add to list file	*/

listb(byt)
	char byt; {
	if (inlst == &lstbuf[512]) {
		if (nerr == 0 && write(lst,lstbuf,512) == -1) {
			os("cannot write ");
			ferror(lstname);
			}
		inlst=&lstbuf;
		}
	*inlst++=byt;
	}

/*	routines to add to object file	*/

objb(byt)
	char byt; {
	if (inobj == &objbuf[512]) {
		if(nerr == 0 && write(obj,objbuf,512) == -1) {
			os("cannot write ");
			ferror(objname);
			}
		inobj=&objbuf;
		}
	*inobj++=byt;
	}

objw(wrd)
	int  wrd; {
	objb(wrd);
	objb(wrd>>8);
	}

objs(str)
	char *str; {
	do
		objb(*str);
	while (*str++);
	}


/*	routines to add to dummy file	*/

/*	write a dummy byte and add to list if needed	*/

ldummyb(byt)
	char byt; {

	if (lopt) {
		if (listnum < 8)
			listbuf[listnum]=byt;
		listnum++;
		}
	if (dummyat == 0 || *dummyat > 188) {
		dummyb(128);
		dummyat=indummy-1;
		}
	(*dummyat)++;
	dummyb(byt);
	}


dummyb(byt)
	char byt; {
	if (indummy == &dbuf[2048]) {
		if (nerr == 0) {
			if (ndummy == 0) {
				if ((dummy=creat(tempname)) == -1)
					ferror("cannot create CTEMP3");
				}
			if(write(dummy,dbuf,2048) == -1) {
				ferror("cannot write CTEMP3");
				}
			}
		ndummy++;
		indummy=&dbuf;
		dummyat=0;
		}
	*indummy++=byt;
	}

dummyw(wrd)
	int  wrd; {
	dummyb(wrd);
	dummyb(wrd>>8);
	}

