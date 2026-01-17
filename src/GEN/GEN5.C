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
/*	GEN5.C	part 5 of generation	*/

#include "PASS2.H"
#include "NODES.H"
#include "inst86.h"

genlab(lab)
	int  lab; {
	if (zopt) asm_label(lab);
	else {
		os("_L");
		onum(lab-1000);
		os(":\n");
		}
	}

static to_jshort[]={5,4,12,13,14,15,2,3,6,7,20};

ojmp(jtype,num)
	char jtype;
	int  num; {

	asm_jump(to_jshort[jtype],num);
	return (nextlab);
	}

get2same(node,vtype,lnode)
	int  node,vtype[],lnode[]; {

	get2(node,vtype,lnode);
	forcerm(lnode);
	forcerm(vtype);
	if (lnode[VT] != vtype[VT]) {
		if (lnode[VT] > vtype[VT]) {
			if (lnode[VT] == CLONG) forcel(vtype);
			else forceint(vtype);
			}
		else {
			if (vtype[VT] == CLONG) forcel(lnode);
			else forceint(lnode);
			}
		}
	}

get2(node,vtype,lnode)
	int  node,vtype[],lnode[]; {
	getval(tree[node+1],vtype);
	if (is_big) forcerm(vtype);
	nopost=0;
	if (floater(vtype)) loadf(vtype);	/* put first arg on stack */
	getval(tree[node+2],lnode);
	if (floater(lnode)) {
		if (lnode[VIS] == FLOATV) {
			if (vtype[VIS] != FLOATV) {
				loadf(vtype);
				builtin(_FXCH);
				}
			}
		else {
			loadf(vtype);
			loadf(lnode);
			}
		}
	if (floater(vtype)) loadf(lnode);
	}

forceind(vtype)
	int  vtype[]; {
	char andit;
	andit=0;
	if (vtype[VT] == CFLOAT || vtype[VT] == CDOUBLE) forceint(vtype);
	if (vtype[VT] == CLONG) {
		if (vtype[VIS] == REGV) {
			if (vtype[VVAL] == AX) regat[DX]=0;
			else regat[BX]=0;
			}
		vtype[VT]=CUNSG;
		}
	if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) {
		if (vtype[VIS] > OFFV) andit=1;
		vtype[VT]=CINT;
		}
	force(vtype,INDPAT);
	vtype[VT]=is_big ? PTRTO: CUNSG;
	if (andit) {
		asm_add(AND86,vtype,tocon(255));
		}
	}

forcebyt(vtype)
	int  vtype[]; {
	if (vtype[VIS] != VARV || floater(vtype)) {
		if (vtype[VT] == CLONG || floater(vtype))
			forceint(vtype);
		force(vtype,BYTEPAT);
		}
	vtype[VT]=CCHAR;
	}

forcereg(vtype)
	int  vtype[]; {
	if (vtype[VT] == PTRTO) force(vtype,INDPAT);
	else force(vtype,ANYPAT);
	}


forcemul(vtype)
	int  vtype[]; {
	if (vtype[VIS] <= OFFV || (vtype[VT] >= BITS && vtype[VT] != PTRTO))
		force(vtype,ANYPAT);
	}

/*	force vtype to be floating	*/

forcef(vtype)
	int  vtype[]; {
	union {double dbl; long lng;};

	if (!floater(vtype)) {
		if (vtype[VIS] == CONSTV) {
			if (vtype[VT] == CLONG)
				((char *)(&vtype[VVAL]))->dbl=((char *)(&vtype[VVAL]))->lng;
			else ((char *)(&vtype[VVAL]))->dbl=vtype[VVAL];
			vtype[VT]=CDOUBLE;
			}
		else loadf(vtype);
		}
	}

forceint(vtype)
	int  vtype[]; {
	union {double dbl; long lng;};

	if (floater(vtype)) {
		if (vtype[VIS] == CONSTV) {
			((char *)(&vtype[VVAL]))->lng=((char *)(&vtype[VVAL]))->dbl;
			vtype[VT]=CINT;
			}
		else {
			loadf(vtype);
			need(AXPAT);
			need(DXPAT);
			builtin(_FSTOREL);
			setax(vtype);
			regat[DX]=0;
			}
		return;
		}

	if (vtype[VT] == CLONG) {
		if (vtype[VIS] == REGV) {
			if (vtype[VVAL] == AX) regat[DX]=0;
			else regat[BX]=0;
			}
		vtype[VT]=CUNSG;
		}
	else if (vtype[VT] == PTRTO) {
		if (vtype[VIS] == REGV || vtype[VIS] == OFFV) {
			regat[vtype[VMORE]]=0;
			vtype[VMORE]=-1;
			}
		vtype[VT]=CUNSG;
		}
	else if (vtype[VT] != CINT && vtype[VT] != CUNSG) {
		if (vtype[VIS] != CONSTV) {
			force(vtype,ANYPAT);
			if (vtype[VT] == CCHAR) {
				if (vtype[VVAL] <= BX) {
					asm_move(toreglow(vtype[VVAL]+4),tocon(0));
					}
				else {
					asm_add(AND86,vtype,tocon(255));
					}
				}
			else if (vtype[VT] == CSCHAR) {
				if(vtype[VVAL] == AX) asm_cbw();
				else {
					asm_xchg(vtype[VVAL], AX);
					asm_cbw();
					asm_xchg(vtype[VVAL], AX);
					}
				}
			}
		vtype[VT]=CINT;
		}
	}

forcerm(vtype)
	int  vtype[]; {

	if (!floater(vtype)) {
		if (vtype[VIS] == OFFV && vtype[VVAL] != 8) {
			if (vtype[VNAME] == 0 && vtype[VOFF] == 0) {
				if (vtype[VVAL] == 4 || vtype[VVAL] == 5) {
					vtype[VIS]=REGV;
					vtype[VVAL]=vtype[VVAL]+2;
					return;
					}
				}
#if OPT
			if (is_reg(vtype)) return;
#endif
			force(vtype,ANYPAT);
			}
		else if (vtype[VT] >= CFLOAT && vtype[VT] < FUNCTION)
			force(vtype,ANYPAT);
		}
	}


forcelv(vtype)
	int  vtype[]; {
	if (vtype[VIS] != VARV) error("bad lvalue");
	}

forceacx(vtype,lnode)
	int vtype[],lnode[]; {
	int  segreg;

	if (lnode[VT] == CINT) forcel(lnode);
	if (lnode[VIS] == REGV && lnode[VVAL] == AX) {
		force(lnode,CXPAT);
		}
	if (vtype[VT] == PTRTO) {
		if (vtype[VIS] == REGV || vtype[VIS] == OFFV) {
			segreg=vtype[VMORE];
			vtype[VMORE]=-1;
			vtype[VT]=CINT;
			if (segreg != DX) {
				need(DXPAT);
				if (segreg == NEED_ES) {
					asm_move(toreg(DX),toseg(vtype[VNAME]));
					}
				else asm_move(toreg(DX),toreg2(segreg));
				regat[segreg]=0;
				regat[DX]=vtype;
				}
			force(vtype,AXPAT);
			}
		vtype[VT]=CLONG;
		}				
	force(vtype,AXPAT);
	forcel(vtype);
	if (lnode[VT] == PTRTO) {
		if (lnode[VIS] == REGV || lnode[VIS] == OFFV) {
			segreg=lnode[VMORE];
			lnode[VMORE]=-1;
			lnode[VT]=CINT;
			if (segreg != BX) {
				need(BXPAT);
				if (segreg == NEED_ES) {
					asm_move(toreg(BX),toseg(vtype[VNAME]));
					}
				else asm_move(toreg(BX),toreg2(segreg));
				regat[segreg]=0;
				regat[BX]=vtype;
				}
			force(lnode,CXPAT);
			}
		lnode[VT]=CLONG;
		}
	if (lnode[VIS] == CONSTV) forcel(lnode);
	force(lnode,CXPAT);
	forcel(lnode);
	}

forcel(vtype)
	int  vtype[]; {
	int  reg2;
	union {double dbl; long lng;};

	if (floater(vtype)) {

		if (vtype[VIS] == CONSTV) {
			((char *)(&vtype[VVAL]))->lng=((char *)(&vtype[VVAL]))->dbl;
			}
		else {
			loadf(vtype);
			needax();
			need(DXPAT);
			builtin(_FSTOREL);
			setax(vtype);
			regat[DX]=vtype;
			}
		vtype[VT]=CLONG;
		return;
		}

	if (vtype[VT] != CLONG) {
		if (vtype[VIS] != CONSTV) {
			force(vtype,AXCXPAT);
			if (vtype[VT] == CCHAR) {
				asm_move(toreglow(vtype[VVAL]+4),tocon(0));
				vtype[VT]=CINT;
				}
			else if (vtype[VT] == CSCHAR) {
				if(vtype[VVAL] == AX) asm_cbw();
				else {
					asm_xchg(vtype[VVAL], AX);
					asm_cbw();
					asm_xchg(vtype[VVAL], AX);
					}
				vtype[VT]=CINT;
				}
			reg2=vtype[VVAL]+2;
			if (vtype[VT] != PTRTO) need(regpat[reg2]);
			regat[reg2]=vtype;
			if (vtype[VT] == CINT) {
				if (reg2 == DX) asm_cwd();
				else {
					asm_xchg(AX,CX);
					asm_xchg(DX,BX);
					asm_cwd();
					asm_xchg(AX,CX);
					asm_xchg(DX,BX);
					}
				}
			else if (vtype[VT] == PTRTO) {
				asm_move(toreg(reg2),vtype);
				if (vtype[VMORE] == ES)	regat[ES]=0;
				}
			else {
				asm_add(XOR86,toreg(reg2),toreg2(reg2));
				}
			}
		else {
			vtype[VNAME]=0;
			if(vtype[VT] == CINT) if (vtype[VVAL] & 0x8000) vtype[VNAME]=-1;
			}
		vtype[VT]=CLONG;
		}
	}


/*	FORCE_REAL_ES -- MUST REALLY ADDRESS USING ES. */

force_real_es(vtype,is_float)
	int vtype[];
	char is_float; {
#if LIMITED == 0
	int  reg;

	forcees(vtype);
	if (vtype[VMORE] != ES) {
		needes();
		if (is_float == 0 && (vtype[VMORE] == -1 || vtype[VT] != PTRTO)) {
			reg=need(ANYPAT);
			asm_add(XOR86,toreg(reg),toreg2(reg));
			asm_move(tosegrv(ES),toreg(reg));
			}
		else if (vtype[VMORE] >= 8) {
			reg=need(ANYPAT);
			asm_move(toreg(reg),toreg2(vtype[VMORE]));
			asm_move(tosegrv(ES),toreg(reg));
			}
		else asm_move(tosegrv(ES),toreg(vtype[VMORE]));
		vtype[VMORE]=ES;
		regat[ES]=vtype;
		}
#endif
	}


/*	FORCEES  --	make the vtype addressable by one of the segment registers. */
/*				ES value must be in a register.	*/

forcees(vtype)
	int vtype[]; {
	int  reg;
#if LIMITED == 0

	if (vtype[VMORE] >= AX && vtype[VMORE] <= DI) {
		needes();
		asm_move(tosegrv(ES),toreg(vtype[VMORE]));
		regat[vtype[VMORE]]=0;
		regat[ES]=vtype;
		vtype[VMORE]=ES;
		}
	else if (vtype[VMORE] == NEED_ES) {
		needes();
		asm_move(tosegrv(ES),toptr(vtype[VNAME],2));
		regat[ES]=vtype;
		vtype[VMORE]=ES;
		}
#endif
	}



/*	FORCE  --	force a vtype into a register	*/

force(vtype,patern)
	int  vtype[],patern; {
	char reg;
	int  i, vval, temp, offtemp, oldvmore;

	if (vtype[VT] == CCHAR) patern&=BYTEPAT;
	else if (vtype[VT] == CLONG) {
		if (patern & AXCXPAT) patern&=AXCXPAT;
		else if (patern & INDPAT && is_big); /* leave it alone */
		else forceint(vtype);
		}
	else if (floater(vtype)) {
		loadf(vtype);
		needax();
		need(DXPAT);
		builtin(_FSTOREL);
		setax(vtype);
		vtype[VT]=CLONG;
		regat[DX]=vtype;
		}

	switch (vtype[VIS]) {
		case CONSTV:	movreg(need(patern),vtype);
						break;
		case OFFV:		if ((reg=regused[vtype[VVAL]]) != 8) {
							if (vtype[VNAME] == 0 && vtype[VOFF] == 0) {
								vtype[VIS]=REGV;
								vtype[VVAL]=reg;
								if ((patern & regpat[reg]) == 0) {
									movreg(need(patern),vtype);
									}
								}
							else {
								oldvmore=vtype[VMORE];
								vtype[VMORE]=-1;
								if ((patern & regpat[reg]) == 0) {
									reg=need(patern);
									freev(vtype);
									}
								vtype[VIS]=VARV;
								asm_lea(reg,vtype);
								vtype[VIS]=REGV;
								vtype[VVAL]=reg;
								vtype[VOFF] = 0;
								regat[reg]=vtype;
								vtype[VMORE]=oldvmore;
								if (oldvmore == NEED_ES) forcees(vtype);
								}
							}
						else {
							reg=need(patern);
							regat[reg]=vtype;
							if (vtype[VVAL] == 6) {
								vtype[VMORE] = -1;
								vtype[VIS]=VARV;
								vtype[VFROM]=0;
								asm_lea(reg,vtype);
								vtype[VIS]=REGV;
								vtype[VVAL]=reg;
								vtype[VOFF] = 0;
								regat[reg]=vtype;
								vtype[VMORE] = SS;
								}
							else if (vtype[VMORE] == NEED_ES) {
								/* address of an external symbol */
								if (regat[ES] != vtype) needes();
								vtype[VIS] = VARV;
								temp = vtype[VT];
								offtemp = vtype[VOFF];
								vtype[VOFF]=0;
								vtype[VT] = CFOREIGN;
								vtype[VMORE] = -1;
								asm_les(reg,vtype);
								vtype[VT] = temp;
								if (offtemp) {
									asm_add(ADD86,toreg(reg),tocon(offtemp));
									}
								vtype[VIS]=REGV;
								vtype[VVAL]=reg;
								vtype[VNAME] = vtype[VOFF] = 0;
								regat[reg]=regat[ES]=vtype;
								vtype[VMORE]=ES;
								}
							else {
								movreg(reg,vtype);
								}
							}
						break;
		case REGV:		if ((regpat[vtype[VVAL]] & patern) == 0)
							movreg(need(patern),vtype);
						break;
		case VARV:
#if OPT
						if (is_reg(vtype)) return;
#endif
						reg=regused[vtype[VVAL]];
						if (reg == 8 ||	(patern & regpat[reg]) == 0)
							reg=need(patern);
						movreg(reg,vtype);
#if OPT
						reg_from[reg]=0;
						if (vtype[VIS] == VARV && vtype[VFROM]
									&& (tree[vtype[VFROM]+3] & 255)) {
							reg_from[reg]=vtype[VFROM];
							reg_dirty[reg]=0;
							reg_weight[reg]=(tree[vtype[VFROM]+3] & 255) + down+20;
							}
#endif
		}
	}

/*	MOVREG  --	mova a vtype into a register.	*/
	
movreg(reg,vtype)
	char  reg;
	int   vtype[]; {
	int  reg2,dest_es;

	regat[reg]=vtype;
	if (is_big && vtype[VIS] == VARV) forcees(vtype);
	dest_es=-1;
	if (vtype[VT] == CLONG) {
		if (regpat[reg] & INDPAT) {
			dest_es=need(ANYPAT);
			asm_move(toreg(dest_es),tormh(vtype));
			}
		else {
			reg2=(reg==AX) ? DX: BX;
			need(regpat[reg2]);
			asm_move(toreg(reg2),tormh(vtype));
			regat[reg2]=vtype;
			}
		}
	if (vtype[VT] == PTRTO && vtype[VIS] == VARV) {
		dest_es=ES;
		if (regat[ES] != vtype)	needes();
		else if (in_post) {
			dest_es=need(ANYPAT);
			asm_move(toreg(dest_es),tormh(vtype));
			}
		if (dest_es == ES) {
			vtype[VT] = PTRTO+1;
			asm_les(reg,vtype);
			}
		}

	if (dest_es != ES) {
		asm_move((vtype[VT] == CCHAR || vtype[VT] == CSCHAR ? toreglow(reg) : toreg(reg)),vtype);
		if(vtype[VT] == CSCHAR) {
			if(reg == AX) asm_cbw();
			else {
				asm_xchg(reg, AX);
				asm_cbw();
				asm_xchg(reg, AX);
				}
			}
		}
	if (vtype[VMORE] < AX || vtype[VMORE] > ES || vtype[VIS] != REGV) freev(vtype);
	else regat[vtype[VVAL]]=0;
	if (dest_es != -1) {
		vtype[VT]=PTRTO;
		vtype[VMORE]=dest_es;
		regat[dest_es]=vtype;
		}
	vtype[VIS]=REGV;
	vtype[VVAL]=reg;
	vtype[VOFF]=0;
	regat[reg]=vtype;
	if (vtype[VT] >= BITS && vtype[VT] < FUNCTION) getbits(vtype);
	}


getbits(vtype)
	int  vtype[]; {
	char dtype;
	char i,shr,bits;
	int  mask;

	dtype=vtype[VT]-BITS;
	vtype[VT]=CUNSG;
	shr=dtype&15;
	if (shr) {
		if (shr > 1 && regat[CX] == 0) {
			asm_move(toreglow(CL),tocon(shr));
			asm_shift_cl(SHR86,vtype);
			}
		else {
			while (shr--) {
				asm_shift_1(SHR86,vtype);
				}
			}
		}
	bits=(dtype>>4)+1;
	if (bits < 16) {
		mask=0;
		for (i=0; i < bits; i++)
			mask+=mask+1;
		asm_add(AND86,vtype,tocon(mask));
		}
	}

needzreg(vtype)
	int  vtype[]; {
	char reg;

	reg=need(ANYPAT);
	asm_add(XOR86,toreg(reg),toreg2(reg));
	vtype[VIS]=REGV;
	vtype[VVAL]=reg;
	vtype[VT]=CINT;
	regat[reg]=vtype;
	}

needax() {
	need(AXPAT);
	}

/*	NEEDES  --	must free up ES register.	*/

needes() {
	int  reg,*wp;
	char old_prefer;
#if LIMITED == 0

	if (regat[ES] == 0) return;
	old_prefer=prefer_si;
	prefer_si=0;
	reg=need(ANYPAT-SIPAT);		/* dont use si to prevent lockups. */
	prefer_si=old_prefer;
	asm_move(toreg(reg),tosegrv(ES));
	wp=regat[ES];
	wp[VMORE]=reg;
	regat[reg]=wp;
	regat[ES]=0;
#endif
	}


need(patern)
	int  patern; {
	char rneed,reg;
	int  *vat,vatis,vtype,savevis,savetype,savemore,savename;

still_need:
	rneed=-1;

	/*	see if prefer to use si to ax	*/
	if (prefer_si) {
		if ((SIPAT & patern) && regat[SI] == 0
#if OPT
			&& reg_from[SI] == 0
#endif
			) return SI;
		if ((DIPAT & patern) && regat[DI] == 0
#if OPT
			&& reg_from[DI] == 0
#endif
			) return DI;
		}

	for (reg=0; reg < 8; reg++) {
		if (regpat[reg] & patern) {
			if (regat[reg] == 0
#if OPT
				&& reg_from[reg] == 0
#endif
				) return reg;
			rneed=reg;
			}
		}
	if (rneed == -1) error("error in register allocation");
	vat=regat[rneed];
	if (vat[VT] != CLONG || vat[VIS] != REGV) {
		reg=7;
		do {
			if (regat[reg] == 0 && reg != 4 && reg != 5
#if OPT
				&& reg_from[reg] == 0
#endif
				) {
				if (vat[VMORE] == rneed) {
					asm_move(toreg(reg),toreg2(rneed));
					vat[VMORE]=reg;
					regat[reg]=vat;
					regat[rneed]=0;
					return rneed;
					}
				if (vat[VT] == CCHAR && reg > BX) continue;
				if (vat[VIS] == OFFV || vat[VIS] == VARV) {
					if (reg != SI && reg != DI && reg != BX) continue;
					if (reg == BX) {
						vatis=vat[VIS];
						vat[VIS]=VARV;
						savemore=vat[VMORE];
						vat[VMORE]=-1;
						vat[VFROM]=0;
						asm_lea(BX,vat);
						freev(vat);
						regat[BX]=vat;
						vat[VIS]=vatis;
						vat[VVAL]=7;
						vat[VMORE]=savemore;
						if (savemore == NEED_ES) forcees(vat);
						vat[VNAME]=vat[VOFF]=0;
						return rneed;
						}
					}

				regat[reg]=vat;
				if (vat[VIS] == VARV || vat[VIS] == OFFV) {
					asm_move(toreg(reg),toreg2(regused[vat[VVAL]]));
					regat[regused[vat[VVAL]]]=0;
					if (vat[VVAL] == 7) {	/* is a [bx]	*/
						vat[VVAL]=reg-2; 	/* now [si] or [di] */
						}
					else if (reg == SI) vat[VVAL]--;
					else vat[VVAL]++;
					return rneed;
					}

				asm_move((vat[VT] == CCHAR ? toreglow(reg) : toreg(reg)),vat);
				freev(vat);
				vat[VIS]=REGV;
				vat[VVAL]=reg;
				return rneed;
				}
			}
		while (reg--);
		}
#if OPT
	for (reg=0; reg < 8; reg++) {
		if (regat[reg] == 0	&& reg_from[reg]) {
			reg_dirty[reg]=0;	/* abandon with prejudice	*/
			abandon(reg);
			goto still_need;
			}
		}
#endif
	pushv(vat,1);
	return rneed;
	}

pushv(vtype,need)
	int  vtype[];
	char need; {

	if (floater(vtype)) {
		pushfp(vtype);
		return;
		}

	if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) forceint(vtype);
	forcemul(vtype);
	if (is_big) forcees(vtype);
	if (vtype[VT] == CLONG || vtype[VT] == PTRTO) {
		if (vtype[VT] == PTRTO && vtype[VMORE] == NEED_ES) forcees(vtype);
		asm_push(PUSH86,tormh(vtype));
		curoff+=2;
		}
	asm_push(PUSH86,vtype);
	freev(vtype);
	curoff+=2;
	if (need) {
		vtype[VIS]=VARV;
		vtype[VVAL]=6;
		vtype[VNAME]=0;
		vtype[VOFF]=-curoff;
		vtype[VMORE]=is_big ? SS : -1;
		}
	}


/*	push a floating value	*/

pushfp(vtype)
	int  vtype[]; {

	if (vtype[VT] == CDOUBLE && vtype[VIS] == CONSTV) {
		getsiof(vtype);
		asm_push_essi(6);
		asm_push_essi(4);
		asm_push_essi(2);
		asm_push_essi(0);
		}
	else {
		loadf(vtype);
		builtin(_FPUSH);
		}
	freev(vtype);
	vtype[VIS]=FLOATV;
	vtype[VT]=CDOUBLE;
	curoff+=8;
	}


/*	load and store from the floating point stack.	*/

/*	load a number onto the floating point stack.	*/

loadf(vtype)
	int  vtype[]; {

	if (vtype[VIS] != FLOATV) {
		if (vtype[VT] == CFLOAT) {
			getsiof(vtype);
			builtin(_FLOADE);
			}
		else if (vtype[VT] == CDOUBLE) {
			getsiof(vtype);
			builtin(_FLOADD);
			}
		else {
			force(vtype,AXPAT);
			forcel(vtype);
			builtin(_FLOADL);
			}
		freev(vtype);
		vtype[VIS]=FLOATV;
		vtype[VT]=CDOUBLE;
		}
	}



/*	get SI to point to a floating point number.	*/

getsiof(vtype)
	int  vtype[]; {

	int  typ;
	if (vtype[VIS] == CONSTV) {
		if (is_big) needes();
		need(SIPAT);
#if OPT
		if (perfect) {
#endif
			wantdseg();
			if (zopt) {
				asmext=--lastlab;
				asm_dw();
				codew(vtype[VVAL]);
				codew(vtype[VNAME]);
				if (vtype[VT] == CDOUBLE) {
					codew(vtype[VOFF]);
					codew(vtype[VMORE]);
					}
				}
			else {
				os("_F");
				onum(++numfloat);
				os(" DW ");
				oword(vtype[VVAL]);
				ocomma();
				oword(vtype[VNAME]);
				if (vtype[VT] == CDOUBLE) {
					ocomma();
					oword(vtype[VOFF]);
					ocomma();
					oword(vtype[VMORE]);
					}
				olf();
				}
			wantcseg();
			if (is_big) {
				asm_move(toreg(SI),tosegrv(DS));
				asm_move(tosegrv(ES),toreg(SI));
				}

			if (zopt) asm_move(toreg(SI),tooff(lastlab));
			else {
				os(" MOV SI,OFFSET _F");
				onum(numfloat);
				olf();
				}
#if OPT
			}
#endif
		vtype[VIS]=OFFV;
		vtype[VVAL]=4;			/* variable at [si]	*/
		vtype[VNAME]=vtype[VOFF]=0;
		if (is_big) vtype[VMORE]=ES; else vtype[VMORE]=-1;
		}
	else if (vtype[VIS] == VARV) {
		typ=vtype[VT];
		vtype[VIS]=OFFV;
		force(vtype,SIPAT);
		vtype[VIS]=VARV;
		vtype[VVAL]=4;			/* variable at [si] */
		vtype[VNAME]=vtype[VOFF]=0;
		vtype[VT]=typ;
		if (is_big) force_real_es(vtype,1); else vtype[VMORE]=-1;
		}
	else error("cannot address");
	}


/*	store a floating point number from the floating stack.	*/

storef(vtype)
	int  vtype[]; {
	int  typ;

	if (vtype[VIS] == VARV || vtype[VIS] == REGV) {
		typ=vtype[VT];
		if (vtype[VIS] == VARV) vtype[VIS]=OFFV;
		force(vtype,SIPAT);
		if (is_big) force_real_es(vtype,1);
		if (vtype[VT] == CFLOAT) builtin(_FSTOREE);
		else builtin(_FSTORED);
		vtype[VIS]=VARV;
		vtype[VVAL]=4;			/* variable at [si] */
		vtype[VNAME]=vtype[VOFF]=0;
		vtype[VT]=typ;
		vtype[VFROM]=0;
		}
	else error("cannot address");
	}


/*	copy a floating point variable to the floating point stack.
	only used by ++ and -- so variable is a lvalue.	*/

copyf(vtype)
	int  vtype[]; {
	int  typ;

	if (vtype[VIS] == VARV) {
		typ=vtype[VT];
		vtype[VIS]=OFFV;
		force(vtype,SIPAT);
		if (is_big) {
			if (vtype[VMORE] != ES) {
				needes();
				asm_push(PUSH86,toreg(vtype[VMORE]));
				asm_push(POP86,tosegrv(ES));
				regat[vtype[VMORE]]=0;
				vtype[VMORE]=ES;
				regat[ES]=vtype;
				}
			}
		if (vtype[VT] == CFLOAT) builtin(_FLOADE);
		else builtin(_FLOADD);
		vtype[VIS]=VARV;
		vtype[VVAL]=4;			/* variable at [si] */
		vtype[VNAME]=vtype[VOFF]=0;
		vtype[VT]=typ;
		vtype[VFROM]=0;
		}
	else error("cannot address");
	}



/*	set vtype to describe AX	*/

setax(vtype)
	int  vtype[]; {

	vtype[VIS]=REGV;
	vtype[VVAL]=AX;
	vtype[VT]=CINT;
	regat[AX]=vtype;
	}

#if OPT

/*	IS_REG	--	return true if vtype is a variable in a register	*/

is_reg(vtype)
	int vtype[]; {
	int i;
	int  vnode,regnode;

	if (vtype[VFROM] && vtype[VIS] == VARV && regused[vtype[VVAL]] == 8) {

		for (i=0; i < 8; i++) {
			if (reg_from[i]) {
				vnode=vtype[VFROM];
				regnode=reg_from[i];
				if (regnode < 0) regnode=tree[-regnode];
				if (tree[vnode+1] == tree[regnode+1] &&
					tree[vnode+2] == tree[regnode+2] &&
					tree[vnode] == tree[regnode]) {

					vtype[VIS]=REGV;
					vtype[VVAL]=i;
					if (tree[vnode+3] & 255) {
						reg_weight[i]=(tree[vnode+3] & 255) + down + 20;
						if (reg_from[i] > 0) {
							reg_from[i]=vnode;
							reg_dirty[i]&=DIRTY;
							}
						else reg_dirty[i] |=USED;
						}
					else {
						reg_dirty[i]|=USED;	/* dont abandon- could be asgn	*/
						reg_weight[i]=30000;
						}
					return 1;
					}
				}
			}
		}
	return 0;
	}
#endif


/*	OFRM	--	output and free a R/M	*/

ofrm(vtype)
	int  vtype[]; {
	freev(vtype);
	orm(vtype);
	}

orm(vtype)
	int  vtype[]; {
	char byt;
	int vval,esreg,segover;
	
	byt=vtype[VT]==CCHAR || vtype[VT] == CSCHAR;
	switch(vtype[VIS]) {
	  case CONSTV:
		onum(vtype[VVAL]);
		break;
	  case REGV:
		if (byt) {
			olow(vtype[VVAL]);
			}
		else     oreg(vtype[VVAL]);
		break;
	 case SEGRV:
		if (vtype[VVAL] == ES) os("ES");
		else if (vtype[VVAL] == DS) os("DS");
		else if (vtype[VVAL] == SS) os("SS");
		else os("CS");
		break;
	  default:
	  	vval = vtype[VVAL];
		if (is_big) {
			if (vtype[VT] == CFOREIGN) {
				os("@");
				oname(vtype[VNAME]);
				if (vtype[VOFF]) {
					oc('+');
					onum(vtype[VOFF]);
					}
				return;
				}
			else if (vtype[VT] == CSEGMENT) {
				os(" SEG ");
				oname(vtype[VNAME]);
				return;
				}
			}
		if (vtype[VIS] == OFFV) os("OFFSET ");
		else {
			if (is_big) {
				segover=vtype[VMORE];
				if (segover == -1) goto no_pre;
				if (vtype[VVAL] <= 3 || vtype[VVAL] == 6) {
					if (segover == SS) goto no_pre;
					}
				else if (segover == DS) goto no_pre;
				oreg(segover);
				oc(':');
				}
no_pre:
			if (vtype[VT] == PTRTO+1) ;
			else if (byt) os("BYTE ");
			else     os("WORD ");
			}
		if (vtype[VNAME]) oname(vtype[VNAME]);
		if(vtype[VVAL] != 8) {
			oc('[');
			switch(vtype[VVAL]) {
				case 2:	os("BP+SI");
						break;
				case 3:	os("BP+DI");
						break;
				case 4:	os("SI");
						break;
				case 5:	os("DI");
						break;
				case 6:	os("BP");
						break;
				case 7:	os("BX");
						break;
				}
			if(vtype[VOFF]) {
				if (vtype[VOFF] > 0) oc('+');
				onum(vtype[VOFF]);
				}
			oc(']');
			}
		else if (vtype[VOFF]) {
			oc('[');
			onum(vtype[VOFF]);
			oc(']');
			}
		}
	}

ormh(vtype)
	int  vtype[]; {

	if (vtype[VIS] == CONSTV) oword(vtype[VNAME]);
	else if (vtype[VIS] == REGV) {
		if (is_big && vtype[VT] == PTRTO) oreg(vtype[VMORE]);
		else oreg(vtype[VVAL]+2);
		}
	else if (vtype[VIS] == OFFV && vtype[VMORE] != -1) {
		oreg(vtype[VMORE]);
		}
	else {
		vtype[VOFF]+=2;
		orm(vtype);
		vtype[VOFF]-=2;
		}
	}

oddrm(vtype)
	int  vtype[]; {

	if (vtype[VNAME]) oname(vtype[VNAME]);
	if (vtype[VOFF]) {
		oc('[');
		onum(vtype[VOFF]);
		oc(']');
		}
	}

oname(ord)
	int  ord; {
	if (*((char *)(extat[ord]+1))) {
		os(extat[ord]+1);
		oc('_');
		}
	else {
		os("__");
		onum(ord);
		}
	}

oreg(reg)
	char reg; {
	os(regname[reg]);
	}

olow(reg)
	char reg; {
	os(reglow[reg]);
	}

ohigh(reg)
	char reg; {
	os(reghigh[reg]);
	}

freev(vtype)
	int  vtype[]; {
	int  reg,v;
	v=vtype[VIS];
	if (v == REGV) {
		if (vtype[VT] == CLONG) regat[(vtype[VVAL] == AX)? DX: BX]=0;
		else if (vtype[VMORE] != -1 && regat[vtype[VMORE]] == vtype)
			regat[vtype[VMORE]] = 0;
		regat[vtype[VVAL]]=0;
		}
	else if (v == VARV || v == OFFV) {
		if (vtype[VMORE] != -1) regat[vtype[VMORE]] = 0;
		v=regused[vtype[VVAL]];
		if (v < 8) regat[v]=0;
		}
	}

/*	ABANDON_ALL  --	call or goto or return.	*/
/*					every register may be different	*/

abandon_all() {
#if OPT
	int  i;

	for (i=0; i < 8; i++)
		if (reg_from[i]) abandon(i);
#endif
	}




/*	ALTERV  --	a vtype is about to be changed. see if can preserve
				a wanted register.	*/

alterv(vtype)
	int  vtype[]; {

#if OPT
	int  reg,oldreg,weight,node;

	if (vtype[VIS] == REGV && reg_from[vtype[VVAL]]) {

		/*	the register is changed		*/
		oldreg=vtype[VVAL];
		reg=want_reg(reg_weight[oldreg],vtype[VT] == CCHAR);
		if (reg != -1) {
			asm_move((vtype[VT] == CCHAR ? toreglow(reg) : toreg(reg)),vtype);
			reg_from[reg]=reg_from[oldreg];
			reg_from[oldreg]=0;
			reg_dirty[reg]=reg_dirty[oldreg] | USED;
			reg_dirty[oldreg]=0;
			reg_weight[reg]=reg_weight[oldreg];
			return;
			}
		reg_dirty[oldreg]|=USED;	/* abandon without prejudice	*/
		abandon(oldreg);
		}
#endif
	}


/*	WANT_REG  --	would like a register for a variable
					return free register number or -1 if none. */

#if OPT
want_reg(weight,is_char)
	int  weight;
	char is_char; {
	int  reg,oldreg,weighti,worsti;

	weighti=weight;
	worsti=-1;
	for (reg=0; reg < 8; reg++) {
		if (reg >= 4) {
			if (is_char) break;
			if (reg <= 5) continue;
			}
		if (regat[reg] == 0) {
			if (reg_from[reg] == 0) {
				reg_weight[reg]=weight;
				return reg;
				}
			if (weighti < reg_weight[reg]) {
				weighti=reg_weight[reg];
				worsti=reg;
				}
			}
		}
	if (worsti != -1) {
		abandon(worsti);
		reg_weight[worsti]=weight;
		}
	return worsti;
	}
#endif



/*	ABANDON  -- the register used by the vtype may be changed	*/

abandon(reg)
	int  reg; {

#if OPT
	char *cptr;
	int vtype[7],node;

	if (reg_from[reg]) {
		node=reg_from[reg];
		if (node < 0) node=tree[-node];
		if (reg_dirty[reg] & DIRTY) {
			node_to_vtype(node,vtype);
			asm_move(vtype,vtype[VT] == CCHAR ? toreglow(reg) : toreg(reg));
			reg_dirty[reg]=0;
			}
		if (reg_dirty[reg] == 0) {
			better=1;
			if (reg_from[reg] < 0) tree[-reg_from[reg]]=0;
			else {
				cptr=&tree[reg_from[reg]+3];
				if (*cptr) *cptr=0;
				else better=0;
				}
			}
		reg_from[reg]=0;
		}
#endif
	}



/*	NODE_TO_VTYPE  --	load a variable  into a vtype	*/

#if OPT

node_to_vtype(node,vtype)
	int  node,vtype[7]; {

	vtype[VIS]=VARV;
	vtype[VNAME]=tree[node+1];
	vtype[VOFF]=tree[node+2];
	if (vtype[VNAME]) vtype[VVAL]=8;
	else vtype[VVAL]=6;
	vtype[VT]=tree[node] >> 8;
	vtype[VFROM]=0;	if (tree[node+3]) vtype[VFROM]=node;
	}
#endif



builtin(n)
	int  n; {

	if (bltused[n] == 0) {
		asm_public(1970+n,bltname[n],0);
		bltused[n]=1;
		}
	if (zopt) {
		if (is_big) asm_lc(1970+n);
		else asm_jump(CALL86,1970+n);
		}
	else {
		if (is_big) os(" LCALL ");
		else os(" CALL ");
		os(bltname[n]);
		olf();
		}
	}
