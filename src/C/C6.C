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
/*	c6.c					part 6 of medium c compiler		*/

#include "PASS1.H"
#include "NODES.H"

tokit() {
	do 
		;
		while (tokone());
	}

strngcpy(char *cp, char qs){

	whitesp();
	nestfrom[nested++]=cur;
	cur=macfrom[nested]=macxp;
	if(qs) *macxp++='"';
	while(*macxp++=*cp++) ;
	if(qs)*(macxp-1)='"';
	else macxp--;
	*macxp++=LF;
	}

builtinMac(){
	char xpnd[32];

	switch(bvalue) {
		case R_LINE: itoa(cline, xpnd, 10);
					 strngcpy(xpnd,0);
					 break;
		case R_FILE: strngcpy(name,1);
					 break;
		case R_DATE: strngcpy(mdate,1);
					 break;
		case R_TIME: strngcpy(mtime,1);
					 break;
		case R_STDC: itoa(__stdc, xpnd, 10);
					 strngcpy(xpnd,0);
					 break;
		}
	}


tokone() {
	char macOK;

	tokat=cur;
	if(!(macOK = ((curch=*cur++) != '$')))
		curch=*(tokat=cur++);
	heir=0;

	switch (ltype[curch]) {
		case LETTER:if(cur < savEnd) lastname=tokat;
					find(macOK);
					if (heir == DEFINED) {
						addnest();
						whitesp();
						return 1;
						}
					if (heir == RESERVED) {
						if(bvalue >= R_LINE && bvalue <= R_STDC) {
							builtinMac();
							return 1;
							}
						if(!xkwd && bvalue >= RASM) heir = UNDEF;
						}
					break;
		case DIGIT: number();
					break;
		case 4:		if (*cur == '=') {
						cur++;
						heir=19;
						bvalue=NE;
						break;
						}
					heir=24;
					bvalue=NOT;
					break;
		case 5:		isstring();
					break;
		case 6:		heir=23;
					opeq(MOD);
					break;
		case 7:		if (*cur == '&') {
						cur++;
						heir=15;
						break;
						}
					heir=18;
					opeq(AND);
					break;
		case 8:		charac();
					break;
		case 9:		heir=23;
					opeq(MUL);
					break;
		case 10:	if (*cur == '+') {
						cur++;
						heir=24;
						bvalue=PREI;
						break;
						}
					heir=22;
					opeq(ADD);
					break;
		case 11:	break;
		case 12:	if (*cur == '-') {
						cur++;
						heir=24;
						bvalue=PRED;
						break;
						}
					if (*cur == '>') {
						cur++;
						heir=25;
						break;
						}
					heir=22;
					opeq(SUB);
					break;
		case 13:	if(*cur=='.') {
						heir=RESERVED;
						if(*++cur=='.') {
							cur++;
							curch=bvalue=RELIPSE;
							}
						else if(xkwd) curch=bvalue=RDOTDOT;
							else {
								error("case range option not on");
								heir=0;
								}
						break;
						}
					if (ltype[*cur] == DIGIT) number();
					else heir=25;
					break;
		case 14:	heir=23;
					opeq(DIV);
					break;
		case 15:	if (*cur == '<') {
						cur++;
						heir=21;
						opeq(SHL);
						break;
						}
					if (*cur == '=') {
						cur++;
						bvalue=LE;
						}
					else bvalue=LT;
					heir=20;
					break;
		case 16:	if (*cur == '=') {
						cur++;
						heir=19;
						bvalue=EQ;
						break;
						}
					heir=12;
					bvalue=ASGN;
					break;
		case 17:	if (*cur == '>') {
						cur++;
						heir=21;
						opeq(SHR);
						break;
						}
					if (*cur == '=') {
						cur++;
						bvalue=GE;
						}
					else bvalue=GT;
					heir=20;
					break;
		case 18:	heir=17;
					opeq(XOR);
					break;
		case 19:	if (*cur == '|') {
						cur++;
						heir=14;
						}
					else heir=16;
					opeq(OR);
					break;
		case 20:	if (incnext == 0) {
						cur--;	/* forever CONTZ */
						break;
						}
					incnext--;			/* end of this include	*/
					close(file);
					_lmove(SAVSIZE, 0, incPara[incnext], name, _showds());
					skipl();
					return 1;
		case 21:	cur--;
					dolf(0);
					return 1;
		default: ;
		}
	if (eopt) {
		obnum(heir);oc(' ');oc(curch);oc(' ');
		obnum(bvalue);oc(' ');ohw(nameat);oc(' ');ohw(cur);
		oc(' ');obnum(nested);ocrlf();
		}
	whitesp();
	return 0;
	}

/*	if name not found, set nameat to structure with type OPERAND,
	nstor of SUNSTOR and nchain to zero. set heir to UNDEF. */
/*	if name found, set heir to type and nameat to address of type. */
/*	if reserved, set bvalue to rvalue. */


find(char macOK) {
	cur--;
	i=hashno=0;
	while ((ltype[*cur] <= DIGIT) && i < 31) {
		hashno+=*cur;
		string[i++]=*cur++;
		}
	hashno+=i;
	if (i == 31) while (ltype[*cur] <= DIGIT) cur++;
	string[i]=0;
	if (eopt) {
		os(string);
		oc(' ');
		}
	hashno&=31;
	search(macOK);
	}

search(char macOK) {

	if(macOK) {
		bptr=machash[hashno];
		while (bptr) {
			j=-1;
			wp=bptr;
			bptr+=2;
			while (*bptr++ == string[++j]);
			if (j > i) {
				int ndx=nested;

				nameat=wp;
				nameat+=i+3;
				while(ndx)
					if(macname[ndx--] == nameat)
						goto recur;
				heir=*nameat;
				return;
				}
			bptr=*wp;
			}
		}
recur:
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
			if (heir == RESERVED) bvalue=nameat->rvalue;
			return;
			}
		bptr=*wp;
		}
	heir=UNDEF;
	}


newname() {
	wp=mfree;
	if(in_stru == 0) {
		*wp=hash[hashno];
		hash[hashno]=wp;
		}
	j=0;
	nameat=wp+1;
	do
		*nameat++=string[j++];
	while (j <= i);
	heir=UNDEF;
	nameat->opcl=OPERAND;
	nameat->nchain=0;
	nameat->nlen=i+1;
	nameat->ntype[0]=CINT;
	}

opeq(val)
	int  val; {
	if (*cur == '=') {
		cur++;
		heir=12;
		bvalue=val+AADD-ADD;
		}
	else bvalue=val;
	}

/*	skip  parenthesized	arguments	*/

skipa() {
	char inquote=0, ch;

	while ((ch = *cur++) != ')' || inquote) 
		switch(ch) {
			case '\'':
			case '"':	if(inquote == 0)
							inquote = ch;
						else if(inquote == ch && *(cur-2) != '\\')
							inquote = 0;
						continue;
			case '(':	skipa();
						continue;
			case LF:	dolf(1);
			}
	}


static char *macmove(char * beg, char * end, char ** free) {
	int len = end - beg;

	*free-=len;
	_move(len, beg, *free);
	return *free;
	}

addnest() {
	char *defat,*defstr,*mname,
		 *nptr=wp+1,
		 nlen=cur-tokat;
	char narg, inquote, plevel;
	char *argfrom[MAXNEST],
		  argstk[1024],
		  *ap, *sp;

	ap=&argstk[1024];
	sp=argstk;
	mname=defat=nameat;
	narg=inquote=plevel=0;

/*	dargs of 255 means none. 0 means need empty parens. more is count.	*/
	if (defat->dargs != 255) {
		whitesp();
		if(cur > argstk) {
			curch = *cur++;
			if(curch == LF) cur--;
			}
		else tokit();
		if (curch != '(') {
			if (nested+1 >= MAXNEST) { 
				error("define too deep");
			return;
			}
			nestfrom[nested]=cur;
			nestfrom[++nested]=macxp;
			macfrom[nested]=macxp;
#ifdef MacHiWater
			if(nested > maxNest) maxNest=nested;
#endif
/*			*macxp++='$'; */
			while(*nptr) *macxp++=*nptr++;
			*macxp++=LF;
			cur=nestfrom[nested];
#ifdef MacHiWater
			if(macxp > MacHiWaterP) MacHiWaterP=macxp;
#endif
			return;
			}
		if (*cur == ')') {	/* empty arg case	*/
			tokit();
			if(defat->dval == DEFEND)
				return;
			}
		else if(defat->dval == DEFEND) {
			skipa();
			return;
			}
		else {
			goto setarg;
			while(1) {
				whitesp();
				tokat=cur;
				if((curch=*cur++) == LF) {
					if(cur > argstk) {
						error("Unexpected EOF in macro argument");
						real_exit(2);
						}
					dolf(1);
					continue;
					}
				if(ltype[curch] == LETTER) while(ltype[*cur] == LETTER) cur++;
				if (curch == ')' && plevel-- == 0) {
					if(*(sp-1) != ' ') *sp++=' ';
					*sp++=LF;
					argfrom[narg]=macmove(argstk, sp, &ap);
					break;
					}
				if (curch == ',' && plevel == 0) {
					if(*(sp-1) != ' ') *sp++=' ';
					*sp++=LF;
					argfrom[narg]=macmove(argstk, sp, &ap);
setarg:				if (++narg == MAXNEST) {
						error("macro buffer overflow");
						return;
						}
					sp=argstk;
					continue;
					}
				if(curch == '(') plevel++;
				while(tokat<cur) {
					char ch;
					if(ltype[(ch=*tokat)] == SPACE) {
						tokat++;
						continue;
						}
					if(ch == '"' || ch == '\'') {
						*sp++=*tokat++;
						while(*tokat != ch || *(tokat-1) == '\\')
							*sp++=*tokat++;
						cur=tokat+1;
						}
					*sp++=*tokat++;
					}
				if(ltype[*cur]==SPACE) *sp++=' ';
				}
			}
xpand:	if (narg != defat->dargs) {
			error("wrong number of arguments");
			return;
			}
		}
	if(defat->dval == DEFEND)
		return;
	_setsp(ap-1);
	nestfrom[nested]=cur;
	if (nested+1 >= MAXNEST) { 
		error("define too deep");
		return;
		}
	nestfrom[++nested]=macxp;
	macfrom[nested]=macxp;
#ifdef MacHiWater
	if(nested > maxNest) maxNest=nested;
#endif
	defstr=&defat->dval;
	*macxp++=LF;
	macname[nested]=mname;
	while ((curch=*defstr) != DEFEND) {
		char *bp;

		macxp--;
		if (curch == DEFSTR) {
			defstr++;
			while((*macxp++=*defstr++) != LF);
			macOp=DEFSTR;
			}
		else if (curch == DEFPAS) {
			while(ltype[*(macxp-1)] == SPACE) macxp--;
			if(*++defstr == DEFSTR) {
				while(ltype[*defstr] == SPACE) defstr++;
				if(*defstr=='$') defstr++;
				while((*macxp++=*defstr++) != LF) ;
				}
			else {
				bp = argfrom[*defstr++];
				while(ltype[*bp] == SPACE) bp++;
				if(*bp=='$') bp++;
				while((*macxp++=*bp++) != LF) ;
				}
			macxp--;
			while(ltype[*(macxp-1)] == SPACE) macxp--;
			*macxp++=LF;
			macOp=DEFPAS;
			}
		else if (curch == DEFSFY) {
			bp = argfrom[*++defstr];
			*macxp++='"';
			while(*bp != LF && ltype[*bp] == SPACE) bp++;
			while(*bp != LF) {
				if(*bp=='$' && ltype[*(bp+1)]==LETTER) bp++;
				if(*bp == '"' || *bp == '\\')
					*macxp++='\\';
				*macxp++=*bp++;
				}
			while(ltype[*(macxp-1)] == SPACE) macxp--;
			*macxp++='"';
			*macxp++=LF;
			defstr++;
			macOp=DEFSFY;
			}
		else if(curch == DEFCPY) {
			bp=argfrom[*++defstr];
			while(*bp != LF) {
				if(*bp=='$' && ltype[*(bp+1)]==LETTER) bp++;
				*macxp++=*bp++;
				}
			defstr++;
			macOp=DEFCPY;
			}
		else {
			char oldnest = nested;

			cur=argfrom[*defstr++];
			while(1) {
				tokat=cur;
				if((curch=*cur++)==LF)
					break;
				if(curch == '$') {
					while(ltype[*cur] == LETTER) *macxp++=*cur++;
					continue;
					}
				else if(ltype[curch] == LETTER) {
					if(strncmp(nptr, tokat, nlen) == 0) {
						wp = nptr-2;
						cur=tokat + nlen;
						nameat = defat;
						heir = DEFINED;
						}
					else
						find(1);
					if(heir == DEFINED
					|| (heir == RESERVED && bvalue >= R_LINE && bvalue <= R_STDC)) {
						if(heir == DEFINED) addnest();
						else builtinMac();
						if(nested != oldnest) {
							cur=nestfrom[--nested];
							nestfrom[nested]=macfrom[nested];
							}
						macxp--;
						if(macOp == DEFPAS) whitesp();
						continue;
						}
					while(tokat < cur) *macxp++=*tokat++;
					}
				else if(curch == '"' || curch == '\'') {
						*macxp++=*tokat++;
						while(*tokat != curch || *(tokat-1) == '\\')
							*macxp++=*tokat++;
						*macxp++=curch;
						cur=tokat+1;
						continue;
						}
				else *macxp++=curch;
				}
			*macxp++=LF;
			macOp=DEFPARM;
			}
		}
	cur=nestfrom[nested];
#ifdef MacHiWater
	if(macxp > MacHiWaterP) MacHiWaterP=macxp;
#endif
	}
	
/*	skip a parenthesized	expression.	*/

skipp() {
	while (*++cur != ')') 
		if (*cur == '(') skipp();
	}


number() {
	char ch,n;
	double strtod();
	heir=CONSTANT;
	dvalue=0;
	if (curch == '0' && (ch = toupper(*cur)) != 'L' && ch != 'U' && ch != '.') {
		if (ch == 'X') {
			n = 0;
			while (1) {
				ch=*++cur;
				if (ch >='a') ch-=32;
				if (ch >='A' && ch <= 'F') ch-='7';
				else if (ltype[ch] == DIGIT) ch-='0';
				else break;
				dvalue=(dvalue<<4)+ch;
				if (n++ == 7) *((long *)(&fvalue)+1) = dvalue;
				}
			if (n > 8 && n != 16) error("illegal double constant");
			else if (n == 16) {
				*((long *)(&fvalue)) = dvalue;
				heir = FDCONSTANT;
				return;
				}
			}
		else {
			while ((ch=*cur-'0') < 10) {
				cur++;
				dvalue=(dvalue<<3)+ch;
				}
			}
		if (dvalue > 65535 || dvalue < 0) heir=LCONSTANT;
		}
	else {
		if (*tokat == '.') cur=tokat;
		dvalue=curch-'0';
		while ((ch=*cur-'0') <= 9) {
			dvalue=dvalue*10+ch;
			cur++;
			}
		if ((*cur == '.' && *(cur+1) != '.') || *cur == 'e' || *cur == 'E') {
/*			cur=tokat+_finput(tokat,&fvalue,100); */
			*((double*)(&fvalue))=strtod(tokat, &cur);
			heir=FDCONSTANT;
			while((ch = toupper(*cur)) == 'L' || ch == 'F') {
				if(ch == 'F') {
					heir=FFCONSTANT;
					*((float*)(&fvalue))=*((double*)(&fvalue));
					}
				cur++;
				}
			return;
			}
		if (dvalue > 32767) heir=LCONSTANT;
		}
	while ((ch = toupper(*cur)) == 'L' || ch == 'U') {
		if(ch == 'L')
			heir=LCONSTANT;
		cur++;
		}
	wvalue=dvalue;
	}

isstring() {
	i=0;
	heir=STRNG;
	while (*cur != '"' && i < 255) {
		if (*cur == '\n' || *cur == CONTZ) {
			error("unmatched \"");
			cur--;
			break;
			}
		string[i++]=getach();
		}
	if (i == 255) { error("string too long"); return; }
	cur++;
	string[i]=0;
	string[i+1]=0xff;
	}

charac() {
	heir=CONSTANT;
	if (*cur == '\'') {
		wvalue=0;
		}
	else {
		wvalue=getach();
		if (*cur != '\'' && *cur != LF) {
			wvalue<<=8;
			wvalue+=getach();
			}
		}
	if (*cur++ != '\'') error("missing '");
	}

getach() {
	char ch,och,max,xd;
getagain:
	ch=*cur++;
	if (ch == '\\') {
		och=ch=*cur++;
		if (ch >= 'a') ch-=32;
		switch (ch) {
			case '\r':	cur++;
			case '\n':	if (addparm == 0 && addproto == 0) lineinc++;
						goto getagain;
			case 'A':	ch=7;
						break;
			case 'B':	ch=8;
						break;
			case 'F':	ch=12;
						break;
			case 'N':	ch=LF;
						break;
			case 'R':	ch=13;
						break;
			case 'T':	ch=9;
						break;
			case 'V':	ch=11;
						break;
			case 'X':	max=3;
						ch=0;
						while (max--) {
							xd=*cur++;
							if (xd >='a')
								xd-=32;
							if (xd >='A' && xd <= 'F')
								xd-='7';
							else if (ltype[xd] == DIGIT)
								xd-='0';
							else {
								cur--;
								break;
								}
							ch=(ch<<4)+xd;
							}
						break;
			default:	if (ch >= '0' && ch <= '7') {
							max=2;
							ch-='0';
							while (*cur >= '0' && *cur <= '7' && max--)
								ch=(ch<<3)+*cur++-'0';
							}
						else ch=och;
			}
		}
	return ch;
	}

whitesp() {
	char ch;
	int bline=cline;
	static char eoferr[] = "EOF within comment beginning on line #    ";
	while (1) {
		if (ltype[*cur] == SPACE)
			cur++;
		else if (*cur == '/' && (cur+1)->byte == '/') {
			cur=lineEnd-1;
			dolf(1);
			}
		else if (*cur == '/' && (cur+1)->byte == '*') {
			cur+=2;
			while(1) {
				ch=*cur;
				while (ch != LF && ch != '*' && ch != 26)
					ch=*++cur;
				if (ch == LF) dolf(1);
				else if (ch == 26) {
					cur--;
					ltoa((long)bline, &eoferr[37], 10);
					error(eoferr);
					return;
					}
				else if (*++cur == '/') {
					cur++;
					break;
					}
				}
			}
		else return;
		}
	}
