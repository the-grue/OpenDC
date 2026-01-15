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
/*	GEN3.C	part 3 of generation	*/

#include "PASS2.H"
#include "NODES.H"
#include "inst86.h"

genfun() {
	if (zopt) asm_label(funord);
	else {
		olf();
		oname(funord);
		oc(':');
		}
	asm_push(PUSH86,toreg(BP));
	asm_move(toreg(BP),toreg2(SP));
	if (locoff) {
		asm_add(SUB86,toreg(SP),tocon(locoff));
		}
	genstmt(pstart);
	}

genunary(node,vtype)
	int  node,vtype[]; {
	char type,reg2;

	type=tree[node];
	getval(tree[node+1],vtype);
	if (vtype[VIS] == CONSTV && vtype[VT] < CLONG) {
		switch (type) {
			case NEG:	vtype[VVAL]=-vtype[VVAL];
						break;
			case NOT:	vtype[VVAL]=vtype[VVAL] ? 0: 1;
						break;
			case COMP:	vtype[VVAL]=-vtype[VVAL]-1;
			}
		if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) vtype[VT]=CINT;
		}
	else {
		if (floater(vtype)) {
			loadf(vtype);
			if (type == NEG) {
				builtin(_FNEG);
				}
			else {
				needax();
				builtin(_FNOT);
				setax(vtype);
				}
			return;
			}
		if (is_big && vtype[VT] == PTRTO && type == NOT) forcel(vtype);
		forcereg(vtype);
		alterv(vtype);
		if (type == NOT) {
			asm_add(OR86,vtype,vtype);
			asm_move_noopt(vtype,tocon(0));
			asm_jump(JNZ86,++nextlab);
			if (vtype[VT] == CLONG) {
				reg2=vtype[VVAL]+2;
				asm_add(OR86,toreg(reg2),toreg(reg2));
				asm_jump(JNZ86,nextlab);
				regat[reg2]=0;
				vtype[VT]=CINT;
				}
			asm_inc(INC86,vtype);
			genlab(nextlab);
			}
		else if (vtype[VT] == CLONG) {
			reg2=vtype[VVAL]+2;
			asm_not(NOT86,toreg(reg2));
			if (type == COMP) {
				asm_not(NOT86,vtype);
				}
			else {
				asm_not(NEG86,vtype);
				asm_add(SBB86,toreg(reg2),tocon(-1));
				}
			}
		else {
			asm_not((type == NEG ? NEG86: NOT86),vtype);
			}
		}
	}

genpre(node,vtype)
	int  node,vtype[]; {

	getval(tree[node+1],vtype);
/*	if /= then may not want pre increment or decrement	*/
	if (nopre) return;
	if (floater(vtype)) {
		copyf(vtype);
		if (tree[node] == PREI) builtin(_FINC);
		else builtin(_FDEC);
		storef(vtype);
		}
	else {
#if OPT
		if (vtype[VIS] == REGV) reg_dirty[vtype[VVAL]]|=DIRTY;
#endif
		iord(tree[node],vtype,tree[node+2]);
		}
	}

genpost(node,vtype)
	int  node,vtype[]; {
	int  lvalue[7];
	int  pat;
	char i,reg,mustmove;

	getval(tree[node+1],vtype);
/*	if /= then may not want post increment or decrement	*/
	if (nopost) return;
	if (floater(vtype)) {
		copyf(vtype);
		copyf(vtype);
		if (tree[node] == POSTI) builtin(_FINC);
		else builtin(_FDEC);
		storef(vtype);
		freev(vtype);
		vtype[VIS]=FLOATV;
		vtype[VT]=CDOUBLE;
		}
	else {
	 	for (i=0; i < 7; i++)
			lvalue[i]=vtype[i];
		pat=ANYPAT;
		if (vtype[VIS] == VARV && vtype[VVAL] < 6) pat-=INDPAT;
		in_post=1;
		force(vtype,pat);
		in_post=0;
		iord(tree[node]-2,lvalue,tree[node+2]);
		freev(lvalue);
		}
	}


iord(pretype,vtype,by)
	int  vtype[],by;
	char pretype; {
	int  lvalue[7];
	char i,isbits;

	isbits=0;
	if (vtype[VT] >= BITS && vtype[VT] < FUNCTION) {
		for (i=0; i < 7; i++)
			lvalue[i]=vtype[i];
		force(vtype,AXCXPAT);
		isbits=1;
		}
	if (is_big) forcees(vtype);
	if (pretype == PREI) {
		if (by == 1 && vtype[VT] != CLONG) {
			asm_inc(INC86,vtype);
			}
		else {
			asm_add(ADD86,vtype,tocon(by));
			}
		if (vtype[VT] == CLONG) {
			asm_add(ADC86,tormh(vtype),tocon(0));
			}
		}

	else {
		if (by == 1 && vtype[VT] != CLONG) {
			alterv(vtype);
			asm_inc(DEC86,vtype);
			}
		else {
			alterv(vtype);
			asm_add(SUB86,vtype,tocon(by));
			}
		if (vtype[VT] == CLONG) {
			asm_add(SBB86,tormh(vtype),tocon(0));
			}
		}
	if (isbits)
		putbits(lvalue,vtype);
	}

genfcal(node,vtype)
	int  node,vtype[]; {
	int  i,j,old_at[9],args,struct_len,old_from[9],old_weight[9];
	char rettype,*struct_off,old_dirty[9];

	rettype=tree[node]>>8;
	if (rettype == PTRTO) {
		need(SIPAT);
		needes();
		}
	else needax();
	if (rettype == CLONG) need(DXPAT);

	/* if returning a structure, reserve stack space and point to area
		as first argument	*/
	struct_len=tree[node+3];
	if (struct_len & 1) struct_len++;
	if (struct_len) {
		asm_add(SUB86,toreg(SP),tocon(struct_len));
		curoff+=struct_len;
		struct_off=curoff;
		}
	for (i=0; i < 9; i++) {
		if (old_at[i]=regat[i]) {
			regat[i]=0;
#if OPT
			old_from[i]=reg_from[i];
			reg_from[i]=0;
			old_dirty[i]=reg_dirty[i];
			old_weight[i]=reg_weight[i];
#endif
			asm_push(PUSH86,toreg(i));
			curoff+=2;
			}
		}
	args=curoff;
	if (tree[node+2]) 	getargs(tree[node+2]);
	if (struct_len) {
		if (is_big) {
			asm_push(PUSH86,tosegrv(SS));
			curoff+=2;
			}
		asm_lea(AX,tobpm(struct_off));
		asm_push(PUSH86,toreg(AX));
		curoff+=2;
		}
	args=curoff-args;

	/* if indirect call, must ignore IND as CALL is implied contents of */
	if ((tree[tree[node+1]] &255) == IND) getval(tree[tree[node+1]+1],vtype);
	else getval(tree[node+1],vtype);
	forcerm(vtype);
	if (is_big) {
		if (vtype[VIS] == OFFV || vtype[VIS] == VARV) {
			if (vtype[VIS] == OFFV) {
				if (zopt) asm_lcall(vtype);
				else {
					os(" LCALL ");
					oname(vtype[VNAME]);
					olf();
					}
				}
			else {
				forcees(vtype);
				if (zopt) {
					asm_lcall(vtype);
					freev(vtype);
					}
				else {
					os(" LCALL ");
					os("DWORD ");
					vtype[VT] = PTRTO+1;	/* so orm knows it's a DWORD argument */
					ofrm(vtype);
					olf();
					}
				}
			}
		else {
			pushv(vtype,0);
			curoff-=4;			/* will remove from stack	*/
			asm_lret();
			}
		}
	else {
#if OPT
		abandon_all();
#endif
		asm_call(vtype);
		}

/*	clean parameters off the stack	*/
	if (args) {
		if (curoff == args) {
			asm_move(toreg(SP),toreg2(BP));
			curoff=0;
			}
		else {
			asm_add(ADD86,toreg(SP),tocon(args));
			curoff-=args;
			}
		}

/*	restore registers preserved on stack	*/

	for (i=8; i > 0; i--) {
		if (old_at[i]) {
			regat[i]=old_at[i];
#if OPT
			reg_from[i]=old_from[i];
			reg_dirty[i]=old_dirty[i];
			reg_weight[i]=old_weight[i];
#endif
			asm_push(POP86,toreg(i));
			curoff-=2;
			}
		}

	vtype[VMORE]=-1;
	if (struct_len) {
		vtype[VIS]=OFFV;
		vtype[VVAL]=6;
		vtype[VNAME]=0;
		vtype[VOFF]=-struct_off;
		vtype[VT]=CSTRUCT;
		vtype[VFROM]=0;
		}
	else if (rettype == CFLOAT || rettype == CDOUBLE) {
		vtype[VIS]=FLOATV;
		vtype[VT]=CDOUBLE;
		}
	else {
		vtype[VIS]=REGV;
		if (rettype == PTRTO) {
			vtype[VVAL]=SI;
			regat[SI]=vtype;
			regat[ES]=vtype;
			vtype[VMORE]=ES;
			}
		else {
			vtype[VVAL]=AX;
			regat[AX]=vtype;
			if (rettype == CLONG) regat[DX]=vtype;
			}
		vtype[VT]=rettype;
		}
	}

getargs(node)
	int  node; {
	int  vtype[7],cur_loc,struct_len;
	char type;

	type=tree[node];
	if (type >= LST+2 && type <= LST+6)
		dolst(node,getargs);
	else {
		cur_loc=curoff;
		if (tree[node] == MOVE) {
			getval(tree[node+1],vtype);
			if (vtype[VIS] != OFFV || vtype[VVAL] != 6 || vtype[VOFF] >= -cur_loc) {
				struct_len=tree[node+3];
				if (struct_len & 1) struct_len++;
				curoff+=struct_len;
			/* allocate structure */
				asm_add(SUB86,toreg(SP),tocon(struct_len));
				asm_move(toreg(AX),toreg2(SP));
				if (is_big) asm_push(PUSH86,tosegrv(SS));
				asm_push(PUSH86,toreg(AX));
										/* push source address	*/
				pushv(vtype,0);
				asm_move(toreg(AX),tocon(tree[node+3]));	/* push length of structure */
				asm_push(PUSH86,toreg(AX));
				builtin(_MOVE);
				asm_add(ADD86,toreg(SP),tocon(is_big ? 10:6));
				curoff-=is_big ? 4: 2;
				}
			else freev(vtype);
			}
		else {
			getval(node,vtype);
			/*	clear off any temporaries */
			if (curoff != cur_loc) {
				if (vtype[VIS] == VARV & vtype[VVAL] == 6) {
					if (vtype[VT] == CDOUBLE || vtype == CFLOAT)
						loadf(vtype);
					else forcereg(vtype);
					}
				asm_add(ADD86,toreg(SP),tocon(curoff-cur_loc));
				curoff=cur_loc;
				}
			pushv(vtype,0);
			}
		}
	}

/*	if bit or FLOAT destination, only ASGN allowed */

genasgn(node,vtype)
	int  node,vtype[]; {
	int  lnode[7],nreg;
	char i,type,reg,vt_type,is_zero;
	unsigned rm2;

	type=tree[node];

/*	if MUL etc or float then get the result now. */
	vt_type=tree[tree[node+1]]>>8;
	if (type != ASGN && (vt_type == CFLOAT || vt_type == CDOUBLE || 
		(type >= AMUL && type <= ASHL))) {
		tree[node]-=AMUL-MUL;

/*	do not allow post increment or decrement yet	*/
		nopost=1;
		getval(node,vtype);
		nopost=0;
		tree[node]+=AMUL-MUL;
		forcerm(vtype);
		nopre=1;
		isasgn=1;
		getval(tree[node+1],lnode);
		isasgn=0;
		nopre=0;
		}

	else {
		getval(tree[node+2],vtype);
		forcerm(vtype);
		isasgn=1;
		getval(tree[node+1],lnode);
		isasgn=0;
		}
	if (floater(lnode)) {
		loadf(vtype);
		storef(lnode);
		flip(lnode,vtype);
		freev(lnode);
		return;
		}
	if (vtype[VIS] == VARV) forcereg(vtype);
	if (lnode[VT] >= BITS && lnode[VT] < FUNCTION) {
		putbits(lnode,vtype);
		return;
		}
	if (floater(vtype)) forcel(vtype);
	if (vtype[VIS] == CONSTV) {
		if (lnode[VT] == CLONG || lnode[VT] == PTRTO) forcel(vtype);
		else vtype[VT]=lnode[VT];
		}
	if (lnode[VT] == CCHAR || lnode[VT] == CSCHAR) {
		if (vtype[VT] != CCHAR && vtype[VT] != CSCHAR) forcebyt(vtype);
		}
	else if (vtype[VT] == CCHAR) {
		forceint(vtype);
		vtype[VT]=CUNSG;
		}
	else if (vtype[VT] == CSCHAR) {
		forceint(vtype);
		vtype[VT]=CINT;
		}
	if (lnode[VT] == CLONG && (vtype[VT] == CINT || vtype[VT] == PTRTO))
		forcel(vtype);

	if (is_big)  {
		if (lnode[VT] == PTRTO  && type == ASGN && vtype[VT] != PTRTO) {
			if (vtype[VT] != CLONG) error("illegal assignment");
			}
		forcees(lnode);
		}
	switch (type) {
		case AADD:	asm_add(ADD86,lnode,vtype);
					break;
		case ASUB:  asm_add(SUB86,lnode,vtype);
					break;
		case AOR:	asm_add(OR86,lnode,vtype);
					break;
		case AAND:	asm_add(AND86,lnode,vtype);
					break;
		case AXOR:	asm_add(XOR86,lnode,vtype);
					break;
		default:	asm_move(lnode,vtype);
		}

#if OPT
	if (lnode[VIS] == REGV) {
		reg=lnode[VVAL];
		if (reg_weight[reg] < 30000) reg_dirty[reg]|=DIRTY;
		else abandon(reg);
		}
#endif
	if (lnode[VT] == CLONG || (type == ASGN && lnode[VT] == PTRTO && 
		(vtype[VT] == PTRTO || vtype[VT] == CLONG))) {
		is_zero=0;
		if (vtype[VT] == CLONG || vtype[VT] == PTRTO) {
			if (vtype[VT] == PTRTO && vtype[VMORE] == NEED_ES) {
				rm2=toseg(vtype[VNAME]);
				}
			else rm2=tormh2(vtype);
			}
		else {
			rm2=tocon(0);
			is_zero=1;
			}
		switch (type) {
			case AADD:	asm_add(ADC86,tormh(lnode),rm2);
						break;
			case ASUB:	asm_add(SBB86,tormh(lnode),rm2);
						break;
			case AOR:	asm_add(OR86,tormh(lnode),rm2);
						break;
			case AAND:	asm_add(AND86,tormh(lnode),rm2);
						break;
			case AXOR:	asm_add(XOR86,tormh(lnode),rm2);
						break;
			default:	if (is_zero && lnode[VIS] == REGV && lnode[VT] == CLONG)
							asm_add(XOR86,tormh(lnode),tormh(lnode));
						else asm_move(tormh(lnode),rm2);
			}
		}

	if (type != ASGN)
		flip(lnode,vtype);
	freev(lnode);
	}


putbits(lnode,vtype)
	int lnode[],vtype[]; {
	int  shl,bits,mask,temp,i;

	shl=(lnode[VT]-BITS) & 15;
	bits=((lnode[VT]-BITS)>>4)+1;
	mask=0;
	for (i=shl; i < shl+bits; i++)
		mask|=1<<i;
	if (is_big) forcees(lnode);
	asm_add(AND86,lnode,tocon(-mask-1));
	if (vtype[VIS] == CONSTV) {
		if(temp=vtype[VVAL]) {
			if (shl) temp<<=shl;
			asm_add(OR86,lnode,tocon(temp&mask));
			}
		freev(lnode);
		}
	else {
		forceint(vtype);
		forcereg(vtype);
		if (shl > 1 && regat[CX] == 0) {
			asm_move(toreglow(CL),tocon(shl));
			asm_shift_cl(SHL86,vtype);
			}
		else while (shl--) {
			asm_shift_1(SHL86,vtype);
			}
		asm_add(AND86,vtype,tocon(mask));
		asm_add(OR86,lnode,vtype);
		freev(lnode);
		}
	}

gencmp (node,vtype)
	int  node,vtype[]; {
	char jtype,reg;
	int  lab;
	needzreg(vtype);
	jtype=compare(node);
	lab=ojmp(jtype,++nextlab);
	asm_inc(INC86,vtype);
	genlab(lab);
	}

compare(node)
	int  node; {
	int  lnode[7],vtype[7];
	char type;
	get2same(node,lnode,vtype);
	type=tree[node];

	if (floater(vtype)) {
		if (type > NE) type+=4;
		builtin(_FCMP);
		return (type-EQ);
		}

	if (lnode[VT] == CLONG || lnode[VT] == PTRTO) {
		forceacx(lnode,vtype);
		builtin(_CMP4);				/* call _CMP4 */
		freev(lnode);
		freev(vtype);
		return (type-EQ);
		}

	if (lnode[VIS] == CONSTV || lnode[VIS] == OFFV) {
		if (vtype[VIS] == REGV || vtype[VIS] == VARV) {
			flip(lnode,vtype);
			type=flipped[type];
			}
		else forcereg(lnode);
		}
	if (type > NE)
		if (vtype[VT] != CINT || lnode[VT] != CINT) type=type+4;
	if (vtype[VIS] == VARV)
		if (lnode[VIS] == VARV) forcereg(lnode);
	if (is_big) forcees(vtype);
	if (is_big) forcees(lnode);
	asm_add(CMP86,lnode,vtype);
	freev(lnode);
	freev(vtype);
	return (type-EQ);
	}

genshift(node,vtype)
	int  node,vtype[]; {
	int sby[7];
	char type,only1;
	type=tree[node];
	get2(node,vtype,sby);

	if (vtype[VT] == CLONG) {
		force(sby,CXPAT);
		force(vtype,AXPAT);
		if (type == SHR) builtin(_SHR4); /* call _SHR4 */
		else builtin(_SHL4);			 /* call _SHL4 */
		freev(sby);
		}
	else if (vtype[VIS] == CONSTV && sby[VIS] == CONSTV) {
		if (type == SHR) vtype[VVAL]>>=sby[VVAL];
		else vtype[VVAL]<<=sby[VVAL];
		if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) vtype[VT]=CINT;
		}
	else {
		if (sby[VIS] == CONSTV && sby[VVAL] == 1) only1=1;
		else if (sby[VIS] == CONSTV && sby[VVAL] == 8 && type == SHR) {
			force(vtype,BYTEPAT);
			alterv(vtype);
			if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) {
				asm_move(vtype,tocon(0));
				}
			else {
				asm_move(toreglow(vtype[VVAL]),toreglow2(vtype[VVAL]+4));
				asm_move(toreglow(vtype[VVAL]+4),tocon(0));
				}
			return;
			}
		else {
			only1=0;
			force(sby,CXPAT);
			if (sby[VT] == CLONG) regat[BX]=0;
			sby[VT]=CCHAR;
			}
		if (type == SHL) forceint(vtype);
		forcereg(vtype);
		alterv(vtype);
		if (only1) asm_shift_1((type == SHR ? SHR86: SHL86),vtype);
		else {
			asm_shift_cl((type == SHR ? SHR86: SHL86),vtype);
			freev(sby);
			}
		}
	}
