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
; void *memcmp(const void *s1, const void *s2, size_t n);

        cseg
        public memcmp_
memcmp_:push    bp
        mov     bp,sp

        if      LARGE_CASE

        push    ds
        lds     si,[bp+6]
        les     di,[bp+10]
        mov     cx,[bp+14]

        else

        mov     ax,ds
        mov     es,ax
        mov     si,[bp+4]
        mov     di,[bp+6]
        mov     cx,[bp+8]

        endif

        xor     ax,ax
        jcxz    eq
        cld
    rep cmpsb
        je      eq
        ja      gt

        dec     ax

        if      LARGE_CASE

        pop     ds
        pop     bp
        lret
gt:     inc     ax
eq:     pop     ds
        pop     bp
        lret

        else

        pop     bp
        ret
gt:     inc     ax
eq:     pop     bp
        ret

        endif
