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
/*	FSTUFF.C	Macro style I/O for C	*/

extern char _flag[];
extern int _files[];

long lseek();

#define	F_ERR	1
#define	F_EOF	2

/*	FOPEN		macro style of open	*/

char *fopen(name,mode)
	char *name,*mode; {
	int  fil, plus;

	plus = (mode[1] == '+' || (mode[1] && mode[2] == '+'));
	switch(*mode) {
		case 'r':	if(plus)
						return (fil=open(name,2)) == -1 ? 0: fil;
					else
						return (fil=open(name,0)) == -1 ? 0: fil;
		case 'w':	return (fil=creat(name)) == -1 ? 0: fil;
		case 'a':	if ((fil=open(name,2)) != -1) {
						lseek(fil,0l,2);
						return fil;
						}
					return (fil=creat(name)) == -1 ? 0: fil;
		default:	return 0;
		}
	}



/*	FCLOSE			error = fclose(file number); */

fclose(filenum)
	int  filenum; {

	return close(filenum);
	}


/*	FREAD		  number read = fread(ptr, size, nitems, file); */

fread(ptr,size,nitems,filenum)
	char *ptr;
	unsigned size,nitems,filenum; {
	unsigned read(), nin;

	nin=read(filenum,ptr,nitems*size);
	if(nin == -1) {
		_flag[filenum] |= F_ERR;
		return 0;
		}
	else if(nin == 0)
		 _flag[filenum] |= F_EOF;
	return nin/size;
	}



/*	FWRITE		number written = fwrite(ptr, size, nitems, file); */

fwrite(ptr,size,nitems,filenum)
	char *ptr;
	unsigned size,nitems,filenum; {
	unsigned write(), nin;

	nin=write(filenum,ptr,nitems*size);
	if(nin == -1 || nin < nitems*size) {
		_flag[filenum] |= F_ERR;
		return 0;
		}
	return nin/size;
	}



/*	FSEEK		error = fseek(file, offset, type of seek);	*/

fseek(filenum,offset,type)
	char *filenum;
	int  type;
	long offset; {

	if(lseek(((int)filenum),offset,type) == -1L) {
		_flag[(int)filenum] |= F_ERR;
		return -1;
		}
	return 0;
	}


/*	REWIND		error = rewind(file);	*/

rewind(filenum)
	int  filenum; {

	filenum = _files[filenum];
	_flag[filenum] &= ~F_EOF;
	if (lseek(filenum,0L,0)) {
		_flag[filenum] |= F_ERR;
		return -1;
		}
	return 0;
	}

/*	LTELL  --	return current file location.	*/

long ltell(filenum)
	int filenum; {

	return lseek(filenum,0L,1);
	}





/*	FGETC		character = fgetc(file);	*/

fgetc(filenum)
	int  filenum; {

	return  getc(filenum);
	}


/*	FPUTC		error = fputc(character, file);	*/

fputc(c,filenum)
	char c;
	int  filenum; {

	return putc(c,filenum);
	}
