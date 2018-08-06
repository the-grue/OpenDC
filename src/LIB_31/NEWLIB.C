/*
 *  Released under the GNU LGPL.  See http://www.gnu.org/licenses/lgpl.txt
 *
 *  This program is part of the DeSmet C Compiler
 *
 *  This library is free software * you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundatation * either version 2.1 of the License, or
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY * without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 */
/*	FILELENGTH  --	get the file length. must use lseek.	*/

/*    synopsis:  int file;
                 long filelength(file); */

long lseek();

long filelength(filenum)
	int filenum; {
	long was_at,end_at;

	was_at=lseek(filenum,0L,1);
	end_at=lseek(filenum,0l,2);
	lseek(filenum,was_at,0);
	return end_at;
	}


/*	FILENO  --	return the file number of a stream	*/

fileno(filenum)
	int filenum; {
	
	return filenum;
	}



/*	FREOPEN	--	reopen a stream.	*/

/*    synopsis:  FILE *freopen(filename,type,stream)
                 char *filename,*type;
                 FILE *stream; */

char *fopen();

char *freopen(name,type,stream)
	char *name,*type;
	int  stream; {

	close(stream);	/* close old file.	*/
	return fopen(name,type);
	}



/*	FTELL  --	return current file location.	*/

/*    synopsis:  int file;
                 long ftell(file);  */

long lseek();

long ftell(filenum)
	int filenum; {

	return lseek(filenum,0L,1);
	}



/*	isatty  --  return the device type for a file.	*/

extern int _files[];

isatty(filenum)
	int  filenum; {

	filenum=_files[filenum];
#asm
	mov		ah,44h				;code for ioctl
	mov		al,0				;code for return info
	if		LARGE_CASE
	mov		bx,[bp+6]
	else
	mov		bx,[bp+4]			;file number
	endif
	int		21h					;call ms-dos
	xor		ax,ax
	test	dx,10000000b		;1 if device
	jz		not_dev
	inc		ax
not_dev:
#
	}


/*	LOCKING  --	lock and unlock a segment of a file.	*/

/*      int locking(file,mode,numbytes)
        int file,mode,numbytes; */

static long fileat;
long lseek();

locking(file,mode,numbytes)
	int  file,mode,numbytes; {

	fileat=lseek(file,0l,1);
	file=_files[file];
#asm
	mov		ah,5ch				;code for lock/unlock
	if		LARGE_CASE
	mov		al,[bp+8]			;lock or unlock
	mov		bx,[bp+6]			;file handle
	mov		di,[bp+10]			;length of lock
	else
	mov		al,[bp+6]			;lock or unlock
	mov		bx,[bp+4]			;file handle
	mov		di,[bp+8]			;length of lock
	endif
	xor		si,si				;length <= 64k
	mov		dx,word fileat_		;current offset
	mov		cx,word fileat_+2
	int		21h					;call ms-dos
	mov		ax,0				;assume success
	jnc		lock_ok
	dec		ax
lock_ok:
#
	}
