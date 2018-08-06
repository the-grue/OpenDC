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
/*	ASM88	ASSEMBLER FOR 8088					ASM2.C	*/

/*	handle all of the mnumonics	*/

#include "ASM88.H"
#include "OBJ.H"

doinst() {
	int  vleft[6],vright[6];
	char type,class;

	type=wvalue->svalue;
	class=wvalue->sclass;
	tokit();

	switch(class) {
		case 0:	twosame(vleft,vright);
				addtype(type,vleft,vright);
				break;
		case 1:	one(vleft);
				pushtype(type,vleft);
				break;
		case 2:	codeb(type);
				break;
		case 3:	one(vleft);
				inctype(type,vleft);
				break;
		case 4:	one(vleft);
				jotype(type,vleft);
				break;
		case 5:	twosame(vleft,vright);
				testtype(vleft,vright);
				break;
		case 6:	twosame(vleft,vright);
				xchgtype(vleft,vright);
				break;
		case 7:	one(vleft);
				calltype(vleft);
				break;
		case 8:	rettype(type);
				break;
		case 9:	twosame(vleft,vright);
				movtype(vleft,vright);
				break;
		case 10:two(vleft,vright);
				lestype(type,vleft,vright);
				break;
		case 11:one(vleft);
				inttype(vleft);
				break;
		case 12:two(vleft,vright);
				rortype(type,vleft,vright);
				break;
		case 13:aamtype(type);
				break;
		case 14:two(vleft,vright);
				esctype(vleft,vright);
				break;
		case 15:two(vleft,vright);
				intype(vleft,vright);
				break;
		case 16:two(vleft,vright);
				outtype(vleft,vright);
				break;
		case 17:one(vleft);
				jmptype(vleft);
				break;
		case 18:one(vleft);
				nottype(type,vleft);
				break;
		case 19:two(vleft,vright);
				leatype(vleft,vright);
				break;
		case 20:one(vleft);
				xlattype(vleft);
				break;
		case 21:one(vleft);
				longtype(type,vleft);
				break;
		case 22:two(vleft, vright);
				x186enter(vleft, vright);
				break;
		case 23:two(vleft, vright);
				v20bitfields(type, vleft, vright);
				break;
		case 24:v20zops(type);
				break;
		case 25:two(vleft, vright);
				v20bitOps(type, vleft, vright);
				break;
		case 26:one(vleft);
				v20bcd(type, vleft);
		}
	}

/*	case 0				add type	*/

addtype(type,vleft,vright)
	char type;
	int  vleft[],vright[]; {
	char signed,from;

	if (vright[VIS] <= OFFV) {
		if (vleft[VIS] == REGV && vleft[VVAL] == AX) {
			codeb(3+realtype(vleft)+(type<<3));
			oconst(vright);
			}
		else {
			signed=(realtype(vleft) == CINT && vright[VIS] == CONSTV &&
				vright[VVAL] <= 127 && 	vright[VVAL] >= -128) ? 2 : 0;
			codeb(127+signed+realtype(vleft));
			orm(type,vleft);
			if (signed) vright[VT]=CCHAR;
			oconst(vright);
			}
		}
	else {
		if (vleft[VIS] == REGV) {
			codeb((type<<3)+realtype(vright)+1);
			orm(vleft[VVAL],vright);
			}
		else if (vright[VIS] == REGV) {
			codeb((type<<3)+realtype(vleft)-CCHAR);
			orm(vright[VVAL],vleft);
			}
		else error("impossable arithmetic");
		}
	}

/*	case 1				push type	*/

pushtype(type,vtype)
	char type;
	int  vtype[]; {
		char signext;

		switch (vtype[VIS]) {
		case REGV:	if (vtype[VT] == CCHAR) error("cannot PUSH byte");
					codeb(vtype[VVAL]+(type<<3)+0x50);
					break;
		case SEGRV:	if (type && vtype[VVAL] == CS) error("cannot POP CS");
					codeb(6+type+(vtype[VVAL]<<3));
					break;
		case VARV:	if (vtype[VT] != CFOREIGN) vtype[VT]=CINT;
					if (type) {
						codeb(0x8f);
						orm(0,vtype);
						}
					else {
						codeb(0xff);
						orm(6,vtype);
						}
					break;
		case CONSTV:
		case OFFV:	if(type)
						error("cannot POP constant");
					else if(Xflag) {
						signext = (vtype[VIS] == CONSTV && vtype[VVAL] <= 127 && vtype[VVAL] >= -128) ? 2 : 0;
						codeb(0x68 | signext);
						if(signext)
							vtype[VT] = CCHAR;
						oconst(vtype);
						}
					else
						error("cannot PUSH constant");
					break;
		default:	badopnd();
		}
	}


/*	case 3				inc type	*/

inctype(type,vtype)
	char type;
	int  vtype[]; {

	if (vtype[VIS] <= OFFV) error("cannot increment");
	else if (vtype[VIS] == REGV && realtype(vtype) == CINT)
		codeb(0X40+vtype[VVAL]+(type<<3));
	else {
		codeb(0xfd+realtype(vtype));
		orm(type,vtype);
		}
	}


/*	case 4				jo type	*/

/*	type 0 to 15   are jo,jno,jb ... jg	*/
/*	type 16 to 19  are loopnz,loopz,loop and jcxz	*/
/*	type 20 and 21 are jmp and call	*/

jotype(type,vtype)
	char type;
	int  vtype[]; {

	if (vtype[VIS] != VARV || vtype[VT] != CLABEL) error("need label");
	else jump(type,vtype[VNAME]);
	}


/*	case 5				test type	*/

testtype(vleft,vright)
	int  vleft[],vright[]; {

	if (vright[VIS] == CONSTV) {
		if (vleft[VIS] == REGV && vleft[VVAL] == AX) {
			if (vleft[VT] == CCHAR) {
				codeb(0xa8);
				codeb(vright[VVAL]);
				}
			else {
				codeb(0xa9);
				codew(vright[VVAL]);
				}
			}
		else {
			codeb(0xf5+realtype(vleft));
			orm(0,vleft);
			oconst(vright);
			}
		}
	else {
		if (vleft[VIS] == REGV) {
			codeb(0x83+realtype(vleft));
			orm(vleft[VVAL],vright);
			}
		else if (vright[VIS] == REGV) {
			codeb(0x83+realtype(vright));
			orm(vright[VVAL],vleft);
			}
		else error("cannot TEST");
		}
	}


/*	case 6				xchg type	*/

xchgtype(vleft,vright)
	int  vleft[],vright[]; {

	if (vleft[VIS] == REGV) {
		if (vright[VIS] == REGV && vleft[VVAL] == AX && realtype(vleft) == CINT)
			codeb(0x90+vright[VVAL]);
		else {
			codeb(0x85+realtype(vleft));
			orm(vleft[VVAL],vright);
			}
		}
	else if (vright[VIS] == REGV) {
		codeb(0x85+realtype(vright));
		orm(vright[VVAL],vleft);
		}
	else badopnd();
	}


/*	case 7				call type	*/

calltype(vtype)
	int vtype[]; {

	if (vtype[VIS] == VARV && vtype[VVAL] == 8 && vtype[VT] == CLABEL) {
		jump(21,vtype[VNAME]);
		}
	else {
		codeb(0xff);
		orm(2,vtype);
		}
	}


/*	case 8				ret type	*/

rettype(type)
	char type; {
	int  vtype[6];

	if (curch == LF || curch == ';') {
		if (type) codeb(0xcb);
		else codeb(0xc3);
		}
	else {
		one(vtype);
		if (vtype[VIS] != CONSTV) error("need constant");
		else {
			if (type) codeb(0xca);
			else codeb(0xc2);
			codew(vtype[VVAL]);
			}
		}
	}


/*	case 9				mov type	*/

movtype(vleft,vright)
	int  vleft[],vright[]; {

	if (vleft[VIS] == SEGRV) {
		codeb(0x8e);
		orm(vleft[VVAL],vright);
		}
	else if (vright[VIS] == SEGRV) {
		codeb(0x8c);
		orm(vright[VVAL],vleft);
		}
	else if (vright[VIS] <= OFFV) {
		if (vleft[VIS] == REGV) {
			codeb(0xb0+((realtype(vleft)-1)<<3)+vleft[VVAL]);
			oconst(vright);
			}
		else {
			codeb(0xc5+realtype(vleft));
			orm(0,vleft);
			oconst(vright);
			}
		}
	else if (vleft[VIS] == REGV) {
		if (vleft[VVAL] == AX && vright[VIS] == VARV && vright[VVAL] == 8) {
			codeb(0xa0+realtype(vleft)-1);
			if (vright[VSEG] >= 0) whatseg(vright);
			codew(vright[VOFF]);
			if (vright[VT] == CFOREIGN)
				fixup(OPTR, vright[VNAME]);
			else if (vright[VT] == CSEGMENT)
				fixup(OSEGPTR, vright[VNAME]);
			else
				fixup(ONAMEREL,vright[VNAME]);
			}
		else {
			codeb(0x8a-1+realtype(vleft));
			orm(vleft[VVAL],vright);
			}
		}
	else if (vright[VIS] == REGV) {
		if (vright[VVAL] == AX && vleft[VIS] == VARV && vleft[VVAL] == 8) {
			codeb(0xa2+realtype(vright)-1);
			if (vleft[VSEG] >= 0) whatseg(vleft);
			codew(vleft[VOFF]);
			if (vleft[VT] == CFOREIGN)
				fixup(OPTR, vleft[VNAME]);
			else if (vleft[VT] == CSEGMENT)
				fixup(OSEGPTR, vleft[VNAME]);
			else
				fixup(ONAMEREL,vleft[VNAME]);
			}
		else {
			codeb(0x88-1+realtype(vright));
			orm(vright[VVAL],vleft);
			}
		}
	else badopnd();
	}


/*	case 10				les type	*/

lestype(type,vleft,vright)
	char type;
	int  vleft[],vright[]; {

	if (Xflag == 0 && type == 0x62) badopnd();
	if (vleft[VIS] != REGV || vright[VIS] != VARV) badopnd();
	if (vright[VT] != CFOREIGN)
		vright[VT]=CINT;
	codeb(type);
	orm(vleft[VVAL],vright);
	}


/*	case 11				int type	*/

inttype(vtype)
	int  vtype[]; {

	if (vtype[VIS] != CONSTV || vtype[VVAL] & 0xff00) badopnd();
	if (vtype[VVAL] != 3) {
		codeb(0xcd);
		codeb(vtype[VVAL]);
		}
	else codeb(0xcc);
	}


/*	case 12				ror type	*/

rortype(type,vleft,vright)
	char type;
	int  vleft[],vright[]; {
		char imm8 = 0;

	if (vright[VIS] == CONSTV)
		if (vright[VVAL] == 1)
			codeb(0xd0-1+realtype(vleft));
		else if (Xflag) {
			imm8++;
			codeb(0xC0 - 1 + realtype(vleft));
			}
		else badopnd();
	else if (vright[VIS] == REGV && vright[VVAL] == CL && vright[VT] == CCHAR)
		codeb(0xd2-1+realtype(vleft));
	else badopnd();
	orm(type,vleft);
	if(imm8)
		codeb(vright[VVAL] & 0xFF);
	}


/*	case 13				aam type	*/

aamtype(type)
	char type; {

	if (type) codew(0x0ad5);
	else codew(0x0ad4);
	}


/*	case 14				esc type	*/

esctype(vleft,vright)
	int  vleft[],vright[]; {

	if (vleft[VIS] != CONSTV || vleft[VVAL] & 0xff00) badopnd();
	codeb(0xd8+(vleft[VVAL]>>3));
	orm(vleft[VVAL] & 7,vright);
	}


/*	case 15				in type	*/

intype(vleft,vright)
	int  vleft[],vright[]; {

	if (vleft[VIS] == REGV && vleft[VVAL] == AX) {
		if (vright[VIS] == REGV && vright[VVAL] == DX && realtype(vright) == CINT) {
			codeb(0xec-1+realtype(vleft));
			}
		else if (vright[VIS] == CONSTV && (vright[VVAL] & 0xff00) == 0) {
			codeb(0xe4-1+realtype(vleft));
			codeb(vright[VVAL]);
			}
		else badopnd();
		}
	else badopnd();
	}


/*	case 16				out type	*/

outtype(vleft,vright)
	int  vleft[],vright[]; {

	if (vright[VIS] == REGV && vright[VVAL] == AX) {
		if (vleft[VIS] == REGV && vleft[VVAL] == DX && realtype(vleft) == CINT) {
			codeb(0xee-1+realtype(vright));
			}
		else if (vleft[VIS] == CONSTV && (vleft[VVAL] & 0xff00) == 0) {
			codeb(0xe6-1+realtype(vright));
			codeb(vleft[VVAL]);
			}
		else badopnd();
		}
	else badopnd();
	}


/*	case 17				jmp type	*/

jmptype(vtype)
	int vtype[]; {

	if (vtype[VIS] == VARV && vtype[VVAL] == 8 && vtype[VT] == CLABEL) {
		jump(20,vtype[VNAME]);
		}
	else {
		codeb(0xff);
		orm(4,vtype);
		}
	}


/*	case 18				not type	*/

nottype(type,vtype)
	int  vtype[]; {

	if ((type == 2 || type == 3) && vtype[VIS] <= OFFV) badopnd();
	codeb(0xf6-1+realtype(vtype));
	orm(type,vtype);
	}


/*	case 19				lea type	*/

leatype(vleft,vright)
	int  vleft[],vright[]; {

	if (vleft[VIS] != REGV) badopnd();
	if (vright[VT] == CLABEL) vright[VT]=CINT;
	codeb(0x8d);
	orm(vleft[VVAL],vright);
	}

/*	case 20				xlat type	*/

xlattype(vtype)
	int  vtype[]; {

	codeb(0xd7);
	}

/*	case 21				lcall or ljmp type	*/

longtype(type,vtype)
	char type;
	int  vtype[]; {

	if (vtype[VIS] != VARV) error("cannot jmp or call");
	if (is_big && vtype[VIS] == VARV && vtype[VVAL] == 8 && vtype[VT] == CLABEL) {
		codeb((type == 3) ? 0x9A:0xEA);	/* lcall, ljmp */
		codew(vtype[VOFF]);
		codew(0);
		fixup(OLNAMEREL, vtype[VNAME]);
		}
	else {
		codeb(0xff);
		if (vtype[VT] == CLABEL) vtype[VT]=CINT;
		orm(type,vtype);
		}
	}

/*	case 22				enter type */

x186enter(vleft, vright)
int vleft[], vright[]; {

	if(vleft[VIS] == CONSTV && vright[VIS] == CONSTV) {
		codeb(0xC8);
		codew(vleft[VVAL]);
		codeb(vright[VVAL] & 0xFF);
		}
	else badopnd();
	}

/* case 23				v20 bit fields */

v20bitfields(type, vleft, vright)
char type;
int vleft[], vright[]; {

	if(vleft[VIS] == REGV && vleft[VT] == CCHAR) {
		codeb(0x0F);
		if(vright[VIS] == REGV)
			if(vright[VT] == CCHAR) {
				codeb(type);
				orm(vright[VVAL], vleft);
				}
			else badopnd();
		else if(vright[VIS] == CONSTV) {
			codeb(type | 0x08);
			orm(0, vleft);
			codeb(vright[VVAL] & 0xFF);
			}
		else badopnd();
		}
	else badopnd();
	}

/*	case 24				v20 2 byte opcodes */

v20zops(type)
char type; {

	if(type < 0xD0) {
		codeb(0x0F);	/* ADD4S, SUB4S, CMP4S */
		codeb(type);
		}
	else {
		codeb(type);	/* CVTDB, CVTBD */
		codeb(0x0A);
		}
	}


/*	case 25				v20 single bit opcodes */

v20bitOps(type, vleft, vright)
char type;
int vleft[], vright[]; {

	codeb(0x0F);		/* prefix */
	if(realtype(VT) == CINT)
		type |= 1;
	if(vright[VIS] == CONSTV)
		type |= 8;
	codeb(type);
	orm(0,vleft);
	if(vright[VIS] == CONSTV)
		codeb(vright[VVAL]);
	}

/*	case 26				v20 bcd opcodes */

v20bcd(type, vleft)
char type;
int vleft[]; {

	if(vleft[VT] == CCHAR) {
		codeb(0x0F);
		codeb(type);
		orm(0, vleft);
		}
	else badopnd();
	}


/*	supply two operands of the same type	*/

twosame(vleft,vright)
	int  vleft[],vright[]; {

	two(vleft,vright);

	if (vleft[VT] != vright[VT]) {
		if (vright[VIS] == CONSTV)
			vright[VT]=vleft[VT];
		else if (vleft[VT] == CLABEL)
			vleft[VT]=vright[VT];
		else if (vright[VT] == CLABEL)
			vright[VT]=vleft[VT];
		else if (vright[VT] == CSEGMENT || vright[VT] == CFOREIGN);
		else error("mismatched types");
		}
	}


/*	supply two operands		*/

two(vleft,vright)
	int  vleft[],vright[]; {

	one(vleft);
	if (ifch(',')) {
		one(vright);
		}
	}

realtype(vtype)
	int vtype[]; {

	if (vtype[VT] == CSEGMENT || vtype[VT] == CFOREIGN) return CINT;
	else return vtype[VT];
	}
