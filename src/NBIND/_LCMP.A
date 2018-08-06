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
; int _lcmp(unsigned off, unsigned seg, unsigned off, unsigned seg, size_t n);

        cseg
        public _lcmp_
_lcmp_:	push    bp
        mov     bp,sp

        push    ds
        lds     si,[bp+4]
        les     di,[bp+8]
        mov     cx,[bp+12]
        xor     ax,ax
        jcxz    eq
        cld
  	rep	cmpsb
        je      eq
        ja      gt

        dec     ax

        pop     ds
        pop     bp
        ret
gt:     inc     ax
eq:     pop     ds
        pop     bp
        ret
