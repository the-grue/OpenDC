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
/*	c3.c			third pard of medium c	*/

#include "pass1.h"
#include "nodes.h"
#if CHECK
#include "OBJ.H"
#endif


statement() {
	int  value,stmtnum,nodeo[3];
	laststmt=0;
	value=stmtnum=0;
	if (specs(SAUTO)) value=allocloc();
	else {
			/*	remember the statement number */
		if (copt && newline) stmtnum=cline;
		newline=0;

		if (heir == RESERVED) switch (bvalue) {
			case RBREAK:	value=dobreak(); break;
			case RCONTINUE:	value=docont(); break;
			case RDO:		value=dodow(); laststmt=0; break;
			case RFOR:		value=dofor(); laststmt=0; break;
			case RGOTO:		value=dogoto(); break;
			case RIF:		value=doif(); laststmt=0; break;
			case RRETURN:	value=doreturn(); laststmt=RET; break;
			case RSWITCH:	value=doswitch(); laststmt=0; break;
			case RWHILE:	value=dowhile(); laststmt=0; break;
			case RCASE:		value=docase(); laststmt=0; break;
			case RDEFAULT:	value=dodflt(); break;
			case RASM:		value=doasm(); break;
			default:		error("bad statement");
			}
		else if(ifch(';')) stmtnum=0;
		else if (ifch('{')) {
			value=compound(mfree,0);
			laststmt=0;
			}

		else if (*cur == ':') value=dolabel();
		else {
			value=expr();
			notch(';');
			}
		}

/*		output a chaining STMT number node */

#if CHECK
	if (stmtnum) {
		nodeo[0]=STMT;
		nodeo[1]=value;
		nodeo[2]=stmtnum;
		value=tree3(nodeo);
		}
#endif
	return value;
	}

dodow() {
	int  nodeo[3],value;
	nodeo[0]=DOW;
	tokit();
	nodeo[1]=statement();
	if (heir != RESERVED || bvalue != RWHILE)
		{ error("missing while"); return 0; }
	nodeo[2]=pexpr();
	notch(';');
	value=tree3(nodeo);
	tree3z();
	return value;
	}

dogoto() {
	int  nodeo[4];
	tokit();
	deflab();
	if (nameat->ntype[0] != CLABEL) { error("need label"); return 0;}
	nodeo[0]=CLABEL<<8;
	nodeo[1]=nameat->noff;
	nodeo[2]=nodeo[3]=0;
	nodeo[1]=tree4(nodeo);
	nodeo[0]=GOTOS;
	tokit();
	notch(';');
	return tree2(nodeo);
	}

doif() {
	int  nodeo[4];
	nodeo[0]=IFE;
	nodeo[1]=pexpr();
	nodeo[2]=statement();
	if (heir == RESERVED && bvalue == RELSE) {
		tokit();
		nodeo[3]=statement();
		}
	else nodeo[3]=0;
	return tree4(nodeo);
	}

doreturn() {
	int  nodeo[2],rnode[4];

	tokit();
	if (ifch(';')) {
		if (fun_ret_len) error("must return structure");
		else if (funtype >= CFLOAT) error("must return float");
		nodeo[1]=0;
		}
	else {
		heir11(nodeo);
		if (nodeo[0] == 0) error("return lacks argument");
		notch(';');
		if (fun_ret_len) {
			if (wopt) warning("returns structure");
			/* construct move to return structure	*/

			struct_type=nodeo[1];
			if (struct_type->sttype != CSTRUCT ||
				struct_type->ststagat->staglen != fun_ret_len)
					error("must return structure");
			rnode[0]=OPND+(is_big ? (PTRTO<<8) : (CUNSG<<8));
			rnode[1]=rnode[3]=0;
			rnode[2]=is_big ? 6: 4;	/* put structure at [bp+4] */
									/* now output the move	*/
			rnode[2]=tree4(rnode);
			nodeo[1]=nodeo[0];		/* take address of source */
			nodeo[0]=TA+(is_big ? (PTRTO << 8) : (CUNSG << 8));
			rnode[1]=tree2(nodeo);
			rnode[3]=fun_ret_len;
			rnode[0]=MOVE;
			nodeo[1]=tree4(rnode);
			}
		else {
			nodeo[1]=nodeo[0];
			nodeo[0]=CAST+(funtype<<8);
			nodeo[1]=tree2(nodeo);
			}
		}
	nodeo[0]=RET;
	return tree2(nodeo);
	}

dowhile() {
	int  nodeo[3],value;
	nodeo[0]=WHIL;
	nodeo[1]=pexpr();
	nodeo[2]=statement();
	value=tree3(nodeo);
	tree3z();
	return value;
	}

dofor() {
	int  nodeo[5],value;
	nodeo[0]=FORS;
	tokit();
	if (notch('(')) return 0;
	nodeo[1]=expr();
	if (notch(';')) return 0;
	nodeo[2]=expr();
	if (notch(';')) return 0;
	nodeo[3]=expr();
	if (notch(')')) return 0;
	nodeo[4]=statement();
	value=treev(5,nodeo);
	tree3z();
	return value;
	}

dolabel() {
	int  nodeo[3];
	deflab();
	if (nameat->nstor != SUNSTOR) { error("duplicate label"); return 0; }
	nodeo[0]=LAB;
	nameat->nstor=SSTATIC;
	nodeo[2]=nameat->noff;
	tokit();
	tokit();
	nodeo[1]=statement();
	return tree3(nodeo);
	}

deflab() {
	char *bptr;

	for(wp=labhash[hashno]; wp; wp=*wp) {
		char ch;

		nameat=wp+1;
		bptr=string;
		while((ch=*nameat++) && ch ==*bptr++) ;
		if(ch==0 && *bptr==0)
			return;
		}
	wp=mfree;
	*wp=labhash[hashno];
	labhash[hashno]=wp;
	nameat=wp+1;
	bptr=string;
	while(*nameat++=*bptr++) ;
	nameat->nchain=hash[32];	/* keep labels	*/
	hash[32]=nameat;
	nameat->opcl=OPERAND;
	nameat->nlen=bptr-string;
	nameat->nstor=SUNSTOR;
	nameat->noff=++ordinal;
	nameat->ntype[0]=CLABEL;
	ctlb(1);
	ctlb(SSTATIC);
	ctlw(ordinal);
	/*	add function name to label name	*/
	bptr=funname;
	while (*bptr) ctlb(*bptr++);
	ctlb('_');
	ctls(mfree+2);
	ctlb(INITFUN);
	mfree=&nameat->ntype[1];
	}

doswitch() {
	int  nodeo[3];
	nodeo[0]=SWIT;
	nodeo[1]=pexpr();
	nodeo[2]=statement();
	return tree3(nodeo);
	}

dobreak() {
	int  value,nodeo[1];
	nodeo[0]=BRK;
	value=tree1(nodeo);
	tokit();
	notch(';');
	return value;
	}

docont() {
	int  value,nodeo[1];
	nodeo[0]=CONT;
	value=tree1(nodeo);
	tokit();
	notch(';');
	return value;
	}

docase() {
	int  nodeo[3],minus,beg,end;

	tokit();
	minus=0;
	if (heir == 22 && curch =='-') {
		minus=1;
		tokit();
		}
	if (heir != CONSTANT && !(heir == RESERVED && bvalue == RSIZEOF) && curch != '(')
		 { error("need constant"); return 0; }
	beg=constexp();
	if (minus)beg=-beg;
	if(ifch(RDOTDOT)) {
		minus=0;
		if (heir == 22 && curch =='-') {
			minus=1;
			tokit();
			}
		if (heir != CONSTANT && curch != '(')
			 { error("need constant"); return 0; }
		end=constexp();
		if (minus)end=-end;
		if(beg>end){
			minus=end;
			end=beg;
			beg=minus;
			}
		}
	else
		end=beg;
	if (notch(':')) return 0;
	nodeo[0]=CAS;
	nodeo[1]=statement();
	while(beg<end) {
		nodeo[2]=beg++;
		nodeo[1]=tree3(nodeo);
		}
	nodeo[2]=end;
	return tree3(nodeo);
	}

dodflt() {
	int  nodeo[2];
	nodeo[0]=DFLT;
	tokit();
	if (notch(':')) return 0;
	nodeo[1]=statement();
	return tree2(nodeo);
	}

/*	output some assembler	*/

doasm() {
	int  nodeat;

	have_asm=1;
	nodeat=OUTASM;
	nodeat=tree1(&nodeat);		/* node for embedded assembler	*/
	skipl();
	while (1) {
		dolf(1);
		if (*cur == '#' || *cur == CONTZ) {
			if (*cur == CONTZ) error("missing end of #asm");
			treew(-1);
			skipl();
			tokit();
			return nodeat;
			}
		while (*cur != LF && *(cur+1) != LF && *cur != CONTZ &&
			*(cur+1) != CONTZ) {
			if(*cur != '#' && *(cur+1) != '#') {
putwrd:			wp=cur;
				treew(*wp);
				cur+=2;
				}
			else {
				char locname[64], *ep=cur;
				int n=0;

				if(*cur != '#')
					locname[n++] = *cur++;
				if(ltype[*(cur+1)] != LETTER) {
					cur=ep;
					goto putwrd;
					}
				cur+=2;
				find(1);
				if(heir == DEFINED && nameat->dargs == 255) {
					char *dp = &nameat->dval+1, suffix = 0;

					while(*dp != LF) {
						char c;

						if((c=ltype[*dp]) == DIGIT || c == LETTER) {
							if(*dp == '0') {
								locname[n++] = '0';
								if(toupper(*++dp) == 'X') {
									dp++;
									suffix = 'H';
									while(ltype[c=*dp] == DIGIT
										|| ((c=toupper(c)) >= 'A' && c <= 'F'))
											locname[n++] = *dp++;
									}
								else if(ltype[*dp] == DIGIT) {
									suffix = 'O';
									while(ltype[*dp] == DIGIT)
										locname[n++] = *dp++;
									}
								else continue;
								if(suffix) {
									locname[n++] = suffix;
									suffix = 0;
									}
								}
							else while((c=ltype[*dp]) == DIGIT || c == LETTER)
								locname[n++] = *dp++;
							}
						else locname[n++] = *dp++;
						}
					goto putloc;
					}
				if(heir != OPERAND && nameat->nstor != SAUTO) {
					cur=ep;
					goto putwrd;
					}
				locname[n++] = '[';
				locname[n++] = 'B';
				locname[n++] = 'P';
				if(nameat->noff >= 0)
				locname[n++] = '+';
				itoa(nameat->noff, &locname[n], 10);
				while(locname[++n]) ;
				locname[n++] = ']';
putloc:			if(n&1)
					locname[n++] = ' ';
				wp = locname;
				while(n) {
					treew(*wp++);
					n-=2;
					}
				}
			}
		if (*cur == LF) treew(LF);
		else {
			wp=cur;
			treew(*wp);
			if (*cur != CONTZ) cur++;
			}
		}
	}

static char undef_msg[40]="undefined - ";

compound(oldfree,isproc)
	char *oldfree,isproc; {
	int  list[7],value,*zero;
	char n,*lastfun,*lowfun,*last_move,**mpp, *mp;

	blevel++;
	n=0;
	list[1]=0;
	while (curch != '}' && curch != CONTZ) {
		if (value=statement()) {
			list[++n]=value;
			if (n == 6) {
				list[0]=LST+6;
				list[1]=treev(7,list);
				n=1;
				}
			}
		}
	if (n <= 1) value=list[1];
	else {
		list[0]=LST+n;
		value=treev(n+1,list);
		}
	if (mfree > oldfree) {
		if (mfree >= maxmem) {
			error("out of memory");
			real_exit(2);
			}
		if (mfree > hiwater) hiwater=mfree;
		for (n=0; n < 32; n++) {
			for (wp=hash[n]; wp >= oldfree; wp=*wp) ;
			hash[n]=wp;
			for (wp=labhash[n]; wp >= oldfree; wp=*wp) ;
			labhash[n]=wp;
			for (wp=machash[n]; wp >= oldfree; wp=*wp) ;
			machash[n]=wp;
			}
		lastfun=hash[32];
		/* clean off the function chain */
		while (hash[32] > oldfree) hash[32]=hash[32]->nchain;
		while (lastfun > oldfree) {
			wp=&lastfun;
			while (((char *)*wp)->nchain > oldfree)
				wp=&((char *)*wp)->nchain;
			if ((lowfun=*wp) == 0) break;
			*wp=0;
			/* abandon labels if exiting a procedure	*/
			if (isproc && lowfun->ntype[0] == CLABEL) {
				if (lowfun->nstor == SUNSTOR) {
					strcpy(&undef_msg[12],lowfun-lowfun->nlen);
					error(undef_msg);
					}
				continue;
				}
			i=lowfun->nlen;
			wp=oldfree+2+i;			/* where OPERAND will go */
			hashno=i-1;
			bptr=lowfun-1;
			while (--i) hashno+=*--bptr;
			hashno&=31;
			if(lowfun->opcl == DEFINED) {
				oldfree->word=machash[hashno];
				machash[hashno]=oldfree;
				}
			else if(lowfun->ntype[0] == CLABEL) {
				oldfree->word=labhash[hashno];/* rebuild label chain */
				labhash[hashno]=oldfree;
				}
			else {
				oldfree->word=hash[hashno];/* rebuild hash chain */
				hash[hashno]=oldfree;
				}
			oldfree+=2;
			last_move=&lowfun->ntype[0];
			if(lowfun->opcl == DEFINED) while(*last_move != DEFEND) last_move++;
			else while (*last_move >= FUNCTION) {
				if (*last_move == ARRAY || *last_move == CSTRUCT)
					last_move+=3;
				else last_move++;
				}
			while (bptr <= last_move)
				*oldfree++=*bptr++;
			((void *)wp)->nchain=hash[32];	/* rebuild function hash chain */
			hash[32]=wp;
			}
		mfree=oldfree;
		}
	retnum=cline;
	notch('}');
	blevel--;
	return value;
	}

allocloc() {
	char islocal,realstor;
	int  value,newvalue,nodeo[3];
	value=0;
	islocal=addstor == SAUTO;
	if (ifch(';')) return 0;
	if (locoff & 1) locoff--;
	findover=mfree-1;
	do {
		if (addvar(0) == 0) {
			error("bad declaration");
			findover=0;
			return 0;
			}
		addproto=addparm=0;
		if (addat->ntype[0] == FUNCTION) {
			realstor=addstor;	/* remember real storage but fix function */
			if (addstor != SSTATIC) addstor=addat->nstor=SEXTERN;
			atype(addtype);
			addstor=realstor;
#if CHECK
			if (copt) objtype(OPTYPE,addat->noff);
#endif
			}
		else {
			was_ext=0;
			atype(addtype);
			if (islocal) {
				locoff-=dsize(&addat->ntype[0]);
				addat->noff=locoff;
				}
#if CHECK
			if (copt) {
				if (addstor == SEXTERN || addstor == SSTATIC)
					objtype(OPTYPE,addat->noff);
				else if (islocal) {
					objtype(OLTYPE,locoff);
					}
				}
#endif
			if (newvalue=doinit()) {
				if (value) {
					nodeo[0]=LST+2;
					nodeo[1]=value;
					nodeo[2]=newvalue;
					value=tree3(nodeo);
					}
				else value=newvalue;
				}
			}
		}
	while (ifch(','));
	findover=0;
	notch(';');
	return value;
	}

expr() {
	int  enode[2];
	heir11(enode);
	return enode[0];
	}

exprnc() {
	int  enode[2];

	heir12(enode);
	if (enode[0] == 0) error("missing expression");
	return enode[0];
	}

pexpr() {
	int  value;
	tokit();
	if (ifch('(')) {
		value=expr();
		if (ifch(')')) return value;
		}
	error("need ()");
	return 0;
	}

ifch(ch)
	char ch; {
	if (curch == ch) {
		tokit();
		return 1;
		}
	return 0;
	}

notch(ch)
	char ch; {
	char *msg;
	if (curch == ch) {
		tokit();
		return 0;
		}
	msg="missing  ";
	*(msg+8)=ch;
	error(msg);
	return 1;
	}
