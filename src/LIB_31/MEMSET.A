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
        include "config.h"

        cseg

;       _setmem(addr,count,byte);       from bds c.

        public  _SETMEM_
_SETMEM_:
        push    bp
        mov     bp,sp
        if      model
        mov     di,[bp+6]
        mov     es,[bp+8]
        mov     cx,[bp+10]
        mov     ax,[bp+12]
        else
        mov     di,[bp+4]
        mov     cx,[bp+6]
        mov     ax,[bp+8]
        mov     bx,ds           ;set es to ds
        mov     es,bx
        endif

        jmp     setit

; void *memset(void *dst, char c, size_t n);

        public  memset_
memset_:
        push    bp
        mov     bp,sp
        if      model
        les     di,[bp+6]
        mov     ax,[bp+10]
        mov     cx,[bp+12]
        else
        mov     di,[bp+4]
        mov     ax,[bp+6]
        mov     cx,[bp+8]
        mov     bx,ds           ;set es to ds
        mov     es,bx
        endif

setit:

        cld                     ;clear up
rep     stosb                   ;set the area to al

        if model
        les     si,[bp+6]
        else
        mov     ax,[bp+4]
        endif
        pop     bp
        _ret
