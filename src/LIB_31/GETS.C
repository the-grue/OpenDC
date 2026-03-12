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
#define CR		13
#define CONTZ	26

/*	GETS			string address=gets(string address);	*/
static char  *getat;

char *gets(str)
	char *str; {

	_gets(getat=str,512);
	return getat;
	}



/*	_gets	gets with a maximum length	*/

_gets(str,max)
	char str[1];
	int  max; {
	int  ch;
	char *oldstr;

	oldstr=str;
	while (--max && (ch=ci()) != CR && ch != CONTZ)
		co(*oldstr++=ch);
	
	if (str == oldstr) getat=0;
gets_end:
	*oldstr=0;
	return oldstr-str;
	}



