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


;       _poke  --  set a character.
;                  poke(char, offset, segment);

        public  _poke_
_poke_: push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        mov     di,[bp+pbase+2]
        mov     es,[bp+pbase+4]
        stosb
        pop     bp
        _ret

        split


;       _peek  --  read a character.
;                  char=peek(offset, segment);

        public  _peek_
_peek_: push    bp
        mov     bp,sp
        mov     si,[bp+pbase]
        mov     es,[bp+pbase+2]
        mov     al,es:[si]
        mov     ah,0
        pop     bp
        _ret



        split

;       _outb  --       output a byte. _outb(byte,port);

        public  _outb_
_outb_: push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        mov     dx,[bp+pbase+2]
        out     dx,al
        pop     bp
        _ret

        split

;       _outw  --       output a word. _outw(word,port);

        public  _outw_
_outw_: push    bp
        mov     bp,sp
        mov     ax,[bp+pbase]
        mov     dx,[bp+pbase+2]
        out     dx,ax
        pop     bp
        _ret


        split

;       _inb  --        input a byte. char = _inb(port);

        public  _inb_
_inb_:  push    bp
        mov     bp,sp
        mov     dx,[bp+pbase]
        in      al,dx
        mov     ah,0
        pop     bp
        _ret

        split


;       _inw  --        input a word. int = _inw(port);

        public  _inw_
_inw_:  push    bp
        mov     bp,sp
        mov     dx,[bp+pbase]
        in      ax,dx
        pop     bp
        _ret

        split


;       _doint  --  cause an interrupt. _doint(int number);

        dseg
        public  _carryf_,_zerof_
        public  _rax_,_rbx_,_rcx_,_rdx_,_rsi_,_rdi_,_res_,_rds_
_carryf_ db     0
_zerof_ db      0
_rax_   dw      0
_rbx_   dw      0
_rcx_   dw      0
_rdx_   dw      0
_rsi_   dw      0
_rdi_   dw      0
_res_   dw      0
_rds_   dw      0


        cseg
oldsp   dw      0
oldds   dw      0

        public  _doint_
_doint_:push    bp
        mov     bp,sp
        mov     oldsp,sp
        mov     oldds,ds
        mov     al,[bp+pbase]           ;int number
        mov     intnum,al
        mov     ax,_rax_
        mov     bx,_rbx_
        mov     cx,_rcx_
        mov     dx,_rdx_
        mov     si,_rsi_
        mov     di,_rdi_
        mov     es,_res_
        cmp     _rds_,-1
        jz      gotds
        mov     ds,_rds_
gotds:

        db      0cdh                    ;do the interrupt
intnum  db      3

        mov     sp,oldsp
        push    ds
        mov     ds,oldds
        pop     _rds_
        mov     _carryf_,0
        jnc     carry_set
        mov     _carryf_,1
carry_set:
        mov     _zerof_,0
        jnz     zero_set
        mov     _zerof_,1
zero_set:


;       restore the registers

        mov     bp,sp
        mov     _rax_,ax
        mov     _rbx_,bx
        mov     _rcx_,cx
        mov     _rdx_,dx
        mov     _rsi_,si
        mov     _rdi_,di
        mov     _res_,es
        pop     bp
        _ret
