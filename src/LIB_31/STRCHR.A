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

;       location = INDEX(string, character);
;       location = strchr(string, character);

        public  INDEX_,STRCHR_
INDEX_: 
STRCHR_:
        push    bp
        mov     bp,sp
        if      model
        push    ds
        lds     si,[bp+pbase]
        mov     bx,[bp+pbase+4]
        else
        mov     si,[bp+pbase]
        mov     bx,[bp+pbase+2]
        endif
        cld
in_loop:lodsb
        or      al,al           ;failure ?
        jz      in_fail
        cmp     al,bl           ;a match ?
        jz      in_quit
        jmp     in_loop
in_fail:mov     ah,0            ;not found
        if      model
        mov     es,ax
        mov     si,ax
        pop     ds
        endif
        pop     bp
        _ret
in_quit:
        if      model
        dec     si              ;address of match
        mov     ax,ds
        mov     es,ax
        pop     ds
        else
        lea     ax,[si-1]       ;address of match
        endif
        pop     bp
        _ret
