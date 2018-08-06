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
	extern.h

	(c)Copyright 1989, Michael Ouye, All Rights Reserved

	External declarations.
*/

/* system */
long lseek();
int  creat();
int  write();
int  read();
void close();
int  unlink();
int  open();
int  rename();
int  exec();
void exit();
void _move();
void _setmem();
char *_MEMORY(), *_SHOWSP();
char toupper();
int  isdigit();
void strcpy();
int  strlen();
void strcat();
int  atoi();
int  isspace();
int  isalpha();

/* see.c */
void message();
void command();
char my_ci();
char key();
char *next_line();
void clear_ci();
void append();
void append_num();
void echo();
void next_cmd();
void execute();
int  get_line();
void Again_cmd();
void ltoa();
void remenu(void);

/* more.c */
unsigned goto_line();
int  List_cmd();
int  FindReplace_cmd();
void InsertExchange_cmd();
void Delete_cmd();
void JumpTag_cmd();
void Set_cmd();
void Copy_cmd();
void Get_cmd();
void Put_cmd();
void Match_cmd();
void auto_in();
void LineNumber();
void Buffer_cmd();
void rubout_char();
void kill_char();
void change_window();

/* buffer.c */
void init_buffer();
void start_buffer();
unsigned free_space();
unsigned buf_size();
long file_size();
int  read_file();
int  write_file();
void rewindow();
int  cb_bufin();
int  cb_bufout();
int  sorttag();
int  makespace();
int  switch_buffer();
int  new_buffer();
void clear_buffers();
int  next_buffer(void);
int  buffer_count(void);

/* block.c */
int putblocks();
int getblocks();
int shuffle();
int makeroom();
int forcetag();

#if 0
/* build.c */
int build();
#endif

/* screen.c */
void init_screen();
char *next_line();
void rewrite_line();
int  backup();
int  forup();
int  new_col();
void dump_line();
void out_line();
void out_co();
void rescreen();
void new_screen();
void review();
void cursor();

/* wrap.c */
int findfile();
void Wrap_cmd();

/* pcfast.a  (or screen interface) */
char scr_curs;
int scr_rows, scr_cols;
char scr_attr;
char scr_scrolldown;
char scr_scrollup;
int  scr_open();
void scr_pick();
void scr_clr();
void scr_rowcol();
void scr_set_cursor();
char scr_ci();
void scr_mark();
void scr_unmark();
void scr_clrl();
void scr_setup();
void scr_cls();
void scr_co();
void scr_scdn();
void scr_scup();
void scr_term();
void scr_curs_setup();

/* move.a */
void sl_move();
void ls_move();

/* extmem.c */
int Get_foreblock(char *ptr);
int Put_foreblock(char *ptr);
int Get_endblock(char *ptr);
int Put_endblock(char *ptr);
void extmem_setup(void);
void extmem_spilldrive();

/* dosint.c */
farptr New_block(void);
void Free_block(/*farptr fptr*/);
farptr CopyNear_block(/*char *ptr, farptr fptr*/);
void CopyFar_block( /*farptr fptr, char *ptr, farptr nextptr */);
farptr ExtractLast_block(/*farptr fptr*/);
farptr Next_block(/*farptr fptr*/);
int environment(/*char *search_str, char *buffer, int buf_len */);
void install_break(void);
int First_file(/* char *ptr, char *filename */);
int Next_file(/* char *filename */);
int  DOSDevice();

/* view.c */
void view_init();
void view_select();
void view_refresh();
int  view_next();
int  view_new();
int  view_count();

/* macro.c */
void macro_init(void);
int macro_restore(void);
int macro_save(void);
int macro_get(void);
void macro_next_cmd(void);
void Macro_cmd(void);
void xMacro_cmd(int repeat, char ch);
int is_macro(char ch);
