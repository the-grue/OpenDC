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
;       string.a - strlen_, strcmp_, STRCPY_ and _setmem.

        cseg

;       strcpy(destination string,source string);
;       strncpy(destination string,source string,maximum);



        public  STRNCPY_,STRCPY_
STRNCPY_:
        push    bp
        mov     bp,sp
        if      model
        push    ds
        les     di,[bp+pbase]   ;destination
        lds     si,[bp+pbase+4] ;source
        mov     cx,[bp+pbase+8] ;count
        else
        mov     di,[bp+pbase]   ;destination offset
        mov     si,[bp+pbase+2] ;source offset
        mov     cx,[bp+pbase+4] ;count
        endif
        jcxz    sc_quit
        jmp     cpycont


STRCPY_:
        push    bp
        mov     bp,sp
        if      model
        push    ds
        les     di,[bp+pbase]   ;destination
        lds     si,[bp+pbase+4] ;source
        else
        mov     di,[bp+pbase]   ;destination
        mov     si,[bp+pbase+2] ;source
        endif
        mov     cx,65535        ;big number
cpycont:
        mov     ax,di           ;return result
sc_loop:mov     bl,[si]         ;from source
        if      model
        mov     es:[di],bl      ;to destination
        else
        mov     [di],bl         ;to destination
        endif
        inc     si
        inc     di
        or      bl,bl           ;till zero copied
        jz      sc_quit
        loop    sc_loop
sc_quit:
        if      model
        pop     ds
        endif
        pop     bp
        if      model
        mov     si,ax           ; pointer return
        endif
        _ret

