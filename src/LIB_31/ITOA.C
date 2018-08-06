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
char *itoa(v, s, r)	/* convert n to characters in s using base b */
int v;
char *s;
int r;
{
	char c;
	char *b=s, *p=s;
	int i, sign=0;

	if(r < 2 || r > 36) {
		*s=0;
		return s;
		}
	if(r==10 && v < 0) {
		sign++;
		v = -v;
		}
	do {	/* generate digits in reverse order */
		i = (unsigned)v % r;	/* get next digit */
		*p++ = (i >= 10) ? i + ('A' - 10) : i + '0';
	} 
	while (((unsigned)v /= r) > 0);	/* delete it */
	if(sign)
		*p++='-';
	*p = '\0';
	while (p > s) {	/* reverse string */
		c = *s;
		*s++ = *--p;
		*p = c;
	}
	return b;
}
