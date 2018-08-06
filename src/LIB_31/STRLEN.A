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


        public  STRLEN_
STRLEN_:
        push    bp
        mov     bp,sp
        if      model
        mov     si,[bp+6]
        mov     es,[bp+8]
        else
        mov     si,[bp+4]
        endif
        xor     ax,ax
        if      model
sl_loop:cmp     es: byte [si],0 ;string char
        else
sl_loop:cmp     byte [si],0     ;string char
        endif
        jz      slret
        inc     ax              ;length
        inc     si
        jmp     sl_loop
slret:  pop     bp
        _ret
