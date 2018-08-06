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
/*	GLINE  --	screen input and output for D88	*/

#include "DEBUG.H"

/* control key translations */
#define left_char		29
#define right_char		28
#define bol_char		200
#define eol_char		201
#define Ins_char		206
#define Del_char		207
#define eof_char		205
#define M1				210
#define M2				211
#define M3				212
#define M4				213
#define M10				219

/*	GLINE  --	input a line with editing.	*/
char scr_mode,oldchar;

gline(lastl)
	char *lastl; {
	int  i;
	char index,ch,iflag,kill;

	if (skipbl()) return;
	strcpy(line,lastl);
	index=0;
	iflag=0;
	kill=1;
	scr_mode=1;
	while (1) {
		if (inmacro == 0) {
			scr_rowcol(1,0);
			dputs(iflag ? "insert  : ": "exchange: ");
			dputs(line);
			scr_clrl();
			scr_rowcol(1,index+10);
			scr_set_cursor();
			}
		ch=dscr_ci();
		switch (ch) {
			case 3:				error("*Control C*");
			case left_char:		if (index) index--;
								break;
			case right_char:	if (line[index]) index++;
								break;
			case 8:				if (index) {
									strcpy(&line[index-1],&line[index]);
									index--;
									}
								break;
			case Ins_char:		iflag=1-iflag;
								break;
			case Del_char:		if (line[index])
									strcpy(&line[index],&line[index+1]);
								break;
			case bol_char:		index=0;
								break;
			case eol_char:		index=strlen(line);
								break;
			case eof_char:		line[index]=0;
								break;
			case 0x1b:
			case '\r':			nextin=line;
								cursor();
								scr_mode=0;
								return;
			default:			i=strlen(line);
								if (i > 69) break;
								if (iflag)
									while (i >= index) {
										line[i+1]=line[i];
										i--;
										}
								else {
									if (kill) line[index]=0;
									if (line[index] == 0) line[index+1]=0;
									}
								line[index]=ch;
								index++;
			}
		kill=0;
		}
	}



/*	GLINENO  --		input a line without reediting. Put into line. */

glineno() {

	gline("\0");
	skipbl();
	}



/*	PROMPT  --  Print a prompt line	*/

prompt(str)
	char *str; {

	if (skipbl() || inmacro) return;
	scr_rowcol(0,0);
	dputs(str);
	xscr_co(' ');
	scr_clrl();
	scr_set_cursor();
	}




/*	INNAME  --	Input a string from the command line. */

inname(name)
	char *name; {
	int  i;

	i=0;
	glineno();
	while (ltype[*nextin] == LETTER || ltype[*nextin] == DIGIT) {
		name[i++]=toupper(*nextin++);
		}
	name[i]=0;
	}


/*	INFNAME  --	Input a filename from the command line. */

infname(name)
	char *name; {
	int  i;

	i=0;
	gline(name);
	skipbl();
	while (*nextin && *nextin != ';' && *nextin != ' ') {
		name[i++]=toupper(*nextin++);
		}
	name[i]=0;
	}


/*	INDIG  --  Input a number.	*/

indig() {
	int dig;

	glineno();
	if (ltype[skipbl()] != DIGIT) error("need a number");
	dig=0;
	while (ltype[*nextin] == DIGIT) dig=dig*10+*nextin++-'0';
	return dig;
	}



/*	INADDR  --  input an address	*/

inaddr(dfrom)
	struct {unsigned off,seg;} *dfrom; {
	unsigned val;
	struct vstruct vt;

	prompt("input [segment:]offset");
	glineno();
	if (skipbl()) {
		expression(&vt);
		if (vt.vtype != OPPV) {
			if (ifop(":")) {
				toint(&vt);
				dfrom.seg=vt.val.valu;
				expression(&vt);
				toint(&vt);
				dfrom.off=vt.val.valu;
				}
			else {
				if (is_big && (*vt.typep == PTRTO || *vt.typep == ARRAY ||
					*vt.typep == FUNCTION)) {
					toptr(&vt);
					*((long *) dfrom)=vt.val.vall;
					}
				else {
					toint(&vt);
					dfrom.off=vt.val.valu;
					}
				}
			}
		}
	}




/*	INCMD  --  input a command   */

incmd(pline)
	char *pline; {
	char cmd;

	repeat=0;
	do {
		if (repeat) {
			scr_rowcol(1,0);
			dprintf("repeat=%d",repeat);
			scr_clrl();
			}
		prompt(pline);
		cmd=inch();
		if (ltype[cmd] == DIGIT) {
			repeat*=10;
			repeat+=cmd-'0';
			cmd=0;
			}
		if (cmd == '/') {
			repeat=32767;
			cmd=0;
			}
		if (cmd == 8) {
			repeat/=10;
			cmd=0;
			}
		}
	while (cmd == 0);
	if (repeat == 0 && inmacro == 0) {
		scr_rowcol(1,0);
		scr_clrl();
		}
	return cmd;
	}



/*	INCH  --  Input a character.	*/

inch() {
	char ch;
	if ((ch=toupper(skipbl())) == 0) {
		if (inmacro == 0) {
			dputs(" ? ");
			scr_clrl();
			scr_set_cursor();
			}
		ch=dscr_ci();
		if (ch == 3) error("*Control C*");
		if (ch == '\r') ch=0;
		else if (inmacro == 0) xscr_co(ch);
		ch=toupper(ch);
		if (inmacro == 0) cursor();
		}
	else nextin++;
	return ch;
	}


	
/*	SKIPBL  --	Skip input characters */

skipbl() {

	while (ltype[*nextin] == SPACE) nextin++;
	return *nextin;
	}


/*	CURSOR  --   Put the cursor back on lineno,0  */

cursor() {
	scr_rowcol(lineno,0);
	scr_set_cursor();
	col=0;
	}



/*	FLIP  --	flip the screen.	*/

flip() {

	if (flipok) {
		scr_rest();
		dscr_ci();
		scr_save();
		}
	}



/*	MACRO  --	input a macro	*/

macro() {
	int i;
	char ch;

	if (macronum) {
		for (i=0; i < 79; i++) {
			if (macro_line[macronum-1][i] == 0) {
				macro_line[macronum-1][i-1]=0;
				break;
				}
			}
		macronum=0;
		dputs("macro is defined");
		}
	else {
		prompt("enter name of macro. F1 F2 F3 or F10=permanent?");
		macronum=scr_ci()-M1+1;
		if (macronum == 10) macronum=4;
		cursor();
		if (macronum < 1 || macronum > 4) {
			macronum=0;
			error("illegal value");
			}
		for (i=0; i < 80; i++)
			macro_line[macronum-1][i]=0;
		dputs("enter another Macro command to end definition");
		lf();
		}
	}





/*	PERMANENT  --  Display permanent information	*/

permanent() {
	}




/*	LF  --	Scroll the screen.	*/

lf() {
	char ch;

	if (lineno == scr_rows-1) scr_scup();
	else {
		if (lineno > 4) scr_clrl();
		lineno++;
		}
	if ((ch=scr_csts()) == 3) error("*Control C*");
	if (ch == 'S'-'@') {
		if (scr_ci() == 3) error("*Control C*");
		}
	else oldchar=ch;
	cursor();
	}


/*	OHEX  --	output a hex word.	*/

ohex(wrd)
	unsigned wrd; {

	dprintf("%x",wrd);
	}

/*	OUNUM  --	output a unsigned word.	*/

ounum(wrd)
	unsigned wrd; {

	dprintf("%u",wrd);
	}


/*	DPUTS  --	use xscr_co to do a puts.	*/

dputs(str)
	char *str; {

	while (*str) xscr_co(*str++);
	}




/*	DSCR_CI  -- input a character and add to macro if required. */

dscr_ci() {
	char ch;
	int  i;

getnext:
	if (inmacro) {
		if (*inmacro) ch=*inmacro++;
		else inmacro=0;
		}
	if (inmacro == 0) {
		if (oldchar) {
			ch=oldchar;
			oldchar=0;
			}
		else ch=scr_ci();
		}

	if (ch >= M1 && ch <= M3) {
		inmacro=macro_line[ch-M1];
		macronum=0;
		goto getnext;
		}

	if (ch == M4) {
		if (macronum) {
			cursor();
			dputs("macro is defined");
			lf();
			macronum=0;
			}
		goto getnext;
		}

	if (macronum) {
		for (i=0; i < 79; i++) {
			if (macro_line[macronum-1][i] == 0) {
				macro_line[macronum-1][i]=ch;
				break;
				}
			}
		}
	return ch;
	}


/*	ADDR  --	print a pointer.	*/

addr(vt)
	struct vstruct *vt; {

	if (is_big) dprintf("%4x:",*(&vt.val.vali+1));
	dprintf("%4x",vt.val.vali);
	}



/*	DPRINTF  --  use xscr_co to do a printf.	*/

dprintf(a,b,c,d,e,f)
	int  a,b,c,d,e,f; {
	char buf[80];

	sprintf(buf,a,b,c,d,e,f);
	dputs(buf);
	}


/*	XSCR_CO  --  output a character and add to col  */

xscr_co(ch)
	int  ch; {
	col++;
	scr_co(ch);
	}



/*	ERROR2  --  print an error message with 2 parts and quit	*/

error2(stra,strb)
	char *stra,*strb; {
	char buf[80];

	strcpy(buf,stra);
	strcat(buf," ");
	strcat(buf,strb);
	error(buf);
	}





/*	ERROR  --	print an error message and bail out */

error(msg)
	char *msg; {

	macronum=0;
	if (flipped) {
		scr_save();
		flipped=0;
		}
	scr_rowcol(1,0);
	dputs(msg);
	scr_clrl();
	line[0]=0;
	nextin=line;
	longjmp(0,0);
	}
