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
/*	ASM4.C		part four of ASM88	*/

/*				evaluation of constant expressions	*/

#include "ASM88.H"
#include "OBJ.H"

one(vtype)					/* operater is conditional (? :) */
	int  vtype[]; {
	int  vleft[6],vright[6];

/* short circuit if only a register	*/
	if (heir == REG) {
		vtype[VIS]=REGV;
		vtype[VT]=wvalue->sclass;
		vtype[VVAL]=wvalue->svalue;
		tokit();
		return;
		}

	heir14(vtype);
	if (ifch('?')) {
		one(vleft);
		notch(':');
		one(vright);
		if (vtype[VIS] != CONSTV) badexp();
		if (vtype[VVAL]) for(i=0; i < 6; i++) vtype[i]=vleft[i];
		else 
			for(i=0; i < 6; i++) vtype[i]=vright[i];
		}
	}

heir14(vtype)				/* operater is or (||) */
	int  vtype[]; {
	int  vleft[6];

	heir15(vtype);
	if (heir == 14) {
		tokit();
		heir14(vleft);
		if (vtype[VIS] != CONSTV || vleft[VIS] != CONSTV) badexp();
		vtype[VVAL]=vtype[VVAL] || vleft[VVAL];
		}
	}


heir15(vtype)				/* operater is and (&&) */
	int  vtype[]; {
	int  vleft[6];
	heir16(vtype);
	if (heir == 15) {
		tokit();
		heir15(vleft);
		if (vtype[VIS] != CONSTV || vleft[VIS] != CONSTV) badexp();
		vtype[VVAL]=vtype[VVAL] && vleft[VVAL];
		}
	}


heir16(vtype)				/* operater is binary or (|) */
	int  vtype[]; {
	int  vleft[6];
	heir17(vtype);
	if (heir == 16) {
		tokit();
		heir16(vleft);
		if (vtype[VIS] != CONSTV || vleft[VIS] != CONSTV) badexp();
		vtype[VVAL]|=vleft[VVAL];
		}
	}

heir17(vtype)				/* operater is xor (^) */
	int  vtype[]; {
	int  vleft[6];
	heir18(vtype);
	if (heir == 17) {
		tokit();
		heir17(vleft);
		if (vtype[VIS] != CONSTV || vleft[VIS] != CONSTV) badexp();
		vtype[VVAL]^=vleft[VVAL];
		}
	}

heir18(vtype)				/* operater is binary and (&) */
	int  vtype[]; {
	int  vleft[6];
	heir19(vtype);
	if (heir == 18) {
		tokit();
		heir18(vleft);
		if (vtype[VIS] != CONSTV || vleft[VIS] != CONSTV) badexp();
		vtype[VVAL]&=vleft[VVAL];
		}
	}

heir19(vtype)				/* operater is equality (== and !=) */
	int  vtype[]; {
	int  vleft[6];
	heir20(vtype);
	if (heir == 19) {
		tokit();
		heir19(vleft);
		if (vtype[VIS] != CONSTV || vleft[VIS] != CONSTV) badexp();
		if (bvalue == EQ) vtype[VVAL]=vtype[VVAL] == vleft[VVAL];
		else vtype[VVAL]=vtype[VVAL] != vleft[VVAL];
		}
	}

heir20(vtype)				/* operater is relational (< <= etc.) */
	int  vtype[]; {
	int  vleft[6];
	heir21(vtype);
	if (heir == 20) {
		tokit();
		heir20(vleft);
		if (vtype[VIS] != CONSTV || vleft[VIS] != CONSTV) badexp();
		switch(bvalue) {
			case GE:	vtype[VVAL]=vtype[VVAL] >= vleft[VVAL];
						break;
			case LT:	vtype[VVAL]=vtype[VVAL] < vleft[VVAL];
						break;
			case GT:	vtype[VVAL]=vtype[VVAL] > vleft[VVAL];
						break;
			case LE:	vtype[VVAL]=vtype[VVAL] <= vleft[VVAL];
						break;
			}
		}
	}

heir21(vtype)				/* operater is shift (<< and >>) */
	int  vtype[]; {
	int  vleft[6];
	char opch;

	heir22(vtype);
	while (heir == 21) {
		opch=curch;
		tokit();
		heir22(vleft);
		if (vtype[VIS] != CONSTV || vleft[VIS] != CONSTV) badexp();
		if (opch == '<') vtype[VVAL]<<= vleft[VVAL];
		else vtype[VVAL] >>= vleft[VVAL];
		}
	}

heir22(vtype)				/* operater is add sub (+ -) */
	int  vtype[]; {
	int  vleft[6];
	char opch;

	heir23(vtype);
	while (heir == 22) {
		opch=curch;
		tokit();
		heir23(vleft);
		if (vtype[VIS] == OFFV && vleft[VIS] == CONSTV) {
			if (opch == '+')  vtype[VOFF]+=vleft[VVAL];
			else vtype[VOFF]-=vleft[VVAL];
			}
		else if (vleft[VIS] == OFFV && vtype[VIS] == CONSTV) {
			if (opch == '+') vleft[VOFF]+=vtype[VVAL];
			else vleft[VOFF]-=vtype[VVAL];
			for(i=0; i < 6; i++)
				vtype[i]=vleft[i];
			}
		else {
			if (vtype[VIS] != CONSTV || vleft[VIS] != CONSTV) badexp();
			if (opch == '+') vtype[VVAL]+= vleft[VVAL];
			else vtype[VVAL]-=vleft[VVAL];
			}
		}
	}

heir23(vtype)				/* operater is mul and div (* and /) */
	int  vtype[]; {
	int  vleft[6];
	char opch;

	heir24(vtype);
	while (heir == 23) {
		opch=curch;
		tokit();
		heir24(vleft);
		if (vtype[VIS] != CONSTV || vleft[VIS] != CONSTV) badexp();
		if (opch == '*') vtype[VVAL]*= vleft[VVAL];
		else if (opch == '/') vtype[VVAL] /= vleft[VVAL];
		else vtype[VVAL]%=vleft[VVAL];
		}
	}

union {long llong; } *ll;
heir24(vtype) 				/* operator is prefix (& - ! not) */
	int vtype[]; {

	if (heir == RESERVED && wvalue->sclass == RSEG) {
		tokit();
		heir24(vtype);
		if (vtype[VIS] != VARV) badexp();
		vtype[VIS]=OFFV;
		vtype[VT]=CSEGMENT;
		return;
		}
							/* accept either OFFSET or & */
	if (heir == RESERVED && wvalue->sclass == ROFFSET) {
		curch='&';
		heir=18;
		}
	switch (curch) {
		case '&':
			if (heir == 18) {
				tokit();
				heir24(vtype);
				if (vtype[VIS] != VARV) badexp();
				vtype[VIS]=OFFV;
				vtype[VT]=CINT;
				return;
				}
			break;
		case '-':
			tokit();
			heir24(vtype);
			if (vtype[VIS] != CONSTV) badexp();
			ll=&vtype[VVAL];
			ll->llong=-ll->llong;
			return;
		case '+':
			tokit();
			heir24(vtype);
			if (vtype[VIS] != CONSTV) badexp();
			return;
		case '!':
			tokit();
			heir24(vtype);
			if (vtype[VIS] != CONSTV) badexp();
			vtype[VVAL]=!vtype[VVAL];
			return;
		case '\176':
			tokit();
			heir24(vtype);
			if (vtype[VIS] != CONSTV) badexp();
			vtype[VVAL]^=0xffff;
			return;
		case '@':
			if (!is_big) badexp();
			tokit();
			heir24(vtype);
			vtype[VT]=CFOREIGN;
			vtype[VSEG]=-1;
			return;
		default: ;
		}
	heir26(vtype);
	}

heir26(vtype)				/* operand or constant or string */
	int  vtype[]; {
	int  vleft[6];
	char borw,sr,low;

	if(ifch('(')) {
		one(vtype);
		notch(')');
		return;
		}
	switch (heir) {
		case RESERVED:	borw=wvalue->sclass;
						if ((borw >= RBYTE && borw <= RTBYTE) ||
							borw == ROFFSET) {
							tokit();
							heir26(vtype);
							if (borw == ROFFSET) {
								if (vtype[VIS] != VARV)
									error("cannot take OFFSET");
								else vtype[VIS]=OFFV;
								}
							else vtype[VT]=CCHAR+borw-RBYTE;
							break;
							}
						if (borw == RST) {
							vtype[VVAL]=0;
							tokit();
							if (curch == '(') {
								tokit();
								one(vtype);
								if (vtype[VIS] != CONSTV || vtype[VVAL] < 0||
									vtype[VVAL] > 7)
										 error("illegal ST value");
								notch(')');
								}
							vtype[VIS]=STV;
							break;
							}
						error("missplaced reserved word");
						break;
		case SSTRING:	wvalue=0;
						i=0;
						while (string[i]) {
							low=wvalue;
							wvalue=(low<<8)+string[i++];
							}
		case CONSTANT:	vtype[VIS]=CONSTV;
						vtype[VVAL]=wvalue;
						vtype[VNAME]=hiword;
						vtype[VT]=CINT;
						tokit();
						break;
		case SEGREG:	sr=wvalue->sclass;
						tokit();
						if (ifch(':')) {
							heir26(vtype);
							if (vtype[VIS] != VARV) badaexp();
							vtype[VSEG]=sr;
							break;
							}
						else {
							vtype[VIS]=SEGRV;
							vtype[VVAL]=sr;
							vtype[VT]=CINT;
							break;
							}
		case REG:		vtype[VIS]=REGV;
						vtype[VT]=wvalue->sclass;
						vtype[VVAL]=wvalue->svalue;
						tokit();
						break;
		case NAME:		vtype[VIS]=VARV;
						vtype[VVAL]=8;
						vtype[VNAME]=wvalue->snum;
						vtype[VOFF]=0;
						vtype[VT]=wvalue->stype;
						vtype[VSEG]=wvalue->svalue == OCSEG ? 1:3;
						tokit();
						plusaddr(vtype);
						break;
		default:		if (curch == '[') {
							vtype[VIS]=VARV;
							vtype[VVAL]=8;
							vtype[VNAME]=vtype[VOFF]=0;
							vtype[VT]=CLABEL;
							vtype[VSEG]=-1;
							plusaddr(vtype);
							}
						else badexp();
		}
	}

plusaddr(vtype)
	int  vtype[]; {

	while (1) {
		if (ifch('[')) {
			plusone(vtype);
			notch(']');
			}
		else if (ifch('+') || curch == '-')
			plusone(vtype);
		else return;
		}
	}

plusone(vtype)
	int  vtype[]; {
	int  vplus[6];

	do {
		heir23(vplus);
		if (vplus[VIS] == CONSTV)
			vtype[VOFF]+=vplus[VVAL];
		else if (vplus[VIS] == REGV && vplus[VT] == CINT) {
			i=vplus[VVAL];
			bptr=rmto;
			for (j=0; j < 12; j++) {
				if (bptr->regis == i && bptr->rmwas == vtype[VVAL]) {
					vtype[VVAL]=bptr->rmis;
					break;
					}
				bptr+=3;
				}
			if (j == 12) {
				badaexp();
				}
			}
		else badaexp();
		}
	while (ifch('+') || curch == '-');
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

badopnd() {
	error("illegal operand");
	}

badexp() {
	error("illegal expression");
	}

badaexp() {
	error("illegal address expression");
	}
