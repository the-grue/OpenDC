/*
/* free: prints the amount of free space available on the indicated disk
/*
/* written by Michael Ouye 8/21/84.
*/

unsigned avail, total, sectors, bytes;

long int free(device)
	int device; {

#asm
	mov		dl, byte [bp+4]		; device
	mov		ah, 36H
	int		21H
	cmp		AX,0FFFFH
	jne		over
	mov		DX,0FFFFH			; return -1 if bad device
	pop		bp
	ret
over:
	mov		word avail_,bx
	mov		word total_,dx
	mov		word sectors_,ax
	mov		word bytes_,cx
#
	return ((long)avail * (long)sectors * (long)bytes);
	}

main(argc, argv)
	int argc; char *argv[]; {
	long int how_much;
	int device;


	argv++;
	if (argc <= 1) 
		device = 0;
	else
		device = toupper(**argv) - '@';

	how_much = free(device);
	if (how_much == -1L) {
		puts("bad device\n");
		exit(1);
		}
	printf("%D bytes available\n", how_much);
	exit(0);
	}
