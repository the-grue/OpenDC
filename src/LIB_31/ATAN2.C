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
#include <math.h>
#include <float.h>
#include <errno.h>

double atan2(double y, double x) {
	int neg_y = 0;
	double arctan;

	if (!(y || x)) {
		errno = EDOM;
		return y;
		}
	if (y < 0) {
		y = -y;
		neg_y++;
		}
	if (y - fabs(x) == y) 
		return (neg_y ? -_pi_2() : _pi_2());
	arctan = x >= DBL_MIN ? atan(y/x) : atan(DBL_MAX);
	if (x > 0) return (arctan);
	if (neg_y) return arctan - _pi();
	return arctan + _pi();
	}
