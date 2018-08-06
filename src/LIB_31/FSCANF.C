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
/*	FSCANF.C  -- FORMATTED FILE INPUT	*/

/*
	fscanf:
	Like scanf, except that the first argument is
	a pointer to a buffered input file buffer, and
	the text is taken from the file instead of from
	the console.
	Usage:
		fscanf(iobuf, format, ptr1, ptr2, ...);
	Returns number of items matched (zero on EOF.)
	Note that any unprocessed text is lost forever. Each
	time scanf is called, a new line of input is gotten
	from the file, and any information left over from
	the last call is wiped out. Thus, the text in the
	file must be arranged such that a single call to
	fscanf will always get all the required data on a
	line. This is not compatible with the way UNIX does
	things, but it eliminates the need for separate
	scanning functions for files, strings, and console
	input; it is more economical to let both "fscanf" and
	"scanf" use "sscanf". If you want to be able to scan
	a partial line with fscanf and have the rest still be
	there on the next fscanf call, you'll have to rewrite
	fscanf to be self contained (not use sscanf) and use
	"ungetc" to push back characters.

	Returns number of items succesfully matched.
*/

char *fgetc();

int fscanf(int *file, char *format) { return _doscan(fgetc, file, 1, &format); }
