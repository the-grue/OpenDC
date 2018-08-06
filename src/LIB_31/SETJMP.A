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
;       setjmp  --      save bp and sp for a longjmp call

        dseg
savesp  dw      0
savebp  dw      0
saveip  dw      0
        if      model
savecs  dw      0
        endif
        cseg

;       setjmp and longjmp allow intermodule goto's

        if      !model
        public  setjmp_
setjmp_:pop     dx
        pop     si                      ;environment pointer or 0
        push    si
        or      si,si                   ;is zero, use local area
        jz      local_save
        mov     [si],sp
        mov     [si+2],bp
        mov     [si+4],dx
        mov     ax,0
        jmp     dx
local_save:
        mov     savebp,bp
        mov     saveip,dx
        mov     savesp,sp
        mov     ax,0
        jmp     dx

        public  longjmp_
longjmp_:
        pop     dx                      ;ignore return address
        pop     si                      ;ignore environment if zero
        pop     ax                      ;return value
        or      si,si                   ;if zero, use local area
        jnz     real_env
        mov     si,offset savesp        ;load from local area
real_env:
        mov     sp,[si]
        mov     bp,[si+2]
        jmp     word [si+4]

        else                            ;large case

        public  setjmp_
setjmp_:pop     dx
        pop     bx                      ;ret addr
        pop     si                      ;environment pointer or 0
        pop     es
        push    es
        push    si
        mov     ax,sp
        push    bx
        push    dx
        or      si,si                   ;is zero, use local area
        jz      local_save
        mov     es:[si],ax
        mov     es:[si+2],bp
        mov     es:[si+4],dx
        mov     es:[si+6],bx
        mov     ax,0
        lret
local_save:
        mov     savesp,ax
        mov     savebp,bp
        mov     saveip,dx
        mov     savecs,bx
        mov     ax,0
        lret

        public  longjmp_
longjmp_:
        pop     dx                      ;ignore return address
        pop     dx
        pop     si                      ;ignore environment if zero
        or      si,si                   ;if zero, use local area
        jnz     real_env
        pop     ax                      ;return value
        mov     si,offset savesp        ;load from local area
        push    ds
        pop     es
        cmp     ax,0                    ;if zero, assume part of addr
        jnz     real_end
        pop     ax
        jmp     real_end
real_env:
        pop     es
        pop     ax                      ;return value
real_end:
        mov     sp,es:[si]
        mov     bp,es:[si+2]
        ljmp    es:[si+4]
        endif
