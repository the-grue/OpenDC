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
	wrap.c

    (c) Copyright 1982-1989 Michael Ouye, All Rights Reserved

	This module contains the code used for word-wrapping paragraphs.
*/

#include "world.h"

/*
  word-wrap:

     remargins the following <repeat count paragraphs> 

     paragraph: all lines up to the next blank (all white space) lines

     indenting: indent of the first line determines the indenting
                  for the paragraph

     word:  sequences of non-white space characters except '-'

     specials:  comma ends a word, single blank placed after it
                period ends a word, double blank placed after it

*/

int cur_col;
int first_word;

/*
	move_word attempts to move a word from the current_char position
	if this would exceed the wrap width, then a CR LF is inserted.
	CR LF from the source stream are deleted
*/

static int move_word() {
	char last_one, *cptr;

	/* pointer to the start of white space */
	buf_ptr[TEMP2] = free_start-1; block[TEMP2] = 0;
	while (1) { /* first, eliminate any white space */
		if (*current_char == ' ' || *current_char == tab || *current_char == cr)
			current_char++; /* ignore it */
		else if (*current_char == lf) {
			current_char++;
			return (1);
		}
		else if (*current_char == eof_mark || current_char == buf_end)
			if (endblocks) {
				if (shuffle(0,1) == -1) return (2);
			}
			else return (2);
		else
			break; /* non-white space encountered */
	}

	/* now process the word */
	/* pointer to start of the word */
	buf_ptr[TEMP3] = current_char; block[TEMP3] = 0;
	cptr = current_char;
	while (1) {
		if (!isspace(*cptr)) {
			if (cptr == buf_end) {
				if (endblocks) {
					if (shuffle(0,1) == -1) return (0);
					else cptr = current_char;
				}
				else return (2);
			}
			last_one = *cptr;
			cptr++;
			cur_col++;
			if ((cur_col >= options.width) && !first_word) {
				/* won't fit, wrap the word to the next line */
				free_start = buf_ptr[TEMP2];   /* delete the old white space */
				*free_start++ = cr;
				*free_start++ = lf;
				rewindow(buf_ptr[TEMP3]);
				cur_col = 0;
				/* add the previous indent */
				cptr = buf_ptr[TEMP1];
				while (isspace(*cptr)) {
					if (*cptr == ' ')
						cur_col++;
					else if (*cptr == tab)
						cur_col += ( abs(tabsize) - (cur_col % abs(tabsize)));
					*free_start++ = *cptr++;
				}
				first_word = true;
				return (0);
			}
		}
		else {
			rewindow(cptr);
			/* now add some white space */
			*free_start++ = ' ';
			cur_col++;
			if (last_one == '.') {
				*free_start++ = ' ';
				cur_col++;
			}
			first_word = false;
			return (0);
		}
	}
}


/*
	wrap words until the paragragh is completed
*/
static void wrap_paragraph() {
	char *ptr;
	int result;

	cur_col = 0;
	first_word = true;
	/* skip the first indent, if blank line, quit */
	while (1) {
		if (current_char == buf_end) {
			if (endblocks) {
				if (shuffle(0,1) == -1) break;
			}
			else break;
		}
		if (*current_char == ' ') {
			rewindow(current_char+1);
			cur_col++;
		}
		else if (*current_char == tab) {
			rewindow(current_char+1);
			cur_col += ( abs(tabsize) - (cur_col % abs(tabsize)));
		}
		else if (*current_char == cr)
			rewindow(current_char+1);
		else if (*current_char == lf) {
			rewindow(current_char+1);
			return;			/* blank line */
		}
		else
			break;
	}

	altered = true;
	while (1) {
		result = move_word();
		if (result == 1) { /* new input line, check for blank line */
			ptr = current_char;
			while (isspace(*ptr) && ptr < buf_end) {
				if (*ptr == lf) {  /* blank line, bye */
					while (*(--free_start) == ' ');
					free_start++;
					*free_start++ = cr;
					*free_start++ = lf;
					return;
				}
				ptr++;
				if (ptr >= buf_end) {
					if (endblocks) {
						if (shuffle(0,1) != -1) ptr = current_char;
					}
					else ptr = buf_end;
				}
			}
		}
		else if (result == 2)
			return; /* end of file */
	}
}


void Wrap_cmd(repeat)
	int repeat; {
	char ch, key();

	command("Wrap");
	message("wrap this paragraph? (y/n): ");
	ch = toupper(key());
	if (ch != 'Y' && ch != 'W') return;

	/* setup last jump pointer */
	buf_ptr[JUMPL] = current_char;
	block[JUMPL] = 0;
	
	backup(0);						/* beginning of the line */
	buf_ptr[TEMP1] = current_char;		/* pointer to the indent */
	block[TEMP1] = 0;
	cur_col = 0;

	while (repeat--) {
		wrap_paragraph();
	}
	buf_ptr[TEMP1] = current_char; block[TEMP1] = 0;
	review(1);
	forcetag(TEMP1);
	rewindow(buf_ptr[TEMP1]);
	rewrite_line(true);
	cursor(1);
	message("");
}
  
