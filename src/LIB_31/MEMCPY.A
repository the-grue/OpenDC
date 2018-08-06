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
; void *memcpy(void *s1, const void *s2, size_t n);

	cseg
	public memcpy_
memcpy_:push	bp
	mov	bp,sp

	if	LARGE_CASE

	push	ds
	les	di,[bp+6]
	lds	si,[bp+10]
	mov	cx,[bp+14]

	else

	mov	ax,ds
	mov	es,ax
	mov	di,[bp+4]
	mov	si,[bp+6]
	mov	cx,[bp+8]

	endif

	jcxz	done
	cld
   rep	movsb

	if	LARGE_CASE

done:	pop	ds
	les	si,[bp+6]
	pop	bp
	lret

	else

done:	mov	ax,[bp+4]
	pop	bp
	ret

	endif
