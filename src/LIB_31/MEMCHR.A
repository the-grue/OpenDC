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
; void *memchr(const void *s, int c, size_t n);

        cseg
        public memchr_
memchr_:push    bp
        mov     bp,sp

        if      LARGE_CASE

        les     di,[bp+6]
        mov     al,[bp+10]
        mov     cx,[bp+12]

        else

        mov     ax,ds
        mov     es,ax
        mov     di,[bp+4]
        mov     al,[bp+6]
        mov     cx,[bp+8]

        endif

        jcxz    notfnd
        cld
  repne scasb
        jne     notfnd

        if      LARGE_CASE

        mov     si,di
        dec     si
        pop     bp
        lret
notfnd: xor     si,si
        mov     es,si
        pop     bp
        lret

        else

        mov     ax,di
        dec     ax
        pop     bp
        ret
notfnd: xor     ax,ax
        pop     bp
        ret

        endif
