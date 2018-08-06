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
	macro.c 
	
	(c) Copyright 1989 Michael Ouye, All Rights Reserved

	This file contains the routines for processing macros.

	The macro buffers is split into two zones, the storage area and
	the macro stack.   When a macro is executed, the old macro pointers
	and the macro itself are copied onto the macro stack.  When
	execution is completed the values are popped off the stack. This
	arrangement is used to allow for nested macro execution.


0	+------------------------+ macro_buffer
	|     macro              |
	+------------------------+
	|     macro              |
	+------------------------+
	|     macro              |
	+------------------------+ mac_free
	|                        |   a_entry
	|     free space         |
	|                        |
	|                        |
	+------------------------+ mac_end
	|     executing macro    |   a_read
	+------------------------+
	|     executing macro    |
	+------------------------+
*/

#include "world.h"
#include "extern.h"

#define MAC_SIZE	3000					/* macro buffer size */
#define MAX_MACROS	32						/* max macro definitions */

typedef struct {
	char ch;
	char *str; 
} macro_table;

typedef struct {
	int  repeat;		/* # times to execute */
	int len;			/* length of the macro */
	char *old_read;		/* previous read pointer */
	char mac_text[1];	/* macro text */
} macro_stack;

char mac_file[] = {"see.mc4"};
char mac_filename[65] ={0};

/* static */ char macro_buffer[MAC_SIZE+10];
/* static */ macro_table macros[MAX_MACROS] = {0};/* pointers to the macros */

/* static */ char *a_entry;
/* static */ char *a_read;
/* static */ char *a_end;
/* static */ char *mac_end;
/* static */ char *mac_free;
/* static */ macro_table *cur_macro = 0;
/* static */ int savemode;
/* static */ int macro_ahead;

/* mapping of byte character codes to macro character names */
macro_table macro_names[] = {
	cr,				"{CR}",
	lf,				"{LF}",
	esc,			"{ESC}",
	bs,				"{BS}",
	tab,			"{TAB}",
	up_char, 		"{UP}",
	down_char,		"{DOWN}",
	left_char,		"{LEFT}",
	right_char,		"{RIGHT}",
	bol_char,		"{HOME}",
	eol_char,		"{END}",
	pageup_char,	"{PGUP}",
	pagedown_char,	"{PGDN}",
	bof_char,		"{CTRL-HOME}",
	eof_char,		"{CTRL-END}",
	Ins_char,		"{INS}",
	Del_char,		"{DEL}",
	NextWord_char,	"{CTRL-RIGHT}",
	PrevWord_char,	"{CTRL-LEFT}",
	sc_down_char,	"{CTRL-PGDN}",
	sc_up_char,		"{CTRL-PGUP}",
	'{',			"{{",
	M1,				"{F1}",				/* 22, MACRO_START */
	M2,				"{F2}",
	M3,				"{F3}",
	M4,				"{F4}",
	M5,				"{F5}",
	M6,				"{F6}",
	M7,				"{F7}",
	M8,				"{F8}",
	M9,				"{F9}",
	M10,			"{F10}",
	M11,			"{F11}",
	M12,			"{F12}",
	1,				"{CTRL-A}",
	2,				"{CTRL-B}",
	4,				"{CTRL-D}",
	5,				"{CTRL-E}",
	6,				"{CTRL-F}",
	7,				"{CTRL-G}",
	11,				"{CTRL-K}",
	12,				"{CTRL-L}",
	14,				"{CTRL-N}",
	15,				"{CTRL-O}",
	16,				"{CTRL-P}",
	17,				"{CTRL-Q}",
	18,				"{CTRL-R}",
	19,				"{CTRL-S}",
	20,				"{CTRL-T}",
	21,				"{CTRL-U}",
	22,				"{CTRL-V}",
	23,				"{CTRL-W}",
	24,				"{CTRL-X}",
	25,				"{CTRL-Y}",
	26,				"{CTRL-Z}",
	alt_A,			"{ALT-A}",
	alt_B,			"{ALT-B}",
	alt_C,			"{ALT-C}",
	alt_D,			"{ALT-D}",
	alt_E,			"{ALT-E}",
	alt_F,			"{ALT-F}",
	alt_G,			"{ALT-G}",
	alt_H,			"{ALT-H}",
	alt_I,			"{ALT-I}",
	alt_J,			"{ALT-J}",
	alt_K,			"{ALT-K}",
	alt_L,			"{ALT-L}",
	alt_M,			"{ALT-M}",
	alt_N,			"{ALT-N}",
	alt_O,			"{ALT-O}",
	alt_P,			"{ALT-P}",
	alt_Q,			"{ALT-Q}",
	alt_R,			"{ALT-R}",
	alt_S,			"{ALT-S}",
	alt_T,			"{ALT-T}",
	alt_U,			"{ALT-U}",
	alt_V,			"{ALT-V}",
	alt_W,			"{ALT-W}",
	alt_X,			"{ALT-X}",
	alt_Y,			"{ALT-Y}",
	alt_Z,			"{ALT-Z}",
	0,				""
};

#define MACRO_START		22

/* scan a macro pair table for the char or the string, return a pointer */
/*		if both ch && str == 0, return the last slot or open (0xFF) slot */
macro_table *macro_scan(mn_ptr, ch, str)
	macro_table *mn_ptr; char ch; char *str; {

	while (mn_ptr->ch != 0) {
		if (ch != 0) {
			/* compare for character match */
			if (mn_ptr->ch == ch)
				return (mn_ptr);
		}
		else if (str != 0) {
			/* compare for string match */
			if (strcmp(mn_ptr->str, str) == 0) 
				return (mn_ptr);
		}
		else if (ch == 0 && str == 0 && mn_ptr->ch == 0xFF) {
			return (mn_ptr);
		}
		mn_ptr++;
	}
	if (ch == 0 && str == 0)
		return mn_ptr;
	return (0);
}

void macro_init(void) {
	a_entry = macro_buffer;
	a_read = macro_buffer;
	a_end = macro_buffer;
	mac_free = macro_buffer;
	mac_end = macro_buffer + MAC_SIZE;
	macros[0].ch = 0;
	macro_ahead = 0;
}

/* strcat_num: convert and concatenate the value to the string */
void strcat_num(str, val)	
	char *str; int val; {

	ltoa(str + strlen(str), (long)val);
}

int macro_read(file_str)
	char *file_str; { 		/* read in the macro file */
	int file_id;
	macro_stack *sptr;
	int len;

	file_id = open(file_str, 0);
	if (file_id < 0) {
		message("cannot open macro file: ");
		append(file_str);
		return (0);
	}

	len = (int)lseek(file_id, 0L, 2);	/* full length of the file */
	lseek(file_id, 0L, 0);				/* back to the start */

	sptr = mac_end - (sizeof(macro_stack)+len);
	if (sptr > mac_free) {
		len = read(file_id, sptr->mac_text, len);
		sptr->old_read = a_read;
		sptr->len = len;
		sptr->repeat = 1;
		a_read = sptr->mac_text;
		a_end = a_read + sptr->len;
		mac_end = sptr;
		use_macro = true;
		message("");
	}
	else {
		message("File too large for macro execution");
	}
	close(file_id);
	return (1);
}

int macro_write(file_str)
	char *file_str; {
	int file_id, i;
	char buffer[256], *ptr, cbuf[2];
	static char crlf[] = {"\r\n"};
	macro_table *mptr;

	file_id = creat(file_str);
	if (file_id < 0) {
		message("cannot create file: ");
		append(file_str);
		return 0;
	}
	*buffer = 0;

	/* signon disable */
	if (!signon) {
		strcat(buffer,"S.\r\n");
	}

	/* screen size */
	strcat(buffer, "SPS");
	strcat_num(buffer, scr_cols);
	strcat(buffer, "{CR}");
	strcat_num(buffer, scr_rows);
	strcat(buffer, "{CR}\r\n");

	/* ignore case */
	if (options.ignore_case == false) {
		strcat(buffer, "SCY\r\n");
	}

	/* auto indent */
	if (options.auto_indent == false) {
		strcat(buffer, "SIY\r\n");
	}

	/* control Z */
	if (options.contZ != false) {
		strcat(buffer, "SPAY\r\n");
	}

	/* cursor height */
	if (options.cursor_height != 0) {
		strcat(buffer, "SPC");
		strcat_num(buffer, options.cursor_height);
		strcat(buffer, "{CR}");
		strcat(buffer, crlf);
	}

	/* screen attribute */
	strcat(buffer, "SPB");
	strcat_num(buffer, (options.screen_attr & 0xF0) >> 4);
	strcat(buffer, "{CR}");
	strcat(buffer, crlf);
	strcat(buffer, "SPF");
	strcat_num(buffer, (options.screen_attr & 0x0F));
	strcat(buffer, "{CR}");
	strcat(buffer, crlf);

	/* wrap width */
	if (options.width != 72) {
		strcat(buffer, "SR");
		strcat_num(buffer, options.width);
		strcat(buffer, "{CR}");
		strcat(buffer, crlf);
	}

	/* spill drive */
	if (options.spill != 0) {
		strcat(buffer, "SS");
		cbuf[0] = options.spill;
		cbuf[1] = 0;
		strcat(buffer, cbuf);
		strcat(buffer, crlf);
	}

	if (options.tab_size < 0) options.tab_size = 0;
	/* tab size */
	if (options.tab_size != 0) {
		strcat(buffer, "ST");
		strcat_num(buffer, options.tab_size);
		strcat(buffer, crlf);
	}

	/* tab expand */
	if (options.tab_expand != 0) {
		strcat(buffer, "ST*\r\n");
	}

	/* word wrap */
	if (options.word_wrap != false) {
		strcat(buffer, "SWY\r\n");
	}

	/* brace indentation */
	if (options.brace_indent != 0) {
		strcat(buffer, "S{{");
		strcat_num(buffer, options.brace_indent);
		strcat(buffer, crlf);
	}

	write(file_id, buffer, strlen(buffer));

	if (*find_replace_target != 0) {
		if (*replace_source != 0) {
			strcpy(buffer, "0R");
			strcat(buffer, find_replace_target);
			strcat(buffer, "{CR}");
			strcat(buffer, replace_source);
			strcat(buffer, "{CR}");
		}
		else {
			strcpy(buffer, "0F");
			strcat(buffer, find_replace_target);
			strcat(buffer, "{CR}");
		}
		strcat(buffer, crlf);
		write(file_id, buffer, strlen(buffer));
	}


	for (i = 0; i < MAX_MACROS; i++) {
		if (macros[i].ch != 0  && macros[i].ch != 0xFF) {
			strcpy(buffer, "M=");
			mptr = macro_scan(macro_names, macros[i].ch, 0);
			if (mptr != 0) {
				strcat(buffer, mptr->str);
			}
			else {
				*(buffer+2) = macros[i].ch;
				*(buffer+3) = 0;
			}
			write(file_id, buffer, strlen(buffer));
			write(file_id, macros[i].str, strlen(macros[i].str));
			*buffer = 0;
			if (mptr != 0) {
				strcat(buffer, mptr->str);
			}
			else {
				*(buffer) = macros[i].ch;
				*(buffer+1) = 0;
			}
			strcat(buffer, crlf);
			write(file_id, buffer, strlen(buffer));
		}
	}

	buffer[0]=0;
	/* auto insert */
	if (options.auto_insert == true) {
		strcat(buffer, "SAY\r\n");
	}
	write(file_id, buffer, strlen(buffer));

	close(file_id);
	message("");
	return (1);
}

int macro_restore(void) {
	int status;
	int chr;

	status = environment("SEEMACRO", mac_filename, sizeof(mac_filename));
	if (status < 0) {
		if (!findfile(mac_file, mac_filename, "PATH")) {
			strcpy(mac_filename, mac_file);
			return 0;
		}
	}

	if (!macro_read(mac_filename)) return 0;

	loading_macros = true;
	while (1) {
		chr = macro_get();
		if (chr == -1) break;
		macro_ahead = chr;
		execute(1);
	}
	loading_macros = false;
	return 1;
}

int macro_save(void) {
	return macro_write(mac_filename);
}

/* get the next macro character */
int macro_get(void) {
	macro_table *ptr;
	char buf[50], *cptr;

	if (macro_ahead) {
		int chr;

		chr = macro_ahead;
		macro_ahead = 0;
		return (chr);
	}

	while (1) {
		if (a_read >= a_end)
			if (!macro_pop()) {
				message("");
				return (-1);
			}

		if (*a_read == cr || *a_read == lf) {
			a_read++;
			continue;
		}

		if (*a_read != '{')
			return (*a_read++);

		if (*(a_read+1) == '{') {
			a_read++;
			return (*a_read++);
		}

		cptr = buf;
		while (a_read < a_end && *a_read != '}') *cptr++ = toupper(*a_read++);
		*cptr++ = '}';
		*cptr++ = 0;

		if (a_read < a_end) a_read++;
		ptr = macro_scan(macro_names, 0, buf);
		if (ptr != 0)
			return (ptr->ch);
		/* error, bad macro name */
		return (0);
	}
}

/* put the macro character */
void xmacro_put(ch, force)
	char ch; int force; {
	macro_table *ptr;
	int len;

	if (!recording_macro && !force) return;
	ptr = macro_scan(macro_names, ch, 0);
	if (ptr != 0) {
		len = strlen(ptr->str);
		if ((a_entry + len) < mac_end) {
			memcpy(a_entry, ptr->str, len);
			a_entry += len;
		}
		/* else buffer full! */
	}
	else {
		if ((a_entry+1) < mac_end)
			*a_entry++ = ch;
	}
}

void macro_put(ch)
	char ch; {

	xmacro_put(ch, false);
}

void macro_next_cmd(void) {
	macro_table *ptr;
	char cbuf[2];

	if (recording_macro) {
		message("recording Macro ");
		ptr = macro_scan(macro_names, cur_macro->ch, 0);
		if (ptr != 0)
			append(ptr->str);
		else {
			cbuf[0] = cur_macro->ch;
			cbuf[1] = 0;
			append(cbuf);
		}
		append(", use Macro key to finish recording");
	}
}

void macro_break(void) {
	use_macro = false;
	recording_macro = false;
	mac_end = macro_buffer + MAC_SIZE;
}

/* 
	macro_push: pushes the macro data onto the macro stack
*/
void macro_push(int repeat, char *start, char *end) {
	int len;
	macro_stack *sptr;

	len = (int)(end-start);
	sptr = mac_end - (sizeof(macro_stack)+len);
	if (sptr > mac_free) {
		_move(len, start, sptr->mac_text);
		sptr->old_read = a_read;
		sptr->len = len;
		sptr->repeat = repeat;
		a_read = sptr->mac_text;
		a_end = a_read + sptr->len;
		mac_end = sptr;
		use_macro = true; 
	}
	/* else buffer full */
}

/* 
	macro_pop: pops the macro data from the macro stack, returns 0
		if the macro stack was emptied.
*/
int macro_pop() {
	macro_stack *sptr;

	sptr = mac_end;
	if (--(sptr->repeat) > 0) {
		a_read = sptr->mac_text;
		return (1);
	}
	mac_end = (char *)sptr + sptr->len + sizeof(macro_stack);
	sptr = mac_end;
	if (mac_end > (macro_buffer + MAC_SIZE)) {
		use_macro = true;
		a_read = sptr->old_read;
		a_end = sptr->mac_text + sptr->len;
		return (1);
	}
	else {
		use_macro = false;
		scr_set_cursor(cursmode = savemode);
		command(0);
		return (0);
	}
}

/*
	to delete a macro, simply move everthing "above" the macro into the
	new space, adjust all of the pointers which pointed higher
	than the deletion endpoint and zero out the macro pointers
*/
void delete_macro(ptr)
	macro_table *ptr; {
	int len, i, rest;
	char *old_end;

	len = strlen(ptr->str) + 1;	/* size of the macro */
	/* first, move everything beyond the deleted macro into the space */

	rest = mac_free - ptr->str;
	old_end = ptr->str+len;
	_move(rest, old_end, ptr->str);

	/* now adjust any of the macro pointers */
	ptr->str = 0;
	ptr->ch = 0xFF;

	for (i = 0; i < MAX_MACROS; i++) {
		if (macros[i].str >= old_end)
			macros[i].str -= len;
	}
	mac_free -= len;	 /* set new free space pointer */
}


void xMacro_cmd(repeat, ch)
	unsigned repeat; char ch; {
	macro_table *ptr;

	if (repeat < 0) repeat = 1;
	savemode = cursmode;

	ptr = macro_scan(macros, ch, 0);
	if (ptr != 0) {
		macro_push(repeat, ptr->str, ptr->str + strlen(ptr->str));
	}
	while (use_macro)
		execute(1);
}

void copy_macro(ch)
	char ch; {
	macro_table *ptr, *nptr;

	ptr = macro_scan(macros, ch, 0);
	if (ptr != 0) {
		buf_ptr[0] = free_start;
		block[0] = 0;
		if (makespace((int)strlen(ptr->str)+16) == -1) return -1;
		strcpy(free_start, "M=");
		free_start += 2;
		nptr = macro_scan(macro_names, ch, 0);
		if (nptr != 0) {
			strcpy(free_start, nptr->str);
			free_start += strlen(nptr->str);
		}
		else
			*free_start++ = ch;

		strcpy(free_start, ptr->str);
		free_start += strlen(ptr->str);

		if (nptr != 0) {
			strcpy(free_start, nptr->str);
			free_start += strlen(nptr->str);
		}
		else
			*free_start++ = ch;
		*free_start++ = cr;
		*free_start++ = lf;

		forcetag(0);
		rewindow(buf_ptr[0]); /* the start of the copied area */
		altered = true;
		review(0);
		rewrite_line(true);
		cursor(1);
	}
}

static char fprompt[] = {"Enter macro filename: "};

void Macro_cmd(void) {
	char ch, cmd;
	char line[128];
	int i;

	if (recording_macro) {
		/* macro complete, stop recording */
		recording_macro = false;
		cur_macro.str = mac_free;
		*--a_entry = 0;
		mac_free = a_entry+1;
		message("Macro recording complete");
		return;
	}

	command("Macro:  Copy  Delete  Execute  Load  Record  Save");
	message("");
	cmd = toupper(key(1));
	if (cmd == '=' || cmd == 'R' || cmd == 'D' || cmd == 'C') {
		/* record a macro */
		ch = 0;
		while (ch == 0) {
			command("select macro key:");
			ch = my_ci();
			if (break_it || ch == esc || ch == control_C)
				return;
			if (ch == '*' && (cmd == 'D' || cmd == 'C'))
				break;

			if (macro_scan(&macro_names[MACRO_START], ch, 0) == 0) {
				ch = 0;
				message("illegal macro character");
				continue;
			}
			cur_macro = macro_scan(macros, ch, 0);
		}
	
		if (cur_macro != 0 && cmd != 'C')
			delete_macro(cur_macro);

		if (cmd == 'D') {
			if (ch == '*') {
				for (i = 0; i < MAX_MACROS; i++) {
					if (macros[i].ch != 0 && macros[i].ch != 0xFF)
						delete_macro(&macros[i]);
				}
				message("All macros deleted");
			}
			else 
				message("Macro deleted");
			return;
		}
		else if (cmd == 'C') {
			/* insert the macro string */
			if (ch == '*') {
				for (i = 0; i < MAX_MACROS; i++) {
					if (macros[i].ch != 0 && macros[i].ch != 0xFF)
						copy_macro(macros[i].ch);
				}
			}
			else
				copy_macro(ch);
			return;
		}

		/* setup for macro recording */
		cur_macro = macro_scan(macros, 0, 0);
		cur_macro.ch = ch;
		cur_macro.str = 0;
		a_entry = mac_free;

		if (cmd == '=') {
			/* load a pre-defined macro */
			while (1) {
				ch = my_ci();
				if (ch == cur_macro.ch) break;
				xmacro_put(ch, true);
			}
			*a_entry++ = 0;
			cur_macro.str = mac_free;
			mac_free = a_entry;
			use_macro = false;
			remenu();
			use_macro = true;
			return;
		}

		recording_macro = true;
		next_cmd(true);
		return;
	}
	else if (cmd == 'S') { /* save the macros and settings */
		message(fprompt);
		strcpy(line, mac_filename);
		if (get_line(line, false)) {
			macro_write(line);
			strcpy(mac_filename, line);
		}
		return;
	}
	else if (cmd == 'L') { /* load the macros and settings */
		message(fprompt);
		strcpy(line, mac_filename);
		if (get_line(line, false)) {
			if (!macro_read(line)) return;
			strcpy(mac_filename, line);
		}
		return;
	}
	else if (cmd == 'E') { /* execute a macro from the buffer */
		macro_push(1, current_char, buf_end);
	}
}

/* is_macro: returns 1 if the character is defined as a macro,
	-1 if illegal as a macro, and 0 if not defined
*/
int is_macro(ch)
	char ch; {

	if (ch >= 0x20 && ch <= 0x7F) return -1;
	if (macro_scan(macros, ch, 0) != 0) return 1;
	return 0;
}
