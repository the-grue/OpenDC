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

extern double fabs(), sqrt(), _adx();

static double dot5 = 	0x3FE0000000000000; /* 0.5 */
static double one = 	0x3FF0000000000000; /* 1.0 */
static double zero = 	0x0000000000000000; /* 0.0 */
static double eps =		0x3E4E1094D643F785; /* 1.4e-8 */
static double a[2] =	{0x0000000000000000, /* 0.0 */
						 0x3FE921FB54442D18}; /* 0.78539816339744830962 */
static double b[2] =	{0x3FF921FB54442D18, /* 1.57079632679489661923 */
						 0x3FE921FB54442D18}; /* 0.78539816339744830962 */
static double p1 =		0xC03B5E55A83A0A61; /*-0.27368494524164255994e2 */
static double p2 =		0x404C9AA7360AD489; /*0.57208227877891731407e2 */
static double p3 =		0xC043D82CA9A6DA9E; /*-0.39688862997504877339e2 */
static double p4 =		0x40244E1764EC3926; /*0.10152522233806463645e2 */
static double p5 =		0xBFE64BBDB5E61E64; /*-0.69674573447350646411 */
static double q0 =		0xC06486C03E2B87CD; /*-0.16421096714498560795e3 */
static double q1 =		0x407A124F101EB844; /*0.41714430248260412556e3 */
static double q2 =		0xC077DDCEFC56A849; /*-0.38186303361750149284e3 */
static double q3 =		0x4062DE7C96591C71; /*0.15095270841030604719e3 */
static double q4 =		0xC037D2E86EF9861E; /*-0.23823859153670238830e2 */

static double _asc(x, y, flag)
double x, y;
	{
	int i;
	double g, r, gpg, qg;

	if(y > dot5)
		{
		if(y > one)
			{
			errno = EDOM;
			return zero;
			}
		i = 1 - flag;
		y = -_adx(sqrt(g = _adx(((dot5 - y) + dot5), -1)), 1);
		}
	else
		{
		i = flag;
		if(y < eps)
			{
			r = y;
			goto A;
			}
		else
			g = y * y;
		}
	gpg = ((((p5 * g + p4) * g + p3) * g + p2) * g + p1) * g;
	qg  = ((((g + q4) * g + q3) * g + q2) * g + q1) * g + q0;
	r = y + y * (gpg / qg);
A:
	if(flag)
		if(x < zero)
			r = b[i] + r + b[i];
		else
			r = a[i] - r + a[i];
	else
		{
		r = a[i] + r + a[i];
		if(x < zero)
			r = -r;
		}
	return r;
	}


double asin(x)
double x;
	{
	double y;

	if((y = fabs(x)) == one)
		if(x < zero)
			return -b[0];	/* pi/2 */
		else
			return b[0];
	return _asc(x, y, 0);
	}


double acos(x)
double x;
	{
	double y;

	if((y = fabs(x)) == one)
		if(x < zero)
			return _adx(b[0], 1);	/* pi */
		else
			return zero;
	return _asc(x, y, 1);
	}
