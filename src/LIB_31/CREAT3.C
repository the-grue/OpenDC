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
/*	CREAT.C	--	CONTAINS CREAT, OPEN, READ, WRITE, AND CLOSE	*/
/*				MS-DOS V2.0 version.	*/


/*	STANDARD FILE NUMBERS	*/

#define	STDIN	0
#define	STDOUT	1
#define	STDERR	2
#define STDPRN	4

/*	FILES contains status of all files	*/


#define CONTC	3
#define BS		8
#define LF		10
#define CR		13
#define CONTX	24
#define CONTZ	26
#define RUB		127

#define	NFILE	20
#define NBUFFER 10
#define BUF_MAX	64

#define BUF_UNUSED 0
#define BUF_IN	1
#define BUF_OUT	2

#define	F_ERR	1
#define	F_EOF	2

#define MODE3	0x80

int  _files[NFILE]={0,1,2,3,4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
				   -1,-1,-1,-1};
int _oldfile=99;		/* last file scanf'ed	*/
int _ungotten[NFILE]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
					  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
char _flag[NFILE]={MODE3};

static char buffer[NBUFFER][BUF_MAX+1];

char *_buffer_at[NFILE],_buffer_is[NFILE];

static _newfile(),_fixname(),alloc_buf(),dev_type();
long lseek(),_lseek();

/*	CREAT			filenum=creat(name)		*/

creat(name)
	char *name; {
	int  fnum,err;
	char newname[65];

	_fixname(newname,name);
	if ((fnum=_newfile()) == -1) return -1;
	err=_files[fnum]=_creat(newname);
	if (err == -1) return -1;
	_flag[fnum]=0;
	alloc_buffer(fnum,err);
	return fnum;
	}



/*	OPEN			filenum=open(name,mode)		*/

open(name,mod)
	char *name,mod; {
	int  fnum,err;
	char newname[65],mode3;
	
	mode3=0;				/* no check for control z	*/
	if (mod > 2 && mod < 6) {
		mod-=3;	/* mode 3 means test for control Z */
		mode3=MODE3;
		}
	_fixname(newname,name);
	if ((fnum=_newfile()) == -1) return -1;
	err=_files[fnum]=_open(newname,mod);
	if (err == -1) return -1;
	alloc_buffer(fnum,err);
	_flag[fnum]=mode3;
	_ungotten[fnum]=-1;
	return fnum;
	}





/*	READ			charactors read = read(filenum,buffer,nbytes);	*/

read(filenum,buffer,nbytes)
	int  filenum;
	unsigned nbytes;
	char *buffer; {
	int i,nin,fnum;
	char *buf;

	if (_buffer_is[filenum]) {
		if (_buffer_is[filenum] == BUF_OUT) fflush(filenum);
		if (nbytes < 16 || _buffer_at[filenum][0] != BUF_MAX+1) {
			buf=_buffer_at[filenum];
			for (i=0; i < nbytes; i++) {
				if (buf[0] == BUF_MAX+1) {
					if ((nin=read(filenum,&buf[1],BUF_MAX)) == 0)
						break;
					buf[0]=BUF_MAX-nin+1;
					if (nin < BUF_MAX) 
						_move(nin,&buf[1],
							&buf[BUF_MAX+1-nin]);
					}
				*buffer++=buf[buf[0]++];
				}
			return i;
			}
		}

	fnum=_files[filenum];
	nin=_read(fnum,buffer,nbytes);
	if (_flag[filenum]&MODE3) {		/* stop at control z	*/
		for (i=nin-1; i >=0; i--)
			if (buffer[i] == CONTZ) {
				_buffer_at[filenum][0]=BUF_MAX+1;
				lseek(filenum,(long)(i-nin),1);
				nin=i;
				}
			}
	return nin;
	}



/*	WRITE			charactors written = write(filenum,buffer,nbytes);	*/

write(filenum,buffer,nbytes)
	int  filenum;
	unsigned nbytes;
	char *buffer; {
	int  i;
	char *buf;

	if(nbytes == 0)
		return 0;
	buf=_buffer_at[filenum];
	if (_buffer_is[filenum] == BUF_IN) {
		if (buf[0] != BUF_MAX+1)
			lseek(filenum,0L,1);
		_buffer_is[filenum]=BUF_OUT;
		buf[0]=1;
		}

	if (_buffer_is[filenum]) {
		if (nbytes < 16) {
			for (i=0; i < nbytes; i++) {
				buf[buf[0]++]=*buffer++;
				if (buf[0] == BUF_MAX+1) {
					if (fflush(filenum) == -1)
						return -1;
					_buffer_is[filenum]=BUF_OUT;
					buf[0]=1;
					}
				}
			return nbytes;
			}
		if (buf[0] !=1) {
			fflush(filenum);
			_buffer_is[filenum]=BUF_OUT;
			buf[0]=1;
			}
		}

	filenum=_files[filenum];
	return _write(filenum,buffer,nbytes);
	}



/*	CLOSE			error = close(file number);	*/

close(filenum)
	int  filenum; {
	int fnum;

	_oldfile=99;
	if (fflush(filenum)) return -1;
	if (_buffer_is[filenum]) *_buffer_at[filenum]=0;
	_buffer_is[filenum]=BUF_UNUSED;
	if (filenum > STDPRN) {
		fnum=_files[filenum];
		_files[filenum]=-1;
		return _close(fnum);
		}
	return 0;
	}




/*	GETC, GETW, PUTC, PUTW		*/

getc(filenum)
	int  filenum; {
	int  value,count;

	value=0;
	if (_ungotten[filenum] != -1) {
		value=_ungotten[filenum];
		_ungotten[filenum]=-1;
		return value;
		}
	if ((count=read(filenum,&value,1)) == 0) {
		_flag[filenum] |= F_EOF;
		value=-1;
		}
	else if(count==-1) {
		_flag[filenum] |= F_ERR;
		value=-1;
		}
	return value;
	}


getw(filenum)
	int  filenum; {
	int  value, count;

	if ((count=read(filenum,&value,2)) == 0) {
		_flag[filenum] |= F_EOF;
		value=-1;
		}
	else if(count==-1) {
		_flag[filenum] |= F_ERR;
		value=-1;
		}
	return value;
	}


putc(ch,filenum)
	int  filenum;
	char ch; {

	if(ch == '\n' && filenum > STDPRN && putc('\r', filenum) == -1)
		return -1;
	if (write(filenum,&ch,1) != 1) {
		_flag[filenum] |= F_ERR;
		return -1;
		}
	return ch;
	}

putw(wrd,filenum)
	int  filenum;
	int  wrd; {

	if (write(filenum,&wrd,2) != 2) {
		_flag[filenum] |= F_ERR;
		return -1;
		}
	return wrd;
	}



ungetc(c,filenum)
	int c;
	int filenum; {

	_ungotten[filenum]=c;
	return c;
	}



/*	FGETS  --	INPUT A LINE FROM A FILE	*/

char *fgets(str,n,filenum)
	char *str;
	int  n,filenum; {
	int  ch;
	char *oldstr;

	oldstr=str;
	while (--n && (ch=getc(filenum)) >= 0 && ch != CONTZ) {
		if (ch != CR) *str++=ch;
		if (ch == '\n') break;
		}
	*str=0;
	if (oldstr == str) oldstr=0;
	return oldstr;
	}



/*	FPUTS  --	WRITE A STRING TO A FILE	*/

fputs(str,filenum)
	char *str;
	int  filenum; {
	int  err=0;
	char ch;

	while ((ch=*str++) && err != -1)
		err=putc(ch,filenum);
	if(err == -1)
		_flag[filenum] |= F_ERR;
	else
		err=0;
	return err;
	}


/*	UNLINK			error code=unlink(file)		*/

unlink(name)
	char *name; {

	return _unlink(name);
	}


/*	remove			error code=remove(file)		*/

remove(name)
	char *name; {

	return _unlink(name);
	}


/*	RENAME			error code = rename(old name, new name);	*/

rename(old,new)
	char *old,*new; {

	return _rename(old,new);
	}



/*	LSEEK			error code = lseek(file number, fileoff, code);
					0=seek from start.
					1=seek from current.
					2=seek from end.			*/

long lseek(filenum,fileoff,code)
	int  filenum;
	long fileoff;
	char code; {

	_oldfile=99;
	fflush(filenum);
			/*	adjust if buffered and doing relative seek	*/
	if (_buffer_is[filenum] == BUF_IN) {
		if(code == 1)
			fileoff+=((int)_buffer_at[filenum][0])-BUF_MAX-1;
		_buffer_at[filenum][0]=BUF_MAX+1;
		}
	filenum=_files[filenum];
	_ungotten[filenum] = -1;
	return _lseek(filenum,fileoff,code);
	}


/*	CLOSEALL  --	close all files.	*/

closeall() {
	int i;
	for (i=5; i < NFILE; i++)
		close(i);
	return 0;
	}


/*	DUP		--	duplicate a file handle	*/
/*;    synopsis:   int dup(file);
;                int dup2(file1,file2);
;                int file,file1,file2;	*/

static int rfile;

dup(file)
	int file; {
	int fnum;

	rfile=_files[file];
#asm
	mov	bx,word rfile_	;old handle
	mov	ah,45h
	int	21h				;Do the DOS Call to duplicae
	jnc	dup_ok
	mov	ax,-1			;failure
dup_ok:
	mov	word rfile_,ax
#
	if (rfile == -1 || (fnum=_newfile()) == -1) return -1;
	_files[fnum]=rfile;
	return fnum;
	}

	
	
/*	DUP2		--	duplicate a file handle	*/

/* ;    synopsis:   int dup(file);
   ;                int dup2(file1,file2);
   ;                int file,file1,file2; */

dup2(file,tfile2)
	int file,tfile2; {
	int fnum;

	if (_files[tfile2] != -1) return -1;

	rfile=_files[file];
#asm
	mov	bx,word rfile_	;old handle
	mov	ah,45h
	int	21h				;Do the DOS Call to duplicae
	jnc	dup2_ok
	mov	ax,-1			;failure
dup2_ok:
	mov	word rfile_,ax
#
	if (rfile == -1) return -1;
	_files[tfile2]=rfile;
	return 0;
	}

	
/*		_newfile allocates a unused file number. */

_newfile() {
	int  i;

	for (i=STDPRN+1; i < NFILE; i++) {
		if (_files[i] == -1) return i;
		}
	return -1;
	}



/*		_fixname removes the colon from a device name.	*/

_fixname(newname,name)
	char *newname,*name; {

	do {
		*newname++=*name;
		}
	while (*name++);
	if (*(newname-2) == ':') *(newname-2)=0; /* chop off the trailing colon */
	}


/*	fflush  --	flush a buffer for a file.	*/

fflush(filenum)
	int  filenum; {
	int  err,nout;
	char *buf;

	err=0;
	if (_buffer_is[filenum]) {
		buf=_buffer_at[filenum];
		nout=buf[0]-1;
		if (_buffer_is[filenum] == BUF_OUT) {
			_buffer_is[filenum]=BUF_IN;
			buf[0]=BUF_MAX+1;
			if (nout) {
				filenum=_files[filenum];
				err=_write(filenum,&buf[1],nout);
				if (err != -1) err=0;
				}
			}
		}
	return err;
	}

		

/*	alloc_buffer  --	allocate a buffer if a disk file.	*/

alloc_buffer(filenum,msnum)
	int  filenum,msnum; {
	int  i;
	
	_buffer_is[filenum]=BUF_UNUSED;		/* no buffer for this file */

	if ((dev_type(msnum) & 0x80) == 0) {	/* have a disk file */

		/*	see if have a free buffer	*/
		for (i=0; i < NBUFFER; i++) {
			if (buffer[i][0] == 0) {
				buffer[i][0]=BUF_MAX+1;
				_buffer_is[filenum]=BUF_IN;
				_buffer_at[filenum]=&buffer[i][0];
				return;
				}
			}
		}
	}




/*	dev_type  --  return the device type for a file.	*/

dev_type(filenum)
	int  filenum; {

#asm
	mov		ah,44h				;code for ioctl
	mov		al,0				;code for return info
	if		LARGE_CASE
	mov		bx,[bp+6]
	else
	mov		bx,[bp+4]			;file number
	endif
	int		21h					;call ms-dos
	mov		ax,dx				;device info
#
	}
