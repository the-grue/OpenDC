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
     buffer.c

	 (c) Copyright 1983-1989 Michael Ouye, All Rights Reserved.

	 This module contains the routines for manipulating the text buffer.
	 
	 The text buffer looks as follows:

					+-----------------------+	 <== buf_end
					|						|
					|						|
					|						|
					+-----------------------+	 <== current_char
					|						|
					|						|
					+-----------------------+	 <== free_start
					|						|
					|						|
					|						|
					+-----------------------+	 <== buf_start


	The extended memory is implemented as two separate 'files', a forward
	file and an end file.  The forward file contains everything ahead of
	the buffer contents and the end file contains everything that has
	been read in and trails the buffer.  The source file may have more
	text which is after the end file.  Once the entire source file has
	been read in, it is no longer touched.

	The extended files contain 4KB blocks of text.  The blocks are numbered
	from 1 to n.  Block number 0 implies empty extensions.

	 Global variables:

		 buf_end:	points at the last character in the upper buffer, always
									 the EOF character.
		 current_char: points at the first char in the lower buffer.
		 free_start: points at the first free byte of memory.
		 buf_start:	points at the first character in the lower buffer.

		 foreblock:	block number in the foward extension
		 endblock:	block number in the end extension
		 srcblocks:	blocks left in the source file
		 srcloc:	location within the source file

	 Procedures

		 init_buffer:	initializes the buffer pointer to an empty buffer.
		 free_space:	returns the amount of free buffer area.
		 read_file:		inputs the contents of a file at the current position
							 as delineated by free_start and current_char
		 write_file:	writes out the marked area to a disk file
*/

#include "world.h"
#include "extern.h"

/* forward declarations */
void rewindow();
int  sorttag();
int makespace();

void start_buffer(bufptr, start, end)
	buff_struct *bufptr; char *start, *end; {
	/* reinitialize the buffer variables */
	int i;

	bufptr->bs_in_use = 0;
	bufptr->bs_buf_start = start;
	bufptr->bs_buf_end = end;
	bufptr->bs_free_start = start;
	bufptr->bs_current_char = end;

	for( i = 0; i < num_tags; i ++ ) {
		bufptr->bs_buf_ptr[i] = bufptr->bs_current_char;
		bufptr->bs_block[i] = 0;
	}
	bufptr->bs_buf_ptr[TEMPA] = end;
	bufptr->bs_block[TEMPA] = 0;

	bufptr->bs_spillfile = 0;
	bufptr->bs_spilladjust = 0;
	strcpy(bufptr->bs_spillfilename,"$:seetmp.00A");
	bufptr->bs_srcloc = 0;
	bufptr->bs_srcblocks = 0;
	bufptr->bs_srcadjust = 0;
	bufptr->bs_foreblocks = 0;
	bufptr->bs_endblocks = 0;
	bufptr->bs_foremem = 0;
	bufptr->bs_endmem = 0;
	bufptr->bs_spillfile = 0;
	bufptr->bs_altered = 0;
}

unsigned free_space(void) { /* returns the number of free area bytes in buffer */
	return (unsigned)(current_char - free_start);
}

unsigned buf_size(void) { /* the number of bytes used in the buffer */
	return ((unsigned)(free_start - buf_start) + 
	            (unsigned)(buf_end - current_char));
}

long file_size(void) {	/* the number of bytes used in the file */
	return ((long)(endblocks+srcblocks) * BLOCKSIZE) + 
			((long)foreblocks * BLOCKSIZE) + buf_size() + srcadjust;
}

/*
/* read_file(file_ptr, offset, all)
/*
/*  read the file, filename given in file_ptr, starting at offset.
/*
/*  Returns -1 if an error occurred, 0 if everything is OK, and
/*  1 if we ran out of room ( disk full condition ).
/*
/*  all is zero if this is the source file.
*/

int read_file(file_ptr, offset, all) /* returns -1 if an error occured,
											1 if file is too large,
											0 if everything is OK */
	char *file_ptr; long int offset; int all; {
	int file,status,blocks;
	unsigned attempt, actual;
	long filelen, lseek();

	buf_ptr[TEMP5] = buf_ptr[TEMPA]; block[TEMP5] = block[TEMPA];
	buf_ptr[TEMP1] = current_char; block[TEMP1] = 0;
	if (offset == -1L) return (0);
	file = open(file_ptr, 3); /* read mode */
	if (file == -1) {
		if (!all) {
			srcblocks = 0;
			srcadjust = 0;
			srcloc = 0;
		}
		return -1;
	}
	filelen = lseek(file, 0L, 2);
	status = (int)lseek(file, offset, 0);
	filelen -= offset;
	attempt = free_space();
	/* align read request to a BLOCKSIZE boundry */
	attempt = attempt - (attempt % BLOCKSIZE);
	actual = read(file, free_start, attempt);
	buf_ptr[TEMPA] = free_start;
	block[TEMPA] = 0;
	if (actual != -1)
		free_start += actual;

	if (all) {
		if (actual == attempt) {
			filelen -= actual;
			blocks = (int)((filelen / BLOCKSIZE) + 1);
			makeroom(1,-blocks);
			while (filelen > 0) {
				attempt = makeroom(0,-((int)(filelen / BLOCKSIZE)+1));
				if (attempt == -1) {
					actual = -1;		/* disk full*/
					break;
				}
				rewindow(buf_ptr[TEMP1]);
				attempt *= BLOCKSIZE;
				actual = read(file, free_start, attempt);
				if (actual == -1 || actual == 0) break;
				filelen -= actual;
				free_start += actual;
			}
		}
	}
	else {
		/* just set up the global source location */
		if (actual == attempt) {
			srcloc = attempt;
			srcblocks = (int)(filelen / BLOCKSIZE);
			srcadjust = (int)(filelen % BLOCKSIZE);
			if (srcadjust != 0) {
				srcblocks++;
				srcadjust = srcadjust - BLOCKSIZE; /* negative value */
			}
			srcblocks -= attempt / BLOCKSIZE;
		}
		else {
			srcblocks = 0;
			srcadjust = 0;
			srcloc = 0;
		}
	}
	close(file);
	if ( (actual == -1) || (status == -1) ) return -1;
	forcetag(0);
	rewindow(buf_ptr[TEMPA]);
	buf_ptr[TEMPA] = buf_ptr[TEMP5]; block[TEMPA] = block[TEMP5];
	return 0;
}
	
int write_file(file_ptr, offset, start_tag, end_tag) /* returns 0 if OK,
													 -1 if file error */
	char *file_ptr[]; long offset;
	int start_tag, end_tag; {
	int file, temp,firsttag;
	int status;
	char *endptr;
	static char controlZ = 0x1a;

	if (offset == 0L)
		file = creat(file_ptr);
	else 
		file = open(file_ptr, 2);
	if (file == -1) return -1;
	if (offset != 0L)
		lseek(file, 0L, 2);		/* seek to the correct position */

	firsttag = sorttag(start_tag,end_tag);

	if (firsttag != start_tag) {
		/* swap the tag values */
		temp = start_tag;
		start_tag = end_tag;
		end_tag = temp;
	}

	if (block[start_tag] == 0 && block[end_tag] == 0) { /* all in memory */
		rewindow(buf_ptr[end_tag]+1);
		if (buf_ptr[end_tag] != buf_ptr[start_tag]) 
			status = write(file,buf_ptr[start_tag],(unsigned)(free_start-buf_ptr[start_tag]));
		else status = (int)(free_start - buf_ptr[start_tag]);
		if (options.contZ)
			write(file, &controlZ, 1);
		close(file);
		if (status != (free_start - buf_ptr[start_tag])) return -1;
	}
	else {	/* have to write out the block in pieces */
		forcetag(start_tag);
		rewindow(buf_ptr[start_tag]);
		while (1) {
			makeroom(0,-32000);			/* get as much room as possible */
			shuffle(0, free_space() / BLOCKSIZE);
			if (block[end_tag] != 0)
				endptr = buf_end;
			else
				endptr = buf_ptr[end_tag] + 1;
			status = write(file,current_char, (unsigned)(endptr-current_char));
			if (status == -1) break;
			if (block[end_tag] == 0) break; /* all done */
			rewindow(buf_end);
		}
		if (options.contZ)
			write(file, &controlZ, 1);
		close(file);
		if (status == -1) return -1;
	}
	return 0;
}

/*
/* rewindow: main buffer management routine.  Rewindow keeps the buf_ptr
/*		pointer array up to date as the buffer is manipulated.
*/
void rewindow(new_end)
	char *new_end; {
	/* move the free area window with current_char set to new_end */
	char **bptr;
	int *blkptr;
	unsigned len;

	if (new_end < buf_start) new_end = buf_start;
	else if (new_end > buf_end) new_end = buf_end;

	if (new_end == 0) new_end = current_char;
	if (new_end < free_start) { /* first buffer */
		/* check for changes to the pointer vector */
		for (bptr = buf_ptr, blkptr = block; bptr < &buf_ptr[num_tags]; bptr++,blkptr++) {
			if (*bptr != 0 && *blkptr == 0) {
				if ( (*bptr >= new_end) && (*bptr <= free_start) )
					*bptr = current_char - (free_start - *bptr);
				else if ( (*bptr >= free_start) && (*bptr < current_char) )
					/* *bptr = 0; */ *bptr = free_start;
			}
			else if (*blkptr != 0) {
				if (*blkptr < 0) {
					if (-*blkptr-1 > endblocks) {
						*blkptr = 0;
						*bptr = 0;
					}
				}
				else {
					if (*blkptr-1 > foreblocks) {
						*blkptr = 0;
						*bptr = 0;
					}
				}
			}
		}
		len = (unsigned)(free_start-new_end);
		_move(len, new_end, current_char - len);
		current_char -= len;
		free_start = new_end;
	}
	else if (new_end > current_char) { /* second buffer */
		for (bptr = buf_ptr, blkptr = block; bptr < &buf_ptr[num_tags]; bptr++,blkptr++) {
			if (*bptr != 0 && *blkptr == 0) {
				if ( (*bptr >= current_char) && (*bptr < new_end) )
					*bptr = free_start + (*bptr - current_char);
				else if ( (*bptr >= free_start) && (*bptr < current_char) )
					/* *bptr = 0; */ *bptr = free_start;
			}
			else if (*blkptr != 0) {
				if (*blkptr < 0) {
					if (-*blkptr-1 > endblocks) {
						*blkptr = 0;
						*bptr = 0;
					}
				}
				else {
					if (*blkptr-1 > foreblocks) {
						*blkptr = 0;
						*bptr = 0;
					}
				}
			}
		}
		_move((int)(new_end-current_char),current_char, free_start);
		free_start += (new_end-current_char);
		current_char = new_end;
	}
	else {
		for (bptr = buf_ptr,blkptr = block; bptr < &buf_ptr[num_tags]; bptr++,blkptr++) {
			if (*blkptr == 0 && (*bptr >= free_start && *bptr < current_char)) 
				/* *bptr = 0; */ *bptr = free_start;
			else if (*blkptr != 0) {
				if (*blkptr < 0) {
					if (-*blkptr-1 > endblocks) {
						*blkptr = 0;
						*bptr = 0;
					}
				}
				else {
					if (*blkptr-1 > foreblocks) {
						*blkptr = 0;
						*bptr = 0;
					}
				}
			}
		}
	}
}
	

/* the following routines maintain the copy buffer */

/*
/* cb_bufin: move the data into the copy buffer, if the data is larger than
/*	the buffer, it is written out to the copy file.
*/
int cb_bufin(start_tag, end_tag) /* copy data to the copy buffer */
	int start_tag, end_tag; {
	int temp;
	long newsize;
	char *ptr;

	if (sorttag(start_tag,end_tag) != start_tag) {
		temp = start_tag;
		start_tag = end_tag;
		end_tag = temp;
	}

	forcetag(start_tag);
	rewindow(buf_ptr[start_tag]);
	if (block[end_tag] == 0 && (newsize = (buf_ptr[end_tag]-buf_ptr[start_tag])+1) <= sizeof(copy_buffer)) {
		/* fits in the memory buffer */
		if (copy_size > sizeof(copy_buffer)) unlink(copyfilename);
		copy_size = newsize;
		_move((int)copy_size, buf_ptr[start_tag], copy_buffer);
		return 0;
	}
	copy_size = sizeof(copy_buffer) +1;
	/* otherwise, write it out to the temp file  */
	ptr = copyfilename;
	if (*ptr == '$') {
		if (options.spill != 0) *ptr = (char)options.spill;
		else ptr += 2;
	}
	if (write_file(ptr, 0L, start_tag, end_tag) == -1) return 1;
	copyfile = 1;
	return(0);
}

int cb_bufout(void) {	/* copy data out of the buffer */
	char *ptr;

	if (copy_size == 0) return 0;
	if (copy_size <= sizeof(copy_buffer)) {
		if (makespace((int)copy_size) == -1) return -1;
		_move((int)copy_size, copy_buffer, free_start);
		free_start += copy_size;
	}
	else {
		ptr = copyfilename;
		if (options.spill != 0) *ptr = (char)options.spill;
		else ptr += 2;
		if (read_file(ptr, 0L, 1) != 0) return -1;
	}
	return 0;
}
	
/*
/* sorttag: tag number which points to the lowest location in the file.
*/
int sorttag(tag1,tag2)
	int tag1, tag2; {
	int blk1, blk2;

	blk1 = block[tag1];
	blk2 = block[tag2];
	if (blk1 == 0 && blk2 == 0)
		return (buf_ptr[tag1] <= buf_ptr[tag2]) ? tag1: tag2;
	if (blk1 == 0)
		return (blk2 > 0) ? tag2: tag1;
	if (blk2 == 0)
		return (blk1 > 0) ? tag1: tag2;

	/* both swapped out */
	if (blk1 == blk2)
		return (buf_ptr[tag1] <= buf_ptr[tag2]) ? tag1: tag2;
	if (blk1 > 0 && blk2 > 0)	/* both in fore file */
		return (blk1 > blk2) ? tag2: tag1;
	if (blk1 < 0 && blk2 < 0)	/* both in end file */
		return (blk1 < blk2) ? tag1: tag2;
	
	return (blk1 > 0) ? tag1: tag2;
}

/* insure space is available, assumes bytes < buf_end - buf_start */
int makespace(bytes)
	unsigned bytes; {
	int blocks,blks;

	blocks = (bytes / BLOCKSIZE) + 1;

	buf_ptr[TEMP5] = current_char; block[TEMP5] = 0;
	if (current_char - free_start < bytes) {
		blks = makeroom(0,-blocks);
		if (blks == -1) return -1;
		blocks -= blks;
		if (blocks > 0) {
			rewindow(buf_ptr[TEMP5]);
			blks = makeroom(1,-(blocks+(free_space() / BLOCKSIZE)));
			if (blks == -1) return -1;
		}
	}
	rewindow(buf_ptr[TEMP5]);
	return 0;
}

/* buffer switch routines */

int switch_buffer(which)
	int which; {

	if (which == active_buffer) return 0;
	if (which >= max_buffers) return -1;
	if (!buffers[which].bs_in_use) return -1;

	flush_buffer();

	cur_buf = &buffers[which];
	active_buffer = which;

	fill_buffer();

	return 0;
}

/* returns an empty buffer index */
int new_buffer(void) {
	int i;

	for (i = 0; i < max_files; i++)
		if (buffers[i].bs_in_use == 0) {
			buffers[i].bs_in_use = 1;
			return i;
		}
	return (-1);
}

/* returns the next used buffer index */
int next_buffer(void) {
	int i, start;

	for (i = (active_buffer+1) % max_files; i != active_buffer; i = (i+1) % max_files )
		if (buffers[i].bs_in_use == 1) {
			return i;
		}
	return (-1);
}

/* returns the number of active buffers */
int buffer_count(void) {
	int i, count;

	for (i = 0, count = 0; i < max_files; i++)
		if (buffers[i].bs_in_use == 1) count++;

	return (count);
}

/* returns the last used buffer index */
int last_buffer(void) {
	int i, start;

	for (i = 0; i < max_files; i++) {
		if (buffers[i].bs_in_use == 0) {
			return (i == 0) ? 0: i-1;
		}
	}
	return (max_files);
}

/* clear out the memory associated with the buffers */
void clear_buffers(void) {
	int i;
	char *cptr;

	for (i = 0; i < max_buffers; i++) {
		extmem_clear(&buffers[i]);
	}
	if (copyfile) {
		cptr = copyfilename;
		if (*cptr == '$') cptr += 2;
		unlink(cptr);
	}
}

/* release the memory associated with the buffers */
int spill_buffers(void) {
	int i;
	char *cptr;

	for (i = 0; i < max_buffers; i++) {
		if (!extmem_flush(&buffers[i]))
			return 0;
	}
	return 1;
}

/* initialize the buffer table */
void init_buffer(void) {
	int i;
    extern char *_MEMORY(), *_SHOWSP();
	char *start, *end;

	/*** this section of code allocates the rest of DS to the edit buffer ***/
	start = _MEMORY(); /* pointer to the beginning of memory */
#if 0
	*(buf_start++) = 0; /*first character always a 0*/
#endif
    /* pointer to the end of memory */
	end = _SHOWSP() - 4000; /* allow for a 4000 byte stack */
	*end = eof;				/* last character always a EOF*/

	for (i = 0; i < max_buffers; i++) {
		start_buffer(&buffers[i], start, end);
	}
	cur_buf = &buffers[0];
	strcpy(copyfilename, "$:seetmp.000");

	buffers[FILE_BUFFER].bs_in_use = true;
}

/* move all of the buffer data out to the spill file */
int flush_buffer(void) {
	unsigned blocks;

	/* save the current location in TEMPA */
	buf_ptr[TEMPA] = current_char; block[TEMPA] = 0;
	rewindow(buf_end);	/* all chars to the forward buffer */
	if (free_start != buf_start) {
		spilladjust = buf_size() % BLOCKSIZE;
		blocks = (buf_size() / BLOCKSIZE);
		if (spilladjust) blocks++;
		putblocks(0, blocks);
	}
	else
		spilladjust = 0;
}

/* drag as much data as possible back in from the spill file */
int fill_buffer(void) {
	int blocks;

	blocks = (current_char - free_start) / BLOCKSIZE;
	getblocks(1, blocks);
	if (spilladjust)
		free_start -= (BLOCKSIZE - spilladjust);
	forcetag(TEMPA);
	rewindow(buf_ptr[TEMPA]);
}
