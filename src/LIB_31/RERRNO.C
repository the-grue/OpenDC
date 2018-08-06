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
/*	RERRNO.C	--	default routine to report on float errors.	*/

#include <errno.h>

static onum();
rerrno() {

	puts("errno=");
	onum(errno);
	switch (errno) {
		case EFLOAT:	puts("  *Error in Floating Point*");
						break;
		case EOVERFLOW:	puts("  *Overflow*");
						return;
		case EDIV0:		puts("  *Divide by Zero*");
						break;
		case EDOM:		puts("  *Domain Error*");
						break;
		case ERANGE:	puts("  *Out of Range*");
						break;
		}
	putchar('\n');
	exit(2);
	}

static onum(num)
	int num; {
	if (num > 9) {
		onum(num/10);
		num=num%10;
		}
	putchar(num+'0');
	}
