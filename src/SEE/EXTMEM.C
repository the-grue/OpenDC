/*
 *  Released under the GNU GPL.  See http://www.gnu.org/licenses/gpl.txt
 *
 *  This program is part of the SEE editor
 *
 *  SEE is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundatation; either version 2 of the License, or any
 *  later version.
 *
 *  SEE is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 */
/*
	extmem.c  
 
	(c)Copyright 1989, Michael Ouye, All Rights Reserved
 
	This file contains the routines for managing the storage of memory
	blocks beyond the SEE editors normal buffer.  It will utilize
	memory until exhausted and then will switch to a disk file.

	This version just uses the extra memory in DOS's 640KB space. 
	Future versions can make use of extended and/or exapanded memory.

	Buffers will be allocated with a 32-bit "pointer" back to the
	previous buffer.  Two lists will be maintained, a forward list for
	the the blocks prior to the cursor and a reverse list for blocks
	following the cursor.  The first block in the file is reserved for
	saving the buffer specific data.
 
*/

#include "world.h"
#include "extern.h"

static struct {
	long next;
	char buffer[BLOCKSIZE];
} copybuf;

int Create_spillfile(buffer)
	buff_struct *buffer; {
	char *ptr;
	int i;

	if (buffer->bs_spillfile != 0) return 1;

	ptr = buffer->bs_spillfilename;
	if (*buffer->bs_spillfilename == '$') {
		if (options.spill != 0) *buffer->bs_spillfilename = (char)options.spill;
		else ptr += 2;
	}
	for (i = 0; i < 30; i++) {
		buffer->bs_spillfile = open(ptr, 0);
		if (buffer->bs_spillfile <= 0) {
			buffer->bs_spillfile = creat(ptr);
			if (buffer->bs_spillfile > 0) {
				buffer->bs_spilloffset = 0L;
				buffer->bs_spillfree = 0xFFFFFFFFL;
				return 1;
			}
		}
		else
			close(buffer->bs_spillfile);
		*(buffer->bs_spillfilename + strlen(buffer->bs_spillfilename) - 1) += 1;
	}
	return 0;
}

long Write_block(buffer, ptr, offset, memlist)
	buff_struct *buffer; char *ptr; long offset; farptr *memlist; {
	long newloc;
	farptr lastblock;

	if (*memlist != 0) {
		/* the new block always stays in memory */
		lastblock = ExtractLast_block(*memlist);
		if (lastblock == *memlist) *memlist = 0;
		CopyNear_block(copybuf.buffer, lastblock);
		if (ptr != 0) {
			CopyFar_block(lastblock, ptr, *memlist);
			*memlist = lastblock;
		}
		else
			Free_block(lastblock);
	}
	else {
		_move(BLOCKSIZE, ptr, copybuf.buffer);
	}

	/* write the copy buffer to the file */
	if (buffer->bs_spillfree == 0xFFFFFFFFL) {
		newloc = buffer->bs_spilloffset;
		buffer->bs_spilloffset += sizeof(copybuf);
	}
	else {
		newloc = buffer->bs_spillfree;
		lseek(buffer->bs_spillfile, newloc, 0);
		read(buffer->bs_spillfile, &buffer->bs_spillfree, sizeof(long));
	}
	lseek(buffer->bs_spillfile, newloc, 0);
	copybuf.next = offset;
	write(buffer->bs_spillfile, &copybuf, sizeof(copybuf));
	return newloc;
}

long Read_block(buffer, ptr, offset)
	buff_struct *buffer; char *ptr; long offset; {

	lseek(buffer->bs_spillfile, offset, 0);
	read(buffer->bs_spillfile, &copybuf, sizeof(copybuf));
	_move(BLOCKSIZE, copybuf.buffer, ptr);
	lseek(buffer->bs_spillfile, offset, 0);
	write(buffer->bs_spillfile, &buffer->bs_spillfree, sizeof(long));
	buffer->bs_spillfree = offset;
	return copybuf.next;
}

/* copy the next forward buffer into the near memory (ptr) */
int Get_foreblock(ptr)
	char *ptr; {
	farptr this;

	if (foreblocks == 0) return 0;

	--foreblocks;
	if (foremem != 0) {
		/* from memory */
		this = foremem;
		foremem = Next_block(foremem);
		CopyNear_block(ptr, this);
		Free_block(this);
	}
	else {
		/* from the file */
		foreoffset = Read_block(cur_buf, ptr, foreoffset);
	}
	return (1);
}

/* copy the near memory block (ptr) into the next forward buffer */
int Put_foreblock(ptr)
	char *ptr; {
	farptr this;

	this = New_block();
	if (this != 0) {
		CopyFar_block(this, ptr, foremem);
		foremem = this;
	}
	else {
		/* write to the file */
		if (Create_spillfile(cur_buf)) {
			foreoffset = Write_block(cur_buf, ptr, foreoffset, &foremem); 
		}
	}
	++foreblocks;
	return (1);
}

/* copy the next end buffer into the near memory (ptr) */
int Get_endblock(ptr)
	char *ptr; {
	farptr this;

	if (endblocks == 0) return 0;

	--endblocks;
	if (endmem != 0) {
		/* from memory */
		this = endmem;
		endmem = Next_block(endmem);
		CopyNear_block(ptr, this);
		Free_block(this);
	}
	else {
		/* from the file */
		endoffset = Read_block(cur_buf, ptr, endoffset);
	}
	return (1);
}

/* copy the near memory block (ptr) into the next end buffer */
int Put_endblock(ptr)
	char *ptr; {
	farptr this;

	this = New_block();
	if (this != 0) {
		CopyFar_block(this, ptr, endmem);
		endmem = this;
	}
	else {
		/* write to the file */
		if (Create_spillfile(cur_buf)) {
			endoffset = Write_block(cur_buf, ptr, endoffset, &endmem); 
		}
	}
	++endblocks;
	return (1);
}

void extmem_clear(buffer)
	buff_struct *buffer; {
	char *ptr;
	long fptr, fnext;

	if (buffer->bs_spillfile != 0) {
		close(buffer->bs_spillfile);
		ptr = buffer->bs_spillfilename;
		if (buffer->bs_spillfilename[0] == '$') ptr += 2;
		unlink(ptr);
	}
	fptr = buffer->bs_endmem;
	while (fptr != 0) {
		fnext = Next_block(fptr);
		Free_block(fptr);
		fptr = fnext;
	}
	fptr = buffer->bs_foremem;
	while (fptr != 0) {
		fnext = Next_block(fptr);
		Free_block(fptr);
		fptr = fnext;
	}
}

/* flush the far spill buffers to disk */
int extmem_flush(buffer)
	buff_struct *buffer; {

	if (buffer->bs_foremem == 0 && buffer->bs_endmem == 0) return 1;

	if (Create_spillfile(buffer)) {
		while (buffer->bs_foremem != 0) {
			buffer->bs_foreoffset = Write_block(buffer, 0, 
												buffer->bs_foreoffset, 
												&buffer->bs_foremem); 
		}
	
		while (buffer->bs_endmem != 0) {
			buffer->bs_endoffset = Write_block(buffer, 0, 
												buffer->bs_endoffset, 
												&buffer->bs_endmem); 
		}
		return (1);
	}
	return (0);
}

void extmem_setup(void) {
	exec("","");	/* releases the rest of memory */
}

/*
/* movefile:  move a spill file from one drive to another.
*/
int movefile(fileptr, name, drive)
	int *fileptr; char *name; char drive; {
	char *ptr, namebuf[65];
	int tarfile, actual;

	/* emergency situation, delete the copy buffer */
	copy_size = 0;
	if (*copyfilename == '$') unlink(copyfilename+2);
	else unlink(copyfilename);

	strcpy(namebuf, name);
	ptr = namebuf;
	if (drive == '0') {
		ptr = namebuf+2;
		*namebuf = '$';
	}
	else *ptr = drive;

	if (*fileptr != 0) {
		tarfile = creat(ptr);
		if (tarfile < 0) return -1;
		lseek(*fileptr, 0L, 0);	/* beginning of the file */

		while (1) {
			actual = read(*fileptr, copy_buffer, sizeof(copy_buffer));
			if (actual == 0) break;
			if (write(tarfile, copy_buffer, actual) == -1) {
				close(tarfile);
				return -1;
			}
		}
		close(*fileptr);
		if (*name == '$') unlink(name+2);
		else unlink(name);
		*fileptr = tarfile;
	}
	strcpy(name, namebuf);
	return 0;
}

/* change the spillfile drive */
void extmem_spilldrive(char drive) {
	int i;

	if (drive == options.spill) return;

	for (i = 0; i < max_buffers; i++) {
		if (buffers[i].bs_spillfile != 0) {
			movefile(&buffers[i].bs_spillfile, 
						buffers[i].bs_spillfilename, 
						drive);
		}
	}
	*copyfilename = '$';
}
