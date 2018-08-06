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

;       location = rindex(string, character);
;       location = strrchr(string, character);

        public  RINDEX_,STRRCHR_
RINDEX_:
STRRCHR_:
        push    bp
        mov     bp,sp
        if      model
        push    ds
        lds     si,[bp+pbase]
        mov     bx,[bp+pbase+4] ;bl is character to find
        else
        mov     si,[bp+pbase]
        mov     bx,[bp+pbase+2]
        endif
        cld
        xor     cx,cx
in_rloop:lodsb
        cmp     al,bl           ;a match ?
        jnz     in_next
        lea     cx,[si-1]       ;address of match
in_next:
        or      al,al           ;failure ?
        jnz     in_rloop
        if      model
        mov     si,cx           ; pointer to last found
        mov     ax,ds
        mov     es,ax
        cmp     cx,0            ; if zero neex zero es
        jnz     got_i
        mov     es,cx
got_i:  pop     ds
        else
        mov     ax,cx           ;return addr of last found
        endif
        pop     bp
        _ret
