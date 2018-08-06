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
;       move.a  --      move some data

;       _move(num,from,to);

        cseg
        public  _MOVE_
_MOVE_: push    bp
        mov     bp,sp
        if      model
        push    ds
        endif
        mov     cx,[bp+pbase]   ;number of bytes
        mov     si,[bp+pbase+2] ;source
        if      model
        mov     ds,[bp+pbase+4] ;source segment
        les     di,[bp+pbase+6] ;destination
        else
        mov     di,[bp+pbase+4] ;destination
        mov     ax,ds           ;set es=ds
        mov     es,ax
        endif

        jmp     moveit

; void *memmove(void * dst, void * src, size_t n)

        public  memmove_
memmove_:

        push    bp
        mov     bp,sp

        if      model

        push    ds
        les     di,[bp+pbase]   ;destination
        lds     si,[bp+pbase+4] ;source
        mov     cx,[bp+pbase+8] ;number of bytes

        else

        mov     di,[bp+pbase]   ;destination
        mov     ax,ds           ;set es=ds
        mov     es,ax
        mov     si,[bp+pbase+2] ;source
        mov     cx,[bp+pbase+4] ;number of bytes

        endif

moveit:

        cld
        cmp     si,di           ;moving up ?
        jae     domov
        add     si,cx           ;change to move down
        sub     si,2
        add     di,cx
        sub     di,2
        std
        shr     cx,1            ;do a word move
rep     movsw
        jnc     move_end
        inc     si
        inc     di
        movsb                   ;move the odd byte
        jmp     move_end

domov:
        shr     cx,1            ;do a word move
rep     movsw
        jnc     move_end
        movsb                   ;move the odd byte
move_end:
        cld
        if      model
        pop     ds
        les     si,[bp+pbase]   ;destination
        else
        mov     ax,[bp+pbase]   ;destination
        endif
        pop     bp
        _ret
