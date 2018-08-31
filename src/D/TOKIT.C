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
 *      Changed to conform to new compiler.
 */
/*	TOKIT.C  --	Expression analyzer for D88 debugger	*/

#include "debug.h"



expression(vt)			/*  input an expression. assignment is first */
	struct vstruct *vt; {
	struct vstruct valr,tempv;
	char op;

	memat=0;
	getop();
	heir22c(vt);
	if (ifop("=")) {
		expression(&valr);
		asgnvt(vt,&valr);
		}
	}

	


heir22c(vt)				/*	operator is + and -	*/
	struct vstruct *vt; {
	struct vstruct valr;
	char op;

	heir23c(vt);
	while (1) {
		if (ifop("+")) op=0;
		else if (ifop("-")) op=1;
		else return;
		heir23c(&valr);
		addvt(vt,&valr,op);
		}
	}

addvt(vt,valr,op)
	struct vstruct *vt,*valr;
	char op; {
	int sizel,sizer;
	char *typeat;
	struct vstruct convt;

	sizel=sizer=1;
	typeat=0;
	if (*vt.typep == PTRTO) {
		sizel=longof(vt->typep+1);
		typeat=vt.typep;
		}
	if (*valr.typep == PTRTO) {
		sizer=longof(valr->typep+1);
		typeat=valr.typep;
		}
	if (*vt.typep == ARRAY) {
		sizel=longof(vt->typep+3);
		typeat=vt.typep;
		}
	if (*valr.typep == ARRAY) {
		sizer=longof(valr->typep+3);
		typeat=valr.typep;
		}
	forceup(vt,valr);
	convt.vtype=CONSTV;
	convt.typep=i_type;
	if (sizel == 1 && sizer > 1) {
		convt.val.vali=sizer;
		mulvt(vt,&convt,0);
		}
	if (sizer == 1 && sizel > 1) {
		convt.val.vali=sizel;
		mulvt(valr,&convt,0);
		}


	switch (*vt.typep) {
		case CLONG:	if (op) vt.val.vall-=valr.val.vall;
					else vt.val.vall+=valr.val.vall;
					break;
		case CDOUBLE:if (op) vt.val.vald-=valr.val.vald;
					else vt.val.vald+=valr.val.vald;
					break;
		default:	if (op) vt.val.vali-=valr.val.vali;
					else vt.val.vali+=valr.val.vali;
		}
	if (op == 1 && sizel > 1 && sizel == sizer) {
		convt.val.vali=sizer;
		mulvt(vt,&convt,1);
		}
	if (typeat) {
		vt.typep=typeat;
		}
	}




heir23c(vt)				/*	operator is * and /	 and % */
	struct vstruct *vt; {
	struct vstruct valr;
	char op;

	heir24c(vt);
	while (1) {
		if (ifop("*")) op=0;
		else if (ifop("/")) op=1;
		else if (ifop("%")) op=2;
		else return;
		heir24c(&valr);
		mulvt(vt,&valr,op);
		}
	}


mulvt(vt,valr,op)
	struct vstruct *vt,*valr;
	char op; {

	forceup(vt,valr);
	switch (*vt.typep) {
		case CUNSG:	if (op== 0) vt.val.valu*=valr.val.valu;
					else if (op == 1) vt.val.valu/=valr.val.valu;
					else vt.val.valu%=valr.val.valu;
					break;
		case CLONG:	if (op== 0) vt.val.vall*=valr.val.vall;
					else if (op == 1) vt.val.vall/=valr.val.vall;
					else vt.val.vall%=valr.val.vall;
					break;
		case CDOUBLE:if (op== 0) vt.val.vald*=valr.val.vald;
					else vt.val.vald/=valr.val.vald;
					break;
		default:	if (op== 0) vt.val.vali*=valr.val.vali;
					else if (op == 1) vt.val.vali/=valr.val.vali;
					else vt.val.vali%=valr.val.vali;
					break;
		}
	}

struct {unsigned word;};

heir24c(vt)				/* operator is prefix * & - */
	struct vstruct *vt; {
	struct vstruct valr,convt,vall;
	char op,*typeat;

	if (ifop("*")) {
		getop();
		heir24c(vt);
		typeat=vt.typep;
		toptr(vt);
		if (*typeat == PTRTO) vt.typep=typeat+1;
		if (*typeat == ARRAY) vt.typep=typeat+3;
		vt.vtype=RMV;
		return;
		}
		
	if (ifop("&")) {
		getop();
		heir24c(vt);
		if (vt.vtype == RMV) {
			vt.vtype=CONSTV;
			vt.typep=p_type;
			}
		else error("illegal address");
		return;
		}

	if (ifop("-")) {
		getop();
		heir24c(vt);
		toconst(vt);
		switch(*vt.typep) {
			case CUNSG:	vt.val.valu=-vt.val.valu;
						break;
			case CLONG:	vt.val.vall=-vt.val.vall;
						break;
			case CDOUBLE:vt.val.vald=-vt.val.vald;
						break;
			default:	vt.val.vali=-vt.val.vali;
			}
		return;
		}

	heir25c(vt);
	}

			

	

heir25c(vt)				/*	must have CONSTV or left parenthesis	*/
	struct vstruct *vt; {
	struct vstruct valr;
	int size,i;
	char *ptr,rettype,member[32],opch,*argat;
	unsigned oldsp,oldip,oldcs;

	if (ifop("(")) {
		expression(vt);
		if (ifop(")") == 0) error("missing )");
		getop();
		return;
		}
	if (operator[0]) error("illegal operator");
	tokit(vt);
	if (vt.vtype == OPPV) error("illegal expression");

/*	search for an array or member */

	opch=operator[0];
	while (ifop("[") || ifop(".") || ifop("->")) {
		if (opch == '[') {	/*	have an array index	*/
			expression(&valr);
			if (ifop("]") == 0) error("missing ]");
			addvt(vt,&valr,0);
			if (*vt.typep == PTRTO) vt.typep++;
			else if (*vt.typep == ARRAY) vt.typep+=3;
			if (*vt.typep != ARRAY) vt.vtype=RMV;
			getop();
			opch=operator[0];
			continue;
			}

		if (opch == '-') toconst(vt);
		inname(member);
		lastsym=symbols;
		while (*lastsym) {
			if (*lastsym == OMTYPE && strcmp(member,lastsym+2) == 0) {
				memat=ptr=lastsym;
				while (*ptr++) ;
				vt.val.valu+=ptr->loc;
				vt.typep=&ptr->type[1];
				vt.vtype=RMV;
				goto gotm;
				}
			lastsym+=*(lastsym+1);
			}
		error("invalid symbol");
gotm:	getop();
		opch=operator[0];
		}

/*	look for a procedure call  */

	if (ifop("(")) {
		got_call=1;
		oldsp=rsp;
		rsp=argat=rsp-80;
		skipbl();
		if (*nextin == ')') {
			getop();
			}
		else do {
			expression(&valr);
			if (valr.vtype == OPPV) break;
			toconst(&valr);
			switch(*valr.typep) {
				case PTRTO:		if (is_big) {
									_lmove(4,&valr.val.vall,localss,argat,rss);
									argat+=4;
									break;
									}
				default:		_lmove(2,&valr.val.vali,localss,argat,rss);
								argat+=2;
								break;
				case CLONG:		_lmove(4,&valr.val.vall,localss,argat,rss);
								argat+=4;
								break;
				case CDOUBLE:	_lmove(8,&valr.val.vald,localss,argat,rss);
								argat+=8;
								break;
				}
			}
		while (ifop(","));
		if (ifop(")") == 0) error("missing )");
		getop();
		if (is_big) {
			rsp-=2;
			_lmove(2,&int3seg,localss,rsp,rss);
			}
		rsp-=2;
		_lmove(2,&int3off,localss,rsp,rss);
		vt.vtype=CONSTV;
		if (*vt.typep == FUNCTION) vt.typep++;
		else vt.typep=i_type;
		oldip=rip;
		oldcs=rcs;
		rip=vt.val.valu;
		switch (*vt.typep) {
			case PTRTO:		xgoto();
							if (is_big) {
								vt.val.vali=rsi;
								*(&vt.val.vali+1)=res;
								break;
								}
			default:		xgoto();
							vt.val.vali=rax;
							break;
			case CLONG:		xgoto();
							vt.val.vali=rax;
							*(&vt.val.vali+1)=rdx;
							break;
			case CFLOAT:	vt.typep=d_type;
			case CDOUBLE:	xgoto();
							/* look for FLOAT_REG to get float value	*/
							lastsym=symbols;
							while (*lastsym) {
								if (*lastsym == OPTYPE &&
									strcmp("FLOAT_REG",lastsym+2) == 0) {
									ptr=lastsym+12;
									if (is_big)  {
										rcs=ptr->loc_seg;
										rsp-=2;
										_lmove(2,&int3seg,localss,rsp,rss);
										}
									rip=ptr->loc;
									rsp-=2;
									_lmove(2,&int3off,localss,rsp,rss);
									xgoto();
									vt.val.vali=rax;
									*(&vt.val.vali+1)=rbx;
									*(&vt.val.vali+2)=rcx;
									*(&vt.val.vali+3)=rdx;
									break;
									}
								lastsym+=*(lastsym+1);
								}
							break;
			}
		rsp=oldsp;
		rip=oldip;
		rcs=oldcs;
		}
	}



/*	Obtain a Vtype.	*/

tokit(vt)
	struct vstruct *vt; {

	skipbl();
	operator[0]=0;
	switch (ltype[*nextin]) {
		case LETTER:find(vt);
					break;
		case DIGIT: number(vt);
					break;
		default:	if (*nextin == '.' && ltype[*(nextin+1)] == DIGIT)
						number(vt);
					else if (*nextin == '"') isstring(vt);
					else if (*nextin == '\'') charac(vt);
					else vt.vtype=OPPV;
		}
	getop();
	}


/*	GETOP  -- set operator if any operator present */

getop() {
	char ch,i;
	i=0;
	skipbl();
	while (i < 5 && ltype[ch=*nextin] == OTHER && ch != ';' && ch != '"'
		&& ch != '\'') {
		operator[i++]=*nextin++;
		if (ch == '(' || ch == ')' || ch == ',' || ch == ']' || ch == '[') break;
		ch=*nextin;
		if (ch == '(' || ch == ')' || ch == ',' || ch == ']' || ch == '['
			|| ch == '*') break;
		}
	operator[i]=0;
	skipbl();
	}

long inhex();

number(vt)
	struct vstruct *vt; {
	char ch,*from;
	long dvalue;

	vt.vtype=CONSTV;
	dvalue=0;
	from=nextin;
	ch=*(nextin+1);
	if (*nextin == '0' && ch != 'l' && ch != 'L' && ch != '.') {
		dvalue=inhex();
		}
	else {
		while ((ch=*nextin-'0') <= 9) {
			dvalue=dvalue*10+ch;
			nextin++;
			}
		if (*nextin == '.' || *nextin == 'e' || *nextin == 'E') {
			nextin=from+_finput(from,&vt.val.vald,20);
			vt.typep=d_type;
			return;
			}
		}
	vt.typep=i_type;
	if (*nextin == 'l' || *nextin == 'L') {
		vt.typep=l_type;
		nextin++;
		}
	else if (dvalue > 32767l || dvalue < -32768l) vt.typep=l_type;
	vt.val.vall=dvalue;
	}


/*	INHEX  -  input a hex number.	*/

long inhex() {
	char ch;
	long dvalue;

	nextin++;
	if (*nextin == 'X' || *nextin == 'x') nextin++;
	dvalue=0;
	while (1) {
		ch=*nextin;
		if (ch >='a') ch-=32;
		if (ch >='A' && ch <= 'F') ch-='7';
		else if (ltype[ch] == DIGIT) ch-='0';
		else break;
		dvalue=(dvalue<<4)+ch;
		nextin++;
		}
	return dvalue;
	}





isstring(vt)
	struct vstruct *vt; {
	int  thiss;

	thiss=nexts;
	vt.vtype=CONSTV;
	nextin++;
	while (*nextin != '"') {
		if (*nextin == '\n' || *nextin == 26) {
			error("missing \"");
			return;
			}
		_poke(getach(),nexts++,origss);
		}
	nextin++;
	_poke(0,nexts++,origss);
	vt.val.valu=thiss;
	if (is_big) *(((unsigned *)(&vt.val.valu))+1)=origss;
	vt.typep=p_type;
	}

charac(vt)
	struct vstruct *vt; {
	int wvalue;
	vt.vtype=CONSTV;
	vt.typep=i_type;
	nextin++;
	if (*nextin == '\'') {
		wvalue=0;
		}
	else {
		wvalue=getach();
		if (*nextin != '\'') {
			wvalue<<=8;
			wvalue+=getach();
			}
		}
	if (*nextin++ != '\'') error("missing '");
	vt.val.vali=wvalue;
	}

getach() {
	char ch,och;
	ch=*nextin++;
	if (ch == '\\') {
		och=ch=*nextin++;
		if (ch >= 'a') ch-=32;
		switch (ch) {
			case 'N':	ch='\n';
						break;
			case 'T':	ch=9;
						break;
			case 'B':	ch=8;
						break;
			case 'R':	ch=13;
						break;
			case 'F':	ch=12;
						break;
			default:	if (ch >= '0' && ch <= '7') {
							ch-='0';
							while (*nextin >= '0' && *nextin <= '7')
								ch=(ch<<3)+*nextin++-'0';
							}
						else ch=och;
			}
		}
	return ch;
	}



/*	FIND  --	find a variable.	*/

find(vt)
	struct vstruct *vt; {
	char name[32],*ptr;
	int  i;
	unsigned regi,*wp;
	inname(name);

	vt.vtype=RMV;

/*	search locals first	*/

	i=0;
	while (localat[i]) {
		if (match(name,localat[i])) {
			ptr=localat[i];
			while (*ptr++) ;
			vt.val.valu=ptr->loc+rbp;
			vt.typep=&ptr->type[1];
			if (is_big) {
				wp=&vt.val.valu;
				*(wp+1)=origss;
				}
			return;
			}
		i++;
		}

/*	search for a public	*/

	ovnum=0;
	lastsym=symbols;
	while (*lastsym) {
		if (*lastsym == OPTYPE && strcmp(name,lastsym+2) == 0 && 
			(ovnum == 0 || ovnum == overley)) {
			ptr=lastsym;
			while (*ptr++) ;
			vt.val.valu=ptr->loc;
			vt.typep=&ptr->type[1];
			if (is_big) {
				wp=&vt.val.valu;
				*(wp+1)=ptr->loc_seg;
				vt.typep=&ptr->big_type[1];
				}
			return;
			}
		if (*lastsym == OOV) ovnum=*(lastsym+2);
		lastsym+=*(lastsym+1);
		}

/*	look for a register	*/

	for (i=0; i < NUMREG; i++) {
		if (strcmp(name,rname[i]) == 0) {
			vt.vtype=REGV;
			vt.val.valu=i;
			vt.typep=i_type;
			return;
			}
		}
	error("invalid symbol");
	}



/*	ISOTHER  --	assume found an operator  */

		

/*	VTIS  --	return true if vt is not zero.	*/

vtis(vt)
	struct vstruct *vt; {

	toint(vt);
	return vt.val.vali ? 1 :0;
	}



/*	ASGNVT  --	set a vt to a vt	*/

asgnvt(vleft,vright)
	struct vstruct *vleft,*vright; {
	union {char byte; unsigned word; long dword; float fword; double dbword;}x;
	unsigned at,atseg,*wp;
	char tpl,isdbl;

	toconst(vright);
	tpl=*vright.typep;
	if (tpl == CINT) vright.val.vall=vright.val.vali;
	if (tpl == CUNSG) vright.val.vall=vright.val.valu;
	if (vleft.vtype == REGV) {
		toint(vright);
		(&rax+(struct ubywo *)vleft.val.valu)->word=vright.val.valu;
		}
	else if (vleft.vtype != RMV) error("illegal assignment");
	else {
		at=vleft.val.valu;
		atseg=rds;
		if (is_big) {
			wp=&vleft.val.valu;
			atseg=*(wp+1);
			}
		isdbl=tpl == CDOUBLE;
		switch (*vleft.typep) {
			case CCHAR:		if (isdbl) x.byte=vright.val.vald;
							else x.byte=vright.val.vali;
							_lmove(1,&x,localss,at,atseg);
							break;
			case CLONG:		if (isdbl) x.dword=vright.val.vald;
							else x.dword=vright.val.vall;
							_lmove(4,&x,localss,at,atseg);
							break;
			case CFLOAT:	if (isdbl) x.fword=vright.val.vald;
							else x.fword=vright.val.vali;
							_lmove(4,&x,localss,at,atseg);
							break;
			case CDOUBLE:	if (isdbl) x.dbword=vright.val.vald;
							else x.dbword=vright.val.vali;
							_lmove(8,&x,localss,at,atseg);
							break;
			case PTRTO:		if (is_big) {
								toptr(vright);
								_lmove(4,&vright.val.vall,localss,at,atseg);
								break;
								}
			default:		if (isdbl) x.word=vright.val.vald;
							else x.word=vright.val.vali;
							_lmove(2,&x,localss,at,atseg);
			}
		}
	}



/*	VTCMP  --	compare two vt's	*/

vtcmp(vleft,vright)
	struct vstruct *vleft,*vright; {
	int  res;

	res=0;
	forceup(vleft,vright);
	switch(*vleft.typep) {
		case CINT:	if (vleft.val.vali > vright.val.vali) res=1;
					else if (vleft.val.vali < vright.val.vali) res=-1;
					break;
		case CUNSG:	if (vleft.val.valu > vright.val.valu) res=1;
					else if (vleft.val.valu < vright.val.valu) res=-1;
					break;
		case CLONG:	if (vleft.val.vall > vright.val.vall) res=1;
					else if (vleft.val.vall < vright.val.vall) res=-1;
					break;
		default:	if (vleft.val.vald > vright.val.vald) res=1;
					else if (vleft.val.vald < vright.val.vald) res=-1;
					break;
		}
	return res;
	}



/*	FORCEUP  --	make both vt's the same type */

forceup(vleft,vright)
	struct vstruct *vleft,*vright; {

	toconst(vleft);
	toconst(vright);

	switch (*vleft.typep) {
		case CLONG:		if (*vright.typep == CDOUBLE) {
							vleft.val.vald=vleft.val.vali;
							vleft.typep=d_type;
							}
						else if (*vright.typep == CUNSG) {
							vright.val.vall=vright.val.valu;
							}
						else if (*vright.typep != CLONG) {
							vright.val.vall=vright.val.vali;
							}
						break;
		case CDOUBLE:	if (*vright.typep == CINT) {
							vright.val.vald=vright.val.vali;
							}
						else if (*vright.typep == CLONG) {
							vright.val.vald=vright.val.vall;
							}
						else if (*vright.typep != CDOUBLE) {
							vright.val.vald=vright.val.valu;
							}
						break;
		case PTRTO:		if (is_big) {
							toconst(vright);
							break;
							}
		default:		if (*vright.typep == CLONG) {
							vleft.val.vall=vleft.val.valu;
							vleft.typep=l_type;
							}
						else if (*vright.typep == CDOUBLE) {
							vleft.val.vald=vleft.val.valu;
							vleft.typep=d_type;
							}
						else if (*vright.typep != CINT || *vleft.typep != CINT) {
							vleft.typep=u_type;
							}
						break;
		}
	}

		

/*	TOINT  --	forct vt to be an integer	*/

toint(vt)
	struct vstruct *vt; {

	toconst(vt);
	if (*vt.typep == CFLOAT) {
		vt.val.vali=*(float *) vt.val.valu;
		vt.typep=i_type;
		}
	else if (*vt.typep == CDOUBLE) {
		vt.val.vali=*(double *) vt.val.valu;
		vt.typep=i_type;
		}
	}


/*	TOPTR  --	force vt to be a pointer	*/

toptr(vt)
	struct vstruct *vt; {
	unsigned *wp;

	if (is_big) {
		toconst(vt);
		if (*vt.typep != PTRTO) {
			wp=&vt.val.vali;
			*(wp+1)=origds;
			}
		}
	else toint(vt);
	}


/*	TOCONST  --	force a VT to be a CONSTV	*/

toconst(vt)
	struct vstruct *vt; {
	char xx[8],*ptr;

	if (vt.vtype == REGV) {
		vt.vtype=u_type;
		vt.val.valu=*(int *)(&rax+vt.val.vali);
		}
	else if (vt.vtype == RMV) {
		ptr=is_big ? *(&vt.val.valu+1) : rds;
		_lmove(8,vt.val.valu,ptr,xx,localss);
		ptr=xx;

		switch (*vt.typep) {
			case CCHAR:		vt.val.vali=*(char *) ptr;
							vt.typep=i_type;
							break;
			case CLONG:		vt.val.vall=*(long *) ptr;
							break;
			case CFLOAT:	vt.val.vald=*(float *) ptr;
							vt.typep=d_type;
							break;
			case CDOUBLE:	vt.val.vald=*(double *) ptr;
							break;
			case ARRAY:
			case CSTRUCT:
			case CLABEL:
			case FUNCTION:  vt.typep=p_type;
							break;
			case PTRTO:		if (is_big) {
								vt.val.vall=*(long *) ptr;
								}
							else vt.val.vali=*(int *) ptr;
							vt.typep=p_type;
							break;
			default:		vt.val.vali=*(int *) ptr;
							if (*vt.typep != CINT) vt.typep=u_type;
							break;
			}
		vt.vtype=CONSTV;
		}
	else if (vt.vtype == OPPV) error("illegal operand");
	}





	
/*	LONGOF  --  return length of thing pointed to. */

longof(ptr)
	char *ptr; {
	int  len;

	switch (*ptr) {
		case CCHAR:		len=1;
						break;
		case PTRTO:		if (is_big) {
							len=4;
							break;
							}
		default:		len=2;
						break;
		case CFLOAT:
		case CLONG:		len=4;
						break;
		case CDOUBLE:	len=8;
						break;
		case ARRAY:		len=((union ubywo *)ptr+1).word*longof(ptr+3);
						break;
		case CSTRUCT:	len=((union ubywo *)ptr+1).word;
		}
	return len;
	}



/*	IFOP  -- match an operator  */

ifop(opr)
	char *opr; {
	char *ptr;

	ptr=operator;
	do {
		if (*opr != *ptr++) return 0;
		}
	while (*opr++);
	operator[0]=0;
	return 1;
	}
