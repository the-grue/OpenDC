;
;  Released under the GNU GPL.  See http://www.gnu.org/licenses/gpl.txt
;
;  This program is part of the DeSmet C Compiler
;
;  DeSmet C is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundatation; either version 2 of the License, or any
;  later version.
;
;  DeSmet C is distributed in the hope that it will be useful, but WITHOUT
;  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
;  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
;  for more details.
;
; int _lcpy(unsigned doff, unsigned dseg, unsigned soff, unsigned sseg);

	cseg
	public _lcpy_
_lcpy_:	push    bp
	mov     bp,sp

	push    ds
	lds     si,[bp+8]
	les     di,[bp+4]
	cld
_lcpy0:	lodsb
	stosb
	or	al,al
	jnz	_lcpy0
	mov	ax,di
	pop     ds
	pop     bp
	ret
