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
;       muldiv.a - four byte multiply, divide and mod.

        dseg
sign    db      0

        cseg
        public  _MUL4,_DIV4,_MOD4

;       _MOD4  --  dx:ax = dx:ax % bx:cx.

_MOD4:  push    si                      ;save si,di
        push    di
        _call   xdiv4
        mov     dx,si
        mov     ax,bx
        pop     di
        pop     si
        _ret


;       _DIV4  --  dx:ax = dx:ax / bx:cx. remainder = si:bx.

_DIV4:  push    si                      ;save si and di
        push    di
        _call   xdiv4
        pop     di
        pop     si
        _ret

;       do the real work of division

xdiv4:  _call   set_sign
        or bx,bx
        jnz dword_dword_div
        cmp cx,dx
        jbe dword_word_long
        div cx
        xor si,si
        mov bx,dx
        xor dx,dx
        jmp     fix_sign
dword_word_long:
        mov bx,ax
        mov ax,dx
        xor dx,dx
        div cx
        xchg bx,ax
        div cx
        xchg dx,bx
        xor si,si
        jmp     fix_sign
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

fix_sign:
        test    sign,1
        jz      posans
        not     dx
        neg     ax
        sbb     dx,-1
posans: test    sign,2                  ;was divisor negative ?
        jz      donediv
        not     si
        neg     bx
        sbb     si,-1
donediv:
        _ret


;       make dx:ax and bx:cx positive. set sign.

set_sign:
        mov     sign,0                  ;assume signs the same
        or      dx,dx                   ;dx:ax positive ?
        jns     dxpos
        mov     sign,3                  ;signs different
        not     dx
        neg     ax
        sbb     dx,-1
dxpos:  or      bx,bx                   ;is bx:cx positive ?
        jns     bxpos
        xor     sign,1
        not     bx
        neg     cx
        sbb     bx,-1
bxpos:  _ret




;       _MUL4  --  dx:ax = dx:ax * bx:cx.


_MUL4:  push    si
        push    di
        _call   set_sign
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
        _call   fix_sign
        pop     di
        pop     si
        _ret

        public  LABS_
LABS_:  push    bp
        mov     bp,sp
        mov     ax,[bp+pbase]
        mov     dx,[bp+pbase+2]
        test    dh,80h
        jz      is_pos
        not     dx
        neg     ax
        sbb     dx,-1
is_pos: pop     bp
        _ret
