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
;       upper.a - routines line toupper(), isdigit() etc.

        cseg

;       charactor = toupper(character);

        public  TOUPPER_
TOUPPER_:
        push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        cmp     al,'a'          ;if lower than a,
        jc      to_done         ; do nothing
        cmp     al,'z'+1        ;or if above z
        jnc     to_done         ; do nothing
        sub     al,'a'-'A'      ;else adjust
to_done:mov     ah,0            ;return an int
        pop     bp
        _ret

        split

;       charactor = isdigit(character);

        public  ISDIGIT_
ISDIGIT_:
        push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        cmp     al,'0'          ;if lower than 0,
        jc      not_dig         ; then false
        cmp     al,'9'+1        ;or if above 9
        jnc     not_dig         ; then false
        mov     ax,1            ;is a digit
        pop     bp
        _ret
not_dig:xor     ax,ax           ;return false
        pop     bp
        _ret

        split


;       the trivial ISALPHA_ etc. routines that are normally macros

        public  ISALPHA_ ,ISUPPER_ ,ISLOWER_ ,ISSPACE_ 
        public  ISALNUM_,ISASCII_,ISCNTRL_,ISPRINT_,ISPUNCT_
ISALPHA_:
        push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        cmp     al,'a'
        jb      already_lower
        sub     al,'a'-'A'
already_lower:
        cmp     al,'A'
        jb      return_false
        cmp     al,'Z'
        ja      return_false
return_true:
        mov     ax,1
        pop     bp
        _ret

return_false:
        xor     ax,ax
        pop     bp
        _ret

ISUPPER_:
        push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        cmp     al,'A'
        jb      return_false
        cmp     al,'Z'
        ja      return_false
        jmp     return_true

ISLOWER_:
        push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        cmp     al,'a'
        jb      return_false
        cmp     al,'z'
        ja      return_false
        jmp     return_true

ISSPACE_:
        push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        cmp     al,' '
        jz      return_true
        cmp     al,09h
        jz      return_true
        cmp     al,10
        jz      return_true
        cmp     al,13
        jz      return_true
        cmp     al,12
        jz      return_true
        jmp     return_false

ISALNUM_:
        push    bp
        mov     bp,sp
        push    [bp+pbase]
        _call   ISALPHA_
        pop     bx
        or      al,al
        jz      testdigit
        pop     bp
        _ret
testdigit:
        mov     al,[bp+pbase]
        cmp     al,'0'          ;if lower than 0,
        jc      return_false    ; then false
        cmp     al,'9'+1        ;or if above 9
        jnc     return_false    ; then false
        jmp     return_true


ISASCII_:
        push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        cmp     al,80h                  ;must be 0 to 7fh
        jb      return_true
        jmp     return_false

ISCNTRL_:
        push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        cmp     al,' '
        jb      return_true
        cmp     al,07fh
        jz      return_true
        jmp     return_false

ISPRINT_:
        push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        cmp     al,' '
        jb      return_false
        cmp     al,07fh
        jb      return_true
        jmp     return_false

ISPUNCT_:
        push    bp
        mov     bp,sp
        push    [bp+pbase]
        _call   ISPRINT_
        pop     bx
        or      al,al
        jnz     try_an
        pop     bp
        _ret
try_an:
        push    [bp+pbase]
        _call   ISALNUM_
        pop     bx
        xor     ax,1                    ;flip result
        pop     bp
        _ret

        split


        public  TOLOWER_
TOLOWER_:
        push    bp
        mov     bp,sp
        mov     al,[bp+pbase]
        cmp     al,'A'
        jb      return_ax
        cmp     al,'Z'
        ja      return_ax
        add     al,'a'-'A'
return_ax:mov	ah,0
        pop     bp
        _ret
