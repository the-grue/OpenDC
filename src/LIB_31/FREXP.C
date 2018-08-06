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
 * frexp returns the mantissa of a double "value" as a double
 * quantity, "x", of magnitude less than 1.0 and stores an integer
 * "n" (such that "value" = "x" * 2 ** "n") indirectly through "eptr"
 */

double frexp(value, eptr)
double value;
int *eptr;
	{
	int *ip;

	if(value == 0.0)
		*eptr = 0;
	else
		{
		ip = &value;
		*eptr = ((ip[3] >> 4) & 0x7FF) - 0x3FE;
		ip[3] = (ip[3] & 0x800F) + 0x3FE0;
		}
	return value;
	}
