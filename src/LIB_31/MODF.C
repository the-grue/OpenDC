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
 * modf returns the positive fractional part of "value" and
 * stores the integer part indirectly through "iptr"
 */

double modf(value,iptr)
double value, *iptr;
	{
	int exp, *ip;

	ip = &value;
	exp = ((ip[3] >> 4) & 0x7FF) - 0x3FF;
	if(exp < 0)
		*iptr=0.0;
	else
		{
		*iptr = value;
		if(exp < 52)
			{
			exp = 52 - exp;
			for(ip = iptr; exp > 0; exp -= 16)
				{
				if(exp >= 16)
					*ip++ = 0;
				else
					*ip++ &= (-1) << exp;
				}
			}
		}
	return value - *iptr;
	}
