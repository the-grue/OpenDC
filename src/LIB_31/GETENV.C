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
/*  getenv.C	*/

/*
	This file contains the routine for searching the environment
	area for a given string.

	Interface:

		getenv(search_str, buffer)

		where search_str is the actual string being searched for,
			  buffer is the location to store the string if found,
	    NULL is returned if the environment variable isn't found,
        otherwise the address of the buffer is returned.
*/

extern unsigned environ;	/* c88 variable with the environment segment */

char * getenv(char *search_str, char buffer[]) {
	int envoff,searchoff;
	char ch, *beg=buffer;

	envoff=0;	
	buffer[0]=0;
	while (ch=_peek(envoff++,environ)) {
		searchoff=0;
		while (ch == search_str[searchoff++]) ch=_peek(envoff++,environ);
		if (ch == '=' && search_str[searchoff-1] == 0) {
			do {
				*buffer++=ch=_peek(envoff++,environ);
				}
			while (ch);
			return beg;
			}
		while (ch=_peek(envoff++,environ));
		}
	return (char *)0;
	}
