;
;  Released under the GNU GPL.  See http://www.gnu.org/licenses/gpl.txt
;
;  This program is part of the DeSmet C Compiler
;
;  DeSmet C is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundatation; either version 2 of the License, or any
;  later version.
;
;  DeSmet C is distributed in the hope that it will be useful, but WITHOUT
;  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
;  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
;  for more details.
;
;       int _lmov(count, from_offset, from_segment, to_offset, to_segment);

;	returns 0 == O.K, !0 == error

	cseg
	public  _lmov_

_lmov_	:	push    bp
	mov     bp,sp
	push    ds			;save ds
	lds     si,[bp+6]		;from
	les     di,[bp+10]		;to

	mov	ax,ds			; get canonical from para
	mov	cl,4
	mov	bx,si
	shr	bx,cl
	add	ax,bx
	jc	err			; wrapped around 1M
	mov	ds,ax
	and	si,15			; set canonical offset

	mov	dx,es			; get canonical to para
	mov	bx,di
	shr	bx,cl
	add	dx,bx
	jc	err			; wrap-around
	mov	es,dx
	and	di,15

	mov     cx,[bp+4]		;count
	mov	bx,si
	add	bx,cx			; test for room
	jc	err
	mov	bx,di
	add	bx,cx
	jc	err

	cmp     ax,dx			;moving up ?
	ja	domov
	jb	movdn
	cmp	si,di
	jae	domov

movdn:
	add     si,cx			;change to move down
	sub     si,2
	mov     di,bx
	sub     di,2
	shr     cx,1			;do a word move
	std
	rep     movsw			;move the bytes
	jnc     end_moved
	inc     si
	inc     di
	movsb
end_moved:
	xor	ax,ax
	pop     ds			;restore ds
	pop     bp
	ret

domov:
	shr     cx,1			;do a word move
	cld
	rep     movsw			;move the bytes
	jnc     end_move
	movsb
end_move:
	xor	ax,ax
	pop     ds			;restore ds
	pop     bp
	ret

err:
	mov	ax,1
	pop	ds
	pop	bp
	ret
