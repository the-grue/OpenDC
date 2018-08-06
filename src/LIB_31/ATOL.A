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

;       ATOI_  --       convert a string with an integer to an integer
;       ATOL_  --       convert a string with an integer to a long

        public  ATOL_,ATOI_
ATOL_:
ATOI_:  push    bp
        mov     bp,sp
        if      model
        mov     bx,[bp+6]
        mov     es,[bp+8]
        else
        mov     bx,[bp+4]
        endif
        xor     ax,ax
        xor     bp,bp                   ;high 2 bytes
        mov     si,0
        if      model
skipbl: mov     cl,es:[bx]
        else
skipbl: mov     cl,[bx]
        endif
        inc     bx
        cmp     cl,' '
        jz      skipbl
        cmp     cl,9
        jz      skipbl
        cmp     cl,'+'
        jz      skipbl
        cmp     cl,'-'                  ;is it minus ?
        jnz     no_sign
        mov     si,1                    ;flag for sign
        if      model
        mov     cl,es:[bx]
        else
        mov     cl,[bx]
        endif
        inc     bx
no_sign:cmp     cl,'0'                  ;is it a digit?
        jc      endatoi
        cmp     cl,'9'
        ja      endatoi
        push    ax                      ;calc hi word times 10
        mov     ax,10
        mul     bp
        mov     bp,ax                   ;old hi word times 10
        pop     ax
        sub     cl,'0'
        mov     ch,0
        mov     di,10
        mul     di
        add     ax,cx
        adc     dx,0
        add     bp,dx                   ;add to hi word
        if      model
        mov     cl,es:[bx]
        else
        mov     cl,[bx]
        endif
        inc     bx
        jmp     no_sign
endatoi:
        mov     dx,bp
        cmp     si,0
        jz      quita
        not     dx
        neg     ax
        sbb     dx,-1
quita:  pop     bp
        _ret
