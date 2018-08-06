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
/*	GEN4.C	part 4 of generation	*/

#include "PASS2.H"
#include "NODES.H"
#include "inst86.h"


genarith(node,vtype)
	int  node,vtype[]; {
	int lnode[7];
	char type;

	type=tree[node];
	get2(node,vtype,lnode);

/*	take care of float first	*/

	if (floater(vtype)) {
		switch (type) {
			case ADD:	builtin(_FADD);
						break;
			case SUB:	builtin(_FSUB);
						break;
			case MUL:	builtin(_FMUL);
						break;
			case DIV:	builtin(_FDIV);
			}
		return;
		}

/*	take care of long next	*/
/*	do not allow long + pointer	*/

	if (lnode[VT] == PTRTO && vtype[VT] == CLONG) forceint(vtype);
	if (vtype[VT] == PTRTO && lnode[VT] == CLONG) forceint(lnode);

	if (lnode[VT] == CLONG || vtype[VT] == CLONG) {
		forceacx(vtype,lnode);
		if (type == ADD) {
			asm_add(ADD86,toreg(AX),toreg2(CX));
			asm_add(ADC86,toreg(DX),toreg2(BX));
			}
		else if (type == SUB) {
			asm_add(SUB86,toreg(AX),toreg2(CX));
			asm_add(SBB86,toreg(DX),toreg2(BX));
			}
		else
			builtin(type-MUL+_MUL4);	/* call _MUL4 etc. */
		freev(lnode);
		return;
		}

	if (type == ADD) genadd(vtype,lnode);
	else if (type == SUB) gensub(vtype,lnode);

	else muldiv(type,vtype,lnode);
	}


muldiv(type,vtype,lnode)
	char type;
	int vtype[],lnode[]; {
	char reg,byt; {
	int mtype;
	char c_typ;

		if (type == MUL && lnode[VIS] == REGV) 
			if (lnode[VVAL] == AX) flip(lnode,vtype);
		if (vtype[VIS] == CONSTV && lnode[VIS] == CONSTV) {
			vtype[VT]=CINT;
			if (type != MUL && lnode[VVAL] == 0) error("divide by zero");
			switch (type) {
				case MUL: vtype[VVAL]*=lnode[VVAL];
						break;
				case DIV: vtype[VVAL]/=lnode[VVAL];
						break;
				case MOD: vtype[VVAL]%=lnode[VVAL];
				}
			return;
			}
		if (lnode[VIS] == CONSTV && type != MOD) {
			if (lnode[VVAL] == 1) return;
			if (lnode[VVAL] == 2 || lnode[VVAL] == 4 || lnode[VVAL] == 8) {

/*	V3.1 03/07/88 -	in case we have been called from IND and need value in AX */

				prefer_si = 0;

				force(vtype,AXPAT);
				alterv(vtype);
				if (type == MUL) mtype=SHL86;
				else if (vtype[VT] == CINT) {
					mtype=SAR86;
					need(DXPAT);
					asm_cwd();
					if (lnode[VVAL] != 2) asm_add(XOR86,toreg(AX),toreg2(DX));
					asm_add(SUB86,toreg(AX),toreg2(DX));
					}
				else mtype=SHR86;
				asm_shift_1(mtype,vtype);
				if (lnode[VVAL] >= 4) {
					asm_shift_1(mtype,vtype);
					if (lnode[VVAL] == 8) {
						asm_shift_1(mtype,vtype);
						}
					if (type != MUL && vtype[VT] == CINT) {
						if (lnode[VVAL] != 2) asm_add(XOR86,toreg(AX),toreg2(DX));
						asm_add(SUB86,toreg(AX),toreg2(DX));
						}
					}
				return;
				}
			}
		force(vtype,AXPAT);
		alterv(vtype);
		byt=vtype[VT]==CCHAR;
		if (byt) {
			if (lnode[VT] != CCHAR) {
				if(lnode[VT] == CSCHAR)
					asm_cbw();
				else
					asm_move(toreglow(AH),tocon(0));
				vtype[VT]=CINT;
				byt=0;
				}
			else if (type != MUL) asm_move(toreglow(AH),tocon(0));
			}
		if (!byt) {
			need(DXPAT);
			regat[DX]=1;
			if (type != MUL)
				if (((c_typ=vtype[VT]) == CINT || c_typ == CSCHAR)
						&& ((c_typ=lnode[VT]) == CINT || c_typ == CCHAR || c_typ == CSCHAR))
					asm_cwd();
				else asm_add(XOR86,toreg(DX),toreg2(DX));
			forceint(lnode);
			}
		forcemul(lnode);
		if (is_big) forcees(lnode);
		if (regat[DX] == 1) regat[DX]=0;
		if (type == MUL) {
			mtype=MUL86;
			if (byt) vtype[VT]=CINT;
			}
		else mtype=DIV86;
		/*	turn into IMUL and IDIV	*/
		if (((c_typ = vtype[VT]) == CINT || c_typ == CSCHAR)
				&& ((c_typ = lnode[VT]) == CINT || c_typ == CSCHAR))
			mtype++;
		asm_not(mtype,lnode);
		freev(lnode);
		if (type == MOD) {
			if (byt) {
				asm_move(toreglow(AL),toreglow2(AH));
				}
			else {
				vtype[VVAL]=DX;
				regat[DX]=regat[AX];
				regat[AX]=0;
				}
			}
		}
	}

genadd(vtype,lnode)
	int  vtype[],lnode[]; {
	int  used;

	if (lnode[VIS] == CONSTV) {
		if (lnode[VVAL] == 0) return;
		if (vtype[VIS] == CONSTV) {
			vtype[VVAL]=vtype[VVAL]+lnode[VVAL];
			if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) vtype[VT]=CINT;
			return;
			}
		if (vtype[VIS] == OFFV) {
			vtype[VOFF]=vtype[VOFF]+lnode[VVAL];
			return;
			}
		}
	if (lnode[VIS] == OFFV && vtype[VIS] != OFFV) flip(lnode,vtype);
	if (vtype[VIS] == OFFV) {
		used=regused[vtype[VVAL]];
		if (used != 8) {
			forceint(lnode);
			if (is_big) forcees(lnode);
			asm_add(ADD86,toreg(used),lnode);
			freev(lnode);
			}
		else {
			forceind(lnode);
			regat[lnode[VVAL]]=vtype;
			if (vtype[VVAL] == 8)
				vtype[1]=(lnode[VVAL] == SI) ? 4:5;
			else if (vtype[VVAL] == 7) {
				vtype[VVAL] = (lnode[VVAL] == SI) ? 0:1;
				}
			else
				vtype[1]=(lnode[VVAL] == SI) ? 2:3;
			}
		}
	else {
		if (lnode[VT] == PTRTO) flip(lnode,vtype);
		if (vtype[VT] != lnode[VT]) {
			if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) forceint(vtype);
			else if (lnode[VT] == CCHAR || lnode[VT] == CSCHAR) forceint(lnode);
			}
		forcerm(lnode);
		forcereg(vtype);
		alterv(vtype);
		if (is_big) forcees(lnode);
		asm_add(ADD86,vtype,lnode);
		freev(lnode);
		}
	}

gensub(vtype,lnode)
	int  vtype[],lnode[]; {

	if (lnode[VIS] == CONSTV) {
		if (lnode[VVAL] == 0) return;
		if (vtype[VIS] == CONSTV) {
			vtype[VVAL]=vtype[VVAL]-lnode[VVAL];
			if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) vtype[VT]=CINT;
			return;
			}
		if (vtype[VIS] == OFFV) {
			vtype[VOFF]=vtype[VOFF]-lnode[VVAL];
			return;
			}
		}
	if (lnode[VT] == PTRTO) flip(lnode,vtype);
	if (lnode[VT] != vtype[VT])
		if (lnode[VT] == CCHAR || lnode[VT] == CSCHAR) forceint(lnode);
		else if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) forceint(vtype);
	forcerm(lnode);
	forcereg(vtype);
	alterv(vtype);
	if (is_big) forcees(lnode);
	asm_add(SUB86,vtype,lnode);
	freev(lnode);
	}

genbin(node,vtype)
	int  node,vtype[]; {
	int  lnode[7],type86;
	char type;

	type=tree[node];
	get2same(node,vtype,lnode);
	if (lnode[VIS] == CONSTV && vtype[VIS] == CONSTV && vtype[VT] != CLONG) {
		if (type == OR) vtype[VVAL]|=lnode[VVAL];
		else if (type == AND) vtype[VVAL]&=lnode[VVAL];
		else vtype[VVAL]^=lnode[VVAL];
		if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) vtype[VT]=CINT;
		return;
		}
	forcerm(lnode);
	forcereg(vtype);
	if (is_big) forcees(lnode);
	alterv(vtype);
	if (type == OR) type86=OR86;
	else if (type == AND) type86=AND86;
	else type86=XOR86;
	asm_add(type86,vtype,lnode);
	freev(lnode);
	if (vtype[VT] == CLONG) {
		asm_add(type86,tormh(vtype),tormh2(lnode));
		}
	}


gentest(node,vtype)
	int  node,vtype[]; {
	int  lab,lab2;
	char reg,reglong,ntype,i;

	ntype=tree[node]>>8;
	if (ntype == FUNCTION) ntype=is_big ? PTRTO: CUNSG;
	/*	sadly, must get rid of all register variables.	*/
	for (i=0; i <8; i++)
		if (regat[i]) pushv(regat[i],1);

	gencond(tree[node+1],0,lab=++nextlab,0);
	getval(tree[node+2],vtype);
	if (ntype == PTRTO) {
		if (vtype[VT] != PTRTO) forcel(vtype);
		force(vtype, INDPAT);
		force_real_es(vtype);
		reg = vtype[VVAL];
		}
	else if (ntype >= CFLOAT) loadf(vtype);
	else {
		if (vtype[VIS] == CONSTV && (vtype[VT] == CCHAR || vtype[VT] == CSCHAR)) vtype[VT]=CINT;
		force(vtype,AXPAT);
		if (ntype == CLONG) forcel(vtype);
		else forceint(vtype);
		}
	freev(vtype);

	lab2=ojmp(10,++nextlab);
	genlab(lab);
	getval(tree[node+3],vtype);
	if (ntype == PTRTO) {
		if (vtype[VT] != PTRTO) forcel(vtype);
		force(vtype, regpat[reg]);
		force_real_es(vtype);
		}
	else if (ntype >= CFLOAT) loadf(vtype);
	else {
		if (vtype[VIS] == CONSTV && (vtype[VT] == CCHAR || vtype[VT] == CSCHAR)) vtype[VT]=CINT;
		force(vtype,AXPAT);
		if (ntype == CLONG) forcel(vtype);
		else forceint(vtype);
		}
	genlab(lab2);
	}

gencond(node,true,lab,keep)
	int  node,lab;
	int *keep;
	char true; {
	char type,otype,jtype;
	int  dolab,reg,vtype[7];
	unsigned cur_loc;

	type=tree[node];
	otype=optype[type];
	if (otype == 7) {
		if (type+true == LAND) {
			gencond(tree[node+1],true,lab,keep);
			gencond(tree[node+2],true,lab,keep);
			}
		else {
			dolab=++nextlab;
			gencond(tree[node+1],!true,dolab,keep);
			gencond(tree[node+2],true,lab,keep);
			genlab(dolab);
			}
		}
	else if (otype == 8) {
		reg=keep[VVAL];
		cur_loc=curoff;
		jtype=compare(node);
		if (true) jtype^=1;
		cleanup(cur_loc);
		if (keep && reg != keep[VVAL]) force(keep,regpat[reg]);
		ojmp(jtype,lab);
		}
	else if (otype == 13) {
		if (true) {
			asm_jump(JMP86,lab);
			}
		}
	else if (type == NOT) {
		gencond(tree[node+1],!true,lab,keep);
		return;
		}
	else {
		reg=keep[VVAL];
		cur_loc=curoff;
		getval(node,vtype);
		forcerm(vtype);
		if (keep && reg != keep[VVAL]) force(keep,regpat[reg]);
		if (vtype[VIS] == CONSTV && !floater(vtype)) {
			if (vtype[VVAL] || vtype[VNAME]) true = !true;
			if (true == 0) {
				cleanup(cur_loc);
				asm_jump(JMP86,lab);
				}
			}
		else {
			if (floater(vtype)) {
				loadf(vtype);
				builtin(_FIS);
				}
			else if (vtype[VT] == CLONG || vtype[VT] == PTRTO) {
				if (vtype[VIS] == VARV || vtype[VT] == PTRTO) {
					reg=need(ANYPAT);
					if (is_big) forcees(vtype);
					asm_move(toreg(reg),tormh(vtype));
					}
				else reg=vtype[VVAL]+2;
				asm_add(OR86,toreg(reg),vtype);
				freev(vtype);
				}
			else {
				if (vtype[VIS] == CONSTV || vtype[VIS] == OFFV) forcereg(vtype);
				if (is_big) forcees(vtype);
				if (vtype[VIS] == REGV && vtype[VT] != CCHAR && vtype[VT] != CSCHAR)
					asm_add(OR86,vtype,vtype);
				else asm_add(CMP86,vtype,tocon(0));
				freev(vtype);
				}
			cleanup(cur_loc);
			asm_jump( (true ? JNZ86: JZ86),lab);
			}
		}
	}

cleanup(cur_loc)
	unsigned cur_loc; {
		
	if (cur_loc < curoff) {
		asm_lea(SP,tobpm(cur_loc));
		curoff=cur_loc;
		}
	}

genlog(node,vtype)
	int  node,vtype[]; {
	int  lab,i;
	char reg;

	/*	sadly, must get rid of all register variables.	*/
	for (i=0; i <8; i++)
		if (regat[i]) pushv(regat[i],1);

	needzreg(vtype);
	gencond(node,0,lab=++nextlab,vtype);
	asm_inc(INC86,vtype);
	genlab(lab);
	}

flip(lnode,vtype)
	int  *lnode,*vtype; {
	char i;
	int  temp;

	for (i=0; i < 9; i++) {
		if (regat[i] == vtype) regat[i]=lnode;
		else if (regat[i] == lnode) regat[i]=vtype;
		}
	i=0;
	while (i <= VFROM) {
		temp=*vtype;
		*vtype++=*lnode;
		*lnode++=temp;
		i++;
		}
	}

/*	return true if vtype addresses a floating point number	*/

floater(vtype)
	int  vtype[]; {

	return (vtype[VT] == CFLOAT || vtype[VT] == CDOUBLE)
			 && vtype[VIS] != OFFV;
	}
