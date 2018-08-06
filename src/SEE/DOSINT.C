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
	dosint.c

	(c) Copyright 1989, Michael Ouye, All Rights Reserved

	SEE interfaces to the DOS specific routines.  
	
	In the FAR memory alloction routines, the first 4 bytes of every
	block is the next block pointer, 0 at the end of the list.

*/

#include "world.h"
#include "extern.h"

/* returns a farptr to a memory block or 0, size must muliple of 16 */
farptr New_block(void) {
#asm
	mov		bx,2048		; BLOCKSIZE
	shr		bx,1
	shr		bx,1
	shr		bx,1
	shr		bx,1		; divide by 16 for paragraph count
;	make the DOS paragraph allocation call
	mov		ah,48H
	int		21H
	jnc		passed
	xor		ax,ax
passed:
	mov		dx,ax
	xor		ax,ax
#
}

/* free the memory block */
void Free_block(farptr fptr) {
#asm
	mov		es,[bp+6]	; fptr segment
	mov		ah,49H
	int		21H
#
}

/* copy a memory block to the near ptr, returns the next pointer */
farptr CopyNear_block(char *ptr, farptr fptr) {
#asm
	push	ds
	pop		es
	cld
	mov		di,[bp+4]		; ptr
	lds		si,[bp+6]		; fptr
	add		si,4			; sizeof(farptr)
	mov		cx,2044			; BUFSIZE
	rep movsb
	push	es
	pop		ds
	les		si,[bp+6]
	mov		ax,es:[si]
	mov		dx,es:[si+2]
#
}

/* copy near data to a far memory fptr, stores the next block pointer */
void CopyFar_block(farptr fptr, char *ptr, farptr nextptr) {
#asm
	cld
	les		di,[bp+4]		; fptr
	mov		ax,[bp+10]
	stosw
	mov		ax,[bp+12]
	stosw
	mov		si,[bp+8]		; ptr
	mov		cx,2044			;BLOCKDATA
	rep movsb
#
}

/* searches to the end of the block list and removes the last block */
farptr ExtractLast_block(farptr fptr) {
	if (fptr == 0) return 0;
#asm
	les		bx,[bp+4]
	xor		dx,dx
	xor		ax,ax
scan:
	cmp		word es:[bx],0
	jne		next
	cmp		word es:[bx+2],0
	je		found
next:
	mov		ax,bx
	mov		dx,es
	les		bx,es:[bx]
	jmp		scan
found:
	or		ax,ax
	jnz		set_prev
	or		dx,dx
	je		ret_ptr
set_prev:
	push	es
	mov		es,dx
	mov		di,ax
	mov		word es:[bx],0
	mov		word es:[bx+2],0
	pop		es
ret_ptr:
	mov		dx,es
	mov		ax,bx
#
}

/* returns the pointer to the next block */
farptr Next_block(farptr fptr) {
#asm
	les		bx,[bp+4]	; fptr
	mov		ax,es:[bx]
	mov		dx,es:[bx+2]
#
}


/*
	This file contains the routine for searching the environment
	area for a given string.

	Interface:

		environment(search_str, buffer, buf_len)

		where search_str is the actual string being searched for,
			  buffer is the location to store the string if found,
			  buf_len is the maximum number of characters to store
			          into the buffer (includes terminator).

        -1 is returned if the environment variable isn't found,
        otherwise the length of the string copied is returned.
*/

extern unsigned _pcb;		/* c88 variable with the PSP segment */

int environment(search_str, buffer, buf_len)
	char *search_str, *buffer; int buf_len; {

#asm

	mov		ax, _pcb_;				; get the env segment into ES
	mov		es, ax					; 
	mov		ax, es:[2CH]
	mov		es,ax

	mov		bx, [6][bp]				; buffer
	mov		byte[bx],0
	mov		si, 0					; offset into the environment
	jmp		outer_begin

outer_loop:
	inc		si
outer_begin:
	mov		di, [4][bp]				; search_str
	cmp		byte es:[si],0
	jne		cmp_loop
									; not found
	mov		ax,-1
	pop		bp
	ret

cmp_loop:
	mov		al, byte es:[si]		; pick up the search character
	cmp		al,'='
	je		end_str
	cmp		al, byte[di]
	jne		outer_next
	inc		si
	inc		di
	jmp		cmp_loop

end_str:
	cmp		byte[di], 0
	jne		outer_next
									; got it, copy them bytes!
	mov		cx, [8][bp]				; buf_len
	inc		si						; beyond the '='

move_loop:
	mov		al, byte es:[si]
	or		al,al
	jz		now_return
	dec		cx
	jz		now_return
	mov		byte[bx],al
	inc		si
	inc		bx
	jmp		move_loop

now_return:
	mov		byte[bx],0
	mov		ax, [8][bp]
	sub		ax, cx
	pop		bp
	ret

outer_next:
	cmp		byte es:[si],0				; skip to the next entry
	je		outer_loop
	inc		si
	jmp		outer_next
#
}

/* Ctrl-Break handler */
void breaker() {
#asm
	mov		break_it_,1
	pop		bp
	iret
#
	}

/* install the interrupt handler for Ctrl-Break */
void install_break() {
#asm
	push	ds
	mov		ax,cs
	mov		ds,ax
	mov		dx, offset breaker_
	mov		al,23H
	mov		ah,25H
	int		21H
	pop		ds
#
	}

/*	FINDFILE.C	*/
/*
	This file contains the routine to locate a file, utilizing the PATH
	environment variable for the directories to search.

	Interface:

		findfile(filename, target_buf)

		where filename is the name of the file to be searched for,
		      target_buf is the buffer to place the path name if found.

		if the file is found, findfile return 1 and the pathname,
		otherwise it returns 0.

	This program uses the environment routines to access the PATH variable.

	Stack requirements:  ~300 bytes

*/
char _msdos2;

int findfile(filename, target_buf, environ)
	char *filename, *target_buf, *environ; {
	int fid;
	char paths[256], *p_ptr, *t_ptr;

	/* first check in the local directory */
	strcpy(target_buf, filename);
	fid = open(target_buf, 0);
	if (fid >= 0)  {				/* got it */
		close(fid);
		return (1);
	}
	if (_msdos2 == 0) return 0;
	fid = environment(environ, paths, 256);
	p_ptr = paths;

	while (*p_ptr != 0) {
		/* copy the directory name */
		t_ptr = target_buf;
		while (*p_ptr != ';' && *p_ptr != 0) {
			*t_ptr++ = *p_ptr++;
		}
		if (*(t_ptr-1) != '/' && *(t_ptr-1) != '\\') *t_ptr++ = '\\';
		*t_ptr = 0;
		if (*p_ptr) p_ptr++;		/* beyond the ';' */
		strcat(target_buf, filename);
		fid = open(target_buf, 0);
		if (fid >= 0)  {				/* got it */
			close(fid);
			return (1);
		}
	}
	strcpy(target_buf, filename);
	return (0);							/* can't find one */
}

/* directory search routines */
static struct {
	char dta_reserved[21];
	char dta_attribute;
	short int dta_time;
	short int dta_date;
	long int dta_file_size;
	char dta_name[13];
} DTA;
static char search_dir[65];

/* set the Data Transfer Address to the local block */
static void dos_set_DTA() {
#asm
	mov		dx, offset DTA_
	mov		ah, 01AH			; set disk transfer address
	int		21H
#
}

static int dos_find_first(ptr)
	char *ptr;{
#asm
	mov		dx,[bp+4]			; ptr
	mov		cx,	0
	mov		ah, 04EH
	int		21H
	mov		ax,0
	jc		ff_over
	inc		ax
ff_over:
	pop		bp
	ret
#
}

static int dos_find_next() {

#asm
	mov		ah, 04FH			; find next
	int		21H
	mov		ax,0
	jc		fn_over
	inc		ax
fn_over:
	pop		bp
	ret
#
}

int First_file(ptr, filename)
	char *ptr, *filename; {
	char *tar;

	strcpy(search_dir, ptr);
	tar = search_dir + strlen(search_dir);	/* pointer to the end of the string */
	while (tar > search_dir && *tar != '/' && *tar != '\\' && *tar != ':') tar--;
	if (*tar == '/' || *tar == '\\' || *tar == ':')
		tar++;
	*tar = 0;

	dos_set_DTA();
	if (dos_find_first(ptr)) {
		strcpy(filename, search_dir);
		strcat(filename, DTA.dta_name);
		return 1;
	}
	return 0;
}

int Next_file(filename)
	char *filename; {

	if (dos_find_next()) {
		strcpy(filename, search_dir);
		strcat(filename, DTA.dta_name);
		return 1;
	}
	return 0;
}

int DOSDevice(char *filename) {
	/* check for MS-DOS special device names, COM1, COM2, LPT1, LPT2, PRN, CON */
	if (strlen(filename) < 5) {
		if (stricmp(filename, "COM1") == 0) return true;
		if (stricmp(filename, "COM2") == 0) return true;
		if (stricmp(filename, "COM3") == 0) return true;
		if (stricmp(filename, "COM4") == 0) return true;
		if (stricmp(filename, "LPT1") == 0) return true;
		if (stricmp(filename, "LPT2") == 0) return true;
		if (stricmp(filename, "PRN") == 0) return true;
		if (stricmp(filename, "CON") == 0) return true;
	}
	return false;
}
