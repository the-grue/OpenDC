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
/*	GO.C  --	Go and step code for D88	*/

#include "DEBUG.H"
char scr_window_top;

/*	GO  --	go command.		*/


go() {
	struct vstruct vt;
	int i;

	flipped=0;
	needname=1;
	inbreak(3);

	if (repeat == 0) repeat=1;
	if (goflip && flipok) {
		scr_rest();
		flipped=1;
		}
	while (repeat--) {
		xstep();
		for (i=0; i < 4; i++) {
			if (brk[i].gseg != -1) {
				brk[i].gbyte=_peek(brk[i].goff,brk[i].gseg);
				_poke(int3,brk[i].goff,brk[i].gseg);
				}
			}

		xgoto();
		while (--i >= 0) {
			if (brk[i].gseg != -1)
				_poke(brk[i].gbyte,brk[i].goff,brk[i].gseg);
			}
		}
	if (flipped) {
		flipped=0;
		scr_save();
		lineno=scr_window_top;
		}

	if (mainflag) {
		goflip=flipok;
		mainflag=0;
		origss=rss;
		origds=rds;
		rsp-=100;			/* make room for strings */
		_lmove(10,rsp+100,origss,rsp,origss);
		highbp=rbp=rsp-2;
		strings=rsp+10;

	/*	look for a variable called num_overley.	*/

		overley=0;	/* must be in root	*/
		overley_at=0;
		lastsym=symbols;
		while (*lastsym) {
			if (*lastsym == OPTYPE && strcmp("NUM_OVERLAY",lastsym+2) == 0) {
				while (*lastsym++) ;
				overley_at=lastsym->loc;
				break;
				}
			lastsym+=*(lastsym+1);
			}
		}

	where();		/* try to list 9 lines of source	*/
	if (golist && curline && curoff < 50) {
		strcpy(listname,curname);
		list(3);
		}
	}




/*	BREAKP  --  set a  sticky breakpoint.	*/

breakp() {
	int  i;

	prompt("enter number of sticky breakpoint, 1 2 or 3.");
	i=inch()-'0';
	if (i < 1 || i > 3) error("illegal value");
	inbreak(i-1);
	}



/*	INBREAK  --  input a breakpoint address.  */

inbreak(bnum)
	int  bnum; {
	char btype,rfile;
	int  bline;
	struct vstruct vt;

	brk[bnum].gseg=-1;
	prompt("Address-break  Line-number-break  Procedure  Forever");
	btype=inch();
	if (btype == 'A' || btype == 'P') {
		prompt("enter procedure name or address");
		glineno();
		expression(&vt);
		toptr(&vt);
		if (vt.vtype == OPPV) error("illegal address");
		brk[bnum].goff=vt.val.vali;
		brk[bnum].gseg=is_big ? *(&vt.val.vali+1) : origcs;
		if (brk[bnum].gseg == origds) brk[bnum].gseg=rcs;
		}
	else if (btype == 'L') {
		prompt("input line number");
		bline=indig();
		lastsym=symbols;
		rfile=0;
		while (*lastsym) {
			if (*lastsym == ONAME) {
				if (strcmp(listname,lastsym+2) == 0) rfile=1; else rfile=0;
				}
			else if (*lastsym == OLINE && rfile && bline == lastsym->lline) {
				brk[bnum].goff=lastsym->lloc;
				if (is_big) brk[bnum].gseg=lastsym->lloc_seg;
				else brk[bnum].gseg=origcs;
				return;
				}
			lastsym+=*(lastsym+1);
			}
		error("line not found");
		}
	else if (btype != 'F') error("illegal command");
	}




/*	STEP  --	execute instructions while user hits space */

step(spopt)
	char spopt; {
	unsigned fromoff,tooff,lineoff,lineseg,firstl,lastl,retaddr,bpwas,newcur;
	unsigned retseg;
	char knowhere,ch,again,dirtych;

	again=dirtych=0;
	flipped=0;
start_over:
	newcur=0;
	if (again) where();
	needname=1;
	fromoff=lastl=0;
	tooff=firstl=0xffff;
	if (spopt) {
		if (procseg == 0) error("not in a C procedure");
		fromoff=procloc;
		tooff=procend;
		}

/*	install breaks	*/
	needline=-1;
	ovnum=0;
	lastsym=symbols;
	while (*lastsym) {
		if (*lastsym == OLINE && (ovnum == 0 || ovnum == overley)) {
			lineoff=lastsym->lloc;
			if (is_big) lineseg=lastsym->lloc_seg;
			else lineseg=origcs;
			if (lineoff >= fromoff && lineoff < tooff &&
				(is_big == 0 || lineseg == rcs)) {
				if (lastsym < firstl) firstl=lastsym;
				if (lastsym > lastl) lastl=lastsym;
				if (lineoff == rip) needline=lastsym;
				else {
					if (is_big) lastsym->big_lbyte=_peek(lineoff,lineseg);
					else lastsym->lbyte=_peek(lineoff,lineseg);
					_poke(int3,lineoff,lineseg);
					}
				}
			}
		else if (*lastsym == OOV) ovnum=*(lastsym+2);
		lastsym+=*(lastsym+1);
		}


	step_prompt(spopt);
	again=knowhere=1;
	while (1) {
		if (knowhere == 0) where();
		if (newcur) curline=newcur;
		if (flipped == 0) {
			scr_rowcol(3,0);
			needname=1;
			header();
			repeat=1;
			list(2);
			while (col < 40) {
				xscr_co(' ');
				}
			dputs(" ? ");
			scr_set_cursor();
			}
		if (dirtych) {
			ch=dirtych;
			dirtych=0;
			}
		else ch=toupper(dscr_ci());

		if (ch == 'F' && flipok) {
			if (flipped) {
				scr_save();
				step_prompt(spopt);
				}
			else scr_rest();
			flipped=1-flipped;
			continue;
			}

		if (ch == 'P') {
			step_cleanup(firstl,lastl);
			spopt=1;
			dirtych=' ';
			goto start_over;
			}

		if (ch == 'S') {
			step_cleanup(firstl,lastl);
			spopt=0;
			dirtych=' ';
			goto start_over;
			}

		if (ch != ' ') {
			if (flipped) {
				scr_save();
				flipped=0;
				}
			step_cleanup(firstl,lastl);
			lf();
			return;
			}
		if (flipped == 0) lf();

		if (spopt) {	/* put break at return address */
			_lmove(2,rbp+2,rss,&retaddr,localss);
			_lmove(2,&int3off,localss,rbp+2,rss);
			if (is_big) {
				_lmove(2,rbp+4,rss,&retseg,localss);
				_lmove(2,&int3seg,localss,rbp+4,rss);
				}
			bpwas=rbp;
			}
		xgoto();
		if (spopt) {	/* fix break at return address */
			_lmove(2,&retaddr,localss,bpwas+2,rss);
			if (is_big) _lmove(2,&retseg,localss,bpwas+4,rss);
			if (rip == int3off) {
				rip=retaddr;
				if (is_big) rcs=retseg;
				step_cleanup(firstl,lastl);
				goto start_over;
				}
			}

		if (overley_at && _peek(overley_at,origds) != overley) {
			step_cleanup(firstl,lastl);
			goto start_over;
			}
		newcur=0;
		if (needline != -1) {
			if (is_big) needline->big_lbyte=_peek(needline->lloc,needline->lloc_seg);
			else needline->lbyte=_peek(needline->lloc,origcs);
			if (is_big) _poke(int3,needline->lloc,
				needline->lloc_seg);
			else _poke(int3,needline->lloc,origcs);
			needline+=*(needline+1);
			if (*needline == OLINE && needline->lloc == rip &&
				(is_big == 0 || needline->lloc_seg == rcs)) {
				newcur=needline->lline;
				if (is_big) _poke(needline->big_lbyte,needline->lloc,
					needline->lloc_seg);
				else _poke(needline->lbyte,needline->lloc,origcs);
				knowhere=1;
				continue;
				}

			needline=-1;
			}

		ovnum=knowhere=0;
		lastsym=firstl;
		while (lastsym <= lastl) {
			if (*lastsym == OLINE && lastsym->lloc == rip &&
				(is_big == 0 || lastsym->lloc_seg == rcs) && 
				(ovnum == 0 || ovnum == overley)) {
				if (is_big) _poke(lastsym->big_lbyte,lastsym->lloc,
					lastsym->lloc_seg);
				else _poke(lastsym->lbyte,lastsym->lloc,origcs);
				newcur=lastsym->lline;
				needline=lastsym;
				break;
				}
			if (*lastsym == OOV) ovnum=*(lastsym+2);
			lastsym+=*(lastsym+1);
			}
		}
	}


step_prompt(spopt)
	char spopt; {
	char pstr[80];

	strcpy(pstr,"Flip   Proc-step   Step   space=");
	if (spopt) strcat(pstr,"Proc-step");
	else strcat(pstr,"Step");
	strcat(pstr,"   default=quit.");
	prompt(pstr);
	}



step_cleanup(firstl,lastl)
	unsigned firstl,lastl; {

	lastsym=firstl;
	while (lastsym <= lastl) {
		if (*lastsym == OLINE) {
			if (is_big) {
				if (lastsym->big_lbyte != 0xcc) _poke(lastsym->big_lbyte,
					lastsym->lloc,lastsym->lloc_seg);
				}
			else if (lastsym->lbyte != 0xcc)
				 _poke(lastsym->lbyte,lastsym->lloc,origcs);
			}
		lastsym+=*(lastsym+1);
		}
	}

