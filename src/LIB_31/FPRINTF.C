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
/*
	fprintf:
	Like printf, except that the first argument is
	a pointer to a stream, and the text
	is written to the file described by the buffer:
	ERROR (-1) returned on error.

	usage:
		fprintf(iobuf, format, arg1, arg2, ...);
*/

int fprintf(int * file, char * format) {
	int fputc();

	return _doprint(fputc, file, &format);
	}
