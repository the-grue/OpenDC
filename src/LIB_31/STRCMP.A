;
;  Released under the GNU LGPL.  See http://www.gnu.org/licenses/lgpl.txt
;
;  This program is part of the DeSmet C Compiler
;
;  This library is free software; you can redistribute it and/or modify
;  it under the terms of the GNU Lesser General Public License as published
;  by the Free Software Foundatation; either version 2.1 of the License, or
;  any later version.
;
;  This library is distributed in the hope that it will be useful, but
;  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
;  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
;  License for more details.
;
	include	"config.h"

	cseg

;	return = strcmp(first string, second string);
;	return = strncmp(first string,second string,maximum);

;		 return < 0 if f < s, 0 if f == s, >0 if f > s

	public	STRCMP_,STRNCMP_
STRNCMP_:
	push	bp
	mov	bp,sp
	if	model
	push	ds
	les	di,[bp+pbase]
	lds	si,[bp+pbase+4]
	mov	cx,[bp+pbase+8]
	else
	mov	di,[bp+pbase]
	mov	si,[bp+pbase+2]
	mov	cx,[bp+pbase+4]
	endif
	jcxz	sp_quit
	jmp	sp_loop


STRCMP_:
	push	bp
	mov	bp,sp
	if	model
	push	ds
	les	di,[bp+pbase]
	lds	si,[bp+pbase+4]
	else
	mov	di,[bp+pbase]
	mov	si,[bp+pbase+2]
	endif
	mov	cx,65535
sp_loop:mov	al,[si]		;from second
	if	model
	cmp	es:[di],al		;to first
	else
	cmp	[di],al		;to first
	endif
	jb	flow
	ja	fhigh
	inc	si		;increment indexes
	inc	di
	or	al,al
	jz	sp_quit
	loop	sp_loop
sp_quit:mov	ax,0		;return a zero as both the same
	if	model
	pop	ds
	endif
	pop	bp
	_ret
flow:	mov	ax,-1		;return -1 as first is low
	if	model
	pop	ds
	endif
	pop	bp
	_ret
fhigh:	mov	ax,1		;return 1 as first is high
	if	model
	pop	ds
	endif
	pop	bp
	_ret
