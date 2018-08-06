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
	screen.c

	(c) Copyright 1983-1989 Michael Ouye, All Rights Reserved

	This module keeps the screen up-to-date.
*/

#include "world.h"
#include "extern.h"

/*
	if scr_scrolldown is true, then rewrite the screen to scroll the
		 text on the screen up a line.
	if scr_scrollup is true, then the simple upward scrolling is assumed
		 and a flag is set to rewrite the prompt line.
*/


void init_screen() {

	start_clip = 0; end_clip = scr_cols-1;
	scr_clr();
	current.row = current.col = 0;
	scr_rowcol(current.row, current.col);
}

char *next_line(ptr, func)
	char *ptr; int (*func) (); {

	while(*ptr != lf) {
		if (ptr < buf_end) {
			if (func != 0 && *ptr != cr) (*func)(*ptr);
			ptr++;
		}
	 	if (ptr >= buf_end) {
			if (endblocks+srcblocks) {
		 		if (shuffle(0,1) != -1) ptr = current_char;
		 		else {
		 			ptr = buf_end;
		 			break;
	 			}
	 		}
			else {
				ptr = buf_end;
				break;
			}
		}
	}
	return (ptr); /* returns pointing to lf or eof */
}

void rewrite_line(whole_line)
	int whole_line; { /* rewrite the current line */
	int save_col;
	char *save;
	 
	buf_ptr[TEMP3] = buf_ptr[TEMPA];
	block[TEMP3] = block[TEMPA];
	save_col = col;
	if (whole_line) {
		cursor(1);
		col = 0;
		buf_ptr[TEMPA] = current_char;
		block[TEMPA] = 0;
		backup(0);
		scr_rowcol(current.row, 0);
	}
	next_line(current_char, out_co);
	if (whole_line) rewindow(buf_ptr[TEMPA]);
	buf_ptr[TEMPA] = buf_ptr[TEMP3];
	block[TEMPA] = block[TEMP3];
	col = save_col;
	scr_clrl();
	scr_rowcol(current.row, col);
}

int backup(count)
	int count; { /* backup count lines, 0 => beginning of line */
	char *ptr, *chr_minus();
	int how_many;
	int special;

	if (count < 0) {
		count = 1;
		special = true;
	}
	else special = false;
	how_many = 0;
	if (!foreblocks && (free_start == buf_start)) return 0;
	ptr = free_start-1;
	while(1) {
		while(1) {
			if (*ptr == lf) break;
			ptr = chr_minus(ptr);
			if (ptr == 0) {
				ptr = buf_start;
				break;
			}
		}
		if (count-- == 0 || (foreblocks == 0 && ptr == buf_start)) break;
		if (*ptr == lf) ptr++;
		how_many++;
		ptr = chr_minus(ptr);
		if (ptr == 0) ptr = buf_start;
		ptr = chr_minus(ptr);
		if (ptr == 0) ptr = buf_start;
	}
	if (ptr > free_start) ptr = current_char;
	else if (*ptr == lf) ptr++;
	rewindow(ptr);
	return(how_many);
}

int forup(count)
	int count; { /* forward count lines, 0 => end of line */
	char *ptr, *chr_plus();
	int how_many;
	int special;

	if (count < 0) {
		count = 1;
		special = true;
	}
	else special = false;

	how_many = 0;
	ptr = current_char;
	while(1) {
		/* skip to the end of the current line */
		while(*ptr != cr && *ptr != lf) {
			ptr = chr_plus(ptr);
			if (ptr == 0) {
				ptr = buf_end;
				break;
			}
		}
		if (count-- == 0 || ptr == buf_end) break;
		/* skip over the end-of-line */
		while(*ptr != lf) {
			ptr = chr_plus(ptr);
			if (ptr == 0) {
				ptr = buf_end;
				break;
			}
		}
		how_many++;
		if (ptr != buf_end)
			ptr++; /* beginning of the next line, may be cr */
		if (count == 0) break;
	}
	rewindow(ptr);
	return(how_many);
}

int new_col(chr) 
	char chr; {

	if (chr == tab) {
		if (tabsize != 0)
			return ( abs(tabsize) - (col % (abs(tabsize))));
		else
			return (default_tab - (col % default_tab));
	}
	else
		return 1;
}

void dump_line(ptr)
	char *ptr; {

	col = 0;
	next_line(ptr, out_co);
};


void out_line(ptr) /* output a line to the screen */
	char *ptr; {

	col = current.col;
	next_line(ptr, out_co);
}
	
void out_co(chr)
	char chr; {
	int count,i;

	if (chr == '\0') chr = ' ';
	else if (chr == 255) chr = ' ';

	if (chr == tab) {
		count = new_col(chr);
		for (i = 1; i <= count; i++) {
			if (col >= start_clip && col <= end_clip) scr_co(' ');
			col++;
		}
	}
	else {
		if (col >= start_clip && col <= end_clip) {
			scr_co( (chr == eof_mark) ? 248 : chr);
		}
		col++;
	}
}

void rescreen(where, start_line)
	char *where; int start_line; {
	char *save;
	int i,j,fix;

	if (buf_ptr[top_line] == buf_end && where == buf_end) {
		/* nothing to display, bug fix for delete leftovers */
		scr_rowcol(top_line,0);
		scr_cls();
		return;
	}
	buf_ptr[TEMP4] = buf_ptr[TEMPB];
	block[TEMP4] = block[TEMPB];
	buf_ptr[TEMPB] = current_char;
	block[TEMPB] = 0;
	backup(0); /* set to beginning of the line */
	for (i = start_line; i < scr_rows; i++) {
		if (where == free_start)	/* second buffer */
			where = current_char;
		if (buf_ptr[i] != where) {
			scr_rowcol(i,0); col = 0;
			if (where == buf_end) {
				buf_ptr[i] = where-1;
				fix = 1;
			}
			else {
				buf_ptr[i] = where;
				fix = 0;
			}
			block[i] = 0;
			where = next_line(where, out_co);
			if (fix) buf_ptr[i]++;
			if (*where == lf) where++;
			scr_clrl();
		}
		else {
			if (where == buf_end) {
				scr_rowcol(i,0); col = 0;
				scr_clrl();
				i = scr_rows;
			}
			where = next_line(where, 0);
			if (*where == lf) where++;
		}
	}
	rewindow(buf_ptr[scr_rows-1]); /* to reset the pointers */
	rewindow(buf_ptr[TEMPB]);
	buf_ptr[TEMPB] = buf_ptr[TEMP4];
	block[TEMPB] = block[TEMP4];
}

void new_screen(where)
	char *where; {

	rescreen(where, top_line);
}

/*
	review: rewrite the entire screen. 
		new_one:	0: rescreen as necessary
					1: rescreen 3 line previous to the current position
					2: force complete rescreen
					3: rewrite the current window
*/
void review(new_one)
	int new_one; {
	char *ptr;
	int i, last_line;

	buf_ptr[TEMP5] = current_char;
	block[TEMP5] = 0;
	backup(0);
	if (new_one) {
		if (new_one == 3) /* this screen */
			rewindow(buf_ptr[top_line]);
		if (new_one >= 2) /* insure rewrite */
			for (i = top_line; i < scr_rows; i++) buf_ptr[i] = 0;
		if (new_one == 1)
			backup(3);
		new_screen(current_char);
	}
	else { /* otherwise, figure out what to do with the screen */
		for(i = scr_rows-1; i > top_line; i--)
			if (buf_ptr[i] != 0) break;
		last_line = i;

		if (block[top_line] != 0 || buf_ptr[TEMP5] < buf_ptr[top_line]) { /* prior to the first line */
			if (!scr_scrolldown && ((next_line(current_char, 0)+1) == buf_ptr[top_line])) { /* scroll */
				for (i = scr_rows-1; i > top_line; i--)
					buf_ptr[i] = buf_ptr[i-1];
				scr_scdn();
				new_screen(current_char);
			}
			else {
				backup(3);
				new_screen(current_char);
			}
		}
		else if (last_line != scr_rows-1) { /* simply redisplay */
			review(1);
		}
		else if (current_char > next_line(buf_ptr[last_line],0)+1 ) {
			buf_ptr[TEMPB] = current_char;
			rewindow(buf_ptr[top_line]);
			if (next_line(buf_ptr[last_line], 0)+1 == buf_ptr[TEMPB]) { /* scroll */
				for(i = top_line; i < scr_rows-1; i++) 
					buf_ptr[i] = buf_ptr[i+1];
				scr_scup();
				if (scr_scrollup) reprompt = true;
				new_screen(buf_ptr[top_line]);
			}
			else {
				rewindow(buf_ptr[TEMPB]);
				backup(3);
				new_screen(current_char);
			}
		}
		else {
			forcetag(top_line);
			new_screen(buf_ptr[top_line]);
		}
	}
	forcetag(TEMP5);
	rewindow(buf_ptr[TEMP5]);
}

static void check_scroll() {	/* now check for right/left scrolling needs */
	int diff;
	int save_col;

	save_col = col;
	if (col < start_clip) { /* must scroll left */
		diff = start_clip - col; /* number of cols to the cursor pos */
		diff = (diff > 15) ? diff : 15;
		start_clip -= diff; 
		if (start_clip < 0) {
			diff += start_clip; /* reduce the difference */
			start_clip = 0;
		}
		end_clip -= diff;
		review(3);
	}
	else if (col >= end_clip) { /* scroll right */
		diff = col - end_clip;
		diff = (diff > 15) ? diff : 15;
		start_clip += diff; end_clip += diff;
		review(3);
	}
	col = save_col;
	current.col = col - start_clip;
}


/*
 * cursor(set_col): set the cursor/current_char position.
 *		set_col ==	0: move current_char to the location nearest the cursor
 *				==	1: move the cursor to the current_char position
 *				==	2: move the cursor col by the last character
 */
void cursor(set_col)	/* set the cursor position */
	int set_col; {
	int i, diff;
	char *ptr, *save;

	buf_ptr[TEMP2] = buf_ptr[TEMPA];
	block[TEMP2] = block[TEMPA];
	buf_ptr[TEMPA] = current_char;
	block[TEMPA] = 0;
	if (current.row < top_line || current.row >= scr_rows-1 ||
			current_char < buf_ptr[current.row] ||
			current_char >= buf_ptr[current.row+1]) {

		for (i = top_line; i < scr_rows; i++)
			if (buf_ptr[i] >= current_char) break;
		while (i > (top_line+1) && buf_ptr[i-1] == buf_ptr[i-2]) i--;

		if (i != top_line) {
			if (buf_ptr[i] == current_char) {
				/* check for special case on end of file */
				rewindow(buf_ptr[i-1]);
				ptr = next_line(current_char, 0); /* if prev ended in eof */
				if (*ptr == eof) current.row = i-1; /* no cr lf at the end */
				else current.row = i;
				rewindow(buf_ptr[TEMPA]);
			}
			else current.row = i-1; /* new current row */
		}
		else current.row = 2;
	}
	
	if (set_col == 2) {
		col += new_col(*(free_start-1));
		check_scroll();
	}
	else if (set_col == 1) {
		/* move the cursor to the current_char location */
		rewindow(buf_ptr[current.row]);
		ptr = current_char; /* beginning of the current line */
		col = 0;
		while (ptr < buf_ptr[TEMPA]) {
			col += new_col(*ptr++);
		}
		check_scroll();

		rewindow(buf_ptr[TEMPA]);
	}
	else { /* move the memory pointer close to the cursor position */
		rewindow(buf_ptr[current.row]);
		ptr = current_char; /* beginning of the current line */
		col = 0;
		while ((*ptr != lf) && (*ptr != cr) && (ptr < buf_end)) {
			if (col >= start_clip) {
				if ((col-start_clip) >= current.col) break;
			}
			col += new_col(*ptr);
			ptr++;
		}
		rewindow(ptr);
	}
	scr_rowcol(current.row, current.col);
	scr_set_cursor(cursmode);
	buf_ptr[TEMPA] = buf_ptr[TEMP2];
	block[TEMPA] = block[TEMP2];
}
