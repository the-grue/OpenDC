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
	sprintf:
	Like fprintf, except a string pointer is specified
	instead of a buffer pointer. The text is written
	directly into memory where the string pointer points.

	Usage:
		sprintf(string,format,arg1, arg2, ...);
*/

static sputc(char ch, char **ptr) { return *(*ptr)++ = ch; }

sprintf(char * buf, char * fmt) { 
	int n;

	n = _doprint(sputc, &buf, &fmt);
	*buf = 0;
	return n;
	}
