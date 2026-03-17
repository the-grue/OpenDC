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
	sscanf:
	Like fscanf, except a string pointer is specified
	instead of a file pointer. The text is read
	directly from memory where the string pointer points.

	Usage:
		sscanf(string,format,arg1, arg2, ...);
*/

static sgetc(char **ptr) { return **ptr ? *(*ptr)++ : -1; }

sscanf(char * buf, char * fmt) { return _doscan(sgetc, &buf, 0, &fmt);	}
