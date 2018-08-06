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
#include <float.h>
#include <errno.h>
/* Largest signed long int power of 2 */
#define MAXSHIFT	30

extern double frexp();

double
ldexp(value, exp)
register double value;
register int exp;
{
	int old_exp;

	if (exp == 0 || value == 0.0) /* nothing to do for zero */
		return (value);
	frexp(value, &old_exp);
	if (exp > 0) {
		if (exp + old_exp > DBL_MAX_EXP) { /* overflow */
			errno = ERANGE;
			return (value < 0 ? -DBL_MAX : DBL_MAX);
		}
		for ( ; exp > MAXSHIFT; exp -= MAXSHIFT)
			value *= (1L << MAXSHIFT);
		return (value * (1L << exp));
	}
	if (exp + old_exp < DBL_MIN_EXP) { /* underflow */
		errno = ERANGE;
		return (0.0);
	}
	for ( ; exp < -MAXSHIFT; exp += MAXSHIFT)
		value *= 1.0/(1L << MAXSHIFT); /* mult faster than div */
	return (value / (1L << -exp));
}
