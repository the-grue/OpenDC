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
	block.c

	(c) Copyright 1986-1989 Michael Ouye, All Rights Reserved

	This file contains the routines for handling the block operations
	for the SEE editor.  There are basically three operations,

	shuffle(direction, blocks): moves blocks in the given direction until
		the indicated number of blocks have been moved.

	makeroom(direction,blocks): makes room in the buffer for the
		indicated number of blocks, if possible, returns the number
		of blocks freed.

	forcetag(tag): insures that the location pointed to by the tag
		is in memory.

    direction is 0 for the end file, 1 for the forward file.
*/

#include "world.h"

/* forward declarations */
int makeroom();

char *spillmsg = "*** spill disk full ***, change to another drive (Set-Spill command)";

int putblocks(direction, blocks)
	int direction; unsigned blocks; {
	int howmany, i;
	unsigned size, startblocks;
	char *ptr;

	if (blocks == 0) return 0;

	if (direction) {
		/* store blocks in the end */
		/* put text from the end of the buffer out to end file */
		ptr = buf_end;
		howmany = blocks;
		startblocks = endblocks;
		while (howmany--) {
			ptr -= BLOCKSIZE;
			if (!Put_endblock(ptr)) {
				message(spillmsg);
				return -1;
			}
		}
		size = (unsigned)(ptr - current_char);
		_move(size, current_char, buf_end - size);
		for (i = 0; i <num_tags; i++) {
			if (block[i] == 0) {
				if (buf_ptr[i] > ptr && buf_ptr[i] < buf_end) {
					block[i] = -(startblocks + ((unsigned)(buf_end - buf_ptr[i]) / BLOCKSIZE) + 1);
					buf_ptr[i] -= (unsigned)(buf_end - (((buf_end - buf_ptr[i]) / BLOCKSIZE+1) * BLOCKSIZE));
				}
				else if (buf_ptr[i] >= current_char && buf_ptr[i] <= ptr) {
					buf_ptr[i] += buf_end - ptr;
				}
			}
		}
		current_char = buf_end - size;
		return blocks;
	}
	else {
		/* put text from buf_start to forefile */
		howmany = blocks;
		startblocks = foreblocks;
		ptr = buf_start;
		while (howmany--) {
			if (!Put_foreblock(ptr)) {
				message(spillmsg);
				return -1;
			}
			ptr += BLOCKSIZE;
		}

		/* fixup any tags that have been swapped out */
		for (i = 0; i < num_tags; i++) {
			if (block[i] == 0) {
				if (buf_ptr[i] >= buf_start && buf_ptr[i] < ptr) {
					block[i] = startblocks + ((unsigned)(buf_ptr[i] - buf_start) / BLOCKSIZE) + 1;
					/* change buf_ptr[i] to be a block offset */
					buf_ptr[i] = (char *)((buf_ptr[i] - buf_start) % BLOCKSIZE);
				}
				else if (buf_ptr[i] >= ptr && buf_ptr[i] <= free_start) {
					buf_ptr[i] -= ptr-buf_start;
				}
			}
		}
		if (free_start > ptr) {
			size = (unsigned)(free_start - ptr);
			_move(size, ptr, buf_start);
			free_start = buf_start + size;
		}
		else
			free_start = buf_start;
		return blocks;
	}
}

int getblocks(direction,blocks)
	int direction; unsigned blocks; {
	int i, file;
	unsigned count, request, actual;
	char *ptr;

	request = blocks;
	if (direction) {
		/* read from the forward file into freestart */
		if (blocks > foreblocks) blocks = foreblocks;
		if (blocks == 0) return 0;

		count = blocks;
		ptr = free_start + (blocks * BLOCKSIZE);

		while (count--) {
			ptr -= BLOCKSIZE;
			Get_foreblock(ptr);
		}
		/* fix up any buf_ptrs that have been read in */
		for (i = 0; i < num_tags; i++) {
			if (block[i]-1 >= foreblocks) {
				buf_ptr[i] += (unsigned)(free_start + ((block[i]-1-foreblocks)*BLOCKSIZE));
				block[i] = 0;
			}
		}
		free_start += (blocks * BLOCKSIZE);
		return blocks;
	}
	else {
		/* read from the end */
		if (blocks > endblocks) blocks = endblocks;
		if (blocks == 0 && srcblocks == 0) return 0;

		if (blocks != 0) {
			count = blocks;
			ptr = buf_end - (blocks * BLOCKSIZE);
			while (count--) {
				Get_endblock(ptr);
				ptr += BLOCKSIZE;
			}
			/* fixup the new tags */
			for (i = 0; i < num_tags; i++) {
				if (-block[i]-1 >= endblocks) {
					buf_ptr[i] += (unsigned)(ptr - (-(block[i] + endblocks) * BLOCKSIZE));
					block[i] = 0;
				}
			}
			current_char = buf_end - (blocks * BLOCKSIZE);
		}
		if (endblocks == 0 && srcblocks > 0 && blocks != request) {
			request -= blocks;
			buf_ptr[TEMP6] = current_char; block[TEMP6] = 0;
			rewindow(buf_end);
			file = open(in_file, 0);
			lseek(file, srcloc, 0);
			request *= BLOCKSIZE;
			actual = read(file, buf_end - request, request);
			close(file);
			srcloc += actual;
			srcblocks -= (request / BLOCKSIZE);
			if (srcblocks <= 0) {
				srcblocks = 0;
				srcadjust = 0;
			}
			if (actual != request)
				_move(actual, buf_end-request, buf_end - actual);
			current_char = buf_end - actual;
			blocks += actual / BLOCKSIZE;
			if (actual % BLOCKSIZE != 0) blocks++;
			if (buf_ptr[TEMP6] != buf_end)
				rewindow(buf_ptr[TEMP6]);
		}
		return blocks;
	}
}

int shuffle(direction,blocks)
	int direction; unsigned blocks; {
	int mvblocks,startblocks;

	startblocks = blocks;
	while (blocks > 0) {
		if (direction)
			rewindow(buf_start);	/* put all text in second buffer */
		else
			rewindow(buf_end);

		mvblocks = makeroom(direction,blocks);
		if (mvblocks ==  -1) return -1;
		/* now, read the data from the file */
		if (getblocks(direction, mvblocks) != mvblocks) break;
		blocks -= mvblocks;
	}
	if (blocks == startblocks) return -1;
	return 0;
}

int makeroom(direction,blocks)
	int direction; int blocks; {
	unsigned mvblocks, freeblocks, canblocks;
	unsigned doblocks, buf_size();

	freeblocks = free_space() / BLOCKSIZE;
	if (blocks < 0) {
		/* special case, request to spill from only one side of the buffer */
		if (direction) {
			mvblocks = (buf_end - current_char) / BLOCKSIZE;
		}
		else {
			mvblocks = (free_start - buf_start) / BLOCKSIZE;
		}
		blocks = -blocks;
		if (blocks > mvblocks+freeblocks) blocks = mvblocks+freeblocks;
	}

	if (blocks == 0) return 0;
	if (direction)
		rewindow(buf_start);
	else
		rewindow(buf_end);

	mvblocks = (unsigned)(buf_end - buf_start) / BLOCKSIZE; /* max blocks */
	if (mvblocks > blocks) mvblocks = blocks;

	/* max blocks that can be freed */
	canblocks = buf_size() / BLOCKSIZE;

	if (freeblocks >= mvblocks)
		/* already enough room */
		return mvblocks;

	doblocks = mvblocks - freeblocks;
	if (doblocks > canblocks)
		doblocks = canblocks;
	if (putblocks(direction, doblocks) == -1) return -1;
	
	return freeblocks + doblocks;
}

int forcetag(tag)
	int tag; {
	int blocks;

	if (block[tag] == 0) return 0;

	if (block[tag] < 0) {
		/* in the end file */
		blocks = -block[tag]-1;
		return shuffle(0, endblocks - blocks);
	}
	else {
		/* in the fore file */
		blocks = block[tag]-1;
		return shuffle(1, foreblocks - blocks);
	}
}

