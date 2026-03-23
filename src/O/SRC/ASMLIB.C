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
			/*  Start of file ASMLIB.C  */

/*
		 O88 Optimizer for DeSmet (C-Ware) C-Compiler
		 --------------------------------------------

			      Copyright (c) 1986

				      by

			    Key Software Products
		    440 Ninth Avenue, Menlo Park, CA 94025
				(415) 364-9847

			    (All rights reserved)


*/


#include "inc\o88.h"

/*lint	-e528	function not referenced		*/
/*lint	-e533	return mode not consistent	*/
/*lint	-e715	variable not referenced		*/

#ifdef	_lint
VOID putchar(CHAR) ;
VOID puts(CHAR *) ;
#endif

BOOLEAN No_Input_File()
	{
#ifndef	_lint
#asm
	mov	ax,4400h
	mov	bx,0		; stdin
	int	21h
	mov	ax,1
	and	dx,8080h	; ISDEV | CHARDEV
	cmp	dx,8080h
	je	Istty
	mov	ax,0
Istty:
#end
#endif
	}


PRIVATE VOID Stderr(str, len)
CHAR *str ;
unsigned len ;
	{
#ifndef	_lint
#asm
	mov	bx,2		; stderr
	mov	dx,[bp+4]	; str
	mov	cx,[bp+6]	; len
	mov	ax,4000h
	int	21h
#end
#endif
	}

VOID Errc(ch)
CHAR ch ;
	{
	if (Equal(dst_file, "STDOUT")) Stderr((CHAR *) &ch, 1) ;
	else putchar(ch) ;
	}

VOID Errs(str)
CHAR *str ;
	{
	if (Equal(dst_file, "STDOUT")) Stderr(str, strlen(str)) ;
	else puts(str) ;
	}

unsigned Read_File(handle, seg, bytes)
HANDLE handle ;
unsigned seg ;
unsigned bytes ;
	{
#ifndef	_lint
#asm
	push	ds
	mov	bx,[bp+4]	; handle
	mov	ds,[bp+6]	; seg
	xor	dx,dx		; off
	mov	cx,[bp+8]	; bytes
	mov	ah,3Fh
	int	21h
	jnc	Read_Input1
	mov	err_code_,ax
	xor	ax,ax
Read_Input1:
	pop	ds
#end
#endif
	}

unsigned Write_File(handle, seg, bytes)
HANDLE handle ;
unsigned seg ;
unsigned bytes ;
	{
#ifndef	_lint
#asm
	push	ds
	mov	bx,[bp+4]	; handle
	mov	ds,[bp+6]	; seg
	xor	dx,dx		; off
	mov	cx,[bp+8]	; bytes
	mov	ax,4000h
	int	21h
	pop	ds
	mov	err_code_,ax
#end
#endif
	}


int DOS_Version()
	{
#ifndef	_lint
#asm
	mov	ah,30h
	int	21h
	xor	ah,ah
#end
#endif
	}


BOOLEAN Found(file)
CHAR *file ;
	{
#ifndef	_lint
#asm
	mov	dx,[bp+4]	; file
	mov	ah,4Eh
	xor	cx,cx
	int	21h
	mov	err_code_,ax
	mov	ax,0
	jc	Found1
	inc	ax
Found1:
#end
#endif
	}


VOID Rename_File(old, new)
CHAR *old, *new ;
	{
#ifndef	_lint
#asm
	mov	dx,[bp+4]	; old
	mov	di,[bp+6]	; new
	mov	ax,ds
	mov	es,ax
	mov	ah,56h
	int	21h
	mov	err_code_,ax
	jnc	Rename_File_Out_
#end
	Error_Msg("Error renaming file: ", old) ;
Out:	;
#endif
	}


VOID Delete_File(filename)
CHAR *filename ;
	{
#ifndef	_lint
#asm
	mov	dx,[bp+4]	; filename
	mov	ah,41h
	int	21h
	mov	err_code_,ax
#end
#endif
	}



/*
	---------------------------------------------------
	The following routines are here for absolute speed!
	---------------------------------------------------
*/

CHAR *Find(key)
CHAR *key ;
	{	 /* Hash table search (See Make_Entry() in O88.C) */
#ifndef	_lint
#asm

	; set-up for compare

	cld
	mov	ax,ds
	mov	es,ax

	; compute initial probe index

	mov	si,word [bp+4]		; pointer to key
	mov	cx,si			; keep a copy nearby
	lodsw				; 1st & 2nd chars of key
	shl	al,1
	add	al,ah			; 2nd char
	shl	al,1
	add	al,byte [si]		; 3rd char
	sub	al,224			; offset: 1st=2nd=3rd=space

	; save to test for end of search

	mov	dh,al			; remember initial (first) index
	mov	dl,al			; initialize current index

	; begin the search loop

Search_Loop:
	mov	bl,dl			; convert current index into
	xor	bh,bh
	shl	bx,1			; word ptr
	mov	di,word table_[bx]	; get entry ptr in table entry
	or	di,di			; empty ?
	jz	Not_Found		; if empty, return not found
 
	; Load string pointers

	mov	si,cx			; reload string argument ptr
	mov	di,[di]			; get pointer to entry's key
Compare:
	lodsb
	scasb
	jne	Not_Equal
	or	al,al
	jnz	Compare
	mov	ax,word table_[bx]	; strings are equal!
	jmp	Located

Not_Equal:
	mov	al,dh			; update current index (DH)
	shl	al,1
	inc	al
	add	dl,al
	cmp	dl,dh			; back at start ?
	jne	Search_Loop		; if not, keep on going

Not_Found:
	xor	ax,ax
Located:
#end
#endif
	}


PRIVATE	VOID Asm_Lib()
	{
#ifndef	_lint
#asm
	;	BOOLEAN EqualN(str1, str2, n)
	;	CHAR *str1 ;
	;	CHAR *str2 ;
	;	unsigned n ;

	PUBLIC	EqualN_
EqualN_:
	pop	bx			; return address
	pop	si			; str1 ptr
	pop	di			; str2 ptr
	pop	cx			; n
	sub	sp,6
	jmp	Str_Cmp


	;	BOOLEAN Equal(str1, str2)
	;	CHAR *str1 ;
	;	CHAR *str2 ;

	PUBLIC	Equal_
Equal_:
	pop	bx			; return address
	pop	si			; str1 ptr
	pop	di			; str2 ptr
	mov	cx,1000			; an upper limit!
	sub	sp,4

Str_Cmp:
	cld
	mov	ax,ds
	mov	es,ax
Check:
	lodsb
	scasb
	jne	Not_Same
	or	al,al
	loopnz	Check
	mov	ax,1
	jmp	bx
Not_Same:
	xor	ax,ax
	jmp	bx

	;	int Value(str, len)
	;	CHAR *str ;
	;	unsigned len ;

	PUBLIC Value_
Value_:
	pop	di		; get return adrs
	pop	si		; get str
	pop	bx		; get len

	xor	ch,ch		; ch==0 => positive
	cmp	byte [si],'-'	; negative ?
	jne	Value1
	inc	ch		; ch!=0 => negative
	inc	si		; str++
	dec	bx		; len--
Value1:
	xor	ax,ax		; initialize value = 0
	cmp	byte [bx+si-1],'H'	; hex ?
	je	Value_Hex

	mov	cl,2
Value_Dec1:
	dec	bx		; decrement char count
	js	Value_Done
	mov	dx,ax		; dx = 1*n
	shl	ax,cl		; ax = 4*n
	add	ax,dx		; ax = 5*n
	shl	ax,1		; ax = 10*n
	add	al,byte [si]	; add the next character
	adc	ah,0
	sub	ax,0030h	; ascii '0'
	inc	si		; bump char ptr
	jmp	Value_Dec1

Value_Hex:
	mov	cl,4
	dec	bx		; len--
Value_Hex1:
	js	Value_Done
	shl	ax,cl		; multiply by 16
	add	al,byte [si]	; add the next character
	adc	ah,0
	sub	ax,0030h	; ascii '0'
	inc	si		; bump char ptr
	dec	bx		; decrement char count
	jmp	Value_Hex1

Value_Done:
	or	ch,ch		; negative ?
	jz	Value_Out
	neg	ax		; complement
Value_Out:
	sub	sp,4
	jmp	di


	;	CHAR *Convert(n, ptr)	/* Converts n into text		*/
	;	unsigned n ;		/* number to be converted	*/
	;	CHAR *ptr ;		/* points to rightmost bfr pos	*/
	;				/* returns ptr to last pos used	*/

	PUBLIC	Convert_
Convert_:
	pop	bx		; return address
	pop	ax		; n
	pop	di		; ptr
	mov	cx,10
Convert1:
	xor	dx,dx
	div	cx		; ax = n/10, dx = n%10
	add	dl,030h
	mov	byte [di],dl
	dec	di
	or	ax,ax
	jnz	Convert1
	inc	di
	mov	ax,di
	sub	sp,4
	jmp	bx


	;	BOOLEAN True()

	PUBLIC	True_
True_:
	mov	ax,1
	ret


	;	BOOLEAN False()

	PUBLIC	False_
False_:
	xor	ax,ax
	ret


	;	CHAR *Append(at, str)
	;	CHAR *at, *str ;

	PUBLIC	Append_
Append_:
	pop	bx	; rtn adrs
	pop	di	; at
	pop	si	; str
	cld
	mov	ax,ds
	mov	es,ax
	lea	cx,Append1
Append1:
	lodsb
	or	al,al
	jz	Append2
	stosb
	jmp	cx
Append2:
	mov	ax,di
	sub	sp,4
	jmp	bx


	;	CHAR *Leading(line)
	;	CHAR *line ;

	PUBLIC	Leading_
Leading_:
	pop	bx
	pop	si
	cld
Leading1:
	lodsb
	or	al,al
	jz	Leading2
	cmp	al,' '
	je	Leading1
Leading2:
	dec	si
	mov	ax,si
	sub	sp,2
	jmp	bx


	;	int Trailing(str, length)
	;	CHAR *str ;
	;	unsigned length ;

	PUBLIC	Trailing_
Trailing_:
	pop	bx
	pop	di		; str
	pop	cx		; length 
	or	cx,cx
	jz	Trailing1
	add	di,cx		; str += length
	dec	di		; di -> last character
	std			; search backwards
	mov	ax,ds
	mov	es,ax
	mov	al,' '		; skip over blanks
	repz	scasb
	add	di,2		; skipped one beyond last non-blank
	xor	al,al		; get a NULL
	stosb			; di -> last blank
	inc	cx		; correct count for last non-blank
Trailing1:
	mov	ax,cx		; return new length
	sub	sp,4
	jmp	bx
#end
#endif
	}

HANDLE Open_File(filename, function)
CHAR *filename ;
unsigned function ;
	{
#ifndef	_lint
#asm
	mov	dx,[bp+4]	; filename
	mov	ax,[bp+6]	; mode (DOS function)
	xor	cx,cx		; attribute (for creat)
	int	21h
	mov	err_code_,ax
	jnc	File_Open
	xor	ax,ax
File_Open:
#end
#endif
	}

BOOLEAN Close_File(handle)
HANDLE handle ;
	{
#ifndef	_lint
#asm
	mov	bx,[bp+4]
	mov	ah,3Eh
	int	21h
	mov	err_code_,ax
	mov	ax,1	; success
	jnc	File_Closed
	xor	ax,ax
File_Closed:
#end
#endif
	}

VOID DOS_Memory()
	{
#ifndef	_lint
#asm
	dseg
	public	_pcb_
	cseg

	mov	WORD dos_size_,0

	mov	bx,_pcb_+2	; original SP
	mov	cl,4
	shr	bx,cl		;BX is stack size in paragraphs
	mov	ax,ds
	add	bx,ax		;BX is max paragraph used
	inc	bx
	sub	bx,_pcb_	;paras need to keep
	mov	es,_pcb_

	mov	ah,4Ah		; Reduce program's memory allocation
	int	21h
	jc	Out

	mov	bx,0FFFFh	; now check what's available
	mov	ah,048h
	int	21h
	jnc	Out		; (had better fail!)
	mov	WORD dos_size_,bx

	mov	ah,048h		; and allocate it
	int	21h
	mov	WORD dos_base_,ax
	jnc	Out
	mov	WORD dos_size_,0; mark the error
Out:
#end
#endif
	}

			 /*  End of file ASMLIB.C  */
