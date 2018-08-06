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

extern double modf(), fabs();

static double ymax = 0x41A908B100000000; /* 2.1e8 */
static double zero = 0x0000000000000000; /* 0.0 */
static double dot5 = 0x3FE0000000000000; /* 0.5 */
static double one =  0x3FF0000000000000; /* 1.0 */
static double oneOverPi = 0x3FD45F306DC9C883; /* 0.31830988618379067154 */
static double pi = 	0x400921FB54442D18; /* 3.14159265358979323846 */
static double eps = 0x3E45798EE2308C3B; /* 1.0e-8 */
static double piOver2 =	0x3FF921FB54442D18; /* 1.57079632679489661923 */
static double r1 = 	0xBFC5555555555555; /* -0.16666666666666665052 */
static double r2 = 	0x3F811111111110B0; /* 0.83333333333331650314e-2 */
static double r3 = 	0xBF2A01A01A013E1B; /* -0.19841269841201840457e-3 */
static double r4 = 	0x3EC71DE3A524F062; /* 0.27557319210152756119e-5 */
static double r5 = 	0xBE5AE6454B5DC0AB; /* -0.25052106798274584544e-7 */
static double r6 = 	0x3DE6123C686AD431; /* 0.16058936490371589114e-9 */
static double r7 = 	0xBD6AE420DC08499B; /* -0.76429178068910467734e-12 */
static double r8 = 	0x3CE880FF6993DF95; /* 0.27204790957888846175e-14 */

static double _sc(x, y, sgn, flag)
double x, y;
	{
	int j, n;
	double xn, f, g, R;

	if(y >= ymax)
		{
		errno = EDOM;
		return zero;
		}
	if(modf(y * oneOverPi, &xn) >= dot5)
		xn += one;
	n = xn;
	if(n & 1)
		sgn = !sgn;
	if(flag)
		xn -= dot5;
	f = fabs(x) - xn * pi;
	if(fabs(f) >= eps)
		{
		g = f * f;
		R = (((((((r8 * g + r7) * g + r6) * g + r5) * g + r4) * g + r3) * g + r2) * g + r1) * g;
		f = f + f * R;
		}
	f=sgn ? -f: f;
	return (f > 1. ? 1.: f);
	}


double sin(X)
double X;
	{
	return _sc(X, fabs(X), X < 0.0 ? 1 : 0, 0);
	}

double cos(X)
double X;
	{
	return _sc(X, fabs(X) + piOver2, 0, 1);
	}
