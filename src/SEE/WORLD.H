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
	world.h

    (c) Copyright 1983-1989 Michael Ouye, All Rights Reserved

	Global constant and data declarations.

*/

#define DEBUG 0

/* useful literals */
#define cr  0x0D
#define lf  0x0A
#define esc 0x1B
#define bs  0x08
#define tab 0x09
#define eof 0xFF
#define eof_mark 0xFE
#define control_C 0x03
#define default_tab	4	/* tab expansion default */

#define REVERSE_ATTR	0x7000

/* control key translations */
#define up_char 			30
#define down_char 			31
#define left_char 			29
#define right_char 			28
#define bol_char 			200
#define eol_char 			201
#define pageup_char 		202
#define pagedown_char 		203
#define bof_char 			204
#define eof_char 			205
#define Ins_char 			206
#define Del_char 			207
#define NextWord_char 		208
#define PrevWord_char 		209
#define sc_down_char 		222
#define sc_up_char   		223

/* function keys */
#define M1 					210
#define M2 					211
#define M3 					212
#define M4 					213
#define M5 					214
#define M6 					215
#define M7 					216
#define M8 					217
#define M9 					218
#define M10 				219
#define M11					220
#define M12					221

/* alt keys */
#define alt_A				174
#define alt_B				175
#define alt_C				176
#define alt_D				177
#define alt_E				178
#define alt_F				179
#define alt_G				180
#define alt_H				181
#define alt_I				182
#define alt_J				183
#define alt_K				184
#define alt_L				185
#define alt_M				186
#define alt_N				187
#define alt_O				188
#define alt_P				189
#define alt_Q				190
#define alt_R				191
#define alt_S				192
#define alt_T				193
#define alt_U				194
#define alt_V				195
#define alt_W				196
#define alt_X				197
#define alt_Y				198
#define alt_Z				199

#define true 				1
#define false 				0

/* editor parameters */
#define max_buffers			17	/* max internal buffers */
#define max_files			16	/* max open files */
#define max_windows 		9	/* max intenal windows */
#define max_views			8	/* max open file windows */

/* total number of tags available */
#define num_tags 			75  /* 60 line max */
/* buf_ptr base for the user tags */
#define tag_base 			(num_tags - 15)

#define JUMPA				(tag_base)
#define JUMPB				(tag_base+1)
#define JUMPC				(tag_base+2)
#define JUMPD				(tag_base+3)
#define JUMPL				(tag_base+4)
#define MARK1				(tag_base+5)
#define MARK2				(tag_base+6)
#define TEMPA				(tag_base+7)
#define TEMPB				(tag_base+8)
#define TEMP1				(tag_base+9)
#define TEMP2				(tag_base+10)
#define TEMP3				(tag_base+11)
#define TEMP4				(tag_base+12)
#define TEMP5				(tag_base+13)
#define TEMP6				(tag_base+14)

#define FILE_VIEW			max_views
#define FILE_BUFFER			max_files

#define BLOCKBUF  			2048
#define BLOCKSIZE 			2044		/* extended memory block size */

/* screen definition */
#define top_line 			2

int first_col;
int start_clip, end_clip;

struct {int row, col;} current;
struct {int row, col;} cmdmess;
char line[80];
int break_it;               /* global break flag */
int cursor_moved;
int reprompt;
int col;					/* actual current column number */

int	use_macro;
int recording_macro;
int loading_macros;
int signon;

/* global mode flag */
char  see_mode;   /* true if in Insert or Exchange mode */

struct {
	int tab_size;		/* tab stop locations */
	int auto_indent;	/* copy previous indent if true */
	int auto_insert;	/* return to insert mode if true */
	int ignore_case;	/* ignore case during find/replace */
	int width;			/* display width */
	int brace_indent;	/* 0,1,2 indentation */
	int word_wrap;		/* true to auto-wrap */
	int spill;			/* spill drive */
	int contZ;			/* non-zero if control-Z to be appended */
	int split_height;	/* split screen window height */
	int tab_expand;		/* non-zero if tabs to be expanded into spaces */
	int cursor_height;	/* cursor height, 0 for no change */
	int screen_attr;	/* initial display attribute */
	int on_exit_save;	/* true to save macros on terminate */
} options; 

char find_replace_target[128];
char replace_source[128];

int more_files;		/* boolean indicating more source in the file */
long int file_index;	/* index into the source file to the last read point */

int cursmode;				/* cursor state */
int active_buffer;			/* current buffer index */
int active_view;			/* current view index */
int only_movement;			/* 0 for normal key interp.  1 for only cursor move keys */
unsigned cb_size;          	/* size of the copy buffer */
char *cb_start;				/* pointer to the copy buffer */
int copyfile;				/* true if copy file is created */
char *buf_top;             	/* absolute top of buffer memory */
char copyfilename[20];
long copy_size;				/* number of bytes in the copy buffer/file */
char copy_buffer[4096];		/* local memory copy buffer */

int error_occurred;			/* error flag */
unsigned error_line;		/* line # */
char error_message[128];	/* error message */
char window_is_full;		/* full window flag */
int  full_id;				/* screen id for the full window */

typedef long farptr;		/* long pointer for far buffers, 32-bit */

/* structure for buffer specific data */
typedef struct {
	char bs_in_use;			/* 1 when in use */
	char *bs_buf_start;		/* beginning of the buffer */
	char *bs_buf_end;		/* end of the buffer */
	char *bs_current_char;	/* current character position */
	char *bs_free_start;	/* first free byte in the gap */
	char *bs_buf_ptr[num_tags]; /* tags into the buffer */
	int bs_block[num_tags];	/* external block number */
	farptr bs_foremem;		/* long pointer to first forward mem block */
	farptr bs_endmem;		/* long pointer to first end mem block */
	long bs_foreoffset;		/* long offset to first forward mem block */
	long bs_endoffset;		/* long offset to first end mem block */
	int bs_foreblocks;		/* number of forward blocks */
	int bs_endblocks;		/* number of end blocks */
	int bs_spillfile;		/* file ID of the spill file, != 0 if used */
	long bs_spilloffset;	/* offset to open space */
	long bs_spillfree;		/* free spill file block list */
	char bs_in_file[65];
	char bs_spillfilename[65];
	int bs_spilladjust;		/* extra bytes in the last spill block */
	int bs_srcblocks;		/* number of blocks left in the source file */
	int bs_srcadjust;		/* minus bytes not used in the last block */
	long bs_srcloc;			/* location within the source file, partial read */
	int bs_tabsize;
	int bs_altered;
} buff_struct;

buff_struct buffers[max_buffers];

buff_struct *cur_buf;

/* buffer pointers */
#define buf_ptr			cur_buf->bs_buf_ptr
#define current_char	cur_buf->bs_current_char
#define buf_start		cur_buf->bs_buf_start
#define buf_end			cur_buf->bs_buf_end
#define free_start		cur_buf->bs_free_start
/* extended memory values */
#define block			cur_buf->bs_block
							/* >0 -> forward extension,
							   <0 -> end extension,
							   =0 -> in memory */
#define foremem			cur_buf->bs_foremem
#define endmem			cur_buf->bs_endmem
#define foreoffset		cur_buf->bs_foreoffset
#define endoffset		cur_buf->bs_endoffset
#define foreblocks		cur_buf->bs_foreblocks
#define endblocks		cur_buf->bs_endblocks
#define spillfile		cur_buf->bs_spillfile
#define spillfilename	cur_buf->bs_spillfilename
#define spilloffset		cur_buf->bs_spilloffset
#define spillfree		cur_buf->bs_spillfree
#define spilladjust		cur_buf->bs_spilladjust
#define srcloc			cur_buf->bs_srcloc		
#define srcblocks		cur_buf->bs_srcblocks
#define srcadjust		cur_buf->bs_srcadjust
#define tabsize			cur_buf->bs_tabsize
#define altered			cur_buf->bs_altered
#define in_file			cur_buf->bs_in_file

#define abs(v)			((v < 0) ? -v:v)


typedef struct {
	int	scr_id;		/* screen window id */
	int buffer_id;	/* buffer being viewed */
} view_struct;

view_struct views[max_windows];

view_struct *cur_view;

