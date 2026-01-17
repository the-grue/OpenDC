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
/*	GEN6	ASSEMBLER OR OBJECT OUTPUT FOR GEN					GEN6.C	*/

#include "PASS2.H"
#include "NODES.H"
#include "OBJ.H"
#include "inst86.h"


/*	the ES,DS,CS,SS values are different in this module.	*/

#define ES	0
#define CS	1
#define SS	2
#define DS	3


/*	reserve some storage	*/

int  obj,dummy,pipelab;
char *inobj,*inpipe,*indummy;
char objbuf[512],pipe[2048],dbuf[2048];

int  offs[5],offsett,laboff[2048/9],labels[2048/9];
;
char *codeat;
char contype[6],shortj[21],ndummy,curseg;
char *tempname,*tempname6,*dummyat;
static int segreg[4]={0,3,2,1};

/*	rmto shows how to change the rm type with the addition of a register */
/*	fields are: new register, old rm, new rm.	*/
	

char rmto[36]={3,8,7,3,4,0,3,5,1,5,8,6,5,4,2,5,5,3,6,8,4,6,
				7,0,6,6,2,7,8,5,7,7,1,7,6,3};

asm_init() {
	int  i;

	inpipe=pipe;				/* pipe is empty */
	codeat=0;					/* points to code fragment (if last)in pipe*/
								/* offs[0]=offs[1]=offs[2]=offs[3]=offs[4]=0;*/
								/* total size of dseg ,cseg, reserved, eseg */
								/* and eseg reserved.	*/

/*	set jump codes. last is jmp short	*/

	for (i=0; i < 16; i++)
		shortj[i]=0x70+i;
	shortj[16]=0xe0;shortj[17]=0xe1;shortj[18]=0xe2;shortj[19]=0xe3;
	shortj[20]=0xeb;


/*	set temporary name to Z drive.	*/

	tempname="Z:CTEMP3";
	if (topt) *tempname=topt;
	else tempname+=2;
	tempname6="Z:CTEMP6";
	if (topt) *tempname6=topt;
	else tempname6+=2;

	if (copt) {
		if ((obj=fopen(objname,"a")) == 0) {
			os("cannot create ");
			error(objname);
			}
		mopt=0;
		}
	else {
		if (mopt) {			/* want Intel object modules.	*/
			nextpgm[2]=0;
			strcat(nextpgm,"TOOBJ ");
			strcat(nextpgm,tempname6);
			strcat(nextpgm," O");
			strcat(nextpgm,objname);
			strcpy(objname,tempname6);
			}
		if ((obj=creat(objname)) == -1) {
			os("cannot create ");
			error(objname);
			}
		}
	inobj=&objbuf;

	ndummy=0;
	indummy=&dbuf;
	if (is_big && copt == 0) objb(OBIG);

/*	next .O and dummy contents will be in DSEG */
	curseg=ODSEG;				/* first stuff is in DSEG */
	objb(ODSEG);
	dummyb(ODSEG);
	}


asm_endit() {
	char *isize;
	unsigned util2;

	flush(1);
	if (ndummy) {
		if (lseek(dummy,0,0,0) == -1)
			error("cannot seek CTEMP3");
		while (ndummy--) {
			if(read(dummy,pipe,2048) == -1)
				error("cannot read CTEMP3");
			isize=pipe;
			while (isize < &pipe[2048])
				objb(*isize++);
			}
		if (close(dummy) == -1)
			error("cannot close CTEMP3");
		if (unlink(tempname) == -1)
			error("cannot unlink CTEMP3");
		}
	isize=dbuf;
	while (isize < indummy)
		objb(*isize++);
	objb(OEOF);
	isize=inobj;
	isize=isize-objbuf;
	if (write(obj,objbuf,isize) == -1) {
		os("cannot write ");
		error(objname);
		}
	if(close(obj) == -1) {
		os("cannot close ");
		error(objname);
		}
	}


/*	ASM_ERROR  --	do cleanup for error and z option	*/

asm_error() {

	if (ndummy)	{
		if (close(dummy) == -1)
			error("cannot close CTEMP3");
		if (unlink(tempname) == -1)
			error("cannot unlink CTEMP3");
		}
	close(obj);
	unlink(objname);
	}


/*	ASM_SIZE  --	report on code size for zopt	*/

asm_size() {

	inout=0;
	oword(offs[1]);
	os(" code   ");
	oword(offs[0]+offs[2]);
	os(" data   ");
	if (is_big) {
		oword(offs[3]+offs[4]);
		os(" extra   ");
		}
	}


/*	ASM_PUBLIC  --	output a PUBLIC NAME	*/

asm_public(pnum,pname,need_)
	int pnum;
	char *pname,need_; {
	char *nameat;

	if (zopt) {
		objb(OPUBLIC);
		while (*pname) objb(*pname++);
		if (need_) objb('_');
		objb(0);
		objw(pnum);
		}
	else {
		os(" PUBLIC ");
		os(pname);
		if (need_) oc('_');
		olf();
		}
	}

/*	ASM_STATIC  --	output a STATIC NAME	*/

asm_static(pnum,pname)
	int pnum;
	char *pname; {
	char *nameat;

	objb(OSTATIC);
	while (*pname) objb(*pname++);
	objb('_');
	objb(0);
	objw(pnum);
	}


/*	ASM_RB  --	output a RB N	*/

asm_rb(num,thisext)
	int  num,thisext; {

	if (zopt) {
		objb(ORESERVE);
		objw(thisext);
		objw(num);
		offs[curseg == ODSEG ? 2: 4]+=num;
		}
	else {
		os(" RB ");
		ouunum(num);
		olf();
		}
	}


/*	ASM_DB	--	output just the DB	*/

asm_db() {

	if (zopt == 0) os(" DB ");
	else if (asmext) {
		asm_label(asmext);
		}
	}


/*	ASM_DW	--	output just the DW	*/

asm_dw() {

	if (zopt == 0) os(" DW ");
	else if (asmext) {
		asm_label(asmext);
		}
	}


/*	ASM_DD	--	output just the DD	*/

asm_dd() {

	if (zopt == 0) os(" DD ");
	else if (asmext) {
		asm_label(asmext);
		}
	}


/*	ASM_ADD	--	generate add type instructions.	*/

char add_inst[8][6]={" ADD "," OR "," ADC "," SBB "," AND "," SUB "," XOR "," CMP "};
asm_add(type,vleft,vright)
	char type;
	int  vleft[],vright[]; {
	char sgned,from;
	int  data_type;

	alterv(vleft);
	if (zopt) {
		if (vright[VIS] <= OFFV) {
			if (vleft[VIS] == REGV && vleft[VVAL] == AX) {
				codeb(3+realtype(vleft)+(type<<3));
				asm_const(vright,vleft[VT]);
				}
			else {
				data_type=realtype(vleft);
				sgned=(data_type == CINT && vright[VIS] == CONSTV &&
					vright[VVAL] <= 127 && 	vright[VVAL] >= -128) ? 2 : 0;
				codeb(127+sgned+realtype(vleft));
				asm_orm(type,vleft);
				asm_const(vright,sgned ? CCHAR: data_type);
				}
			}
		else {
			if (vleft[VIS] == REGV) {
				codeb((type<<3)+realtype(vright)+1);
				asm_orm(vleft[VVAL],vright);
				}
			else if (vright[VIS] == REGV) {
				codeb((type<<3)+realtype(vleft)-CCHAR);
				asm_orm(vright[VVAL],vleft);
				}
			else error("impossable arithmetic");
			}
		}

	else {
		os(add_inst[type]);
		orm(vleft);
		ocomma();
		orm(vright);
		olf();
		}
	}

/*	ASM_MOVE  --	generate MOV type instructions.	*/

asm_move(vleft,vright)
	int  vleft[],vright[]; {

	if (vleft[VIS] == REGV && vleft[VT] != CCHAR && vright[VIS] == CONSTV &&
		vright[VVAL] == 0) {
		asm_add(XOR86,vleft,vleft);
		return;
		}
	asm_move_noopt(vleft,vright);
	}

asm_move_noopt(vleft,vright)
	int  vleft[],vright[]; {

	if (zopt) {
		if (vleft[VIS] == SEGRV) {
			codeb(0x8e);
			asm_orm(segreg[vleft[VVAL]-8],vright);
			}
		else if (vright[VIS] == SEGRV) {
			codeb(0x8c);
			asm_orm(segreg[vright[VVAL]-8],vleft);
			}
		else if (vright[VIS] <= OFFV) {
			if (vleft[VIS] == REGV) {
				codeb(0xb0+((realtype(vleft)-1)<<3)+vleft[VVAL]);
				asm_const(vright,vleft[VT]);
				}
			else {
				codeb(0xc5+realtype(vleft));
				asm_orm(0,vleft);
				asm_const(vright,vleft[VT]);
				}
			}
		else if (vleft[VIS] == REGV) {
			if (vleft[VVAL] == AX && vright[VIS] == VARV && vright[VVAL] == 8) {
				codeb(0xa0+realtype(vleft)-1);
				if (is_big && vright[VMORE] != -1) whatseg(vright);
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
				asm_orm(vleft[VVAL],vright);
				}
			}
		else if (vright[VIS] == REGV) {
			if (vright[VVAL] == AX && vleft[VIS] == VARV && vleft[VVAL] == 8) {
				codeb(0xa2+realtype(vright)-1);
				if (is_big && vleft[VMORE] != -1) whatseg(vleft);
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
				asm_orm(vright[VVAL],vleft);
				}
			}
		else error("bad operand");
		}
	else {
		os(" MOV ");
		orm(vleft);
		ocomma();
		orm(vright);
		olf();
		}
	}


/*	ASM_XCHG				xchg type	*/

asm_xchg(lreg,rreg)
	int  lreg,rreg; {

	if (zopt) {
		if (lreg == AX) codeb(0x90+rreg);
		else {
			codeb(0x87);
			asm_orm(lreg,toreg(rreg));
			}
		}
	else {
		os(" XCHG ");
		oreg(lreg);
		ocomma();
		oreg(rreg);
		olf();
		}
	}


/*	ASM_LES				les type	*/

asm_les(reg,vright)
	int  reg,vright[]; {
	int  oldvt;

	if (zopt) {
		oldvt=vright[VT];
		if (vright[VT] != CFOREIGN) vright[VT]=CINT;
		codeb(0xc4);
		asm_orm(reg,vright);
		vright[VT]=oldvt;
		}
	else {
		os(" LES ");
		oreg(reg);
		ocomma();
		orm(vright);
		olf();
		}
	}


/*	ASM_SHIFT_1  --	shift by 1 type instruction	*/

asm_shift_1(type,vtype)
	char type,vtype[]; {

	if (zopt) {
		codeb(0xd0-1+realtype(vtype));
		asm_orm(type,vtype);
		}
	else {
		if (type == SHL86) os(" SHL ");
		else if (type == SHR86) os(" SHR ");
		else os(" SAR ");
		orm(vtype);
		os(",1\n");
		}
	}


/*	ASM_SHIFT_CL  --	shift by CL type instruction	*/

asm_shift_CL(type,vtype)
	char type,vtype[]; {

	if (zopt) {
		codeb(0xd2-1+realtype(vtype));
		asm_orm(type,vtype);
		}
	else {
		if (type == SHL86) os(" SHL ");
		else if (type == SHR86) os(" SHR ");
		else os(" SAR ");
		orm(vtype);
		os(",CL\n");
		}
	}




/*	ASM_PUSH	--	push or pop */

asm_push(type,vtype)
	char type;
	int  vtype[]; {
	char signext;
	int oldvt;

	if (zopt) {
		switch (vtype[VIS]) {
			case REGV:	codeb(vtype[VVAL]+(type<<3)+0x50);
						break;
			case SEGRV:	codeb(6+type+((segreg[vtype[VVAL]-8])<<3));
						break;
			case VARV:	oldvt=vtype[VT];
						if (vtype[VT] != CFOREIGN) vtype[VT]=CINT;
						if (type) {
							codeb(0x8f);
							asm_orm(0,vtype);
							}
						else {
							codeb(0xff);
							asm_orm(6,vtype);
							}
						vtype[VT]=oldvt;
						break;
			case CONSTV:
			case OFFV:	signext = (vtype[VIS] == CONSTV && vtype[VVAL] <= 127 && vtype[VVAL] >= -128) ? 2 : 0;
						codeb(0x68 | signext);
						if(signext)
							vtype[VT] = CCHAR;
						asm_const(vtype,vtype[VT]);
			}
		}
	else {
		if (type) os(" POP "); else os(" PUSH ");
		orm(vtype);
		olf();
		}
	}	


/*	ASM_PUSH_ESSI  --	push es:[si+n]	*/

asm_push_essi(off)
	int  off; {

	if (zopt) {
		if (is_big)codeb(0x26);
		codeb(0xff);
		if (off) {
			codeb(0x74);
			codeb(off);
			}
		else codeb(0x34);
		}
	else {
		os(" PUSH ");
		if (is_big) os("ES:");
		os("[SI");
		if (off) {
			oc('+');
			onum(off);
			}
		os("]\n");
		}
	}




/*	ASM_INC				inc type	*/

asm_inc(type,vtype)
	char type;
	int  vtype[]; {

	if (zopt) {
		if (vtype[VIS] == REGV && realtype(vtype) == CINT)
			codeb(0X40+vtype[VVAL]+(type<<3));
		else {
			codeb(0xfd+realtype(vtype));
			asm_orm(type,vtype);
			}
		}
	else {
		os(type ? " DEC ": " INC ");
		orm(vtype);
		olf();
		}
	}


/*	ASM_NOT  --	not,neg,mul,div type type	*/

static char *notname[]={" TEST ",""," NOT "," NEG "," MUL "," IMUL "," DIV "," IDIV "};
asm_not(type,vtype)
	int  vtype[]; {

	if (zopt) {
		codeb(0xf6-1+realtype(vtype));
		asm_orm(type,vtype);
		}
	else {
		os(notname[type]);
		orm(vtype);
		olf();
		}
	}


/*	ASM_LEA  --				lea type	*/

asm_lea(reg,vright)
	int  reg,vright[]; {

	if (zopt) {
		if (vright[VT] == CLABEL) vright[VT]=CINT;
		codeb(0x8d);
		asm_orm(reg,vright);
		}
	else {
		os(" LEA ");
		oreg(reg);
		ocomma();
		orm(vright);
		olf();
		}
	}


/*	ASM_CALL --				call type	*/

asm_call(vtype)
	int vtype[]; {

	if (vtype[VIS] == OFFV) {
		asm_jump(21,vtype[VNAME]);
		}
	else {
		if (zopt) {
			codeb(0xff);
			asm_orm(2,vtype);
			freev(vtype);
			}
		else {
			os(" CALL ");
			ofrm(vtype);
			olf();
			}
		}
	}


/*	ASM_LCALL -- 				lcall  type	*/

asm_lcall(vtype)
	int  vtype[]; {

	if (vtype[VIS] == OFFV) asm_lc(vtype[VNAME]);
	else {
		codeb(0xff);
		if (vtype[VT] == CLABEL) vtype[VT]=CINT;
		asm_orm(3,vtype);
		}
	}

asm_lc(num)
	int  num; {

	codeb(0x9A);	/* lcall */
	codew(0);
	codew(0);
	fixup(OLNAMEREL,num);
	}


/*	ASM_RET  --	RET and other one byte instructions	*/

asm_ret() {

	if (zopt) codeb(0xc3);
	else os(" RET\n");
	}

asm_lret() {

	if (zopt) codeb(0xcb);
	else os(" LRET\n");
	}


asm_cbw() {

	if (zopt) codeb(0x98);
	else os(" CBW\n");
	}

asm_cwd() {

	if (zopt) codeb(0x99);
	else os(" CWD\n");
	}


/*	ASM_JUMP				jmp type	*/

/*	type 0 to 15   are jo,jno,jb ... jg	*/
/*	type 16 to 19  are loopnz,loopz,loop and jcxz	*/
/*	type 20 and 21 are jmp and call	*/

static char *jname[]={0,0," JB "," JAE "," JZ "," JNZ "," JBE "," JA ",
				0,0,0,0," JL "," JGE "," JLE "," JG ",0,0,0,0," JMP "," CALL "};
asm_jump(type,num)
	char type;
	int  num; {

	if (zopt) jump(type,num);
	else {
		os(jname[type]);
		if (num >= 1000) {
			os("_L");
			onum(num-1000);
			}
		else oname(num);
		olf();
		}
	}


/*	ASM_JUMP_BX	--	jump relative to BX	*/

asm_jump_bx(num)
	int  num; {

	if (zopt) {
		codeb(0x2e);
		codeb(0xff);
		codeb(0xa7);
		asm_const(toext(num),CINT);
		}
	else {
		os(" JMP BYTE _L");
		onum(num-1000);
		os("[BX]\n");
		}
	}



/*	ASM_LABEL  --	have a label in code or data	*/

asm_label(num)
	int  num; {

	codeat=0;
	*inpipe++=OLOCAL;
	inpipe->word=num;
	inpipe+=2;
	asmext=0;
	}

wantcseg() {

	if (curseg != OCSEG) {
		if (zopt) {
			flush(1);
			objb(OCSEG);
			dummyb(OCSEG);
			dummyat=0;
			}
		else os(" CSEG \n");
		curseg=OCSEG;
		}
	}

wantdseg() {

	if (curseg != ODSEG) {
		if (zopt) {
			flush(1);
			objb(ODSEG);
			dummyb(ODSEG);
			dummyat=0;
			}
		else os(" DSEG \n");
		curseg=ODSEG;
		}
	}

wanteseg() {

	if (curseg != OESEG) {
		if (zopt) {
			flush(1);
			if (is_big == 0) error("ESEG illegal in small case");
			objb(OESEG);
			dummyb(OESEG);
			dummyat=0;
			}
		else os(" ESEG \n");
		curseg=ODSEG+3;
		}
	}

/*	routines to add to the pipe	*/

/*	ASM_LINE  --	need a LINE n statement.	*/

asm_line(lineno)
	int  lineno; {

	line_num=lineno;
	if (zopt) {
		codeat=0;
		*inpipe++=OLINE;
		inpipe->word=lineno;
		inpipe+=2;
		}
	else {
		os(" LINE ");
		onum(lineno);
		olf();
		}
	}


/*	ASM_CONST  --	output a two byte constant.	*/

asm_const(vtype,type)
	int  vtype[];
	char type; {

	if ((type == CCHAR || type == CSCHAR) && vtype[VIS] != OFFV) {
		codeb(vtype[VVAL]);
		}
	else {
		switch (vtype[VIS]) {
			case CONSTV:	codew(vtype[VVAL]);
							break;

			case VARV:
			case OFFV:		if (vtype[VVAL] != 8 || vtype[VNAME] == 0)
								error("invalid OFFSET");
							if (vtype[VT] == CSEGMENT) {
								codew(0);
								fixup(OSEGPTR,vtype[VNAME]);
								}
							else {
								codew(vtype[VOFF]);
								fixup(ONAMEREL,vtype[VNAME]);
								}
							break;
			default:		error("invalid constant");
			}
		}
	}

/*	ASM_PTR  --	output a 4 byte pointer.	*/

asm_ptr(vtype)
	int vtype[]; {

	codew(vtype[VOFF]);
	codew(0);
	fixup(OLNAMEREL, vtype[VNAME]);
	}


asm_orm(reg,vtype)
	char reg;
	int  vtype[]; {
	char mod;

	switch (vtype[VIS]) {
		case REGV:	codeb(192+(reg<<3)+vtype[VVAL]);
					break;
		case VARV:	if (vtype[VT] == CLABEL) error("missing type");
					if (is_big) {
						if (vtype[VT] == CFOREIGN) {
							codeb(6+(reg<<3));
							codew(vtype[VOFF]);
							fixup(OPTR,vtype[VNAME]);
							return;
							}
						if (vtype[VMORE] != -1) whatseg(vtype);
						}

					if (vtype[VVAL] == 8) {
						codeb(6+(reg<<3));
						codew(vtype[VOFF]);
						fixup((vtype[VT] == CFOREIGN) ? OPTR : ONAMEREL,
							vtype[VNAME]);
						}
					else {
						mod=0;
						if (vtype[VVAL] == 6 || vtype[VOFF] != 0) mod=64;
						if (vtype[VNAME] ||vtype[VOFF]<-128||vtype[VOFF]>127)
							mod=128;
						codeb(mod+(reg<<3)+vtype[VVAL]);
						if (mod == 64) codeb(vtype[VOFF]);
						else if (mod == 128) {
							codew(vtype[VOFF]);
							if (vtype[VNAME]) 
								fixup((vtype[VT] == CFOREIGN) ? OPTR : ONAMEREL,
								vtype[VNAME]);
							}
						}
					break;
		default:	error("invalid R/M");
		}
	}

/*	TOCON  --	turn a constant into a vtype.	*/

static int consttype[7]={CONSTV,0,0,0,0,CINT,0};
tocon(con)
	int  con; {

	consttype[VVAL]=con;
	return consttype;
	}

	
/*	TOOFF  --	turn a variable number into an offset  */

static int ajvtype[]={OFFV,8,0,0,0,CINT,0};
tooff(num)
	int  num; {

	ajvtype[VNAME]=num;
	return ajvtype;
	}


/*	TOREG  --	turn a register into a vtype.	*/

static int regtype[7]={REGV,0,0,0,0,CINT,0};
static int regtype2[7]={REGV,0,0,0,0,CINT,0};
toreg(reg)
	int  reg; {

	if (reg >= 8) {
		regtype[VIS]=SEGRV;
		regtype[VVAL]=reg;
		}
	else {
		regtype[VIS]=REGV;
		regtype[VVAL]=reg;
		}
	return regtype;
	}

toreg2(reg)
	int  reg; {

	if (reg >= 8) {
		regtype2[VIS]=SEGRV;
		regtype2[VVAL]=reg;
		}
	else {
		regtype2[VIS]=REGV;
		regtype2[VVAL]=reg;
		}
	return regtype2;
	}

	
/*	TOREGLOW  --	turn a  byte register into a vtype.	*/

static int reglowtype[7]={REGV,0,0,0,0,CCHAR,0};
toreglow(reg)
	int  reg; {

	reglowtype[VVAL]=reg;
	return reglowtype;
	}

static int reglowtype2[7]={REGV,0,0,0,0,CCHAR,0};
toreglow2(reg)
	int  reg; {

	reglowtype2[VVAL]=reg;
	return reglowtype2;
	}


/*	TOEXT  --	turn a external index into a vtype.	*/

static int exttype[7]={VARV,8,0,0,0,CINT,0};
toext(ext)
	int  ext; {

	exttype[VNAME]=ext;
	return exttype;
	}

	
/*	TOSEG  --	turn a name into a SEG vtype.	*/

static int segtype[7]={OFFV,8,0,0,0,CSEGMENT,0};
toseg(seg)
	int  seg; {

	segtype[VNAME]=seg;
	return segtype;
	}

	
/*	TOPTR  --	turn a name into a @ vtype.	*/

static int ptrtype[7]={VARV,8,0,0,0,CFOREIGN,0};
toptr(name,off)
	int  name,off; {

	ptrtype[VNAME]=name;
	ptrtype[VOFF]=off;
	return ptrtype;
	}

	
/*	TOSEGRV  --	turn a name into a xsegment register.	*/

static int segrvtype[7]={SEGRV,8,0,0,0,CINT,0};
tosegrv(seg)
	int  seg; {

	segrvtype[VVAL]=seg;
	return segrvtype;
	}

	
/*	TOBPM  --	need a vtype for [BP-n]	*/

static bpvtype[7]={VARV,6,0,0,-1,PTRTO+1,0};
tobpm(num)
	int  num; {

	bpvtype[VOFF]=-num;
	return bpvtype;
	}


/*	TORMH  --	turn a vtype into a high part of a vtype.	*/

static int rmhtype[7],rmhtype2[7];

tormh2(vtype)
	int  vtype[]; {
	tormh(vtype);
	_move(sizeof(rmhtype),rmhtype,rmhtype2);
	return rmhtype2;
	}

tormh(vtype)
	int  vtype[]; {

	_move(sizeof(rmhtype),vtype,rmhtype);
	
	if (vtype[VIS] == CONSTV) rmhtype[VVAL]=rmhtype[VNAME];
	else if (vtype[VIS] == REGV) {
		rmhtype[VT]=CINT;
		if (is_big && vtype[VT] == PTRTO) {
			rmhtype[VVAL]=vtype[VMORE];
			if (rmhtype[VVAL] >= 8) {
				rmhtype[VVAL]=rmhtype[VVAL];
				rmhtype[VIS]=SEGRV;
				}
			}
		else rmhtype[VVAL]+=2;
		}
	else if (vtype[VIS] == OFFV && vtype[VMORE] != -1) {
		
		rmhtype[VIS]=REGV;
		rmhtype[VT]=CINT;
		rmhtype[VVAL]=vtype[VMORE];
		if (rmhtype[VVAL] >= 8) {
			rmhtype[VVAL]=rmhtype[VVAL];
			rmhtype[VIS]=SEGRV;
			}
		}
	else {
		rmhtype[VOFF]+=2;
		}
	return rmhtype;
	}

whatseg(vtype)
	int  vtype[]; {
	char segover,inst,pre;

	if (is_big) {
		segover=vtype[VMORE];
		if (segover == -1) goto no_pre;
		if (vtype[VVAL] <= 3 || vtype[VVAL] == 6) {
			if (segover == 10) goto no_pre; /* SS */
			}
		else if (segover == 9) goto no_pre;

		if (segover == 8) pre=0x26; /* ES */
		else if (segover == 9) pre=0x3e;	/* DS */
		else if (segover == 10) pre=0x36; /* SS */
		else pre=0x2e; /* CS */
		inpipe--;
		inst=*inpipe;
		*inpipe++=pre;
		codeb(inst);
		}
no_pre:;
	}

fixup(type, num)
	int  type, num; {

	if (num) {
		codeat=0;
		*inpipe++=type;
		inpipe->word=num;
		inpipe+=2;
		}
	}

jump(jnum,num)
	char jnum;
	int  num; {

	codeat=0;
	*inpipe++=OJUMP;
	*inpipe++=jnum;
	inpipe->word=num;
	inpipe+=2;
	}

codeb(byt)
	char byt; {

	if (codeat == 0 || *codeat > 200) {
		codeat=inpipe;
		*inpipe++=128;
		}
	(*codeat)++;
	*inpipe++=byt;
	}

union {char bytes[2];};

revcodew(wrd)
	unsigned wrd; {

	codeb(((char*)(&wrd))->bytes[1]);
	codeb(wrd);
	}

codew(wrd)
	int  wrd; {

	if (codeat == 0 || *codeat > 200) {
		codeat=inpipe;
		*inpipe++=128;
		}
	(*codeat)+=2;
	inpipe->word=wrd;
	inpipe+=2;
	}

olf() {
	oc(LF);
	}



/*	routines for object module output	*/

test_flush(needed)
	int  needed; {

	if (inpipe+needed+30 >= &pipe[sizeof(pipe)]) flush(0);
	}


flush(eoseg)
	char eoseg; {
	int  lastoff,want,dif,saved,i;
	char mlab,slab,jtype,*epipe,*stoppipe;

	if (inpipe == pipe) return;

	epipe=inpipe;
	inpipe=pipe;
	offsett=0;

/*	remember how many labels left from last time */
	slab=pipelab;

/*	accumulate labels and worst case offsets	*/

	while (inpipe < epipe) {
		switch(*inpipe++) {
			case ONAMEREL:
			case OSEGPTR:
			case OPTR:
			case OLNAMEREL:	inpipe+=2;
							break;
			case OLOCAL:	if (pipelab < 2048/9) {
								labels[pipelab]=inpipe->word;
								laboff[pipelab++]=offsett;
								}
							inpipe+=2;
							break;
			case OJUMP:		offsett+=5;
							inpipe+=3;
							break;
			case OEVEN:		offsett++;
			case OLINE:		inpipe+=2;
							break;
			default:		i=*(inpipe-1)-128;
							offsett+=i;
							inpipe+=i;
			}
		}

/*	second pass over pipe data. flag long jumps and calculate relative
	offsets. a few jump downs are incorrectly flagged long. a fourth pass
	would solve this problem	*/

	inpipe=pipe;
	mlab=slab;
	saved=offsett=0;

	while (inpipe < epipe) {
		switch(*inpipe++) {
			case ONAMEREL:
			case OSEGPTR:
			case OPTR:
			case OLNAMEREL:	inpipe+=2;
							break;
			case OLOCAL:	if (mlab < 2048/9) {
								laboff[mlab++]=offsett;
								}
							inpipe+=2;
							break;
			case OJUMP:		jtype=*inpipe;
							want=(inpipe+1)->word;
							if (jtype <= 20) {
								for (i=0; i < pipelab; i++) {
									if (want == labels[i]) {
										if (laboff[i] <= offsett)
											dif=offsett-laboff[i]+3;
										else dif=laboff[i]-offsett-saved;
										if (dif <= 129) {
											inpipe+=3;
											offsett+=2;
											saved+=3;
											goto endpipe2;
											}
										else break;
										}
									}
								}
							*inpipe|=0x80;	/* flag as a long jump */
							if ((jtype&0x7f) < 20 ) offsett+=5;
							else {
								offsett+=3;
								saved+=2;
								}
							inpipe+=3;
							break;
			case OEVEN:		if (offsett & 1) offsett++;
							else saved++;
			case OLINE:		inpipe+=2;
							break;
			default:		i=*(inpipe-1)-128;
							offsett+=i;
							inpipe+=i;
endpipe2:;	}
		}

/*	write pipe bytes to dummy file	*/

	inpipe=pipe;
	stoppipe= eoseg ? epipe: epipe-256;
	offsett=0;
	mlab=slab;

	while (inpipe < stoppipe) {
		switch (*inpipe++) {
			case ONAMEREL:
			case OPTR:
			case OSEGPTR:
			case OLNAMEREL:
							dummyb(*(inpipe-1));
							dummyw(inpipe->word);
							dummyat=0;
							inpipe+=2;
							break;
			case OLOCAL:	if (laboff[mlab++] != offsett)
							  error("internal error in jump optimization");
							objb(OLOCAL);
							objw(inpipe->word);
							inpipe+=2;
							objw(offsett+offs[curseg-ODSEG]);
							break;
			case OJUMP:		jtype=*inpipe++;
							want=inpipe->word;
							inpipe+=2;
							if ((jtype & 0x80) == 0) {
								ldummyb(shortj[jtype]);
								i=-1;
								while (labels[++i] != want) ;
								offsett+=2;
								ldummyb(laboff[i]-offsett);
								}
							else {
								jtype&=0x7f;
								if (jtype == 18 || jtype == 19)
									error("JCXZ or LOOP can only jump 128 bytes");
								if (jtype < 20) {
									if (jtype < 18) jtype^=1;
									ldummyb(shortj[jtype]);
									ldummyb(3);
									offsett+=2;
									}
								if (jtype <= 20) ldummyb(0xe9);
								else ldummyb(0xe8);
								offsett+=3;
								ldummyb(0);
								ldummyb(0);
								dummyat=0;
								dummyb(OJUMPREL);
								dummyw(want);
								}
							break;
			case OEVEN:		if (offsett & 1) {
								ldummyb(0);
								offsett++;
								}
							break;
			case OLINE:		dummyb(OLINE);
							dummyw(inpipe->word);
							inpipe+=2;
							dummyat=0;
							break;

			default:		i=*(inpipe-1)-128;
							offsett+=i;
							while (i--) {
								ldummyb(*inpipe++);
								}
			}
		}

	/*	save labels that may be needed by next flush	*/

	mlab=pipelab;
	pipelab=0;
	if (eoseg == 0) {
		lastoff=offsett-128;
		for (i=0; i < mlab; i++) {
			if (laboff[i] > lastoff) {
				if (laboff[i] > offsett) break;
				labels[pipelab]=labels[i];
				laboff[pipelab++]=laboff[i]-offsett;
				}
			}
		}

	if (offs[curseg-ODSEG]+offsett < offs[curseg-ODSEG])
		error("segment over 64K");
	else offs[curseg-ODSEG]+=offsett;

/*		move left over stuff to top of pipe	*/

	stoppipe=pipe;
	while (inpipe < epipe)
		*stoppipe++=*inpipe++;
	inpipe=stoppipe;
	codeat=0;
	dummyat=0;
	}

/*	routines to add to object file	*/

objb(byt)
	char byt; {
	if (inobj == &objbuf[512]) {
		if(write(obj,objbuf,512) == -1) {
			os("cannot write ");
			error(objname);
			}
		inobj=&objbuf;
		}
	*inobj++=byt;
	}

objw(wrd)
	int  wrd; {
	objb(wrd);
	objb(wrd>>8);
	}


/*	routines to add to dummy file	*/

/*	write a dummy byte and add to list if needed	*/

ldummyb(byt)
	char byt; {

	if (dummyat == 0 || *dummyat > 188) {
		dummyb(128);
		dummyat=indummy-1;
		}
	(*dummyat)++;
	dummyb(byt);
	}


dummyb(byt)
	char byt; {
	if (indummy == &dbuf[2048]) {
		if (ndummy == 0) {
			if ((dummy=creat(tempname)) == -1)
				error("cannot create CTEMP3");
			}
		if(write(dummy,dbuf,2048) == -1) {
			error("cannot write CTEMP3");
			}
		ndummy++;
		indummy=&dbuf;
		dummyat=0;
		}
	*indummy++=byt;
	}

dummyw(wrd)
	int  wrd; {
	dummyb(wrd);
	dummyb(wrd>>8);
	}

realtype(vtype)
	int vtype[]; {

	if (vtype[VT] == CCHAR || vtype[VT] == CSCHAR) return CCHAR;
	return CINT;
	}
