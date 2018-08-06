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
/*	DC.C			dump .CHK file  */

/*	D88 .CHK files have the following format:

	each record is of the form:
		record type byte. values are in OBJ.H.
		length byte. total length of record.
		contents of record.

	D88 reports "some symbols lost" if the .CHK file is over 55K as it
	reads it into memory. larger files need not be supported. to reduce
	.chk file size, the -c (checkout) option should be removed from
	compiles of code that does not need to be debugged.

	.CHK records

	OBIG		OBIG,2
				if present, will be first record of the file. note that
				the presence of OBIG changes the format of the OPTYPE
				and OLINE records.

	OPTYPE		OPTYPE,length,address,type.
				public type record. address is 4 byte segment:offset
				(needs fixup) if OBIG, 2 byte DS relative if no OBIG.
				name is zero terminated. note: all names are forced to
				upper case.	type is described below.

	OMTYPE		OMTYPE,length,offset,type.
				member type record. offset is from start of structure.
				name is zero terminated. note: all names are forced to
				upper case.	type is described below. member names are not
				associated with structures. D88 assumes the first name
				match is correct.

	OLTYPE		OLTYPE,length,offset,type.
				local and parameter type record. offset is BP relative.
				name is zero terminated. note: all names are forced to
				upper case.	type is described below. record is only
				relevant if the previous OLNAME record names the
				procedure currently being executed.

	OLINE		OLINE,length,line number,address of line.
				line number record. line number is within filename supplied
				by last ONAME record. if OBIG, address is segment:offset
				and must be fixed up. if not OBIG, address is 2 byte CS
				relative.

	ONAME		ONAME,length,file name.
				file name for following OLINE records.

	OLNAME		OLNAME,length,procedure name.
				name of procedure for following OLTYPE records.

	OOV			OOV,length,overlay number.
				following records are for overlay n. n is 1 to 32.
				the public NUM_OVERLAY found only in small (no OBIG)
				programs indicates that overlays are used and supplies
				the current overlay number. 0 means no overlay has been
				loaded yet.


	type		length of rest of type subfield,type bytes.
*/

#define CCHAR		1
#define CINT		2
#define CUNSG		3
#define CLONG		4
#define CFLOAT		5
#define CDOUBLE		6
#define CLABEL		7
#define CSTRUCT		8	/*	FOLLOWED BY LENGTH OF STRUCTURE	*/

/*	THE FOLLOWING ARE THE TYPE MODIFIERS	*/
#define FUNCTION	253
#define ARRAY		254	/* FOLLOWED BY DIMENSION */
#define PTRTO		255

/*	examples:
	1,CINT							integer.
	4,ARRAY,5,0,CLONG				five element array of longs.
	3,PTRTO,PTRTO,CDOUBLE			pointer to a pointer to a double.
*/
	
#include "OBJ.H"

int  infile;
char inbuf[650],name[20],*inin,*endin;
char suppress,is_big;

union {int word; char byte;};

main(argc,argv)
	int  argc;
	char *argv[]; {
	if (argc > 2) suppress=1;
	doopen(argv[1]);
	dofile();
	}

doopen(file)
	char *file; {
	char i;

	i=0;
	while (*file && *file != '.')
		name[i++]=*file++;
	strcpy(&name[i],".CHK");
	if ((infile=open(name,0)) == -1) {
		os("Cannot open ");
		os(name);
		ocrlf();
		exit();
		}
	inin=endin=0;
	}

dofile() {
	char len,nout,tlen,type_was;
	int  nump=0,numm=0,numl=0,numline=0,numname=0,numlname=0,numov=0;
	unsigned offset;

	checkin();

	while (*inin != OEOF) {
		checkin();
		switch(*inin++) {
			case OBIG:		os("Big");
is_big=1;
							inin++;
							break;
			case OPTYPE:	nump++;
							os("Ptype=");
							goto type;
			case OMTYPE:	numm++;
							os("Mtype=");
							goto type;
			case OLTYPE:	numl++;
							os("Ltype=");
type:						type_was=*(inin-1);
							inin++;
							while (*inin)
								oc(*inin++);
							inin++;
							oc(' ');
							offset=inin->word;
							if (is_big && type_was == OPTYPE) {
								inin+=2;
								ohex(inin->word);
								oc(':');
								}
							ohex(offset);
							oc(' ');
							inin+=2;
							tlen=*inin++;
							while (tlen--) {
								oc(' ');
								switch (*inin++) {
									case CCHAR:		os("char");
													break;
									case CINT:		os("int");
													break;
									case CUNSG:		os("unsg");
													break;
									case CLONG:		os("long");
													break;
									case CFLOAT:	os("float");
													break;
									case CDOUBLE:	os("double");
													break;
									case CLABEL:	os("label");
													break;
									case CSTRUCT:	os("struct len=");
													onum(inin->word);
													inin+=2;
													tlen-=2;
													break;
									case FUNCTION:	os("fun");
													break;
									case ARRAY:		os("[");
													onum(inin->word);
													inin+=2;
													tlen-=2;
													os("]");
													break;
									case PTRTO:		os("ptr");
													break;
									default:		os("mystery=");
													onum(*(inin-1));
									}
								}
							break;

			case OLINE:		numline++;
							os("Line=");
							inin++;
							onum(inin->word);
							inin+=2;
							os(" at ");
							offset=inin->word;
							if (is_big) {
								inin+=2;
								ohex(inin->word);
								oc(':');
								}
							ohex(offset);
							inin+=3;
							break;
			case ONAME:		numname++;
							os("Name=");
							inin++;
							while (*inin)
								oc(*inin++);
							inin++;
							break;
			case OLNAME:	numlname++;
							os("Lname=");
							inin++;
							while (*inin)
								oc(*inin++);
							inin++;
							break;
			case OOV:		numov++;
							os("Overlay=");
							inin++;
							onum(*inin++);
							break;
			default:		os("Mystery=");
							onum(*(inin-1));
			}
		ocrlf();
		}
	printf("Ptype=%d   Mtype=%d   Ltype=%d   Line=%d   Name=%d   Lname=%d   Ov=%d\n",
		nump,numm,numl,numline,numname,numlname,numov);
	}


checkin() {
	char *temp;

	if (inin+128 > endin) {
		temp=inbuf;
		while(inin < endin)
			*temp++=*inin++;
		read(infile,temp,512);
		inin=inbuf;
		endin=temp+512;
		}
	}

onum(num)
	int  num; {

	if (num < 0) {
		oc('-');
		num=-num;
		}
	if (num >= 9)
		onum(num/10);
	oc((num % 10)+'0');
	}

ohex(num)
	unsigned num; {

	onibble(num>>12);
	onibble(num>>8);
	onibble(num>>4);
	onibble(num);
	oc(' ');
	}

ohnum(num)
	char num; {

	onibble(num>>4);
	onibble(num);
	oc(' ');
	}

onibble(nib)
	char nib; {

	nib&=15;
	if (nib > 9) oc(nib+55);
	else oc(nib+'0');
	}

ocrlf() {
	oc(13);
	oc(10);
	}

os(str)
	char *str; {
	
	while (*str)
		oc(*str++);
	}

oc(ch)
	char ch; {
	if (suppress == 0) putchar(ch);
	}
