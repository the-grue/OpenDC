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
#include <errno.h>
#include <machar.h>

extern double fabs();

static double eps = 	0x3E4E1094D643F785; /* 1.4e-8 */
static double eps1 = 	0x0105F1CA820511B7; /* 1.0e-303 */
static double YMAX = 	0x4197D78400000000; /* 1.0e8 */
static double twoOverPi = 0x3FE45F306DC9C883; /* 0.63661977236758134308 */
static double c1 = 		0x3FF9220000000002; /* 1.57080078125 */
static double c2 = 		0xBED2AEEF4B9EE59E; /* -4.454455103380768678308e-6 */
static double p1 = 		0xBFC06B97BF03648A; /* -0.1282834704095743847 */
static double p2 = 		0x3F66FC6FE2982A7B; /* 0.2805918241169988906e-2 */
static double p3 = 		0xBEDF637DEA7C0B2B; /* -0.7483634966612065149e-5 */
static double q1 = 		0xBFDD8B2134D70767; /* -0.4616168037429048840 */
static double q2 = 		0x3F97E7B68D9A55F3; /* 0.2334485282206872802e-1 */
static double q3 = 		0xBF2B525B10D0A169; /* -0.2084480442203870948e-3 */
static double dot5 = 	0x3FE0000000000000; /* 0.5 */
static double zero = 	0x0000000000000000; /* 0.0 */

static double _tc(x,iflag)
double x;
int iflag;
	{
	int n;
	double y, xn, f, g, xnum, xden;

	y = fabs(x);
	if(iflag && y < eps1)
		{
		errno = ERANGE;
		return (x < zero) ? -XMAX : XMAX;
		}
	if(y > YMAX)
		{
		errno = ERANGE;
		return zero;
		}
	xn = n = _intrnd(x * twoOverPi);
	f = (x - xn * c1) - xn * c2;
	if(fabs(f) < eps)
		{
		xnum = f;
		xden = 1.0;
		}
	else
		{
		g = f * f;
		xnum = ((p3 * g + p2) * g + p1) * g * f + f;
		xden = ((q3 * g + q2) * g + q1) * g + dot5 + dot5;
		}
		if(n & 1)
			xnum = -xnum;
		if(iflag == (n & 1))
			return xnum / xden;
		return xden / xnum;
	}

double tan(x)
double x;
	{
	return _tc(x,0);
	}


double cot(x)
double x;
	{
	return _tc(x,1);
	}
