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
/*	STRTOK  --	return tokens from a string.
			    example:   char *tptr,*strtok();
	               tptr=strtok("   , aaa , bbb"," ,\t"); */

static char *nxt;

char *strpbrk();

#define	NULL (char *)0

char *strtok(char *str, char *toks) {
	char *beg, *end;

	if ((beg = (str == NULL) ? nxt : str) == NULL)
		return NULL;
	beg += strspn(beg,toks);
	if (*beg == '\0')
		return NULL;
	if((end = strpbrk(beg,toks)) == NULL)
		nxt = NULL;
	else {
		*end++ = '\0';
		nxt = end;
		}
	return beg;
	}
