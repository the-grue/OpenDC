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
;       umuldiv.a - four byte unsigned multiply, divide and mod.

        cseg
        public  _UMUL4,_UDIV4,_UMOD4

;       _UMOD4  --  dx:ax = dx:ax % bx:cx.

_UMOD4: push    si                      ;save si,di
        push    di
        _call   xdiv4
        mov     dx,si
        mov     ax,bx
        pop     di
        pop     si
        _ret


;       _UDIV4  --  dx:ax = dx:ax / bx:cx. remainder = si:bx.

_UDIV4: push    si                      ;save si and di
        push    di
        _call   xdiv4
        pop     di
        pop     si
        _ret

;       do the real work of division

xdiv4:  or bx,bx
        jnz dword_dword_div
        cmp cx,dx
        jbe dword_word_long
        div cx
        xor si,si
        mov bx,dx
        xor dx,dx
        _ret
dword_word_long:
        mov bx,ax
        mov ax,dx
        xor dx,dx
        div cx
        xchg bx,ax
        div cx
        xchg dx,bx
        xor si,si
        _ret
dword_dword_div:
        push bp
        mov bp,cx
        mov di,bx
        xor     si,si
        mov     bx,si
        mov     cx, 32
div_mod_loop:
        shl     bx, 1
        rcl     si, 1
        shl     ax, 1
        rcl     dx, 1
        adc     bx, 0
        sub     bx, bp
        sbb     si, di
        inc     ax
        jnc     inc_remainder
        add     bx, bp
        adc     si, di
        dec     ax
inc_remainder:
        loop    div_mod_loop
        pop     bp
        _ret

;       _mul4  --  dx:ax = dx:ax * bx:cx.


_UMUL4: push    si
        push    di
        mov di,ax
        mov ax,dx
        mul cx
        mov si,ax
        mov ax,bx
        mul di
        add si,ax
        mov ax,cx
        mul di
        add dx,si
        pop     di
        pop     si
        _ret
