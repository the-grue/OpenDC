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
;       return = strcmpi(first string, second string);

;                return < 0 if f < s, 0 if f == s, >0 if f > s

        cseg
        public  STRCMPI_
STRCMPI_:
        public  STRICMP_
STRICMP_:
        push    bp
        mov     bp,sp
        if      model
        push    ds
        les     di,[bp+pbase]
        lds     si,[bp+pbase+4]
        else
        mov     di,[bp+pbase]
        mov     si,[bp+pbase+2]
        endif
        mov     cx,65535
sp_loop:mov     al,[si]         ;from second
        cmp     al,'A'          ;see if upper case
        jb      lup
        cmp     al,'Z'+1
        jnb     lup
        add     al,'a'-'A'
lup:
        if      model
        mov     ah,es:[di]              ;to first
        else
        mov     ah,[di]         ;to first
        endif
        cmp     ah,'A'          ;see if upper case
        jb      rup
        cmp     ah,'Z'+1
        jnb     rup
        add     ah,'a'-'A'
rup:    cmp     ah,al
        jb      flow
        ja      fhigh
        inc     si              ;increment indexes
        inc     di
        or      al,al
        jz      sp_quit
        loop    sp_loop
sp_quit:mov     ax,0            ;return a zero as both the same
        if      model
        pop     ds
        endif
        pop     bp
        _ret
flow:   mov     ax,-1           ;return -1 as first is low
        if      model
        pop     ds
        endif
        pop     bp
        _ret
fhigh:  mov     ax,1            ;return 1 as first is high
        if      model
        pop     ds
        endif
        pop     bp
        _ret


        split
;    example:   int val;
;               val = strcspn("hello there","aeiou");

        public  STRCSPN_
STRCSPN_:
        push    bp
        mov     bp,sp
        if      model
        push    ds
        les     di,[bp+pbase]
        lds     si,[bp+pbase+4]
        else
        mov     di,[bp+pbase]
        mov     si,[bp+pbase+2]
        endif
        mov     cx,0
cspn_loop:
        if      model
        mov     al,es:[di]              ;to first
        else
        mov     al,[di]                 ;to first
        endif
        inc     di
        or      al,al                   ;at end?
        jz      done
        mov     bx,si                   ;addr of ok list
mlist:  mov     ah,[bx]                 ;next to match to
        inc     bx
        or      ah,ah
        jz      bump_count
        cmp     ah,al                   ;matching char ?
        jz      cspn_loop
        jmp     mlist
bump_count:
        inc     cx
        jmp     cspn_loop
done:   if      model
        pop     ds
        endif
        pop     bp
        mov     ax,cx
        _ret


        split
;    example:   char *ptr,*strpbrk();
;               char *at;
;               at = strpbrk("hello world",aeiou");

        public  STRPBRK_
STRPBRK_:
        push    bp
        mov     bp,sp
        if      model
        push    ds
        les     si,[bp+pbase]
        else
        mov     si,[bp+pbase]
        endif
        dec     si
pb_loop:
        inc     si
        if      model
        lds     di,[bp+pbase+4]
        cmp     byte es:[si],0          ;no match ?
        jz      pb_done
        else
        mov     di,[bp+pbase+2]
        cmp     byte [si],0             ;no match ?
        jz      pb_done
        endif
next_ch:
        mov     al,[di]                 ;list of wanted chars
        inc     di
        or      al,al                   ;done list of candidates ?
        jz      pb_loop
        if      model
        cmp     al,es:[si]              ;have m match
        jnz     next_ch
        pop     ds
        pop     bp                      ;have es:si at match
        _ret
        else
        cmp     al,[si]                 ;have m match
        jnz     next_ch
        mov     ax,si                   ;return pointer to match
        pop     bp                      ;have ax at match
        _ret
        endif

pb_done:
        if      model
        pop     ds
        xor     si,si                   ;return zero pointer
        mov     es,si
        else
        mov     ax,0
        endif
        pop     bp
        _ret


        split
;    example:   int val;
;               val = strspn("hello world","help");

        public  STRSPN_
STRSPN_:
        push    bp
        mov     bp,sp
        if      model
        push    ds
        les     si,[bp+pbase]
        else
        mov     si,[bp+pbase]
        endif
        mov     bx,-1
spn_loop:
        inc     bx
        if      model
        lds     di,[bp+pbase+4]
        cmp     byte es:[si+bx],0               ;no match ?
        jz      spn_done
        else
        mov     di,[bp+pbase+2]
        cmp     byte [si+bx],0          ;no match ?
        jz      spn_done
        endif
spn_nch:
        mov     al,[di]                 ;list of wanted chars
        inc     di
        or      al,al                   ;done list of candidates ?
        jz      spn_done
        if      model
        cmp     al,es:[si+bx]           ;have m match
        else
        cmp     al,[si+bx]                      ;have m match
        endif
        jnz     spn_nch
        jmp     spn_loop
spn_done:
        mov     ax,bx
        if      model
        pop     ds
        endif
        pop     bp
        _ret


