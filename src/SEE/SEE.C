/*
 *  Released under the GNU GPL.  See http://www.gnu.org/licenses/gpl.txt
 *
 *  This program is part of the SEE editor
 *
 *  SEE is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundatation; either version 2 of the License, or any
 *  later version.
 *
 *  SEE is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 */
/*
	 SEE: V4.2 7/6/92 A screen editor for the IBM personal computer.
		 
	 	(c) Copyright 1982-1993 Michael Ouye, All Rights Reserved
*/

#include "world.h"
#include "extern.h"

#define SIGNON "SEE (tm): Screen Editor V4.2: Copyright 1982-1993 Michael Ouye"

char *signon_screen[] = {
	"SEE (tm): Screen Editor, V4.2",
	"",
	"Copyright 1982-1993, Michael Ouye",
	"All Rights Reserved",
	"",
	"",
	"Distributed by"
	"",
	"CWARE Corporation",
	"P.O. Box 428",
	"Paso Robles, Ca. 93447  USA",
	"(805) 239-4620",
	"",
	"",
	"Hit a key to continue",
	0
};


extern char scr_scrollup;

/* globals */
unsigned repeat;
char *farray[25];
char no_file[1];
char old_ch, ch;					/* last command character */
char ahead = 0;						/* single character lookahead */

char last_again = esc;				/* last F, R or - command for again */

/* forward declarations */
void do_special();
void up_cmd();
void down_cmd();
void left_cmd();
void right_cmd();
void bol_cmd();
void eol_cmd();
void bof_cmd();
void eof_cmd();
void Newline_cmd();
void pageup_cmd();
void pagedown_cmd();
void skip_word();
void Again_cmd();
void Del_end_cmd();
void Quit_cmd();
void Literal();
void DOS_cmd();
void View_cmd();
void Other_cmd();
void Full_cmd();
void install_break();
void initialize();
void display_signon();

int main(argc, argv)
	int argc;
	char *argv[]; {
	int i;
	int top,left,rows,cols;

	extmem_setup();
	install_break();
	window_is_full = 0;
	cursmode = 2;		/* command cursor */
	signon = 1;
	scr_setup();
	options.tab_size = 0;
	options.auto_indent = true;
	options.auto_insert = false;
	options.ignore_case = true;
	options.width = 72;					/* no wrap */
	options.screen_attr = scr_attr;		/* standard video */
	options.brace_indent = false;
	options.word_wrap = false;			/* no word wrap */
	options.tab_expand = 0;

	scr_attr = (char)options.screen_attr;
	scr_curs = (char)options.cursor_height;

	init_buffer();
	view_init();
	macro_init();

	scr_pick(0);
	scr_clr();

	macro_restore();

	/* sets the color attribute */
	scr_info(0,&top,&left,&rows,&cols);
	scr_open(0,top,left,rows,cols);

	scr_pick(views[active_view].scr_id);

	display_signon();

	view_refresh();
	initialize(argc, argv, false);

	switch_buffer(0);
	scr_set_cursor(cursmode);

	while(1) {
		if (options.auto_insert) auto_in();
		execute(1);
	}
	return (0);
}

void clear_ci() { /* blow out the input buffer */
	char scr_csts();

	while (scr_csts())/* scr_ci()*/;
}

void display_signon() {
	int i, len, rows;
	char *ptr;

	if (signon) {
		for (rows = 0, rows; signon_screen[rows] != 0; rows++);

		/* computer row center */
		rows = (scr_rows / 2) - (rows / 2);

		for (i = 0; signon_screen[i] != 0; i++) {
			ptr = signon_screen[i];
			if (*ptr) {
				len = strlen(ptr);
				/* center the line */
				scr_rowcol(i+rows, (scr_cols / 2) - (len / 2));
				while (*ptr)
					scr_co(*ptr++);
			}
		}
		clear_ci();
		scr_ci();
		scr_clr();
	}
}


char my_ci() {
	char scr_ci();
	int new_ch;

	old_ch = ch;
	if (use_macro) { /* read from the macro buffer */
		if (!loading_macros) {
			if (scr_csts() == control_C) {
				break_it = true; 
				return (control_C);
			}
		}
		new_ch = macro_get();
		if (new_ch != -1) {
			if (recording_macro)
				macro_put(new_ch);
			return (new_ch);
		}
		else {
			command(0);
			return (0);
		}
	}

	if (ahead) {
		ch = ahead;
		ahead = 0;
	}
	else
		ch = scr_ci();

	if (ch == control_C) { 
		break_it = true; 
		return ch; 
	}
	if (recording_macro)
		macro_put(ch);
	return ch;
}


static void set_tab(ptr)
	char *ptr; {	/* set the tab 8 to if the file ends in .a anything */
	char *started;

	if (options.tab_size <= 0) {
		started = ptr;
		while (*ptr) ptr++;	/* move to the end of the string */
		while (1) {
			if (ptr <= started) return;	 /* no . */
			if (*ptr == '.') break;
			ptr--;
		}
		/* if we reach this point, a dot was found */
		ptr++;
		if (toupper(*ptr) == 'A') options.tab_size = -8;
		else options.tab_size = -default_tab;
	}
	tabsize = options.tab_size;
}


static void initialize(argc, argv, use_current) /* complete editor initialization */
	int argc; char *argv[]; int use_current; {
	int to_line, buf, first_buf;
	char *ptr;
	char filename[65];
	int found;

	init_screen();
	/* parameter defaults */
	break_it = false;

	to_line = 0;
	message(SIGNON);
	argc--;
	argv++;
	first_buf = -1;

	if (argc == 0) {
		buf = new_buffer();
		if (buf != -1)
			switch_buffer(buf);
		else
			message("Out of Buffers");
	}

	while (argc--) {
		if (**argv != '-') {
			/* input file */
			if (use_current) {
				extmem_clear(cur_buf);
				start_buffer(cur_buf, cur_buf->bs_buf_start, cur_buf->bs_buf_end);
				cur_buf->bs_in_use = true;
				buf = active_buffer;
				active_buffer = -1;
			}
			else {
				buf = new_buffer();
				if (buf == -1) {
					message("Out of Buffers");
					goto stop_processing;
				}
			}

			switch_buffer(buf);
			if (first_buf == -1)
				first_buf = active_buffer;

			if (!First_file(*argv, filename))
				strcpy(filename, *argv);
			argv++;

			do {
				strcpy(in_file,filename);
				set_tab(in_file);
				message("reading file: "); append(in_file); append(" ... ");
				switch (read_file(in_file, 0L, 0)) {
					case -1:	/* error in file read */
						append(" -- new file ");
						break;
					case 0:		/* no problem */
						break;
					case 1:		/* file too large for disk buffering */
						append(" *** disk full *** ");
						break;
				}
				append_num(file_size());
				append(" characters");
				review(1);
				cursor(1);
				found = Next_file(filename);
				if (found) {
					buf = new_buffer();
					if (buf == -1) {
						message("Out of Buffers");
						goto stop_processing;
					}
					switch_buffer(buf);
				}
			} while (found);
		}
		else { /* check for options */
			ptr = *argv++;
			switch(toupper(*(ptr+1))) {
				case 'L':
					to_line = atoi(ptr+2);
					LineNumber(to_line);
					break;
				default:
					break;
			}
		}
	}
stop_processing:

	next_cmd(true);
	only_movement = 0;

	if (first_buf != -1) {
		switch_buffer(first_buf);
		message(in_file);
	}
	views[active_view].buffer_id = active_buffer;
	review(3);
	cursor(1);

}

void put_line(line)
	char *line; {

	if (use_macro) return;
	
	while(*line != 0 && *line != cr && *line != lf) {
		if (cmdmess.col < end_clip) scr_co(*line);
		cmdmess.col++;
		line++;
	}
}

void message(line)	/* write out an editor message */
	char *line; {
	char bufs[10];
	int i;

	scr_rowcol(1,0); cmdmess.row = 1; cmdmess.col = 0;
	if (!use_macro) {
		ltoa(bufs, (long)(active_buffer+1));
		put_line(bufs);
		for (i = cmdmess.col; i < scr_cols; i++)
			scr_co('-');
		scr_rowcol(1,5);
		cmdmess.col = 5;

		put_line(line);
	}

	scr_rowcol(current.row, current.col);
}

char *last_command;	/* save for F9 restart */

void command(line) /* write out a line to the menu command line */
	char *line; {

	if (!use_macro) {
		if (line == 0)
			line = last_command;
		scr_rowcol(0,0); cmdmess.row = cmdmess.col = 0;
		put_line(line);
		scr_clrl();
		last_command = line;
	}
	scr_rowcol(current.row, current.col);
}

void append(line)
	char *line; {

	if (!use_macro) {
		scr_rowcol(cmdmess.row, cmdmess.col);
		put_line(line);
	}
	scr_rowcol(current.row, current.col);
}

void ltoa(buf, val)	/* convert long to decimal */
	char *buf; long val; {
	long div;
	char c;
	int lead_zero;

	lead_zero = true;
	div = 1000000000L;
	while (div != 0) {
		c = (char)((val / div) + '0');
		if (c == '0') {
			if (!lead_zero)
				*buf++ = c;
		}
		else {
			lead_zero = false;
			*buf++ = c;
		}
		val %= div;
		div /= 10;
	}
	if (lead_zero) *buf++ = '0';
	*buf = '\0';
}

void append_num(val)
	long val; {
	long int big_val;
	char buf[20];

	if (use_macro && cmdmess.row == 0) return;
	ltoa(buf, val);
	append(buf);
}

void echo(ch)
	char ch; {

	if (use_macro) return;

	scr_rowcol(cmdmess.row, cmdmess.col);
	if (cmdmess.col < end_clip) scr_co(ch);
	cmdmess.col++;
	scr_rowcol(current.row, current.col);
}

char *menu[2] = {"Again  Buffer  Copy  Delete  Find  -find  Get  Insert  Jump  List     --space--",
			     "Macro  Other  Put  Quit  Replace  Set  Tag  View  Wrap  Xchange       --space--"};
int which_menu;

void remenu(void) {
	if (!use_macro) {
		scr_rowcol(0,0); cmdmess.col = 0; cmdmess.row = 0;
		put_line(menu[which_menu]);
		cmdmess.row = 1; cmdmess.col = 5;
		last_command = menu[which_menu];
	}
	scr_rowcol(current.row, current.col);
	reprompt = false;
}

void next_cmd(cont_mac)
	int cont_mac; { /* setup for a new command */

	repeat = 0;

	if (use_macro && !cont_mac) break_it = true;

	if (!use_macro)
		remenu();

	if (recording_macro)
		macro_next_cmd();
}


void execute(rep)
	int rep;	{ /* main command execution routine */
	char my_ci();

	if (break_it) {
		use_macro = false;
	}

restart:
	if (break_it) rewrite_line(false);
	break_it = false;
	ch = my_ci();
	if (isdigit(ch)) {
		repeat = 0; message("");
		while (isdigit(ch)) {
			repeat = (repeat * 10) + (ch - '0');
			echo(ch);
			ch = my_ci();
		}
	}
	else if (ch == '?') {
		message("?");
		repeat = -1;
		ch = my_ci();
	}
	else if (ch == '/') {
		message("/");
		repeat = 32767;
		ch = my_ci();
	}
	else repeat = 1;

	/* backspace cancels the repeat entry */
	if ((ch == bs && repeat != 1)) {
		message("");
		goto restart;
	}
	if (ch == control_C) break_it = true;
	else {
		if (!loading_macros) {
			if ((ahead = scr_csts()) == control_C) {
				 break_it = true;
			 }
		 }
	 }

	if (break_it) {
		use_macro = false;
		break_it = false;
		return;
	}

	if ((ch >= 200 || ch == bs || ch == up_char || ch == down_char ||
			 ch == lf || ch == left_char || ch == right_char) &&
			 ch != Ins_char || (is_macro(ch) == 1) || ch == 0) {
		do_special(ch, repeat); 
		if (recording_macro)
			macro_next_cmd();
		if (repeat != 1)
			message("");
		return;
	}

	/* else not special */
	switch(toupper(ch)) {
		case ' ':		which_menu = (++which_menu & 1);
						next_cmd(true);
						goto restart;
		case cr:		Newline_cmd(repeat); next_cmd(true); break;
		case 'A':		Again_cmd(repeat); next_cmd(true); break;
		case 'B':		Buffer_cmd(); next_cmd(true); break;
		case 'C':		Copy_cmd(repeat); next_cmd(true); break;
		case 'D':		Delete_cmd(); next_cmd(true); break;
		case 'F':		next_cmd(FindReplace_cmd(repeat,1)); break;
		case '-':		next_cmd(FindReplace_cmd(repeat,3)); break;
		case 'G':		Get_cmd(); next_cmd(true); break;
		case 'Q':		Quit_cmd(); next_cmd(true); break;
		case Ins_char:
		case 'I':		InsertExchange_cmd(repeat, true); next_cmd(true); break;
		case 'J':		JumpTag_cmd(repeat, false); next_cmd(true); break;
		case 'L':		next_cmd(List_cmd(repeat)); break;
		case 'M':		Macro_cmd(); next_cmd(true); break;
		case 'O':		Other_cmd(repeat); next_cmd(true); break;
		case 'P':		Put_cmd(); next_cmd(true); break;
		case 'R':		next_cmd(FindReplace_cmd(repeat,2)); break;
		case 'S':		Set_cmd(); next_cmd(true); break;
		case 'T':		JumpTag_cmd(repeat, true); next_cmd(true); break;
		case 'V':		View_cmd(repeat);next_cmd(true); break;
		case 'W':		Wrap_cmd(repeat); next_cmd(true); break;
		case 'X':		InsertExchange_cmd(0, false); next_cmd(true); break;
		case esc:		message(""); cursor(1); next_cmd(true); break;
		case '[':
		case ']':
		case '(':
		case ')':
		case '}':
		case '{':		Match_cmd(repeat); next_cmd(true); break;
		case '#':		LineNumber(repeat); next_cmd(true); break;
		case '\\':		Literal(repeat); next_cmd(true); break;
		default : 		message("bad command"); next_cmd(true); break;
	}

	if (break_it) {
		use_macro = false;
		break_it = false;
		return;
	}
}

/* process a special key */
void do_special(chr, rep)
	char chr; int rep; {
	char scr_csts();

	if (!loading_macros) {
		if (rep == 1) {
			while ((ahead = scr_csts()) == chr);
		}
	}
	else
		rep = 1;

	if (rep < 0) rep = 1;
	switch(chr) {
		case 0:				break;
		case lf:			if (!only_movement) Del_end_cmd(repeat); break;
		case up_char:		up_cmd(rep,0); break;
		case sc_up_char:	up_cmd(rep,1);break;
		case down_char:		down_cmd(rep, 0); break;
		case sc_down_char:	down_cmd(rep,1);break;
		case left_char:		left_cmd(rep); break;
		case right_char:	right_cmd(rep);break;
		case bol_char:		bol_cmd(rep); break;
		case eol_char:		eol_cmd(rep); break;
		case pageup_char:	pageup_cmd(rep); break;
		case pagedown_char: pagedown_cmd(rep); break;
		case bof_char:		bof_cmd(rep); break;
		case eof_char:		eof_cmd(rep); break;
		case bs:			if (!only_movement) rubout_char(rep); break;
		case Del_char:		if (!only_movement) kill_char(rep); break;
		case NextWord_char: skip_word(rep, true); break;
		case PrevWord_char: skip_word(rep, false); break;
		case M9:			DOS_cmd(rep); break;
		case M10:			Full_cmd(rep); break;
		default:
			if (is_macro(chr) == 1) xMacro_cmd(rep, chr);
			break;
	}
}

char key(rep)
	int rep; { /* input routine for insert and exchange */
	char ch;

	while (1) { /* until a non-special character is recieved */
		ch = my_ci();
		if ((ch >= 200 || ch == bs || ch == up_char || ch == down_char ||
				ch == lf || ch == left_char || ch == right_char) &&
				ch != Ins_char || (is_macro(ch) == 1) || ch == 0) 
			do_special(ch,rep);
		else break;
		rep = 1;
	}
	return ch;
}

void up_cmd(repeat, sc)
	unsigned repeat; int sc; {
	unsigned actual;

	cursor_moved = true;
	if (sc) {
		buf_ptr[TEMP1] = current_char;
		rewindow(buf_ptr[top_line]);
		if ((actual = backup(repeat)) != repeat) {
			if (use_macro) break_it = true;
		}
		if (actual != 0)
			review(2);

		rewindow(buf_ptr[TEMP1]);
		if (actual != 0) {
			backup(actual);
			cursor(0);
		}
		else
			up_cmd(repeat,0);	/* one level recursive call */
	}
	else {
		if (backup(repeat) == repeat) {
			review(0);
			cursor(0);
		}
		else {
			if (use_macro) break_it = true;
			review(0);
			cursor(1);
		}
	}
}

void down_cmd(repeat, sc)
	unsigned repeat; int sc; {
	unsigned actual;

	cursor_moved = true;
	if (sc && (repeat < (scr_rows - top_line))) {
		buf_ptr[TEMP1] = current_char;
		rewindow(buf_ptr[scr_rows-1]);
		if ((actual = forup(repeat)) != repeat) {
			if (use_macro) break_it = true;
		}
		if (actual != 0) {
			rewindow(buf_ptr[top_line+actual]);
			review(2);
		}
		rewindow(buf_ptr[TEMP1]);
		if (actual != 0) {
			forup(actual);
			cursor(0);
		}
		else
			down_cmd(repeat,0);	/* one level recursive call */
	}
	else {
		if (forup(repeat) == repeat) {
			review(0);
			cursor(0);
		}
		else {
			if (use_macro) break_it = true;
			review(0);
			cursor(1);
		}
	}
}

/*
 * chr_minus(ptr): returns a pointer to the previous character.  If
 *		it's already at the beginning of the file, it returns 0.
 */
 char *chr_minus(ptr)
	char *ptr; {

	ptr--;
   	if (ptr < buf_start) {
   		if (foreblocks) {
   			if (shuffle(1,1) != -1) return free_start-1;
   			else return 0;
  			}
		else {
			return 0;
		}
	}
  	return ptr;
}

/*
 * chr_plus(ptr): returns a pointer to the next character. If it's
 *		already at the end of the buffer, it returns 0.
 */
char *chr_plus(ptr)
	char *ptr; {

	ptr++;
   	if (ptr >= buf_end) {
		if (endblocks+srcblocks) {
    		if (shuffle(0,1) != -1) return current_char;
    		else return 0;
		}
   		else return 0;
	}
  	return ptr;
}


void left_cmd(repeat)
	unsigned repeat; {
	char *ptr;

	cursor_moved = true;
	if (free_start == buf_start) {
		if (use_macro) break_it = true;
		return;
	}
	ptr = free_start;
	while(repeat > 0 || *ptr == lf) {
		if (*ptr != lf) repeat--;
		ptr = chr_minus(ptr);
		if (ptr == 0) {
			ptr = buf_start;
			break;
		}
	}
	rewindow(ptr);
	review(0);
	cursor(1);
}

void right_cmd(repeat)
	unsigned repeat; {
	char *ptr;

	cursor_moved = true;
	ptr = current_char;
	if (ptr == buf_end && use_macro) break_it = true;
	while((repeat > 0 && ptr < buf_end) || *ptr == lf) {
		if (*ptr != lf) repeat--;
		ptr = chr_plus(ptr);
		if (ptr == 0) {
			ptr = buf_end;
			break;
		}
   	}
	rewindow(ptr);
	review(0);
	cursor(1);
}

void bol_cmd() {
	
	cursor_moved = true;
	if (old_ch == bol_char) {
		rewindow(buf_ptr[top_line]); /* go to the top of the screen instead */
		ch = old_ch = esc;
	}
	else
		backup(0);	/* beginning of the current line */
	review(0);
	cursor(1);
}

void eol_cmd() {
	
	cursor_moved = true;
	if (old_ch == eol_char) {
		rewindow(buf_ptr[scr_rows-1]); /* cursor to the last line */
		ch = old_ch = esc;
	}
	else
		forup(0);	/* cursor to the end of the current line */
	review(0);
	cursor(1);
}

void Newline_cmd(repeat)
	unsigned repeat; {
	
	bol_cmd();
	down_cmd(repeat, 0);
}

void pageup_cmd(repeat)
	unsigned repeat; {
	int hit, back;

	cursor_moved = true;
	message("");
	rewindow(buf_ptr[top_line]);
	hit = false;
	if ((scr_rows-2) > 9) back = scr_rows - 9;
	else back = scr_rows - 5;
	if (back <= 0) back = scr_rows - 1;

	while (repeat--) {
	if (backup(back) < back) { hit = true; break; }
	}
	review(1);
	cursor(0);
}

void pagedown_cmd(repeat)
	unsigned repeat; {
	int foreward;

	cursor_moved = true;
	message("");
	rewindow(buf_ptr[scr_rows-1]);
	foreward = scr_rows - 3;
	if (foreward <= 4) {
		foreward = scr_rows - 1;
		forup(3);
	}

	while(--repeat) {
		if (forup(foreward) < foreward) break;
	}
	review(1);
	cursor(0);
}

void bof_cmd() {
	
	cursor_moved = true;
	message("");
	shuffle(1,foreblocks);	/* drag all forward blocks in */
	rewindow(buf_start);
	review(1);
	cursor(1);
}

void eof_cmd() {
	
	cursor_moved = true;
	message("");
	shuffle(0,endblocks+srcblocks);	/* drag all end blocks in */
	rewindow(buf_end);
	review(1);
	rewindow(buf_end);
	cursor(1);
}

void skip_word(repeat, direction)
	unsigned repeat, direction; {
	char *ptr, cur_type;
	
	ptr = current_char;
	if (*ptr == eof || ptr == buf_start) 
		if (use_macro) break_it = true;

	cursor_moved = true;
	if (direction) { /* forwards */
		ptr = current_char;
		while(repeat--) {
			while(ptr != 0 && (isalpha(*ptr) || isdigit(*ptr) || *ptr == '_') && *ptr != eof) {
				ptr = chr_plus(ptr); /* skip current word */
			}
			if (ptr == 0) {
				ptr = buf_end;
				break;
			}
			while(ptr != 0 && !(isalpha(*ptr) || isdigit(*ptr) || *ptr == '_')) {
				if (*ptr == eof) break;
				ptr = chr_plus(ptr);
			}
			if (ptr == 0) {
				ptr = buf_end;
				break;
			}
		}
	}
	else { /* backwards */
		cur_type = 0;
		if (isalpha(*current_char) || isdigit(*current_char) || *ptr == '_') cur_type = 1;
		if (free_start == buf_start) return;
		ptr = free_start-1;
		while(repeat--) {
			if (cur_type) {
				while(ptr != 0 && (isalpha(*ptr) || isdigit(*ptr) || *ptr == '_')) {
					ptr = chr_minus(ptr); /* skip current word */
				}
				if (ptr == 0) {
					ptr = buf_start;
					break;
				}
			}
			else cur_type = 1;
			while(ptr != 0 && !(isalpha(*ptr) || isdigit(*ptr) || *ptr == '_')) {
				ptr = chr_minus(ptr);
			}
			if (ptr == 0) {
				ptr = buf_start;
				break;
			}
			while(ptr != 0 && (isalpha(*ptr) || isdigit(*ptr) || *ptr == '_')) {
				ptr = chr_minus(ptr); /* skip current word */
			}
			if (ptr == 0) {
				ptr = buf_start;
				break;
			}
			else ptr++;
		}
	}
	rewindow(ptr);
	review(0);
	cursor(1);
}

void Again_cmd(repeat)
	unsigned repeat; {

	if (last_again == 'F')
		FindReplace_cmd(repeat, -1);
	else if (last_again == '-')
		FindReplace_cmd(repeat, -3);
	else if (last_again == 'R')
		FindReplace_cmd(repeat, -2);
}

/*
 * Del_end_cmd: delete to the end of the line.
 */
void Del_end_cmd(rep)
	unsigned rep; {
	char *ptr;
	unsigned count;

	count = 0;
	ptr = current_char;
	while (*ptr != cr) {
		ptr = chr_plus(ptr);
		if (ptr == 0) break;
		count++;
	}
	if (count != 0) {
		kill_char(count);
	}
	/* setup last jump pointer */
	buf_ptr[JUMPL] = current_char;
	block[JUMPL] = 0;
	
}

int verify_overwrite(filename)
	char *filename; {
	int file;
	char ch;

	if (DOSDevice(filename)) return true;
	file = open(filename, 0); /* open for read */
	if (file > 0) {
		close(file);
		while (1) {
			message(filename);
			append(" already exists, overwrite? (y/n)");
			clear_ci();
			ch = scr_ci();
			if (toupper(ch) == 'Y') return (true);
			if (toupper(ch) == 'N') return (false);
			if (ch == esc) return (false);
		}
	}
	return (true);
}

int write_out_file(outfile, verify)
	char *outfile; {
	char ch;
	int result;

	buf_ptr[TEMP1] = buf_ptr[TEMPA];
	block[TEMP1] = block[TEMPA];
	buf_ptr[TEMPA] = current_char;
	block[TEMPA] = 0;
retry_write:
	message("writing to: ");
	append(outfile);
	append(" ... ");
	shuffle(1,foreblocks);	/* beginning of the file */
	rewindow(buf_start);
	buf_ptr[TEMP2] = current_char;
	block[TEMP2] = 0;
	shuffle(0,endblocks+srcblocks);	/* end of the file */
	rewindow(buf_end-1);		/* point at the last character */
	buf_ptr[TEMP3] = current_char;
	block[TEMP3] = 0;

	if (verify) {
		if (verify_overwrite(outfile))
			result = write_file(outfile, 0L, TEMP2,TEMP3);
		else
			result = 1;
	}
	else
		result = write_file(outfile, 0L, TEMP2,TEMP3);

	forcetag(TEMPA);
	rewindow(buf_ptr[TEMPA]);
	buf_ptr[TEMPA] = buf_ptr[TEMP1];
	block[TEMPA] = block[TEMP1];

	if (result) {
		message("can't write to file ");
		append(outfile);
		clear_ci();
		append(" try again? (y/n)");
		ch = toupper(scr_ci());
		if (ch == 'Y') goto retry_write;
		message("");
		return 0;
	} 
	else {
		altered = false;
		message("");
		return 1;
	}
}

/*
 * get_line(line, blankit): line-edited input routine.
 *		if blankit is (1), start from scratch, otherwise
 *		edit the data in the line.  If blankit is (2),
 *		move the cursor to the end of the line
 */
int get_line(line,blankit)
	char *line;
	int blankit; { /* read in a line-edited line */
	int mode, collect_space;
	int start_col, tempcol;
	char ch, *ptr, *end;
	char *cptr, chr;
	char *up_ptr, *down_ptr;
	int edited = true;

	scr_mark(*current_char);
	mode = 1;	/* start in exchange mode */
	ptr = line;
	see_mode = true;
	start_col = cmdmess.col;
	scr_rowcol(cmdmess.row, cmdmess.col); scr_set_cursor(mode);
	scr_clrl();
	up_ptr = current_char;
	if (copy_size == 0 || copy_size > sizeof(copy_buffer))
		down_ptr = 0;
	else
		down_ptr = copy_buffer;

	if (blankit == 1) {
		*line = 0;
		end = line;
		scr_clrl();
	}
	else {
		end = line+strlen(line);
		append(line);
		ptr = line;
		if (blankit == 0)
			cmdmess.col = start_col;
		else {
			cmdmess.col = start_col+strlen(line);
			ptr = end;
		}
		scr_rowcol(cmdmess.row, cmdmess.col); scr_set_cursor(mode);
	}
	ch = my_ci();
	if (ch == esc || ch == cr) {
		cmdmess.col = start_col + strlen(line);
		scr_rowcol(current.row, current.col); scr_set_cursor(cursmode);
		see_mode = false;
		edited = true;
		goto end_lineedit;
	}
	cptr = current_char;
	while (1) {
		if (break_it) {
			message("");
			scr_rowcol(current.row, current.col); scr_set_cursor(cursmode);
			see_mode = false;
			break_it = true;
			edited = false;
			goto end_lineedit;
		}
		if (ch == esc || ch == cr || ch == 0) {
			if (ch == esc)
				*end = 0;
			else {
				*ptr = 0;
				scr_clrl();
			}
			cmdmess.col = start_col + strlen(line);
			scr_rowcol(current.row, current.col); scr_set_cursor(cursmode);
			see_mode = false;
			goto end_lineedit;
		}
		else if (ch == bs) {
			if (ptr > line) {
				_move(end - ptr+1, ptr, ptr-1);
				end--;
				ptr--;
				scr_rowcol(cmdmess.row, --cmdmess.col);
				scr_clrl();
				tempcol = cmdmess.col;
				append(ptr);
				scr_rowcol(cmdmess.row, cmdmess.col = tempcol); scr_set_cursor(mode);
			}
		}
		else if (ch == lf) {
			/* delete to the end of the line */
			*ptr = 0;
			end = ptr;
			scr_clrl();
		}
		else if (ch == Del_char) {
			if (ptr < end) {
				_move(end - ptr+1, ptr+1, ptr);
				end--;
				tempcol = cmdmess.col;
				append(ptr);
				scr_rowcol(cmdmess.row, cmdmess.col);
				scr_clrl();
				scr_rowcol(cmdmess.row, cmdmess.col = tempcol);
				scr_set_cursor(mode);
			}
		}
		else if (ch == left_char) {
			if (ptr > line) {
				ptr--;
				scr_rowcol(cmdmess.row, --cmdmess.col);
				scr_set_cursor(mode);
			}
		}
		else if (ch == right_char) {
			if (ptr < end) {
				ptr++;
				scr_rowcol(cmdmess.row, ++cmdmess.col);
				scr_set_cursor(mode);
			}
		}
		else if (ch == bol_char) {
			ptr = line;
			scr_rowcol(cmdmess.row, cmdmess.col = start_col);
			scr_set_cursor(mode);
		}
		else if (ch == eol_char) {
			ptr = end;
			scr_rowcol(cmdmess.row, cmdmess.col = start_col+strlen(line));
			scr_set_cursor(mode);
		}
		else if (ch == Ins_char) {
			mode = !mode;
			scr_set_cursor(mode);
		}
		else if (ch == PrevWord_char) {
			;	/* ignore it for now */
		}
		else if (ch == NextWord_char) {
			;	/* ignore it for now */
		}
		else if (ch == up_char) {
			collect_space = 1;
			cptr = up_ptr;
			while (cptr) {
				if (isspace(*cptr) && !collect_space)
					break;
				if (!isspace(*cptr))
					collect_space = 0;

				if (mode == 0) {
					_move(end-ptr, ptr, ptr+1);
					end++;
				}
				chr = *cptr;
				*ptr++ = chr;
				cptr = chr_plus(cptr);

				if (ptr >= end)
					end = ptr;

				*end = 0;
				if (!use_macro) {
					scr_co(chr);
					tempcol = ++cmdmess.col;
					append(ptr);
					scr_rowcol(cmdmess.row, cmdmess.col = tempcol);
					scr_set_cursor(mode);
				}
			}
			up_ptr = cptr;
		}
		else if (ch == down_char) {
			collect_space = 1;
			cptr = down_ptr;
			while (cptr) {
				if ((isspace(*cptr) && !collect_space) ||
						((unsigned)(cptr - copy_buffer) >= copy_size))
					break;
				if (!isspace(*cptr))
					collect_space = 0;

				if (mode == 0) {
					_move(end-ptr, ptr, ptr+1);
					end++;
				}
				chr = *cptr;
				*ptr++ = chr;
				cptr++;
				if (cptr > copy_buffer + copy_size)
					cptr = 0;

				if (ptr >= end)
					end = ptr;

				*end = 0;
				if (!use_macro) {
					scr_co(chr);
					tempcol = ++cmdmess.col;
					append(ptr);
					scr_rowcol(cmdmess.row, cmdmess.col = tempcol);
					scr_set_cursor(mode);
				}
			}
			down_ptr = cptr;
		}
		else {
			if (mode == 0) {
				_move(end-ptr, ptr, ptr+1);
				end++;
				*end = 0;
			}

			*ptr++ = ch;
			if (ptr >= end) {
				end = ptr;
			}
			*end = 0;
			if (!use_macro) {
				scr_co(ch);
				tempcol = ++cmdmess.col;
				append(ptr);
				scr_rowcol(cmdmess.row, cmdmess.col = tempcol);
				scr_set_cursor(mode);
			}
		}
		ch = my_ci();
	}
	see_mode = false;
end_lineedit:
    scr_unmark(*current_char);
	return edited;
}

int verify() {
	char ch;

	while (1) {
		message(in_file);
		append(" modified, ignore the changes? (y/n):");
		clear_ci();
		ch = scr_ci();
		if (toupper(ch) == 'Y') return (true);
		if (toupper(ch) == 'N') return (false);
		if (ch == esc) return (false);
	}
	return (false);
}

int do_init(use_current, save_buf, active)
	int use_current, save_buf, active; {
	char edline[81], *filechr, *cptr;
	int args;
	int switch_back = false;

	if (use_current) {
		if (save_buf != -1) {
			if (buffers[save_buf].bs_altered) {
				switch_back = true;
				switch_buffer(save_buf);
				view_select(active);
				views[FILE_VIEW].scr_id = -1;
				view_refresh();
				if (!verify()) return false;
			}
		} else {
			if (altered)
				if (!verify()) return true;
		}

		if (switch_back) {
			views[FILE_VIEW].scr_id = 0;
			switch_buffer(FILE_BUFFER);
			view_select(FILE_VIEW);
			review(3);
			cursor(1);
		}

		command("Initialize");
		message("file ");
	}
	else {
		command("New");
		message("file [ file... ] ");
	}

	strcpy(edline, in_file);

	if (!get_line(edline,0)) return false;

	if (save_buf != -1) {
		switch_buffer(save_buf);
		view_select(active);
		views[FILE_VIEW].scr_id = -1;
		view_refresh();
	}

	_move(sizeof(line), edline, line);
	args = 1;
	filechr = line;
	farray[0] = "";

	while(*filechr) {
		while(isspace(*filechr)) filechr++;
		farray[args] = filechr;
		while((!isspace(*filechr)) && (*filechr != 0)) filechr++;
		if (isspace(*filechr)) *filechr++ = 0; /* terminator */
		args++;
	}
	if (use_current && (args > 2))
		args = 2;
	initialize(args,farray,use_current);
	return true;
}

int update_file(backup)
	int backup; {
	char BAKname[65], *filechr;
	char savefile[65];
	int BAKlen,status;

	if (*in_file == 0) {
		message("no input file");
		clear_ci();
		return 0;
	}

	if (backup) {
		BAKlen = 0;
		filechr = in_file;
		while ( (ch = *filechr++) && ch != '.') BAKname[BAKlen++] = ch;
		strcpy(&BAKname[BAKlen],".BAK");
		unlink(BAKname); /* delete the old BAK file */
		status = rename(in_file, BAKname);
		if (status < 0) {
			message(" cannot create .BAK file");
			return 0;
		}
		/* copy the BAK name into in_file in case source is still
				in the file */
		strcpy(savefile, in_file);
		strcpy(in_file, BAKname);
	}
	else
		strcpy(savefile, in_file);

	if (!write_out_file(savefile, false)) {
		/* rename the BAK file back to the original name, if fails */
		if (backup)
			rename(BAKname, in_file);
		return 0;
	}
	strcpy(in_file, savefile);
	altered = false;
	return 1;
}

void Quit_cmd() {
	char write_name[128];
	char filename[65];
	char ch, *cptr;
	int i;
	int all = false;
	int active, save_buf = -1;

	active = active_view;
	write_name[0] = 0;
try_again:
	message(in_file);
	append(": ");
	if (altered)
		append(" has been modified, ");
	append_num(file_size());
	append(" characters");

	while(1) {
		command("Quit: [/]BAKup  DOS  Exit  Files  Init  New  [/]Save  [/]Update  Write");
		ch = toupper(key(1));
		if (ch == '/') {
			all = true;
			continue;
		}

		if (ch == esc || break_it) { 
			message(""); 
			break_it = false; 
			goto exit_quit;
		}
		if (ch == 'B') { /* write out file, create BAK file if old file */
			command("BAKup");
			if (!all)
				update_file(true);
			else {
				for (i = 0; i < max_files; i++) {
					if (buffers[i].bs_in_use && buffers[i].bs_altered) {
						switch_buffer(i);
						message(in_file);
						if (!update_file(true)) {
							clear_ci();
							review(3);
							cursor(1);
						}
					}
				}
			}
		}
		else if (ch == 'E') {
time_to_go:
			for (i = 0; i < max_files; i++) {
				if (buffers[i].bs_in_use && buffers[i].bs_altered) {
					switch_buffer(i);
					review(3);
					cursor(1);
					if (!verify()) goto try_again;
					altered = false;
				}
			}

			command("Exit");
			if (options.on_exit_save)
				macro_save();

			/* delete any temporary files */
			clear_buffers();

			message("Bye! (");
			append(in_file);
			append(")");
			scr_pick(0);		/* make sure it's the whole screen */
			scr_rowcol(scr_rows-1,0);
			scr_set_cursor(0);
			scr_term();
			exit(0);
		}
		else if (ch == 'I') {
			if (do_init(true, save_buf, active))
				save_buf = -1;
			goto exit_quit;
		}
		else if (ch == 'N') {
			if (do_init(false, save_buf, active))
				save_buf = -1;
			goto exit_quit;
		}
		else if (ch == 'U' || ch == 'S') {
			if (ch == 'U') command("Update");
			else {
				command("Save and Exit");
			}
			if (!all)
				update_file(false);
			else {
				for (i = 0; i < max_files; i++) {
					if (buffers[i].bs_in_use && buffers[i].bs_altered) {
						switch_buffer(i);
						message(in_file);
						if (!update_file(false)) {
							clear_ci();
							review(3);
							cursor(1);
						}
					}
				}
			}
			if (ch == 'S') goto time_to_go;
		}
		else if (ch == 'W') {
			command("Write");
			message("output file: ");
			if (!get_line(write_name,1)) 
				goto exit_quit;
			if (write_name[0] == 0) 
				goto exit_quit;
			cptr = write_name;
			while(isspace(*cptr)) cptr++;
			write_out_file(cptr, true);
		}
		else if (ch == 'F') {
			message("filename specification: ");
			if (!get_line(write_name, 1))
				goto exit_quit;
			if (write_name[0] == 0)
				strcpy(write_name, "*.*");

			if (save_buf == -1) {
				save_buf = active_buffer;
				views[FILE_VIEW].scr_id = 0;
				switch_buffer(FILE_BUFFER);
				view_select(FILE_VIEW);
				scr_clr();
				review(1);
				cursor(1);
				message("");
			}

			/* delete the old contents */
			rewindow(buf_start);
			buf_ptr[TEMPA] = current_char;
			block[TEMPA] = 0;
			rewindow(buf_end);
			buf_ptr[TEMPB] = current_char;
			block[TEMPB] = 0;
			delete_chars(TEMPA, TEMPB);
			buffers[FILE_BUFFER].bs_altered = false;

			if (First_file(write_name, filename)) {
				do {
					strcpy(free_start, filename);
					free_start += strlen(filename);
					*free_start++ = cr;
					*free_start++ = lf;
				} while (Next_file(filename));
				rewindow(buf_start);	/* back to the beginning */
				review(1);
				cursor(1);
				message(write_name);
			}
			else {
				message("No filenames match ");
				append(write_name);
			}
		}
		else if (ch == 'D') {
			DOS_cmd(32767);	/* bring up DOS & roll out data */
			return;
		}
	}
exit_quit:
	if (save_buf != -1) {
		switch_buffer(save_buf);
		view_select(active);
		views[FILE_VIEW].scr_id = -1;
		view_refresh();
		cursor(1);
	}

	buffers[FILE_BUFFER].bs_altered = false;
}

void Literal(rep)
	int rep; {
	int val, atoi();

	if (rep < 0) rep = 1;
	command("Literal: enter the decimal equivalent of the character");
	while (rep--) {
		message("");
		if (!get_line(line, 1)) break;
		val = atoi(line);
		if (val >= 0 && val <= 255 && (free_start != current_char)) {
			makespace(1);
			if (current_char == buf_ptr[current.row])
				buf_ptr[current.row] = free_start;
			*free_start++ = (char) val;
			rewrite_line(true);
		}
	}
	cursor(1);
	/* setup last jump pointer */
	buf_ptr[JUMPL] = current_char;
	block[JUMPL] = 0;
}

/* PC-DOS command interpreter */
 
void DOS_cmd(repeat)
	unsigned repeat; {
	extern int scr_rows;
	int status;
	char exec_line[80];
	extern char _msdos2;

	if (repeat == 32767) {
		message("writing data to the spill file");
		if (!spill_buffers()) {
			message("unable to write data to the spill file");
			return;
		}
	}

	message("");
	if (_msdos2 == 0) return;
	scr_pick(0);		/* make sure it's the whole screen */

	scr_rowcol(scr_rows-1, 0);
	scr_set_cursor(0);
	status = environment("COMSPEC", exec_line, sizeof(exec_line));
	if (status > 0)
		status = exec(exec_line, "");

	/* whole screen is still selected */
	scr_attr = (char)options.screen_attr;
	scr_clr();
	view_refresh();

	command(0);
	if (status < 0) {
		message("exit code: ");
		append_num((long)status);
	}
	else
		message("");
	review(3);
	cursor(1);
}

void View_cmd(repeat)
	int repeat; {
	int view;

	if (repeat < 0 || (view_count() <= 1 && repeat != 32767)) {
		message(SIGNON);
		review(1);
		cursor(1);
		return;
	}

	if (window_is_full) Full_cmd();

	if (repeat == 32767) {
		/* on /V, split the current view in two */
		view = view_new();
		if (view == -1) return;
		view_select(view);
		if (current.row >= scr_rows)
			review(1);
	}
	else if (repeat == 0) {
		view_select(0);
	}
	else {
		while (repeat--)
			view_select(view_next());
	}
	switch_buffer(views[active_view].buffer_id);
	review(3);
	cursor(1);
	message(in_file);
}

void Other_cmd(repeat)
	int repeat; {

	if (buffer_count() <= 1) {
		do_init(false, -1, active_view);
		return;
	}

	if (repeat < 0)
		repeat = 1;

	if (repeat == 0) {
		switch_buffer(0);
	}
	else if (repeat == 32767) {
		switch_buffer(last_buffer());
	}
	else {
		while (repeat--)
			switch_buffer(next_buffer());
	}
	review(3);
	cursor(1);
	views[active_view].buffer_id = active_buffer;
	message(in_file);
}

void Full_cmd() {
	int i, active;

	active = active_view;
	if (window_is_full) {
		window_is_full = 0;
		views[active_view].scr_id = full_id;
		view_refresh();
		view_select(active);
		switch_buffer(views[active_view].buffer_id);
		if (current.row > scr_rows)
			review(1);
		cursor(1);
	}
	else {
		window_is_full = 1;
		full_id = views[active_view].scr_id;
		views[active].scr_id = 0;		/* the full screen view */
		active_view = -1;
		view_select(active);
		review(3);
		cursor(1);
	}
	message(in_file);
}

