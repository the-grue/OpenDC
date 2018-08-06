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
/*	part 2 of generation	*/

#include "PASS2.H"
#include "NODES.H"
#include "inst86.h"

genstmt(node)
	int  node; {
	char type,otype,i,*chasm,before_dirty[8],after_dirty[8];
	int  before_from[8],after_from[8];
	int  lab,lab2,vtype[7],i,reg;

/*	clear off any temporaries */
	if (curoff != locoff) {
		asm_add(ADD86,toreg(SP),tocon(curoff-locoff));
		curoff=locoff;
		}
/*	insure that no registers are locked */
	for (i=0; i < 9; i++) {
		if (regat[i] != 0) {
			os("stuck=");
			oreg(i);
			if (line_num) {
				os(" at ");
				onum(line_num);
				}
			olf();
			regat[i]=0;
			}
#if OPT
		if (reg_from[i] && reg_weight >= 30000) abandon(i);
#endif
		}

	if (zopt) test_flush(256);
	type=tree[node];
	otype=optype[type];
#if DEBUG
	if (eopt) {
		os("stmt "); onum(node);
		os(" type "); onum(type);
		os(" otype "); onum(otype);
		olf();
		}
#endif

	if (otype == 1) {	/* statement */
		switch (type) {

		  case LAB:
			abandon_all();
			if (zopt) asm_label(tree[node+2]);
			else {
				oname(tree[node+2]);
				os(":\n");
				}
			genstmt(tree[node+1]);
			break;

		  case IFE:
			gencond(tree[node+1],0,lab=++nextlab,0);
#if OPT
			save_from(before_from,before_dirty);
#endif
			genstmt(tree[node+2]);
			if (tree[node+3]){
#if OPT
				save_from(after_from,after_dirty);
				_move(16,before_from,reg_from);
				_move(8,before_dirty,reg_dirty);
#endif
				lab2=++nextlab;
				asm_jump(JMP86,lab2);
				genlab(lab);
				genstmt(tree[node+3]);
				genlab(lab2);
#if OPT
				test_change(after_from,after_dirty);
#endif
				}
			else {
				genlab(lab);
#if OPT
				test_change(before_from,before_dirty);
#endif
				}
			break;

		  case WHIL:
			if (++numwhil >= MAXFOR) error("Too Many Whiles");
			whilcont[numwhil]=lab=++nextlab;
#if OPT
			while_regs(node+3);
			save_from(before_from,before_dirty);
#endif
			genlab(lab);
			gencond(tree[node+1],0,lab2=++nextlab,0);
			whilbrk[numwhil]=lab2;
			genstmt(tree[node+2]);
			--numwhil;
			asm_jump(JMP86,lab);
			genlab(lab2);
#if OPT
			test_change(before_from,before_dirty);
#endif
			break;

		  case DOW:
			if (++numwhil >= MAXFOR) error("Too Many Whiles");
#if OPT
			while_regs(node+3);
			save_from(before_from,before_dirty);
#endif
			lab=++nextlab;
			whilcont[numwhil]=++nextlab;
			genlab(lab);
			whilbrk[numwhil]=lab2=++nextlab;
			genstmt(tree[node+1]);
			genlab(whilcont[numwhil]);
			gencond(tree[node+2],1,lab,0);
			--numwhil;
			genlab(lab2);
#if OPT
			test_change(before_from,before_dirty);
#endif
			break;

		  case SWIT:
			doswit(node);
			break;

		  case FORS:
			if (++numwhil >= MAXFOR) error("Too Many Fors");
			getnoval(tree[node+1],vtype);
			/*	clear off any temporaries */
			if (curoff != locoff) {
				asm_add(ADD86,toreg(SP),tocon(curoff-locoff));
				curoff=locoff;
				}
#if OPT
			while_regs(node+5);
			save_from(before_from,before_dirty);
#endif
			genlab(lab=++nextlab);
			gencond(tree[node+2],0,whilbrk[numwhil]=++nextlab,0);
			whilcont[numwhil]=++nextlab;
			genstmt(tree[node+4]);
			genlab(whilcont[numwhil]);
			getnoval(tree[node+3],vtype);
			asm_jump(JMP86,lab);
			genlab(whilbrk[numwhil--]);
#if OPT
			test_change(before_from,before_dirty);
#endif
			break;

		  case RET:
			if (tree[node+1]) {
				if (tree[tree[node+1]] == MOVE) {
					genmove(tree[node+1],vtype);
					}				
				else {
					if (tree[tree[node+1]] == (PTRTO << 8)+CAST) {
						prefer_si=1;
						getval(tree[tree[node+1]+1],vtype);
						prefer_si=0;
						if (vtype[VIS] == CONSTV) {
							if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) vtype[VT]=CINT;
							if (vtype[VT] != CLONG) forcel(vtype);
							}
						if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) forceint(vtype);
						force(vtype, SIPAT);
						force_real_es(vtype,0);
						freev(vtype);
						}
					else {
						getval(tree[node+1],vtype);
						if (vtype[VIS] == CONSTV && (vtype[VT] == CCHAR || vtype[VT] == CSCHAR))
							vtype[VT]=CINT;
						if (floater(vtype)) loadf(vtype);
						else {
							force(vtype,AXPAT);
							if (vtype[VT] == CCHAR)
								asm_move(toreglow(AH),tocon(0));
							else if (vtype[VT] == CSCHAR)
								asm_cbw();

							freev(vtype);
							}
						}
					}
				}
			abandon_all();
			if (locoff || curoff) asm_move(toreg(SP),toreg2(BP));
			asm_push(1,toreg(BP));
			if (is_big) asm_lret();
			else asm_ret();
			curoff=locoff;
			break;

		  case GOTOS:
			getval(tree[node+1],vtype);
			if (vtype[VT] != CLABEL) error("Bad GOTO Target");
			asm_jump(JMP86,vtype[VNAME]);
			break;

		  case BRK:
			if (numwhil == 0) error("Break Not in Switch");
			asm_jump(JMP86,whilbrk[numwhil]);
			break;

		  case CONT:
			if (numwhil == 0 || whilcont[numwhil] == -1)
				error("Continue Not in While");
			asm_jump(JMP86,whilcont[numwhil]);
			break;

		  case CAS:
			if (numwhil == 0) error("Case Not in Switch");
			genlab(++switnext[numwhil]);
			genstmt(tree[node+1]);
			break;

		  case DFLT:
			if (numwhil == 0) error("Default Not in Switch");
			genlab(++switnext[numwhil]);
			genstmt(tree[node+1]);

		  case NULL:
			break;

		  case OUTASM:
			chasm=&tree[node+1];
			while (*chasm != 255) {
				if (*chasm && *chasm != '\r') oc(*chasm);
				chasm++;
				}
			break;
			}
		}
	else if (otype == 14)		/* list node */
		dolst(node,genstmt);
	else if (otype == 15) {		/* statement number node */
		asm_line(line_num=tree[node+2]);
		genstmt(tree[node+1]);
		}

	else
		getnoval(node,vtype);
/*	clear off any temporaries */
	if (curoff != locoff) {
		asm_add(ADD86,toreg(SP),tocon(curoff-locoff));
		curoff=locoff;
		}
	}

getnoval(node,vtype)
	int  node,vtype[]; {
	char type;
	type=tree[node];
	if (type != NULL) {
		/* change postincrement to preincrement */
		if (optype[type] == 11) tree[node]=tree[node]-2;
		getval(node,vtype);
		if (vtype[VIS] == FLOATV) builtin(_FCLEAR);
		freev(vtype);
		}
	}

dolst(node,listof)
	int  node,(*listof)(); {
	char inlist,i,last;
	inlist=tree[node]-LST;
	/*	cannot call genstmt recursively if hundreds of chained list nodes.
		run out of stack.	*/
	i=1;
	if (listof == genstmt && tree[tree[node+1]] == LST+2) {
		dolst(tree[++node],genstmt);
		i=2;
		}
	for (; i<= inlist; i++) {
		(*listof)(tree[++node]);
		}
	}

doswit(node)
	int  node; {
	int  lab,vtype[7],minv,maxv,tmp,disper[100],numdisp,deflab,displab;
	int  i;
	char  gotdef;

	if (++numwhil >= MAXFOR) error("Too Many Switches");
	numswit=0;
	casefind(tree[node+2]);
	if (numswit == 0) error("No Cases");
	if (numswit >= MAXCASE-1) error("Too Many Cases");
	getval(tree[node+1],vtype);
	deflab=9999;
	gotdef=0;
	for (i=0; i < numswit; i++) {
		if (swits[i] == 0x8000) {
			gotdef=1;
			deflab=nextlab+i+1;
			}
		}
	if (deflab == 9999) {
		swits[numswit++]=0x8000;
		deflab=nextlab+numswit;
		}
	whilcont[numwhil]= numwhil > 1 ? whilcont[numwhil-1]: -1;
	switnext[numwhil]=nextlab;
	if (vtype[VIS] == CONSTV && (vtype[VT] == CCHAR || vtype[VT] == CSCHAR)) vtype[VT]=CINT;
	force(vtype,BXPAT);
	forceint(vtype);
	freev(vtype);
	abandon_all();

/*	see if can do a fast dipatch for switch	*/

	minv=32767;
	maxv=-32767;
	for (i=0; i < numswit; i++) {
		if ((tmp=swits[i]) != 0x8000) {
			if (tmp < minv) minv=tmp;
			if (tmp > maxv) maxv=tmp;
			}
		}
	numdisp=maxv-minv+1;
	if (numswit+4  > numdisp/3 && numdisp <= 100 && numdisp > 0) {
		for (i=0; i < numdisp; i++)
			disper[i]=deflab;
		for (i=0; i < numswit; i++)
			if (swits[i] != 0x8000) disper[swits[i]-minv]=++nextlab;
			else nextlab++;

		if (gotdef) nextlab++;
	
/*	output code for fast dispatch	*/

		asm_add(CMP86,toreg(BX),tocon(minv));
		asm_jump(JL86,deflab);
		asm_add(CMP86,toreg(BX),tocon(maxv));
		asm_jump(JG86,deflab);
		if (minv) {
			asm_add(SUB86,toreg(BX),tocon(minv));
			}
		asm_shift_1(SHL86,toreg(BX));
		asm_jump_bx(nextlab+1);
		genlab(nextlab+1);
		for (i=0; i < numdisp; i++) {
			asm_dw();
			if (zopt) {
				asm_const(toext(disper[i]),CINT);
				if (zopt) test_flush(100);
				}
			else {
				os("_L");
				onum(disper[i]-1000);
				olf();
				}
			}
		}
	else {

/*	use _SWITCH to dispatch the SWITCH	*/

		builtin(_SWITCH);		/* CALL _SWITCH */
		asm_dw();
		if (zopt) codew(numswit);
		else {
			onum(numswit);
			olf();
			}
		for (i=0; i < numswit; i++) {
			asm_dw();
			if (zopt) {
				if (zopt) test_flush(100);
				codew(swits[i]);
				asm_const(toext(++nextlab),CINT);
				}
			else {
				onum(swits[i]);
				os(",_L");
				onum(++nextlab-1000);
				olf();
				}
			}
		if (gotdef) nextlab++;
		}

/*	generate the code for the cases	*/

	lab=whilbrk[numwhil]=nextlab;
	nextlab++;
	genstmt(tree[node+2]);
	--numwhil;
	genlab(lab);
	}

casefind(node)
	int  node; {
	char type;
	type=tree[node];
	if (type >= LST+2 && type <= LST+6)
		dolst(node,casefind);
	else if (type == CAS) {
		if (numswit < MAXCASE) swits[numswit++]=tree[node+2];
		casefind(tree[node+1]);
		}
	else if (type == DFLT) {
		if (numswit < MAXCASE) swits[numswit++]=0x8000;
		casefind(tree[node+1]);
		}
	else if (type == LAB || type == STMT) {
		casefind(tree[node+1]);
		}
	}


/*	put nodes in following order
		1	CONST
		2	variable
		2+	number of nodes involved in expression
		20	function call
		100	& variable
*/
reorder(node)
	int  node; {
	char type,left,right,ctemp;
	int  temp,i,j,nnode;

	type=tree[node];
	if (commut[type]) {
		if ((left=reorder(tree[node+1])) < (right=reorder(tree[node+2]))) {
			tree[node]=flipped[tree[node]];
			temp=tree[node+1];
			tree[node+1]=tree[node+2];
			tree[node+2]=temp;
			ctemp=left;
			left=right;
			right=ctemp;
			}
		if (left == 100) right=0;
		return (left+right);
		}
	if (optype[type] == 0) {	/* data */
		if (type == OPND) {
			if (tree[node+1] > 0)
				if (*extat[tree[node+1]] >= 128) return 4;
			return 2;
			}

		if (type == TA) {
			reorder(tree[node+1]);
			return 100;
			}
		if (type == IND) {
			reorder(tree[node+1]);
			return 3;
			}
		if (type == CAST) return reorder(tree[node+1]);
		return 1;
		}

/*	see if a while, do-while or for.	*/

	j=ctemp=0;
	nnode=1;
	switch (type) {
		case WHIL:
		case DOW:	j=2;
					break;
		case FORS:	ctemp=reorder(tree[node+1]);
					nnode=2;
					j=4;
		}
	if (j) {
		for (; nnode <= j; nnode++)
			ctemp+=reorder(tree[node+nnode]);
		}

	else {
		j=subn[type];
		for (i=1; i <= j; i++)
			ctemp+=reorder(tree[node+i]);
		}
	return ctemp;
	}



/*	CANDIDATE  --	select candidates for optimization	*/

#if OPT
candidate(node)
	int  node; {
	char type;
	int i,j,nnode,test_node,worst,worsti,best,in_union;

	type=tree[node];
	if (optype[type] == 0) {	/* data */
		if (type == OPND) {
			in_union=tree[node+3];
			tree[node+3]=ANYPAT << 8;
			if (in_union) return;
			if (tree[node] < (CLONG<<8)) {
				tree[node+3]=ANYPAT<<8;
				down++;
				for (i=0; i < NUMREC; i++) {
					if ((test_node = recat[i]) &&
						tree[node+1] == tree[test_node+1] &&
						tree[node+2] ==	tree[test_node+2] &&
						tree[node] == tree[test_node]) {

						if (down-recent[i] < 20) {
							tree[test_node+3]+=down-recent[i];
							recat[i]=node;
							recent[i]=down;
							weight[i]+=2;
							return;
							}
						}
					}
				worst=down;
				for (i=0; i < NUMREC; i++) {
					if (recent[i] < worst) {
						worst=recent[i];
						worsti=i;
						}
					}
				recat[worsti]=node;
				recent[worsti]=down;
				weight[worsti]=2;
				}
			return;
			}

		if (type == TA) {
			candidate(tree[node+1]);
			}
		else if (type == IND) {
			candidate(tree[node+1]);
			}
		return;
		}

/*	see if a while, do-while or for.	*/

	j=0;
	nnode=1;
	switch (type) {
		case WHIL:
		case DOW:	j=2;
					break;
		case FORS:	candidate(tree[node+1]);
					nnode=2;
					j=4;
		}
	if (j) {
		for (i=0; i < NUMREC; i++) {
			weight[i]=1;
			}
		for (; nnode <= j; nnode++)
			candidate(tree[node+nnode]);
		best=20000;
		for (j=nnode; j < nnode+3; j++) {
			worst=1;
			for (i=0; i < NUMREC; i++) {
				if (weight[i] > worst && weight[i] < best) {
					worst=weight[i];
					worsti=i;
					}
				}
			if (worst > 1) {
				tree[node+j]=recat[worsti];
				best=worst;
				}
			else break;
			}
		}

	else {
		if (type >= ASGN && type <= AXOR) {
			candidate(tree[node+2]);
			candidate(tree[node+1]);
			}
		j=subn[type];
		for (i=1; i <= j; i++)
			candidate(tree[node+i]);
		}
	}
#endif



/*	GENMOVE	--	generate a structure move	*/

genmove(node,vtype)
	int node;
	int vtype[7]; {
	int vfrom[7];

	/* get the source	*/
	getval(tree[node+1],vfrom);
	/*	push destination address	*/
	getval(tree[node+2],vtype);
	pushv(vtype,0);
	/*	push source address	*/
	pushv(vfrom,0);
	freev(vfrom);
	/*	push the length	*/
	vfrom[VIS]=CONSTV;
	vfrom[VVAL]=tree[node+3];
	vfrom[VOFF]=0;
	vfrom[VT]=CINT;
	pushv(vfrom,0);
	builtin(_MOVE);
	if (is_big) {
		asm_add(ADD86,toreg(SP),tocon(10));
		curoff-=10;
		}
	else {
		asm_add(ADD86,toreg(SP),tocon(6));
		curoff-=6;
		}
	}



#if OPT

/*	SAVE_FROM  --	remember reg_from for later test change */

save_from(before_from,before_dirty)
	int before_from[8];
	int before_dirty[8]; {

	_move(16,reg_from,before_from);
	_move(8,reg_dirty,before_dirty);
	}



/*	TEST_CHANGE  --		see if a wanted register was lost.	*/

test_change(oldfrom,olddirty)
	int oldfrom[8];
	char olddirty[8]; {
	int  i,oldnode,newnode;
	char old_dirty;
	
	for (i=0; i < 8; i++) {
		if (oldfrom[i] != reg_from[i]) {
			oldnode=oldfrom[i];
			if (oldnode < 0) oldnode=tree[-oldnode];
			newnode=reg_from[i];
			if (newnode < 0) newnode=tree[-newnode];
			if (tree[oldnode+1] == tree[newnode+1] && 
				tree[oldnode+2] == tree[newnode+2] &&
				tree[oldnode] == tree[newnode]) {
					reg_dirty[i]|=olddirty[i];
					}
			else {
				if (oldnode) {
					if (oldfrom[i] < 0) tree[-oldfrom[i]]=0;
					else tree[oldnode+3] &=0xff00;
					better=1;
					}
				if (newnode) {
					reg_dirty[i]=0;		/* abandon and kill	*/
					abandon(i);
					}
				}
			}
		}
	}


/*	WHILE_REGS  --	see if can load any registers for a while loop */

while_regs(node)
	int node; {
	int  i,reg,vtype[7],onode;
	char type;

	for (i=0; i < 3; i++) {
		if (tree[node+i]) {
			onode=tree[node+i];
			type=tree[onode+3];
			node_to_vtype(onode,vtype);
			if (is_reg(vtype) == -1 &&
					(reg=want_reg(numwhil,type == CCHAR || type == CSCHAR)) != -1) {
				movreg(reg,vtype);
				regat[reg]=0;			/* register not actually used */
				reg_from[reg]=-node-i;
				reg_dirty[reg]=0;
				}
			}
		}
	}

#endif


ocomma() {
	oc(',');
	}
