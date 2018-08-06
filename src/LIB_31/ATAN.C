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
extern double fabs();

static double a[4] =	{0x0000000000000000,/* 0.0 */
						 0x3FE0C152382D7365, /* 0.52359877559829887308 */
						 0x3FF921FB54442D18, /* 1.57079632679489661923 */
						 0x3FF0C152382D7365}; /* 1.04719755119659774615 */
static double one =		0x3FF0000000000000; /* 1.0 */
static double twoMinusSqrt3 = 0x3FD126145E9ECD56; /* 0.26794919243112270647 */
static double sqrt3Minus1 =	0x3FE76CF5D0B09955; /* 0.73205080756887729353 */
static double sqrt3 =	0x3FFBB67AE8584CAA; /* 1.73205080756887729353 */
static double dot5 =	0x3FE0000000000000; /* 0.5 */
static double eps =		0x3E4E1094D643F785; /* 1.4e-8 */
static double p0 =		0xC02B60A651061CE1; /* -0.13688768894191926929e+2 */
static double p1 =		0xC034817FB9E2BCCA; /* -0.20505855195861651981e+2 */
static double p2 =		0xC020FD3F5C8D6A63; /* -0.84946240351320683534e+1 */
static double p3 =		0xBFEACD7AD9B187BD; /* -0.83758299368150059274 */
static double q0 =		0x4044887CBCC495A9; /* 0.41066306682575781263e+2 */
static double q1 =		0x40558A12040B6DA4; /* 0.86157349597130242515e+2 */
static double q2 =		0x404DCA0A320DA3D5; /* 0.59578436142597344465e+2 */
static double q3 =		0x402E0C49E14AC70F; /* 0.15024001160028576121e+2 */
static double zero =	0x0000000000000000; /* 0.0 */
static double pi =		0x400921FB54442D19; /* 3.14159265358979323846264 */
static double _atan(f)
double f;
	{
	int n;
	double g, gpg, qg;

	if(f > one)
		{
		f = one / f;
		n = 2;
		}
	else
		n = 0;
	if(f > twoMinusSqrt3)
		{
		f = (((sqrt3Minus1 * f - dot5) - dot5) + f) / (sqrt3 + f);
		++n;
		}
	if(fabs(f) > eps)
		{
		g = f * f;
		gpg = (((p3 * g + p2) * g +p1) * g +p0) * g;
		qg  = (((g + q3) * g + q2) * g + q1) * g + q0;
		f += f * (gpg / qg);
		}
	if(n > 1)
		f = -f;
	f += a[n];
	return f;
	}

double atan(x)
double x;
	{
	double r;

	r = _atan(fabs(x));
	return x < zero ? -r : r;
	}

