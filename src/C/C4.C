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
/*	C4.C					PART4 OF MEDIUM C COMPILER	*/

#include "pass1.h"
#include "nodes.h"

/*	get a constant expression	*/

/*	uses toknlf instead if tokit. this does not allow a constant
	expression to span a line.	*/

toknlf() {
	do {
		if (*cur == LF && cur < savEnd) {
			heir=0;
			curch=LF;
			break;
			}
		}
	while (tokone());
	}

long heir25c() {				/*	must have constant or left parenthesis	*/
	long value, constexp();
	char needrp,savestor,savenest;
	unsigned backup,saveat,saveloc,savetype;
	int  temptree[30],ointree,ontree,omfree,node[2];

	if (heir == RESERVED && bvalue == RSIZEOF) {
		if(mactokn) {
			error_l("sizeof operator not allowed in #if/#elif");
			while (curch != ')') toknlf();
			return 0;
			}
		tokit();
		needrp=0;
		backup=cur;
		savenest=nested;
		/*	save add information in case dimentioning a variable */
		savestor=addstor;
		savetype=addtype;
		saveat=addat;
		saveloc=addloc;
		if (ifch('(')) {

			if (specs(SUNSTOR)) {
				omfree=mfree;
				mfree+=20;
				value= dsize(abstract(0));
				mfree=omfree;
				addstor=savestor;
				addtype=savetype;
				addat=saveat;
				addloc=saveloc;
				return value;
				}
			else {
				cur=backup;
				nested=savenest;
				tokit();
				needrp=1;
				}
			}
/*	if SIZEOF, do not allow a logical expression.	*/
/*	do not output any to tree if SIZEOF	*/
		ointree=intree;
		intree=temptree;
		ontree=ntree;
		heir24(node);
		addstor=savestor;
		addtype=savetype;
		addat=saveat;
		addloc=saveloc;
		intree=ointree;
		ntree=ontree;
		value=dsize(node[1]);
		if (needrp) {
			if (curch == ')') toknlf();
			else notch(')');
			}
		return value;
		}

/*	look for a parened constant expression	*/

	if (ifch('(')) {
		value=constexp();
		if (curch == ')') toknlf();
		else notch(')');
		return value;
		}

	if(mactokn && strcmp(string, "defined") == 0) {
		if(needrp = (curch=*cur++) == '(') {
			whitesp();
			curch=*cur++;
			}
		if (ltype[curch] == LETTER) {
			find(1);
			whitesp();
			value=(heir==DEFINED || (heir == RESERVED && bvalue >= R_LINE
					&& bvalue <= R_STDC));
			heir=LCONSTANT;
			if (needrp) {
				toknlf();
				if (curch == ')') toknlf();
				else notch(')');
				}
			}
		else {
			value = 0;
			error_l("invalid identifier in defined() expression");
			}
		return value;
		}
	value=wvalue;
	if (heir != CONSTANT && heir != LCONSTANT)
		if(mactokn) {
			toknlf();
			return 0;
			}
		else error("must have constant");
	else toknlf();
	return value;
	}



long heir24c() {				/* operator is prefix -	+ */

	if (heir == 22 && curch == '-') {
		tokit();
		return -heir24c();
		}
	else if (heir == 22 && curch == '+') {
		tokit();
		return heir24c();
		}
	else if (heir == 24) {
		tokit();
		return !heir24c();
		}
	heir25c();
	}


long heir23c() {				/*	operator is * and /	*/
	long value;
	char op;

	value=heir24c();
	while (heir == 23) {
		op=bvalue;
		toknlf();
		if (op == MUL) value*= heir24c();
		else if (op == DIV) value/= heir24c();
		else value%= heir24c();
		}
	return value;
	}


long heir22c() {				/*	operator is + and -	*/
	long value;
	char op;

	value=heir23c();
	while (heir == 22) {
		op=bvalue;
		toknlf();
		if (op == ADD) value+= heir23c();
		else value-= heir23c();
		}
	return value;
	}

long heir21c() {				/*	operator is << and >>	*/
	long value;
	char op;

	value=heir22c();
	while (heir == 21) {
		op=bvalue;
		toknlf();
		if (op == SHL) value<<= heir22c();
		else value>>= heir22c();
		}
	return value;
	}

long heir20c() {				/*	operator is relational	*/
	long value;
	char op;

	value=heir21c();
	while (heir == 20) {
		op=bvalue;
		toknlf();
		switch (op) {
			case LT:	value=value < heir21c();
						break;
			case LE:	value=value <= heir21c();
						break;
			case GE:	value=value >= heir21c();
						break;
			case GT:	value=value > heir21c();
			}
		}
	return value;
	}

long heir19c() {				/*	operator is == and !=	*/
	long value;
	char op;

	value=heir20c();
	while (heir == 19) {
		op=bvalue;
		toknlf();
		value=value != heir20c();
		if (op == EQ) value= !value;
		}
	return value;
	}

long heir18c() {				/*	operator is &	*/
	long value;

	value=heir19c();
	if (heir == 18) {
		toknlf();
		value&= heir18c();
		}
	return value;
	}

long heir17c() {				/*	operator is ^	*/
	long value;

	value=heir18c();
	if (heir == 17) {
		toknlf();
		value=value ^ heir17c();
		}
	return value;
	}

long heir16c() {				/*	operator is |	*/
	long value;

	value=heir17c();
	if (heir == 16) {
		toknlf();
		value|=heir16c();
		}
	return value;
	}

long heir15c() {				/*	operator is &&	*/
	long value,valr;

	value=heir16c();
	if (heir == 15) {
		toknlf();
		valr=heir15c();
		value= value && valr;
		}
	return value;
	}

long heir14c() {				/*	operator is ||	*/
	long value,valr;

	value=heir15c();
	if (heir == 14) {
		toknlf();
		valr=heir14c();
		value= value || valr;
		}
	return value;
	}

long constexp() {				/*	operator is ? :	*/
	long value,vall,valr;

	value=heir14c();
	if (ifch('?')) {
		vall=constexp();
		if (notch(':')) return 0;
		valr=constexp();
		if (value) value=vall;
		else value=valr;
		}
	return value;
	}

/*	expression evaluator	*/


heir11(node)				/* operator is , */
	int  node[]; {
	int  list[7];
	char n,lval;
	n=0;
	do {
		lval=heir12(node);
		if (n && node[0] == 0) nooper();
		list[++n]=node[0];
		if (n == 6) {
			list[0]=LST+6;
			list[1]=treev(7,list);
			n=1;
			}
		}
	while (ifch(','));
	if (n > 1) {
		list[0]=LST+n;
		node[0]=treev(n+1,list);
		lval=0;
		}
	return lval;
	}

heir12(node)				/* operater is assignment */
	int  node[]; {
	int  nodeo[4],node2[2],tanode[2];
	char lval,addop,ltype,tp;

	lval=heir13(node);
	if (heir == 12) {
		struct_type=node[1];		/* see if structure assignment */
		if (struct_type->sttype == CSTRUCT && bvalue == ASGN) {
			nodeo[0]=MOVE;
			tanode[0]=TA + (is_big ? (PTRTO << 8) : (CUNSG << 8));
			tanode[1]=node[0];
			nodeo[2]=tree2(tanode);
			nodeo[3]=struct_type->ststagat->staglen;
			tokit();
			heir12(node2);
			tanode[1]=node2[0];
			nodeo[1]=tree2(tanode);
			struct_type=node2[1];
			if (struct_type->sttype != CSTRUCT ||
				struct_type->ststagat->staglen != nodeo[3])
					error("illegal structure assignment");
			node[0]=tree4(nodeo);
			return 0;
			}
		if (lval == 0) {
			error("need lval");
			return 0;
			}
		nodeo[0]=bvalue;
		addop=0;
		ltype=((char *)node[1])->byte;
		if (ltype == PTRTO && ((bvalue == AADD) || (bvalue == ASUB)))
			addop=1;
		nodeo[1]=node[0];
		tokit();
		heir12(node2);
		nodeo[2]=node2[0];
		if (nodeo[1] == 0 || nodeo[2] == 0) nooper();
		struct_type=node2[1];
		if (struct_type->sttype == CSTRUCT)
			error("illegal structure assignment");
		if ((ltype >= CFLOAT) && (ltype < FUNCTION) && (nodeo[0] != ASGN)) {
			if (nodeo[0] == AMOD) nofloat(node[1],node[1]);
			if (ltype >= BITS && ltype < FUNCTION) {
				nodeo[0]-=AADD-ADD;
				nodeo[2]=tree3(nodeo);
				nodeo[0]=ASGN;
				}
			addop=0;
			}
		if (addop) addptr(nodeo[0],node,node2);
		else node[0]=tree3(nodeo);
		return 0;
		}
	return lval;
	}

heir13(node)				/* operater is conditional (? :) */
	int  node[]; {
	int  nodeo[4],node2[2],node3[2];
	char lval,ntype;
	int  typ;
	lval=heir14(node);
	if (ifch('?')) {
		nodeo[0]=TEST;
		nodeo[1]=node[0];
		heir12(node2);
		nodeo[2]=node2[0];
		if (notch(':')) return 0;
		heir12(node3);
		nodeo[3]=node3[0];
		if (nodeo[1] == 0 || nodeo[2] == 0 || nodeo[3] == 0) nooper();
		node[1]=maxtype(node2[1],node3[1]);
		if (*node[1] >= BITS && *node[1] < FUNCTION) node[1]=u_type;
		typ=typeof(node[1]);
		if (typ == (ARRAY<<8) || typ == (CSTRUCT<<8))
			typ=is_big ? (PTRTO<<8) : (CUNSG<<8);
		nodeo[0]+=typ;
		node[0]=tree4(nodeo);
		return 0;
		}
	return lval;
	}

heir14(node)				/* operater is or (||) */
	int  node[]; {
	int  nodeo[3];
	char lval;
	lval=heir15(node);
	if (heir == 14) {
		nodeo[0]=LOR;
		nodeo[1]=node[0];
		tokit();
		heir14(node);
		nodeo[2]=node[0];
		if (nodeo[1] == 0 || nodeo[2] == 0) nooper();
		node[0]=tree3(nodeo);
		node[1]=int_type;
		return 0;
		}
	return lval;
	}


heir15(node)				/* operater is and (&&) */
	int  node[]; {
	int  nodeo[3];
	char lval;
	lval=heir16(node);
	if (heir == 15) {
		nodeo[0]=LAND;
		nodeo[1]=node[0];
		tokit();
		heir15(node);
		nodeo[2]=node[0];
		if (nodeo[1] == 0 || nodeo[2] == 0) nooper();
		node[0]=tree3(nodeo);
		node[1]=int_type;
		return 0;
		}
	return lval;
	}


heir16(node)				/* operater is binary or (|) */
	int  node[]; {
	int  nodeo[3],node2[2];
	char lval;
	lval=heir17(node);
	if (heir == 16) {
		nodeo[0]=OR;
		nodeo[1]=node[0];
		tokit();
		heir16(node2);
		nodeo[2]=node2[0];
		if (nodeo[1] == 0 || nodeo[2] == 0) nooper();
		node[0]=tree3(nodeo);
		node[1]=nofloat(node[1],node2[1]);
		return 0;
		}
	return lval;
	}

heir17(node)				/* operater is xor (^) */
	int  node[]; {
	int  nodeo[3],node2[2];
	char lval;
	lval=heir18(node);
	if (heir == 17) {
		nodeo[0]=XOR;
		nodeo[1]=node[0];
		tokit();
		heir17(node2);
		nodeo[2]=node2[0];
		if (nodeo[1] == 0 || nodeo[2] == 0) nooper();
		node[0]=tree3(nodeo);
		node[1]=nofloat(node[1],node2[1]);
		return 0;
		}
	return lval;
	}

heir18(node)				/* operater is binary and (&) */
	int  node[]; {
	int  nodeo[3],node2[2];
	char lval;
	lval=heir19(node);
	if (heir == 18) {
		nodeo[0]=AND;
		nodeo[1]=node[0];
		tokit();
		heir18(node2);
		nodeo[2]=node2[0];
		if (nodeo[1] == 0 || nodeo[2] == 0) nooper();
		node[0]=tree3(nodeo);
		node[1]=nofloat(node[1],node2[1]);
		return 0;
		}
	return lval;
	}

heir19(node)				/* operater is equality (== and !=) */
	int  node[]; {
	int  nodeo[3];
	char lval;
	lval=heir20(node);
	while (heir == 19) {
		nodeo[0]=bvalue;
		nodeo[1]=node[0];
		tokit();
		heir20(node);
		nodeo[2]=node[0];
		if (nodeo[1] == 0 || nodeo[2] == 0) nooper();
		node[0]=tree3(nodeo);
		node[1]=int_type;
		lval=0;
		}
	return lval;
	}

heir20(node)				/* operater is relational (< <= etc.) */
	int  node[]; {
	int  nodeo[3];
	char lval;
	lval=heir21(node);
	while (heir == 20) {
		nodeo[0]=bvalue;
		nodeo[1]=node[0];
		tokit();
		heir21(node);
		nodeo[2]=node[0];
		if (nodeo[1] == 0 || nodeo[2] == 0) nooper();
		node[0]=tree3(nodeo);
		node[1]=int_type;
		lval=0;
		}
	return lval;
	}

heir21(node)				/* operater is shift (<< and >>) */
	int  node[]; {
	int  nodeo[3],node2[2];
	char lval;
	lval=heir22(node);
	while (heir == 21) {
		nodeo[0]=bvalue;
		nodeo[1]=node[0];
		tokit();
		heir22(node2);
		nodeo[2]=node2[0];
		if (nodeo[1] == 0 || nodeo[2] == 0) nooper();
		node[0]=tree3(nodeo);
		node[1]=nofloat(node[1],node2[1]);
		lval=0;
		}
	return lval;
	}
heir22(node)				/* operater is add sub (+ -) */
	int  node[]; {
	int  noder[2];
	char lval,addop;
	lval=heir23(node);
	while (heir == 22) {
		addop=bvalue;
		lval=0;
		tokit();
		noder[0]=0;
		heir23(noder);
		if (node[0] == 0 || noder[0] == 0) nooper();
		else addptr(addop,node,noder);
		}
	return lval;
	}

addptr(addop,node,noder)
	char addop;
	int  node[],noder[]; {
	int  nodeo[3],sizel,sizer,val;
	sizel=scaled(node[1]);
	sizer=scaled(noder[1]);
	if (sizel == 1 && sizer > 1) {
		scale(node,sizer);
		node[1]=noder[1];
		}
	else if (sizer == 1 && sizel > 1) scale(noder,sizel);
	/*	if subtracting pointers, change to offset only	*/
	if (is_big && *noder[1] >= FUNCTION &&
		(addop == SUB || addop == ASUB)) {	
		if (*node[1] < FUNCTION ) error("illegal arithmetic");
		node[1]=node[0];
		node[0]=CAST+(CUNSG<<8);
		node[0]=tree2(node);
		node[1]=u_type;
		noder[1]=noder[0];
		noder[0]=CAST+(CUNSG<<8);
		noder[0]=tree2(noder);
		noder[1]=u_type;
		}		
	nodeo[0]=addop;
	nodeo[1]=node[0];
	nodeo[2]=noder[0];
	node[0]=tree3(nodeo);
	node[1]=maxtype(node[1],noder[1]);
	if (sizer > 1 && sizer == sizel && (addop == SUB || addop == ASUB)) {
		nodeo[2]=oconst(sizer);
		nodeo[0]=addop+DIV-SUB;
		nodeo[1]=node[0];
		node[0]=tree3(nodeo);
		node[1]=int_type;
		}
	}

scale(node,times)
	int  node[],times; {
	int  nodeo[3];
	char typ;

	nodeo[2]=node[0];
	typ=*((char *)(node[1]));
	if (typ == CCHAR || typ == CSCHAR || typ == PTRTO) {
		node[1]=node[0];
		node[0]=CAST+(CINT<<8);
		nodeo[2]=tree2(node);
		}
	nodeo[1]=oconst(times);
	nodeo[0]=MUL;
	node[0]=tree3(nodeo);
	node[1]=int_type;
	}

/*	return a pointer to the maximum types of the nodes pointer to. report
	an error if a type is floating point.	*/

nofloat(typea,typeb)
	char *typea,*typeb; {
	char *typ;

	typ=maxtype(typea,typeb);
	if (*typ == CFLOAT || *typ == CDOUBLE) {
		error("illegal use of FLOAT");
		typ=int_type;
		}
	return typ;
	}


/*	return the type that will be produced by arighmetic on the two
	arguments.	*/

maxtype(typea,typeb)
	char *typea,*typeb; {

	return *typea > *typeb ? typea: typeb;
	}
