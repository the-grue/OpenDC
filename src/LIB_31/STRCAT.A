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


;       strcat(destination string,source string);
;       strncat(destination string,source string, maximum);

        public  STRNCAT_,STRCAT_
STRNCAT_:
        push    bp
        mov     bp,sp
        if      model
        push    ds
        les     di,[bp+pbase]
        lds     si,[bp+pbase+4]
        mov     cx,[bp+pbase+8]
        else
        mov     di,[bp+pbase]
        mov     si,[bp+pbase+2]
        mov     cx,[bp+pbase+4]
        endif
	jcxz	cc_quit
        jmp     catcont


STRCAT_:
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
        mov     cx,65535        ;big number
catcont:mov     ax,di           ;return pointer to result
        if      model
skiplp: mov     dl,es:[di]      ;skip source
        else
skiplp: mov     dl,[di]         ;skip source
        endif
        or      dl,dl
        jz      cc_loop
        inc     di
        jmp     skiplp
cc_loop:mov     dl,[si]         ;from source
        if      model
        mov     es:[di],dl      ;to destination
        else
        mov     [di],dl         ;to destination
        endif
        inc     si
        inc     di
        or      dl,dl           ;till zero copied
        jz      cc_quit
        loop    cc_loop
        mov     byte [di],0
cc_quit:
        if      model
        pop     ds
        mov     si,ax           ; pointer return
        endif
        pop     bp
        _ret
