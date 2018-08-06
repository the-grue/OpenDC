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
; void *memicmp(const void *s1, const void *s2, size_t n);

	cseg
	public memicmp_
memicmp_:
	push	bp
	mov	bp,sp

	if	LARGE_CASE

	push	ds
	lds	si,[bp+6]
	les	di,[bp+10]
	mov	cx,[bp+14]

	else

	mov	ax,ds
	mov	es,ax
	mov	si,[bp+4]
	mov	di,[bp+6]
	mov	cx,[bp+8]

	endif

	xor	dx,dx
	jcxz	eq
	cld
	dec	di
c:    	lodsb
	cmp	al, 'a'
	jb	l
	cmp	al, 'z'
	ja	l
	and	al, 0DFH
l:	inc	di
	mov	ah,es:[di]
	cmp	ah, 'a'
	jb	r
	cmp	ah, 'z'
	ja	r
	and	ah, 0DFH
r:	cmp	al,ah
  	ja	gt
	jb	lt
	loop	c

eq:	mov	ax,dx

	if	LARGE_CASE

	pop	ds
	pop	bp
	lret

	else

	pop	bp
	ret

	endif

gt:	inc	dx
	jmp	eq
lt:	dec	dx
	jmp	eq
