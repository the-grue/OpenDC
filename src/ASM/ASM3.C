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
/*      the-grue - 20180809
 *      Changed lines 318, 324-327 to conform to new compiler.
 */

/*	ASM88	ASSEMBLER FOR 8088					ASM3.C	*/

/*	handle all of the floating point mnumonics	*/

#include "ASM88.H"

dofinst() {
	int  vleft[6],vright[6];
	char type,class;

	type=wvalue->svalue;
	class=wvalue->sclass;
	tokit();

	vleft[VIS]=vright[VIS]=ILLV;
	if (curch != ';' && curch != LF) {
		one(vleft);
		if (ifch(',')) 
			one(vright);
		}

	switch(class) {
		case 0:	float0(type,vleft,vright);
				break;
		case 1:	float1(type,vleft,vright);
				break;
		case 2:	float2(type,vleft,vright);
				break;
		case 3: float3(type,vleft,vright);
				break;
		case 4:	float4(type,vleft,vright);
				break;
		case 5: float5(type,vleft,vright);
				break;
		case 6: float6(type,vleft,vright);
				break;
		case 7: float7(type,vleft,vright);
				break;
		case 8: float8(type,vleft,vright);
				break;
		case 9: float9(type,vleft,vright);
				break;
		case 10: float10(type,vleft,vright);
		}
	}


/*	float type 0. no operands	*/

unsigned float0i[]={0xd9f0,0xd9e1,0xd9e0,0xdbe2,0xded9,0xd9f6,0xdbe1,
					0xdbe0,0xd9f7,0xdbe3,0xd9ec,0xd9ed,0xd9ea,0xd9e9,
					0xd9eb,0xd9ee,0xd9e8,0xd9d0,0xd9f3,0xd9f8,0xd9f2,
					0xd9fc,0xd9fd,0xd9fa,0xd9e4,0xd9e5,0xd9f4,0xd9f1,
					0xd9f9};

float0(type,vleft,vright)
	int  type,vleft[],vright[]; {

	if (vleft[VIS] == ILLV && vright[VIS] == ILLV) {
		wait();
		revcodew(float0i[type]);
		}
	else badopnd();
	}




/*		float type 1, no operand and no wait.	*/

unsigned float1i[]={0xdbe2,0xdbe1,0xdbe0,0xdbe3};

float1(type,vleft,vright)
	int  type,vleft[],vright[]; {

	if (vleft[VIS] == ILLV && vright[VIS] == ILLV) {
		revcodew(float1i[type]);
		}
	else badopnd();
	}




/*		float type 2, load and store.	*/

unsigned float2st[]={0xd9c0,0xddd0,0xddd8};
unsigned float2mem[3][3]={0xd900,0xdd00,0xdb05,
						  0xd902,0xdd02,0xdd02,
						  0xd903,0xdd03,0xdb07};

float2(type,vleft,vright)
	int  type,vleft[],vright[]; {
	unsigned m;

	if (vright[VIS] == ILLV) {
		wait();
		if (vleft[VIS] == STV)
			revcodew(float2st[type]+vleft[VVAL]);
		else if (vleft[VIS] == VARV) {
			m=0;
			if (vleft[VT] == CQWORD) m=1;
			else if (vleft[VT] == CTBYTE) m=2;
			ofrm(float2mem[type][m],vleft);
			}
		else badopnd();
		}
	else badopnd();
	}




/*		float type 3, refers to a stack element only.	*/

unsigned float3i[]={0xddc0,0xd9c8};

float3(type,vleft,vright)
	int  type,vleft[],vright[]; {

	wait();
	if (vleft[VIS] == ILLV)
		revcodew(float3i[type]+type);
	else if (vright[VIS] == ILLV && vleft[VIS] == STV) {
		revcodew(float3i[type]+vleft[VVAL]);
		}
	else badopnd();
	}




/*		float type 4, ST(i), ST category.	*/

unsigned float4i[]={0xdec0,0xdef8,0xdef0,0xdec8,0xdee8,0xdee0};

float4(type,vleft,vright)
	int  type,vleft[],vright[]; {

	if (vleft[VIS] != STV || vright[VIS] != STV || vright[VVAL] != 0)
		badopnd();
	else {
		wait();
		revcodew(float4i[type]+vleft[VVAL]);
		}
	}




/*		float type 5, addition type.	*/

unsigned float5i[6][5]={0xd8c0,0xdcc0,0xdec1,0xd800,0xdc00,
						0xd8f0,0xdcf8,0xdef9,0xd806,0xdc06,
						0xd8f8,0xdcf0,0xdef1,0xd807,0xdc07,
						0xd8c8,0xdcc8,0xdec9,0xd801,0xdc01,
						0xd8e0,0xdce8,0xdee9,0xd804,0xdc04,
						0xd8e8,0xdce0,0xdee0,0xd805,0xdc05};

float5(type,vleft,vright)
	int  type,vleft[],vright[]; {
	unsigned m;

	wait();
	if (vleft[VIS] == STV && vright[VIS] == STV) {
		if (vleft[VVAL] == 0) revcodew(float5i[type][0]+vright[VVAL]);
		else if (vright[VVAL] == 0) revcodew(float5i[type][1]+vleft[VVAL]);
		else badopnd();
		}

	else if (vleft[VIS] == ILLV && vright[VIS] == ILLV) {
		revcodew(float5i[type][2]);
		}

	else if (vleft[VIS] == VARV && vright[VIS] == ILLV) {
		m=3;
		if (vleft[VT] == CQWORD) m=4;
		ofrm(float5i[type][m],vleft);
		}
	else badopnd();
	}






/*		float type 6, floating operations with a memory operation
		and a wait	*/

unsigned float6i[]={0xdf04,0xdf06,0xd905,0xd904,0xdd04,0xdd06,0xd907,0xd906,
					0xdd07};

float6(type,vleft,vright)
	int  type,vleft[],vright[]; {

	if (vleft[VIS] == VARV && vright[VIS] == ILLV) {
		wait();
		ofrm(float6i[type],vleft);
		}
	else badopnd();
	}




/*		float type 7, floating operations with a memory operation
		and no wait	*/

unsigned float7i[]={0xdd06,0xd907,0xd906,0xdd07};

float7(type,vleft,vright)
	int  type,vleft[],vright[]; {

	if (vleft[VIS] == VARV && vright[VIS] == ILLV) {
		ofrm(float7i[type],vleft);
		}
	else badopnd();
	}




/*		float type 8, integer operations.	*/

char float8i[]={0,2,3,6,7,1,4,5};

float8(type,vleft,vright)
	int  type,vleft[],vright[]; {
	unsigned m;

	if (vleft[VIS] == VARV && vright[VIS] == ILLV) {
		wait();
		m=float8i[type];
		if (vleft[VT] == CLONG) m+=0xda00;
		else m+=0xde00;
		ofrm(m,vleft);
		}
	else badopnd();
	}




/*		float type 9, integer load and store.	*/

unsigned float9i[3][3]={0xdb00,0xdf00,0xdf05,
						0xdb02,0xdf02,0xdf06,
						0xdb03,0xdf03,0xdf07};

float9(type,vleft,vright)
	int  type,vleft[],vright[]; {
	unsigned m;

	if (vleft[VIS] == VARV && vright[VIS] == ILLV) {
		wait();
		m=1;
		if (vleft[VT] == CLONG) m=0;
		else if (vleft[VT] == CQWORD) m=2;
		ofrm(float9i[type][m],vleft);
		}
	else badopnd();
	}




/*		float type 10, compare operations.	*/

unsigned float10i[2][4]={0xd8d1,0xd8d0,0xd802,0xdc02,
						 0xd8d9,0xd8d8,0xd803,0xdc03};

float10(type,vleft,vright)
	int  type,vleft[],vright[]; {
	unsigned m;

	wait();
	if (vleft[VIS] == ILLV) revcodew(float10i[type][0]);
	else if (vleft[VIS] == STV && vright[VIS] == ILLV)
		revcodew(float10i[type][1]+vleft[VVAL]);
	else if (vleft[VIS] == VARV && vright[VIS] == ILLV) {
		m=2;
		if (vleft[VT] == CQWORD) m=3;
		ofrm(float10i[type][m],vleft);
		}
	else badopnd();
	}





/*	output the byte for a wait.	*/

wait() {
	codeb(0x9b);
	}



/*		output an instruction and r/m for a float.	*/

union bcode {char bytes[2]; };

ofrm(ftype,vtype)
	unsigned ftype;
	int  vtype[]; {

	union bcode *bctype = &ftype;

	codeb(bctype->bytes[1]);
	orm(bctype->bytes[0],vtype);
	}




/*		TAKE CARE OF THE LISTING	*/

listing() {

	linit();
	while (not_done) {
		ldoline();
		if (*cur == CONTZ) curch=CONTZ;
		else curch=*(cur-1);
		}
	lendit();
	}

linit() {

	not_done=1;
	line=0;
	curseg=0;
	lopt=0;

/*	rewind the source 	*/

	if (lseek(infile,0l,0) == -1) {
		puts("cannot seek ");
		ferror(name);
		}
	fill();		/* fill the source buffer	*/

/*	rewind CTEMP5	*/

	if (lseek(lst,0l,0) == -1) {
		puts("cannot seek");
		ferror(lstname);
		}
	inlst=&lstbuf[512];

/*	open the list output file. use obj and pipe for buffer	*/

	if ((obj=creat(outname)) == -1) {
		puts("cannot create ");
		ferror(outname);
		}
	inpipe=pipe;
	}



lendit() {

	currow=0;
	outb('\f');
	if (write(obj,pipe,inpipe-pipe) == -1) {
		puts("cannot write ");
		ferror(outname);
		}
	if(close(obj) == -1) {
		puts("cannot close ");
		ferror(outname);
		}
	if (close(lst) == -1) {
		os("cannot close ");
		ferror(lstname);
		}
	if (unlink(lstname) == -1) {
		os("cannot unlink ");
		ferror(lstname);
		}
	}

ldoline() {
	int  i,class,codelen;
	char needinc,needej,*ptr,incok;

	needline();
	codelen=lstb();
	incok=0;
	if (codelen == 255) {
		incok=1;
		codelen=lstb();
		}
	if (*cur == CONTZ) return;
	if (!not_done) return;


	needinc=needej=0;

/*	see if title, eject, dseg, cseg or include		*/

	ptr=cur;
	tokit();
	if (curch == CONTZ) return;
	nest_cur=0;		/* chop off any equates	*/
	if (heir == RESERVED) {
		switch (wvalue->sclass) {
			case RDSEG:		curseg=0;
							break;
			case RCSEG:		curseg=1;
							break;
			case RESEG:		curseg=3;
							break;
			case RTITLE:	tokit();
							if (heir == DSTRING) strcpy(title,string);
							else title[0]=0;
							break;
			case REJECT:	needej=1;
							break;
			case RINCLUDE:
							if (incok) needinc=1;
			}
		}

/*	print page header if needed		*/

	if (currow >= maxrow-4) page();

/*	if any code with this line, print it.	*/


	if (codelen) {
		ohwobj(loff[curseg]);
		loff[curseg]+=codelen;
		if (codelen > 8) codelen=8;
		while (codelen--) {
			ohbobj(lstb());
			}
		}
	while (curcol < 21)
		outb(' ');

	oxnum(line);
	outb(' ');

/*	print the line		*/

	do {
		outb(*ptr);
		}
	while (*ptr != CONTZ && *ptr++ != LF);

/*	take care of eject and include	*/

	if (needej) currow=999;
	if (needinc) {
		doinc();
		lstb();
		}
	else cur=ptr;
	}


/*	print the page header	*/

page() {

	curcol=currow=0;
	objstr("\fASM88 Assembler     ");
	objstr(name);
	objstr("\r\n\r\n");
	objstr(title);
	objstr("\r\n\r\n");
	}




/*	output stuff	*/

oxnum(num)
	int  num; {
	char fill=' ',dig,i;
	int  div=1000;

	for (i=0; i < 4; i++) {
		dig=num/div+'0';
		if (i == 3) fill='0';
		if (dig != '0') {
			outb(dig);
			fill='0';
			}
		else outb(fill);
		num%=div;
		div/=10;
		}
	}

ohbobj(bnum)
	int  bnum; {
	char temp;

	temp=bnum;
	temp>>=4;
	outb(temp >= 10 ? temp+'7' : temp+'0');
	temp=bnum&15;
	outb(temp >= 10 ? temp+'7' : temp+'0');
	}

ohwobj(num)
	int  num; {
	ohbobj(num>>8);
	ohbobj(num);
	outb(' ');
	}

/*	output a string	*/

objstr(str)
	char *str; {

	while (*str) outb(*str++);
	}

/*	input a byte from the lst (intermediate file	*/

lstb() {

	if (inlst == &lstbuf[512]) {
		if (read(lst,lstbuf,512) == -1) {
			puts("cannot read ");
			ferror(lstname);
			}
		inlst=lstbuf;
		}

	return *inlst++;
	}


/*	output a byte	*/

outb(ch)
	char ch; {

	if (inpipe == &pipe[2048]) {
		if (write(obj,pipe,2048) == -1) {
			puts("cannot write");
			ferror(objname);
			}
		inpipe=pipe;
		}

	if (ch == CR) {
		curcol=0;
		*inpipe++=ch;
		}
	else if (ch == LF) {
		currow++;
		*inpipe++=ch;
		}
	else if (ch == TAB) {
		do {
			outb(' ');
			}
		while ((curcol-26) & 0x7);
		}
	else {
		if (curcol == maxcol-1)
			objstr("\r\n                          ");
		*inpipe++=ch;
		curcol++;
		}
	}
