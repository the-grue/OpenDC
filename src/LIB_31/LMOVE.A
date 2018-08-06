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
;       lmove.a  --     long move routine

;       _lmove(count, from_offset, from_segment, to_offset, to_segment);

        cseg
        public  _lmove_

_lmove_:        push    bp
        mov     bp,sp
        mov     si,[bp+pbase+2]         ;from offset
        mov     di,[bp+pbase+6]         ;to offset
        mov     es,[bp+pbase+8]         ;destination segment
        push    ds                      ;save ds
        mov     ds,[bp+pbase+4]         ;source segment
        cld
        mov	ax,ds
	mov	cl,4
	mov	bx,si
	shr	bx,cl
	add	ax,bx
	jc	err
	mov	ds,ax
	and	si,15
        mov	dx,es
	mov	bx,di
	shr	bx,cl
	add	dx,bx
	jc	err
	mov	es,dx
	and	di,15
        mov     cx,[bp+pbase]           ;count
        mov	bx,di
        add	bx,cx			; test for room
        jc	err
        cmp     ax,dx                   ;moving up ?
        ja	domov
        jb	movdn
        cmp	si,di
        jae	domov
movdn:
        add     si,cx                   ;change to move down
        jc	err
        sub     si,2
        mov     di,bx
        sub     di,2
        std
        shr     cx,1                    ;do a word move
rep     movsw                           ;move the bytes
        jnc     end_moved
        inc     si
        inc     di
        movsb
end_moved:
        cld
        xor	ax,ax
        pop     ds                      ;restore ds
        pop     bp
        _ret
domov:
        shr     cx,1                    ;do a word move
rep     movsw                           ;move the bytes
        jnc     end_move
        movsb
end_move:
	xor	ax,ax
        pop     ds                      ;restore ds
        pop     bp
        _ret
err:
	mov	ax,1
	pop	ds
	pop	bp
	_ret
