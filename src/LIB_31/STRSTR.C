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
char *strstr(char * s1, char * s2) {
	char * w, c = *s2, *strchr();

	while(w = strchr(s1, c)) {
		char *t1 = w, *t2 = s2;

		while(*t2 && *t1++ == *t2++) ;
		if(*t2 == 0)
			return w;
		s1 = w + 1;
		}
	return (char *)0;
	}
