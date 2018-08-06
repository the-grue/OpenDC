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
	more.c

	(c) Copyright 1983-1989 Michael Ouye

	More SEE commands.
*/

#include "world.h"
#include "extern.h"

/* forward declarations */
char movechr();
int  blankline();

int match_string(target, ptr, direction)
	char *target, *ptr; int direction; {
	char *tar, *source;
	int len;

#if DEBUG
	if (ptr < buf_start || ptr > buf_end || (ptr >= free_start && 
			ptr < current_char)) { 
		debug_str("MORE: ptr is out of bounds %x-%x %x %x-%x", buf_start, 
							free_start, ptr, current_char, buf_end);
		scr_term();
		exit(1); 
	}
#endif
	if (direction) {
		source = ptr;
		tar = target;
		while (*tar != 0) {
			if (options.ignore_case) {
				if (toupper(*tar) != toupper(*source)) break;
			}
			else {
				if (*tar != *source) break;
			}
			tar++; source++;
			if (source > buf_end) break;
		}
		if (*tar == 0) {
			if (*source == '\n') source--;
			buf_ptr[TEMP3] = source; block[TEMP3] = 0;
			return true;
		}
		return false;
	}
	else {
		len = strlen(target);
		if (len == 1) {
			if (*ptr == '\n') ptr--;
			buf_ptr[TEMP3] = ptr; block[TEMP3] = 0;
			return true;
		}
		tar = &target[len-1];
		source = ptr;
		while (tar >= target) {
			if (options.ignore_case) {
				if (toupper(*tar) != toupper(*source)) break;
			}
			else {
				if (*tar != *source) break;
			}
			tar--; source--;
			if (source < buf_start) break;
		}
		if (tar == target - 1) {
			if (*(source+1) == '\n') source--;
			buf_ptr[TEMP3] = source+1; block[TEMP3] = 0;
			return true;
		}
		return false;
	}
}

static process_literals(dest, src)
	char *dest, *src; {

	while (*src) {
		if (*src == '~') {
			src++;
			switch (*src) {
				case 'r': *dest++ = '\r'; src++; break;
				case 't': *dest++ = '\t'; src++; break;
				case 'n': *dest++ = '\n'; src++; break;
				default:  *dest++ = *src++; break;
			}
		}
		else
			*dest++ = *src++;
	}
	*dest = 0;
}


int List_cmd(repeat) /* list lines with the given string */
	int repeat; {
	int listed, status;
	char *ptr, ch, *f_find(), *chr_plus(), *chr_minus();
	char target[128];
	char fline[128]; /* for find string editing */


	if (repeat < 0) repeat = 1;
	command("List:  <esc | Q> to quit");
	message("");
	strcpy(fline, find_replace_target);
	if (get_line(fline,0)) {
		strcpy(find_replace_target,fline);
	}
	if (break_it) return(false);
	if (find_replace_target[0] == 0) return(false); /* nothing to find */
	scr_rowcol(2,0); scr_cls();
	message("");
	listed = top_line;
	buf_ptr[TEMP3] = ptr = current_char; block[TEMP3] = 0;
	backup(0); /* beginning of the current line */
	status = true;

	process_literals(target, find_replace_target);

	while (repeat-- > 0) {
		while(1) {
			if (ptr >= buf_end ||
					(ptr = f_find(target[0], ptr, buf_end-ptr+1, options.ignore_case)) >= buf_end) {
				if (endblocks+srcblocks) {
					if (shuffle(0,1) != -1) ptr = current_char;
				}
				else ptr = buf_end;
				if (ptr >= buf_end) {
					message("can't find \""); append(find_replace_target);append("\"");
					append(" hit a key to continue");
					my_ci();
					status = false;
					goto exit_list;
				}
			}
			else  {
				if ((ptr + strlen(target)) >= buf_end) {
					buf_ptr[TEMP2] = ptr;
					shuffle(0,1);
					rewindow(buf_ptr[TEMP2]);
					ptr = current_char;
				}

				if (match_string(target, ptr, true)) {
					if (listed++ >= scr_rows-1) {
						message(" hit a key to continue");
						ch = my_ci();
						if(break_it || ch == esc || toupper(ch) == 'Q') { 
							message(""); 
							goto exit_list; 
						}
						listed = top_line;
						scr_rowcol(top_line,0); scr_cls();
					}
					scr_rowcol(listed,0);
					while(*ptr != lf && ptr > current_char) ptr--;/*beginning*/
					if (*ptr == lf) ptr++;
					dump_line(ptr);
					forcetag(TEMP3);
					ptr = buf_ptr[TEMP3];
					/* move to the end of the line */
					while(*ptr != lf && ptr != buf_end) {
						ptr = chr_plus(ptr);
						if (ptr == 0) {
							ptr = buf_end;
							break;
						}
					}
					break;
				}
				else ptr++;
			}
		}
/*		if (repeat != 0) ptr++; */
	}
	message("done -- hit a key to continue");
	my_ci();
exit_list:
	message("");
	forcetag(TEMP3);
	rewindow(buf_ptr[TEMP3]);
	review(2);
	cursor(1);
	return (status);
}


int FindReplace_cmd(repeat, direction)
	int repeat,direction; { /* find the given string */
	char *ptr, *f_find(), *r_find(), ch;
	int sc, last_line, i, status;
	extern char last_again;
	int query;
	int find_str_len, replace_str_len;
	char target[128], replace[128];
	char fline[128]; /* for find string editing */

	buf_ptr[TEMP1] = buf_ptr[TEMPA];
	block[TEMP1] = block[TEMPA];

	if (repeat < 0) {
		query = 1;
		repeat = 32767;
	}
	else query = 0;

	if (abs(direction) == 1) {
		last_again = 'F';
		command("Find");
	}
	else if (abs(direction) == 2) {
		command("Replace");
		if (repeat != 0)
			last_again = 'R';
	}
	else {/* 3 || -3 */
		command("Reverse Find");
		last_again = '-';
	}
	message("");
	strcpy(fline, find_replace_target);
	if ((direction > 0) && get_line(fline,0)) {
		strcpy(find_replace_target,fline);
	}
	if (break_it) {
		return(false);
	}

	if ((abs(direction) == 2) && find_replace_target[0] != 0) {
		append(" with ");
		strcpy(fline, replace_source);
		if (direction > 0) {
			if (get_line(fline, 0)) {
				strcpy(replace_source, fline);
			}
		}
	}
	if (break_it) {
		return(false);
	}

	buf_ptr[TEMP3] = current_char; block[TEMP3]= 0;
	if (current_char == buf_end) buf_ptr[TEMP3] = free_start-1;
	if (abs(direction) < 3) ptr = current_char;
	else ptr = free_start-1;
	if (find_replace_target[0] == 0) {
		message("");
		return(false); /* nothing to find */
	}

	message("searching for \"");
	append(find_replace_target);
	append("\"");

	process_literals(target, find_replace_target);
	process_literals(replace, replace_source);
	find_str_len = strlen(target);
	replace_str_len = strlen(replace);

	status = true;
	while (repeat-- > 0) {
		if (abs(direction) < 3) {
			while(1) {
				if (scr_csts() == control_C) break_it = 1;
				if (break_it) {
					review(2);
					cursor(1);
					return (false);
				}
				if (ptr >= buf_end ||
						(ptr = f_find(target[0], ptr, buf_end-ptr+1, options.ignore_case)) >= buf_end) {
					if (endblocks+srcblocks) {
						if (shuffle(0,1) != -1) ptr = current_char;
					}
					else ptr = buf_end;
					if (ptr >= buf_end) {
						message("can't find \""); append(find_replace_target);append("\"");
						status = false;
						goto exit_find;
					}
				}
				else {
					if ((ptr + find_str_len) >= buf_end) {
						buf_ptr[TEMP2] = ptr;
						block[TEMP2] = 0;
						rewindow(ptr);
						if (shuffle(0,1) == -1) {
							message("can't find \""); append(find_replace_target);append("\"");
							status = false;
							rewindow(buf_ptr[TEMP2]);
							goto exit_find;
						}
						rewindow(buf_ptr[TEMP2]);
						ptr = current_char;
					}
					if (match_string(target, ptr, direction)) {
						if (query) {
							rewindow(ptr);
							review(0);
							cursor(1);
							ptr = current_char;
							if (abs(direction) == 1) {
								message("continue ? (y/n)");
								ch = toupper(scr_ci());
								message("");
								if (ch != 'Y') {
									repeat = 0;
									break;
								}
							}
							else {
								message("replace ? (y/n) or quit (q)");
								ch = toupper(scr_ci());
								message("");
								if (ch == 'Q') {
									repeat = 0;
									break;
								}
								else if (ch != 'Y') {
									ptr++;
									break;
								}
							}
						}
						if (abs(direction) == 1) {
							if (repeat != 0) ptr++;
							break;
						}
						/* otherwise replace it */
						altered = true;
						rewindow(ptr); /* string is at current char */
						if (buf_ptr[current.row] == current_char) buf_ptr[current.row] = free_start;
						current_char += find_str_len; /* delete it */
						if (makespace(replace_str_len) == 0) {
							strcpy(free_start, replace);
							free_start += replace_str_len; /* insert it */
						}
						ptr = current_char;
						if (query) rewrite_line(true);
						break;
					}
					else ptr++;
				}
			}
		}
		else { /* reverse search */
			while(1) {
				if (scr_csts() == control_C) break_it = 1;
				if (break_it) {
					review(2);
					cursor(1);
					return (false);
				}
				if (ptr <= buf_start ||
						(ptr = r_find(target[find_str_len-1],
													ptr, ptr-buf_start, options.ignore_case)) <= buf_start) {
					if (foreblocks != 0) {
						if (shuffle(1,1) != -1) ptr = free_start-1;
					}
					else ptr = buf_start;
					if (ptr <= buf_start) {
						message("can't find \""); append(find_replace_target);append("\"");
						status = false;
						goto exit_find;
					}
				}
				else {
					if ((ptr - find_str_len) < buf_start) {
						buf_ptr[TEMP2] = ptr;
						block[TEMP2] = 0;
						rewindow(ptr);
						if (shuffle(1,1) == -1) {
							message("can't find \""); append(find_replace_target);append("\"");
							status = false;
							rewindow(buf_ptr[TEMP2]);
							goto exit_find;
						}
						rewindow(buf_ptr[TEMP2]);
						ptr = current_char;
					}
					if (match_string(target, ptr, (abs(direction) == 3) ? 0: 1 )) {
						if (query) {
							rewindow(ptr);
							review(0);
							cursor(1);
							ptr = free_start;
							message("continue? (y/n)");
							ch = toupper(scr_ci());
							message("");
							if (ch != 'Y') goto exit_find;
						}
						break;
					}
					else ptr--;
				}
			}
			if (repeat != 0) ptr--;
		}
	}
exit_find:
	forcetag(TEMP3);
	rewindow(buf_ptr[TEMP3]);
	if (abs(direction) == 3) {
		buf_ptr[TEMPA] = current_char;
		block[TEMPA] = 0;
		backup(0);
		review(0);
		forcetag(TEMPA);
		rewindow(buf_ptr[TEMPA]);
	}
	else if (abs(direction) == 1) {
		review(0);
	}
	else { /* replace */
		for (i = scr_rows-1; i >= top_line; i--) {
			if (buf_ptr[i] != 0) {
				last_line = i;
				break;
			}
		}
		if (sorttag(TEMP3,last_line) == TEMP3) {
			review(3); /* insure a rewrite */
		}
		else {
			review(2); /* rewrite the screen */
		}
	}
	cursor(1);
	buf_ptr[TEMPA] = buf_ptr[TEMP1];
	block[TEMPA] = block[TEMP1];

	if (status) {
		/* setup last jump pointer */
		buf_ptr[JUMPL] = current_char;
		block[JUMPL] = 0;
	}

	return (status);
}

/*static*/ int indent_chars = 0;
static int in_insert = false;
static int rub_ins = false;

void InsertExchange_cmd(repeat, which)
	int repeat, which; {
	char ch, key();
	char *ptr, *last_ptr;
	int add_indent, rewrite, reset, i, tablen;

	see_mode = true; /* indicate in Insert/Exchange */	
change_mode:
	add_indent = false;
	if (which) {
		command("Insert: <cursor keys>, Esc to exit, Ins for Exchange");
		in_insert = true;
		scr_set_cursor(cursmode = 0);
	}
	else {
		command("Exchange: <cursor keys>, Esc to exit, Ins for Insert");
		in_insert = false;
		scr_set_cursor(cursmode = 1);
	}
	rub_ins = false;
	cursor(1);
	indent_chars = 0;
	if (repeat == 32767) {
		repeat = 0;
		*(--current_char) = lf;
		*(--current_char) = cr;
		review(0);
		cursor(1);
		scr_clrl();
		cursor(1);
	}
	add_indent = false;
	while(1) {
		cursor_moved = false;
		ch = key(1);
		if ((ch == esc) || break_it) { /* time to quit */
			if (options.auto_indent && add_indent && !cursor_moved)
				free_start += indent_chars;
			else if (buf_ptr[current.row] == free_start)
				buf_ptr[current.row] = current_char;
			scr_rowcol(current.row, 0);
			rewrite_line(true);
			cursor(1);
			message("");
			add_indent = false; indent_chars = 0;
			in_insert = false;
			see_mode = false;
			scr_set_cursor(cursmode = 2);
			return;
		}
		else {
			if (free_start+100 >= current_char) {
				/* free up another block of memory */
				buf_ptr[TEMPA] = current_char;
				block[TEMPA] = 0;
				/* first try from the forward buffer, then the end buffer */
				if (makeroom(0,-1) <= 0) {
					if (makeroom(1,-1) <= 0) {
						message("no more room for insertions");
						add_indent = false;
						indent_chars = 0;
						in_insert = false;
						see_mode = false;
						forcetag(TEMPA);
						rewindow(buf_ptr[TEMPA]);
						return;
					}
				}
				forcetag(TEMPA);
				rewindow(buf_ptr[TEMPA]);
			}
			if (ch == Ins_char) { /* switch modes */
				which = !which;
				if (options.auto_indent && add_indent && !cursor_moved) free_start += indent_chars;
				cursor(1);
				goto change_mode;
			}
			altered = true;
			if (ch == cr) {
				scr_clrl();

				add_indent = false;
				last_ptr = free_start-1;
				*free_start++ = cr;
				*free_start++ = lf;
				if (current_char == buf_ptr[current.row])
					buf_ptr[current.row] = free_start-2;

				if (start_clip != 0) {
					buf_ptr[TEMPA] = current_char;
					block[TEMPA] = 0;
					backup(1);
					cursor(1);
					forcetag(TEMPA);
					rewindow(buf_ptr[TEMPA]);
				}

				cursor(1);
				if (options.auto_indent) {

					/* copy the blank spaces from the previous line */
					buf_ptr[TEMPA] = current_char;
					block[TEMPA] = 0;
					indent_chars = 0;
					rub_ins = true;

					if (backup(1) == 1) {
						buf_ptr[TEMPB] = current_char;
						block[TEMPB] = 0;
						forcetag(TEMPA);
						rewindow(buf_ptr[TEMPA]);
						ptr = buf_ptr[TEMPB];
						while (*ptr == ' ' || *ptr == tab) {
							*free_start++ = *ptr++;
							indent_chars++;
						}
					}
					else {
						forcetag(TEMPA);
						rewindow(buf_ptr[TEMPA]);
					}
					if (options.brace_indent) { 
						if (*last_ptr == '{') {
							if (options.tab_expand) {
        						tablen = abs(tabsize);
		        				for (i = 0; i < tablen; i++) {
				        			*free_start++ = ' ';
    	    						indent_chars++;
   	    						}
							}
							else {
    							*free_start++ = tab;
	    						indent_chars++;
    						}
						}
					else if (*last_ptr == '}') {
							if (options.brace_indent == 2) {
								if (*(free_start-1) == tab) {
									free_start--;
									indent_chars--;
								}
							}
						}
					}
					review(0);
					cursor(1);
					/* now delete the blanks if blank line */
					if (blankline(current_char, indent_chars)) {
						free_start -= indent_chars;
					}
					else indent_chars = 0;
				}
				else {
					review(0);
					cursor(1);
				}
			}
			else {
				if (cursor_moved) cursor(1);
				if (which || (*current_char == cr) || (*current_char == eof)) {
					add_indent = false;
					if (options.auto_indent && !cursor_moved) {
						free_start += indent_chars;
					if (options.brace_indent == 1 && ch == '}' && indent_chars != 0) {
							free_start--;
							if (*free_start != tab) {
								for (i=0; i < abs(tabsize)-1 && i < indent_chars; i++)
									free_start--;
							}
							if (buf_ptr[current.row] == free_start)
								buf_ptr[current.row] = current_char;
							review(0);
							cursor(1);
						}
					}
					indent_chars = 0;
					tablen = 0;
					if (ch == tab && options.tab_expand) {
						tablen = new_col(tab);
						for (i = 0; i < tablen; i++)
							*--current_char = ' ';
						if (current.col == 0)
							buf_ptr[current.row] = current_char;
						rewrite_line(true);
						rewindow(current_char + tablen);
						cursor(1);
						continue;
					}
					else {
						*--current_char = ch;
						if (current.col == 0)
							buf_ptr[current.row] = current_char;
						/* setup last jump pointer */
						buf_ptr[JUMPL] = current_char;
						block[JUMPL] = 0;
					}
				}
				else { /* exchange */
					if (ch == tab && options.tab_expand) {
						tablen = new_col(tab);
						for (i = 0; i < tablen; i++) 
							*current_char-- = ' ';
						current_char++;
						if (current.col == 0)
							buf_ptr[current.row] = current_char;
						rewrite_line(true);
						rewindow(current_char + tablen);
						cursor(1);
						/* setup last jump pointer */
						buf_ptr[JUMPL] = current_char;
						block[JUMPL] = 0;
						continue;
					}
					else
						*current_char = ch;
				}
				if (options.word_wrap && ch != ' ') {
					if (current.col + start_clip >= options.width) {
						ptr = free_start-1;
						while (*ptr != ' ' && *ptr != tab && *ptr != lf) ptr--;
						if (*ptr == lf) ptr = current_char;
						else ptr++;
						buf_ptr[TEMP1] = current_char+1;
						block[TEMP1] = 0;
						rewindow(ptr);
						cursor(1);
						scr_clrl();
						*free_start++ = cr;
						*free_start++ = lf;
						if (current_char == buf_ptr[current.row])
							buf_ptr[current.row] = free_start-2;
						if (start_clip != 0) {
							cursor(1);
							review(3);
							cursor(1);
						}
						if (options.auto_indent) {
							/* copy the blank spaces from the previous line */
							buf_ptr[TEMPA] = current_char;
							block[TEMPA] = 0;
							indent_chars = 0;
							rub_ins = true;
							if (backup(1) == 1) {
								buf_ptr[TEMPB] = current_char;
								block[TEMPB] = 0;
								forcetag(TEMPA);
								rewindow(buf_ptr[TEMPA]);
								ptr = buf_ptr[TEMPB];
								while (*ptr == ' ' || *ptr == tab) {
									*free_start++ = *ptr++;
								}
							if (ch == '}' && options.brace_indent == 1)
									free_start--;
							}
							else {
								forcetag(TEMPA);
								rewindow(buf_ptr[TEMPA]);
							}
							add_indent = false;
						}
						forcetag(TEMP1);
						rewindow(buf_ptr[TEMP1]);
						review(0);
						cursor(1);
					}
					else {
						rewrite_line(false);
						rewindow(current_char+1);
						cursor(2);
					}
				}
				else {
					rewrite_line(false);
					rewindow(current_char+1);
					cursor(2);
				}
			}
		}
	}
}

void delete_chars(start,end) /* delete chars between tags */
	int start, end; {
	int i,firsttag,temp;

	altered = true;
	firsttag = sorttag(start,end);
	if (firsttag != start) {
		temp = start;
		start = end;
		end = temp;
	}

	forcetag(start);
	rewindow(buf_ptr[start]);
	while (1) {
		if (block[end] == 0) {
			current_char = buf_ptr[end]+1;
			break;
		}
		current_char = buf_end;
		rewindow(current_char);
		shuffle(0, free_space() / BLOCKSIZE);
	}
	/* special case if deleting the first line */
	if ((buf_ptr[top_line] >= free_start && buf_ptr[top_line] < current_char)){
		buf_ptr[top_line] = current_char;
	}
	rewindow(current_char);
	/* special case if deleting first char of last line */
	if ((buf_ptr[scr_rows-1] >= free_start && buf_ptr[scr_rows-1] < current_char)){
		buf_ptr[scr_rows-1] = current_char;
	}
	/* special case bug code */
	if (free_start < buf_start) free_start = buf_start;
	if (current_char > buf_end) current_char = buf_end;
	review(0);
	cursor(1);
	rewrite_line(false);
}

/* delete characters */
void kill_char(repeat)
	int repeat; {
	char *ptr;

	if (repeat < 0) repeat = 1;
	buf_ptr[TEMP5] = current_char; block[TEMP5] = 0;
	ptr = current_char;
	while(repeat-- > 0) {
		if (ptr >= buf_end) {
			if (endblocks+srcblocks) {
				if (shuffle(0,1) != -1) ptr = current_char;
			}
			else {
				ptr = buf_end;
				break;
			}
		}
		if (*ptr++ == cr) ptr++;
	}
	cursor_moved = true;
	if (current_char == buf_ptr[TEMP5] && current_char == buf_end) return;
	buf_ptr[TEMP4] = ptr-1; block[TEMP4] = 0;
	delete_chars(TEMP5,TEMP4);
}

/* delete the character(s) prior to the cursor */
void rubout_char(repeat)
	int repeat; {
	char *ptr;
	int redo;

	if (repeat < 0) repeat = 1;
	if (options.auto_indent && rub_ins) {
		free_start += indent_chars;
		rub_ins = false;
		indent_chars = 0;
	}
	if (free_start == buf_start && foreblocks == 0) return;
	buf_ptr[TEMP5] = free_start-1; block[TEMP5] = 0;
	ptr = free_start;
	while(repeat-- > 0) {
		if(ptr <= buf_start) {
			if (foreblocks) {
				if (shuffle(1,1) != -1) ptr = free_start-1;
			}
			else {
				ptr = buf_start;
				break;
			}
		}
		if (*(--ptr) == lf) ptr--;
	}
	cursor_moved = true;
	buf_ptr[TEMP4] = ptr; block[TEMP4] = 0;
	if (buf_ptr[TEMP4] < buf_ptr[top_line]) redo = true;
	else redo = false;
	delete_chars(TEMP4,TEMP5);
	if (redo) {
		backup(0);
		review(0);
	}
	cursor(1);
	/* setup last jump pointer */
	buf_ptr[JUMPL] = current_char;
	block[JUMPL] = 0;
}

void Delete_cmd() {
	char ch, my_ci();
	int no_fit, remess, repeat;
	
	/* write out the start marker */
	buf_ptr[TEMP1] = buf_ptr[TEMPA];
	block[TEMP1] = block[TEMPA];
	cursor(1);
	buf_ptr[TEMPA] = current_char; block[TEMPA] = 0;
    scr_mark(*current_char);
	remess = 1;
	message("select the delete endpoint");
	do {
		if (remess) {
			command("Delete: <cursor keys>  <esc | D>  Again  Find  -find  Jump");
			remess = 0;
		}
		ch = movechr(&remess);
		if (break_it) {
			forcetag(TEMPA);
			rewindow(buf_ptr[TEMPA]);
			review(2);
			cursor(1);
			return;
		}
	} while (ch != esc && ch != 'D');
	no_fit = false;
	/* put the deleted area into the copy buffer */

	if (buf_ptr[TEMPA] == current_char) {
		rewrite_line(true);
		cursor(1);
		return;
	}
	buf_ptr[TEMP4] = free_start-1; block[TEMP4] = 0;
	if (sorttag(TEMPA,TEMP4) != TEMPA) {
		buf_ptr[TEMP4] = current_char; block[TEMP4] = 0;
		buf_ptr[TEMPA]--;
	}
	no_fit = cb_bufin(TEMPA, TEMP4);

	if (no_fit) {
		message("area too large for the copy buffer, delete anyway? (y/n)");
		clear_ci();
		ch = toupper(my_ci()); /* ask the user, no macro input */
		if (ch != 'Y' || break_it) {
			rewindow(buf_ptr[TEMPA]);
			review(2);
			cursor(1);
			return;
		}
	}
	/* if we get here, do the delete */
	delete_chars(TEMPA,TEMP4);
	buf_ptr[TEMPA] = buf_ptr[TEMP1];
	block[TEMPA] = block[TEMP1];
	/* setup last jump pointer */
	buf_ptr[JUMPL] = current_char;
	block[JUMPL] = 0;
}

unsigned goto_line(repeat)
	unsigned repeat; {
	unsigned how_far;

	/* jump to the appropriate line */
	command("Goto line #");
	append_num((long)repeat);
	if (foreblocks)
		shuffle(1,32000);			/* the very beginning */
	rewindow(buf_start);
	if (repeat != 1)
		how_far = forup(repeat-1); /* down repeat-1 lines */
	else how_far = 1;
	review(0);
	cursor(0);
	return how_far;
}


void LineNumber(repeat)
	int repeat; {
	unsigned line_count, how_far;
	extern int col; /* column counter from screen.c */

	if (repeat < 0) repeat = 1;
	if (repeat > 1) {
		how_far = goto_line(repeat);
		message("line # ");
		append_num((long)how_far+1);
		return;
	}
	else {
		buf_ptr[TEMPA] = current_char;
		block[TEMPA] = 0;
		backup(0);
		buf_ptr[TEMPB] = current_char;
		block[TEMPB] = 0;
		line_count = 0;
		if (endblocks == 0 && srcblocks == 0 &&
				buf_ptr[TEMPA] == buf_ptr[TEMPB] &&
				current_char == buf_end) {
		    backup(1);
		    buf_ptr[TEMPB] = current_char;
			if (buf_ptr[TEMPB] != buf_ptr[TEMPA]) line_count = 1;
	    }
		message("line # ");
		if (foreblocks)
			shuffle(1,32000);
		rewindow(buf_start);
		while (!(block[TEMPB] == 0 && current_char == buf_ptr[TEMPB])) {
			forup(1);
			if (++line_count >= 65535) break;
		}
		append_num((long)line_count+1);
		forcetag(TEMPA);
		rewindow(buf_ptr[TEMPA]);
		append(", column # ");
		append_num((long)col+1);
	}
}

/* a preceding repeat count implies jump to line ### */
void JumpTag_cmd(repeat, which)
	int repeat, which; { /* set/jump to user tag */
	char ch, cvt_buf[128];
	int tag_num;

	if (repeat <= 0) repeat = 1;
	if (repeat <= 1) {
		if (which) command("Tag: A, B, C, D, Edit");
		else command("Jump: A, B, C, D, Edit, Line");

		message("");
		ch = toupper(key(1));
		if (break_it) return;
		switch(ch) {
			case 'A':
				tag_num = JUMPA;
				break;
			case 'B':
				tag_num = JUMPB;
					break;
			case 'C':
				tag_num = JUMPC;
				break;
			case 'D':
				tag_num = JUMPD;
				break;
			case 'E':
			case 'J':
				tag_num = JUMPL;
				break;
			case 'L':
				*cvt_buf = 0;
				if (get_line(cvt_buf, 1)) {
					repeat = atoi(cvt_buf);
					goto goto_line;
				}
				return;
			case esc: 
				return;
			default: 
				message("bad tag name");
				return;
		}
	
		if (which){
			buf_ptr[tag_num] = current_char;
			block[tag_num] = 0;
		}
		else {
			forcetag(tag_num);
			rewindow(buf_ptr[tag_num]);
			backup(0);
			review(0);
			rewindow(buf_ptr[tag_num]);
			cursor(1);
		}
	}
	else {
goto_line:
		/* jump to the appropriate line */
		command("Goto line ###");
		if (foreblocks)
			shuffle(1,32000);
		rewindow(buf_start);
		forup(repeat-1); /* down repeat-1 lines */
		review(0);
		cursor(0);
	}
}


/* this routine simply executes an insert and a command for auto insert
	 mode operation.
*/
void auto_in() {

	while(options.auto_insert) {
		InsertExchange_cmd(0, true);
		next_cmd(true);
		execute(1);
	}
}

/* set editor parameters, tab-size, auto-indent, auto-insert */
void Set_cmd() {
	char chr, key(), cvt_buf[128];
	int old_width,val,which_cmd, i;
	int new_height,old_rows, old_cols;
	int top,left,rows,cols;

	which_cmd = 0;

set_command:
	if (which_cmd == 0) {
		command("Set:  Auto-ins(");
		if (options.auto_insert)		append("on");
		else							append("off");
		append(")  Case(");
		if (options.ignore_case)		append("yes");
		else							append("no");
		append(")  Flags  Indent(");
		if (options.auto_indent)		append("yes");
		else							append("no");
		append(")  PC");
	}
	else {
		command("Set:  Right(");
		ltoa(cvt_buf, (long)options.width);
		append(cvt_buf);
		append(")  Spill(");
		if (options.spill) cvt_buf[0] = (char)options.spill;
		else cvt_buf[0] = '@';
		cvt_buf[1] = 0;
		append(cvt_buf);
		append(")  Tabs(");
		if (options.tab_expand)
			append("*");

		if (options.tab_size == 0)
		    ltoa(cvt_buf, (long)abs(default_tab));
		else
		    ltoa(cvt_buf, (long)abs(options.tab_size));
		append(cvt_buf);
		append(")  Word-wrap(");
		if (options.word_wrap == 0)
			append("off");
		else
			append("on");
		append(")  '{'-indent(");
	    if (options.brace_indent == 0)
			append("off)");
	    else if (options.brace_indent == 1)
    		append("1)");
	    else
    		append("2)");
	}
	_setmem(cvt_buf, sizeof(cvt_buf), ' ');
	cvt_buf[scr_cols - cmdmess.col - 10] = 0;
	append(cvt_buf);
	append("--space--");
	message("");
	chr = toupper(key(1));

	switch(chr) {
		case '.': /* disable signon screen */
			signon = 0;
			break;

		case ' ': /* next menu */
			which_cmd = !which_cmd;
			goto set_command;

		case 'A': /* auto-insert mode */
			if (options.auto_insert) message("Reset auto-insert mode? (y/n):");
			else message("Set auto-insert mode? (y/n):");
			chr = toupper(key(1));
			if (chr == 'Y') options.auto_insert = !options.auto_insert;
			if (!use_macro)
				auto_in();
			message("");
			break;

		case 'C': /* case ignore switch */
			if (options.ignore_case) message("Make case significant on searches? (y/n):");
			else message("Ignore case on searches? (y/n):");
			chr = toupper(key(1));
			if (chr == 'Y') options.ignore_case = !options.ignore_case;
			message("");
			break;

		case 'F': /* no longer supported, left for macro compatibility */
			chr = toupper(key(1));
			get_line(cvt_buf, 0);
			break;

		case 'I': /* auto indent mode */
			if (options.auto_indent) message("Reset auto-indent mode? (y/n):");
			else message("Set auto-indent mode? (y/n):");
			chr = toupper(key(1));
			if (chr == 'Y') options.auto_indent = !options.auto_indent;
			message("");
			break;

		case 'P': /* PC color and cursor selection */
			command("Set PC:  Add-^Z  Back-color  Cursor  Fore-color");
			message("");
			chr = toupper(key(1));
			switch(chr) {
				case 'A': /* add control-Z to end of file */
					if (options.contZ) message("Stop adding control-Z? (y/n): ");
					else message("Add control-Z at the end of files? (y/n): ");
					chr = toupper(key(1));
					if (chr == 'Y') options.contZ = !options.contZ;
					message("");
					break;

				case 'C': /* cursor height */
					command("Enter cursor height in pixels");
					message("");
					if (!get_line(line, 1)) return;
					if (scr_curs != 0) scr_set_cursor(0);
					options.cursor_height = atoi(line);
					scr_curs_setup(options.cursor_height);
					cursor(1);
					break;

				case 'F': /* Foreground color */
					command("Enter forground color attribute");
					message("");
					if (!get_line(line, 1)) return;
					val = atoi(line);
					scr_attr = (scr_attr & 0xF0) + (val & 0x0F);
					if (active_view == 0)
						options.screen_attr = scr_attr;
					scr_info(views[active_view].scr_id,&top,&left,&rows,&cols);
					/* set the color */
					scr_open(views[active_view].scr_id,top,left,rows,cols);
					scr_clr();
					review(3);
					break;

				case 'B': /* Background color */
					command("Enter backround color attribute");
					message("");
					if (!get_line(line, 1)) return;
					val = atoi(line);
					scr_attr = (scr_attr & 0x0F) + ((val & 0x0F) << 4);
					if (active_view == 0)
						options.screen_attr = scr_attr;
					scr_info(views[active_view].scr_id,&top,&left,&rows,&cols);
					/* set the color */
					scr_open(views[active_view].scr_id,top,left,rows,cols);
					scr_clr();
					review(3);
					break;

				case 'S': /* screen width & height */
					scr_info(0,&top,&left,&rows,&cols);

					if (!get_line(line, 1)) return;
					val = atoi(line);
					if (val >= 20 && val < 256) {
						cols = val;
					}
					if (!get_line(line, 1)) return;
					val = atoi(line);
					if (val >= 8 && val < num_tags) {
						rows = val;
					}
					scr_rows = rows;
					scr_cols = cols;
					/* modify base view parameters */
					scr_open(0,top,left,rows,cols);
					scr_info(views[active_view].scr_id,&top,&left,&old_rows,&old_cols);
					scr_open(views[active_view].scr_id,top,left,rows,cols);
					scr_clr();
					review(3);
					break;
			}
			message("");
			break;

		case 'R': /* set right column */
			message("enter wrap right column: ");
			old_width = options.width;
			get_line(cvt_buf, 1);
			options.width = atoi(cvt_buf);
			if (options.width <= 0 || options.width > 256) {
				message("bad wrap width (0 < x < 256)");
				options.width = old_width;
			}
			else
				message("");
			break;

		case 'S': /* set spill device */
			message("enter spill device letter: (A-Z, 0 for default)");
			do {
				chr = toupper(key(1));
				if (break_it || chr == esc || chr == cr) {
					message("");
					return;
				}
			} while ((chr < 'A' || chr > 'Z') && chr != '0');
			if (chr != (char)options.spill) {
				extmem_spilldrive(chr);
			}
			if (chr == '0') options.spill = 0;
			else options.spill = (int)chr;
			message("");
			break;

		case 'T': /* set tab size */
			message("select tab size (1 .. 9), 0 for default, * to toggle expansion to spaces:");
			chr = key(1);
			if (isdigit(chr)) {
				options.tab_size = chr - '0';
				tabsize = options.tab_size;
				review(3);
				cursor(1);
			}
			else if (chr == '*') {
				options.tab_expand = !options.tab_expand;
			}
			message("");
			break;

		case 'W':	/* enable word wrapping */
			if (options.word_wrap) message("Reset word wrap mode? (y/n):");
			else message("Set word wrap mode? (y/n):");
			chr = toupper(key(1));
			if (chr == 'Y') options.word_wrap = !options.word_wrap;
			message("");
			break;

			
		case '{':
			message("enter indent mode (0, 1, 2) ");
			chr = toupper(key(1));
			message("");
			if (chr == '0') options.brace_indent = 0;
			else if (chr == '1') options.brace_indent = 1;
			else if (chr == '2') options.brace_indent = 2;
			else message("bad indent mode");
			break;

		case esc:
		case cr:
			message("");
			break;
		default:
			message("bad set name");
	}
}


void Buffer_cmd() {
	char ch;
	int no_fit,i, remess, repeat, reverse;
	
	buf_ptr[TEMP1] = buf_ptr[TEMPA];
	block[TEMP1] = block[TEMPA];
	cursor(1);
	buf_ptr[TEMPA] = current_char;
	block[TEMPA] = 0;
    scr_mark(*current_char);
	remess = 1;
	message("select the buffering endpoint");
	do {
		if (remess) {
			command("Buffer: <cursor keys>  <esc | B>  Again  Find  -find  Jump");
			remess = 0;
		}
		ch = movechr(&remess);
		if (break_it) {
			forcetag(TEMPA);
			rewindow(buf_ptr[TEMPA]);
			review(2);
			cursor(1);
			buf_ptr[TEMPA] = buf_ptr[TEMP1];
			block[TEMPA] = block[TEMP1];
			return;
		}
	} while (ch != esc && ch != 'B');
	if (buf_ptr[TEMPA] == current_char) {
		rewrite_line(true);
		cursor(1);
		return;
	}
	buf_ptr[TEMP4] = free_start-1; block[TEMP4] = 0;
	reverse = 1;
	if (sorttag(TEMPA,TEMP4) != TEMPA) {
		buf_ptr[TEMP4] = current_char; block[TEMP4] = 0;
		buf_ptr[TEMPA]--;
		reverse = 0;
	}
	no_fit = cb_bufin(TEMPA,TEMP4);
	forcetag(TEMP4);
	rewindow(buf_ptr[TEMP4]+reverse);
	if (no_fit) {
		message("area too large for the copy buffer");
		forcetag(TEMPA);
		rewindow(buf_ptr[TEMPA]);
		review(2);
		cursor(1);
	}
	else {
		review(0);
		if (buf_ptr[TEMPA] >= buf_ptr[top_line] && 
				buf_ptr[TEMPA] <= next_line(buf_ptr[scr_rows-1],0)) {
			buf_ptr[TEMPA] = current_char;
			rewindow(buf_ptr[top_line]);
			review(2);
			rewindow(buf_ptr[TEMPA]);
		}
		cursor(1);
	}
	buf_ptr[TEMPA] = buf_ptr[TEMP1];
	block[TEMPA] = block[TEMP1];
}

	
void Copy_cmd(repeat)
	int repeat; {
	unsigned free_space();
	int blocks,freeblocks,newblocks;
	int fix;
	
	if (repeat < 0) repeat = 1;
	if (repeat == 32767) repeat = 1;

	command("Copy");
	message("");

	/* setup last jump pointer */
	buf_ptr[JUMPL] = current_char;
	block[JUMPL] = 0;
	
	if (free_start == buf_start) {
		fix = 0;
		buf_ptr[TEMPA] = free_start;
	}
	else {
		fix = 1;
		buf_ptr[TEMPA] = free_start-1;
	}
	block[TEMPA] = 0;
	while(repeat-- > 0) {
		if (cb_bufout() == -1) {
			message("*** disk full ***");
			repeat = 0;
		}
		forcetag(TEMPA);
		rewindow(buf_ptr[TEMPA]+fix); /* the start of the copied area */
	}
	if (copy_size != 0)
		altered = true;
	if (!fix) {
		rewindow(buf_start);
		review(1);
	}
	else {
		review(0);
		rewrite_line(true);
	}
	cursor(1);
	return;
}


void Get_cmd() {
	char filename[80];
	int fix;

	/* setup last jump pointer */
	buf_ptr[JUMPL] = current_char;
	block[JUMPL] = 0;
	
	buf_ptr[TEMPA] = free_start-1;
	fix = 1;
	block[TEMPA] = 0;
	command("Get: from what file?");
	message("");
	get_line(filename, 1);
	if (break_it || filename[0] == 0) return;
	message("reading from ");
	append (filename);
	append (" ...");
	switch(read_file(filename, 0L, 1)) {
		case -1:
			append(" can't read file ");
			break;
		case 1:
			append(" *** disk full ***");
			break;
		default:
			append(" completed");
	}
	altered = true;
	forcetag(TEMPA);
	rewindow(buf_ptr[TEMPA]+fix);
	review(2);
	cursor(1);
}

void Put_cmd() {
	char filename[80],chr;
	int repeat, remess, reverse;
	/* should put out a marker */

	buf_ptr[TEMP1] = buf_ptr[TEMPA];
	block[TEMP1] = block[TEMPA];
	cursor(1);
	buf_ptr[TEMPA] = current_char;
	block[TEMPA] = 0;
    scr_mark(*current_char);
    remess = 1;
	message("select the put endpoint");
	do {
		if (remess) {
			command("Put: <cursor keys>, <esc | P>  Again  Find  -find  Jump");
			remess = 0;
		}
		chr = movechr(&remess);
		if (break_it) {
			forcetag(TEMPA);
			rewindow(buf_ptr[TEMPA]);
			review(2);
			cursor(1);
			buf_ptr[TEMPA] = buf_ptr[TEMP1];
			block[TEMPA] = block[TEMP1];
			return;
		}
	} while (chr != esc && chr != 'P');
	buf_ptr[TEMP4] = free_start-1; block[TEMP4] = 0;
	reverse = 1;

	if (sorttag(TEMPA,TEMP4) != TEMPA) {
		buf_ptr[TEMP4] = current_char; block[TEMP4] = 0;
		buf_ptr[TEMPA]--;
		reverse = 0;
	}

	filename[0] = 0;
	if (buf_ptr[TEMPA] != current_char) {
		command("Put: to what file?");
		message("");
		get_line(filename, 1);
	}
	if (!break_it && filename[0] != 0 && verify_overwrite(filename)) {
		message("writing to ");
		append(filename);
		append(" ...");
		switch(write_file(filename, 0L, TEMPA, TEMP4)) {
			case -1:
				append(" can't write to file");
				break;
			default:
				append(" completed");
		}
	}

	if (buf_ptr[TEMPA] >= buf_ptr[top_line] && 
			buf_ptr[TEMPA] <= next_line(buf_ptr[scr_rows-1],0)) {
		buf_ptr[TEMP6] = current_char;
		rewindow(buf_ptr[TEMPA]);
		cursor(1);
		scr_mark(*current_char);
		rewindow(buf_ptr[top_line]);
		review(2);
		rewindow(buf_ptr[TEMP6]);
	}
	cursor(1);
	buf_ptr[TEMPA] = buf_ptr[TEMP1];
	block[TEMPA] = block[TEMP1];
}

/*
 Match: if the character under the cursor is a {,},(,),[,], move
	 to the corresponding open/close character.
*/
void Match_cmd(rep) 
	int rep; {
	char matching, matchit, *xptr;
	int direction, countdown;

	if (rep < 0) rep = 1;
	matching = *current_char;
	countdown = 1;
	switch(matching) {
		case '{': 
		matchit = '}';
			direction = 1;
			break;
		case '(':
			matchit = ')';
			direction = 1;
			break;
		case '[':
			matchit = ']';
			direction = 1;
		break;
		
	case '}': 
			matchit = '{';
			direction = 0;
			break;
		case ')': 
			matchit = '(';
			direction = 0;
			break;
		case ']':
			matchit = '[';
			direction = 0;
			break;

		default:
			return;
	}

	buf_ptr[TEMP1] = current_char; block[TEMP1] = 0;

	if (direction) { /* search forward from current_char */
		xptr = current_char+1;
		while(countdown != 0) {
			if (xptr >= buf_end) {
				if (endblocks+srcblocks) {
					if (shuffle(0,1) != -1) xptr = current_char;
				}
				else xptr = buf_end;
				if (xptr >= buf_end) break;
			}
			if (*xptr == matching) {
				countdown++; /* added level of nesting */
				xptr++;
			}
			else if (*xptr == matchit) {
				countdown--;
				if (countdown != 0) xptr++;
			}
			else
				xptr++;
		}
	}
	else { /* search backward */
		xptr = free_start-1;
		while (countdown != 0) {
			if (xptr <= buf_start) {
				if (foreblocks != 0) {
					if (shuffle(1,1) != -1) xptr = free_start-1;
				}
				else xptr = buf_start;
				if (xptr <= buf_start) break;
			}
			if (*xptr == matching) {
				countdown++;
				xptr--;
			}
			else if (*xptr == matchit) {
				countdown--;
				if (countdown != 0) xptr--;
			}
			else
				xptr--;
		}
	}
	if (countdown != 0) {
		message("can't find the matching character");
		forcetag(TEMP1);
		rewindow(buf_ptr[TEMP1]);
	}
	else
		rewindow(xptr);
	review(0);
	cursor(1);
}


char movechr(remess)
	int *remess; {
	char chr;
	int repeat;

	only_movement = 1;			/* disallow Del and Backspace for now */
	chr = toupper(key(1));
	if (break_it) {
		only_movement = 0;
		return 0;
	}
	repeat = 1;
	if (chr == '/') {
		message("/");
		repeat = 32767;
		chr = toupper(key(repeat));
		if (break_it) {
			only_movement = 0;
			return 0;
		}
	}
	if (isdigit(chr)) {
		repeat = 0; message("");
		while (isdigit(chr)) {
			repeat = (repeat * 10) + (chr - '0');
			echo(chr);
			chr = toupper(key(repeat));
			if (break_it) {
				only_movement = 0;
				return 0;
			}
		}
	}
	if (repeat == 0 || (repeat != 1 && chr == bs)) {
		message("");
		only_movement = 0;
		return 0;
	}
	if (chr == 'A') {
		Again_cmd(repeat);
		*remess = 1;
	}
	else if (chr == 'F') {
		FindReplace_cmd(repeat, 1);
		*remess = 1;
	}
	else if (chr == '-') {
		FindReplace_cmd(repeat, 3);
		*remess = 1;
	}
	else if (chr == 'J') {
		JumpTag_cmd(repeat,0);
		*remess = 1;
	}
	only_movement = 0;
	return chr;
}

int blankline(ptr, indent)
	char *ptr; int indent; {
	int thisindent;

	thisindent = 0;
	while (*ptr == ' ' || *ptr == tab) {
		ptr++;
		thisindent++;
	}
	if (*ptr == cr || *ptr == lf || (*ptr & 0xff) == eof) return 1;
	if (thisindent >= indent) return 1;
	return 0;
}

