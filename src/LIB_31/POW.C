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
 *	pow	returns "x" to the "y" power
 */

#include <errno.h>

extern double log(), exp();
static double zero = 0.0;

double pow(x, y)
double x, y;
	{
	long l;
	double temp;

	if(x <= zero)
		{
		if(x == zero)
			{
			if(y <= zero)
				errno = EDOM;
			return zero;
			}
		l = y;
		if(l != y)
			{
			errno = EDOM;
			return zero;
			}
		temp = exp(y * log(-x));
		if(l & 1)
			temp = -temp;
		}
	else
		temp = exp(y * log(x));
	return temp;
	}
