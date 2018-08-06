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
/*	c7.c					part 7 of medium c compiler		*/

#include "PASS1.H"
#include "NODES.H"
#if CHECK
#include "OBJ.H"
#endif

e_msg(reale,str)
	int reale;
	char *str; {
	onum(cline);
	oc(' ');
	if (see_exit == 0 && reale >= 0) {
		bptr=lineBeg;
		do {
			if (bptr == tokat) os(" $$ ");
			if (*bptr == 9) os("    ");
			else if (*bptr != CR && *bptr != CONTZ) oc(*bptr);
			}
		while ( *bptr != CONTZ && *bptr++ != LF);
		os("      ");
		}
	if (incnext) {
		os("file: ");
		os(name);
		os("    ");
		}
	if (reale > 0) {
		os(" error:");
		nerr++;
		}
	else if (reale == 0)
		os(" warning:");
	else
		os("#error:");
	os(str);
	if (see_exit) real_exit(2);
	ocrlf();
	}

error(str)
	int  str; {

	if (tokat) e_msg(1,str);
	if (curch == ';' || curch == CONTZ) {
		tokat=0;
		return;
		}
	if (tokat == 0) {
		tokit();
		return;
		}
	while (1) {
		while (*cur != LF && *cur != ';' && *cur != CONTZ && *cur != '{'
			&& *cur != '}') cur++;
		if (*cur == LF) dolf(1);
		else {
			tokit();
			tokat=0;
			return;
			}
		}
	}

nooper() {

	error("missing operand");
	}

error_l(str)
	int  str; {
	e_msg(1,str);
	skipl();
	}

warning(str)
	int  str; {

	e_msg(0,str);
	nwarn++;
	}

typeof(ptr)
	char *ptr; {
	int  val;
	if (*ptr == PTRTO && is_big == 0) val=CUNSG;
	else val=*ptr;
	return val<<8;
	}

unsigned dsize(ptr)
	char *ptr; {
	int  tot;
	if (*ptr == ARRAY) {
		wp=ptr+1;
		tot=*wp;
		if (tot == -1) tot=0;
		return (tot*dsize(ptr+3));
		}
	if (*ptr == CCHAR || *ptr == CSCHAR) tot=1;
	else if (*ptr == CLONG || *ptr == CFLOAT) tot=4;
	else if (*ptr == CDOUBLE) tot=8;
	else if (*ptr == CSTRUCT) {
		wp=ptr+1;
		wp=*wp;
		tot=((void *)wp)->staglen;
		if (tot == 0) error("undefined structure");
		}
	else if (is_big && *ptr == PTRTO) tot=4;
	else if (*ptr == FUNCTION) return dsize(ptr+1);
	else tot=2;
	return tot;
	}

/*	stuff for SEE interface.	*/

static unsigned see_off,see_seg,see_ret,see_bp;
static char see_msg[80],see_index;

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
			see_call(-1,see_msg,incnext ? lastline : cline);
			}
		}
	exit(cc);
	}


scaled(ptr)
	char *ptr; {
	int  val;
	if (*ptr == ARRAY) val=dsize(ptr+3);
	else if (*ptr == PTRTO) val=dsize(ptr+1);
	else val=1;
	return val;
	}

#if CHECK

dumpsym() {
	char ch,*lastt;
	int  num;

	bptr=_memory();
	while (bptr < mfree) {
		ohw(bptr);
		oc(' ');
		ohw(bptr->word);
		oc(' ');
		bptr+=2;
		while (*bptr)
			oc(*bptr++ & 0x7f);
		oc(' ');
		lastt=++bptr;
		switch (*bptr) {
			case RESERVED:	os("RESERVED ");
							obnum(*++bptr);
							bptr++;
							break;
			case STREF:
			case STAG:		os("STAG ");
							ohw(bptr->schain);
							oc(' ');
							onum(bptr->staglen);
							bptr+=5;
							break;
			case DEFINED:	os("DEFINED ");
							bptr+=2;
							while (*bptr != DEFEND) {
								if (*bptr != DEFSTR) {
									oc(' ');
									onum(*bptr++);
									oc(' ');
									}
								else {
									bptr++;
									do
										if (*bptr >= ' ') oc(*bptr);
									while (*bptr++ != LF);
									}
								}
							bptr++;
							break;
			case OPERAND:	os("OPERAND ");
							ohw(bptr->nchain);
							oc(' ');
							onum(bptr->nlen);
							oc(' ');
							onum(bptr->nstor);
							oc(' ');
							onum(bptr->noff);
							bptr+=7;
							listtype();
							break;
			default:		os("Mystery Symbol ");
							ohw(bptr->word);
			}
		ocrlf();
		if (bptr->word == 0xffff) bptr=lastt+18;
		}

	os("Normal End");
	ocrlf();
	}

listtype() {
	char tp;

	do {
		oc(' ');
		tp=*bptr++;
		switch (tp) {
			case CCHAR:		os("CCHAR");
							break;
			case CSCHAR:	os("CSCHAR");
							break;
			case CINT:		os("CINT");
							break;
			case CUNSG:		os("CUNSG");
							break;
			case CLONG:		os("CLONG");
							break;
			case CFLOAT:	os("CFLOAT");
							break;
			case CDOUBLE:	os("CDOUBLE");
							break;
			case CLABEL:	os("CLABEL");
							break;
			case CSTRUCT:	os("CSTRUCT ");
							ohw(bptr->word);
							bptr+=2;
							break;
			case FUNCTION:	os("FUNCTION");
							break;
			case ARRAY:		os("ARRAY ");
							onum(bptr->word);
							bptr+=2;
							break;
			case PTRTO:		os("PTRTO");
							break;
			default:		os("BITS ");
							onum(tp & 15);
							oc(' ');
							onum((tp >> 4)+1);
							break;
			}
		}
	while (tp >= FUNCTION);
	}
#endif

oc(ch)
	char ch; {

	if (see_exit) {
		if (see_index < 78 && ch != '\n') see_msg[see_index++]=ch;
		}
	else putchar(ch);
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

obnum(bnum)
	char bnum; {
	int  num;
	onum(num=bnum);
	}

ohb(bnum)
	int  bnum; {
	char temp;
	temp=bnum;
	temp>>=4;
	oc(temp >= 10 ? temp+'7' : temp+'0');
	bnum&=15;
	oc(bnum >= 10 ? bnum+'7' : bnum+'0');
	}

ohw(num)
	int  num; {
	ohb(num>>8);
	ohb(num);
	}

ocrlf() {
	oc(LF);
	}

ctlb(byt)
	char byt; {
	if(!soopt) {
		if (inctl == &ctlbuf[512]) {
			if (write(control,ctlbuf,512) == -1) {
				os("cannot write temporary");
				real_exit(2);
				}

			inctl=&ctlbuf;
			}
		*inctl++=byt;
		}
	}

ctlw(wrd)
	int  wrd; {
	ctlb(wrd);
	ctlb(wrd>>8);
	}

ctls(str)
	char *str; {
	do
		ctlb(*str);
	while (*str++);
	}

treew(wrd)
	int  wrd; {
	if(!soopt) {
		if (intree == &treebuf[512]) {
			if (write(tree,treebuf,512) == -1) {
				os("cannot write temporary");
				real_exit(2);
				}
			intree=treebuf;
			}
		*intree++=wrd;
		ntree++;
		}
	}

tree1(branch)
	int  *branch; {
	int  at;
	at=ntree;
	treew(*branch);
	return at;
	}

tree2(branch)
	int  branch[]; {
	int  at;
	at=ntree;
	treew(branch[0]);
	treew(branch[1]);
	return at;
	}

tree3(branch)
	int  branch[]; {
	int  at;
	at=ntree;
	treew(branch[0]);
	treew(branch[1]);
	treew(branch[2]);
	return at;
	}

tree4(branch)
	int  branch[]; {
	int  at;
	at=ntree;
	treew(branch[0]);
	treew(branch[1]);
	treew(branch[2]);
	treew(branch[3]);
	return at;
	}

tree5(branch)
	int  branch[]; {
	int  at;
	at=ntree;
	treew(branch[0]);
	treew(branch[1]);
	treew(branch[2]);
	treew(branch[3]);
	treew(branch[4]);
	return at;
	}

treev(num,branch)
	int  branch[],num; {
	int  at;
	char i;
	at=ntree;
	for (i=0; i < num; i++)
		treew(branch[i]);
	return at;
	}

tree3z() {
	treew(0);
	treew(0);
	treew(0);
	}

/*	debug stuff to the object file	*/

#if CHECK

objb(byt)
	char byt; {
	if(!soopt) {
		if (inobj == &objbuf[512]) {
			if (write(objnum,objbuf,512) == -1) {
				os("cannot write temporary");
				real_exit(2);
				}
			inobj=objbuf;
			}
		*inobj++=byt;
		}
	}

objw(wrd)
	int  wrd; {
	objb(wrd);
	objb(wrd>>8);
	}

objs(str)
	char *str; {
	do
		objb(toupper((*str) & 0x7f));
	while (*str++);
	}


static char last_seg=ODSEG;

objtype(otype,num)
	char otype;
	int  num; {
	int  len;
	char *bp,shorter,ttyp,*sptr,*bptr;

	/*	must say if in data or code for statics	*/
	if (addat->ntype[0] == FUNCTION) {
		if (last_seg != OCSEG) objb(last_seg=OCSEG);
		}
	else if (last_seg != ODSEG) objb(last_seg=ODSEG);
	objb(otype);
	bp=addat-addat->nlen;
	if (otype != OPTYPE) objs(bp);
	else {
		if (addat->nstor == SSTATIC && funname) {
			bptr=funname;
			while (*bptr) objb(*bptr++);
			objb('_');
			}
		while (*bp) objb(toupper(*bp++));
		objb('_');
		objb(0);
		}

	objw(num);
/*	get the length of the type info and output first	*/

	len=1;
	bp=addat->ntype;
	shorter=0;
	while (*bp >= FUNCTION) {
		if (len >= 9) {
			shorter=1;
			break;
			}
		if (*bp == ARRAY) {
			bp+=2;
			len+=2;
			}
		bp++;
		len++;
		}
	if (*bp == CSTRUCT) {
		len+=2;
		}
	objb(len+shorter);
	bp=addat->ntype;
	while (len--) {
		objb(ttyp=*bp++);
		if (ttyp == ARRAY) {
			objb(*bp++);
			objb(*bp++);
			len-=2;
			}
		if (ttyp == CSTRUCT) {
			sptr=bp->word;
			bp+=2;
			objw(sptr->staglen);
			len-=2;
			}
		}
	if (shorter) objb(CINT);
	}

#endif
