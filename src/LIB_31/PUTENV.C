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
/*  putenv.C	*/

/*
	This file contains the routine for searching the environment
	area for a given string and replacing it.

	Interface:

		putenv(search_str, buffer)

		where search_str is the actual string being searched for,
			  buffer is the location of new string.
        -1 is returned if the environment variable isn't found,
        otherwise 0 is returned.
*/

extern unsigned environ;

int putenv(search_str, buffer)
	char *search_str, *buffer; {
	int  envoff,searchoff,len,oldlen,newlen,envrep;
	char ch,count,first;

	envoff=0;	
	while (ch=_peek(envoff++,environ)) {
		searchoff=0;
		while (ch == search_str[searchoff++]) ch=_peek(envoff++,environ);
		if (ch == '=' && search_str[searchoff-1] == 0) {
			count=0;
			first=1;
			len=2;
			oldlen=0;
			envrep=envoff;
			while (count  < 2) {
				while (_peek(envoff++,environ)) {
					len++;
					if (first) oldlen++;
					}
				first=0;
				if (_peek(envoff++,environ) == 0) count++;
				else len++;
				}
			newlen=strlen(buffer);
# if defined LARGE_CASE
			_move(len,envrep-newlen+oldlen,environ,envrep,environ);
			_move(newlen,buffer,envrep,environ);
# else
			_lmove(len,envrep-newlen+oldlen,environ,envrep,environ);
			_lmove(newlen,buffer,_showds(),envrep,environ);
# endif
			return 0;
			}
		while (ch=_peek(envoff++,environ));
		}
	return -1;
	}

