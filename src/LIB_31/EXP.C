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
 * exp returns the exponential function of "x"
 */

#include "machar.h"
#include "errno.h"

extern double fabs(), ldexp();
extern int _intrnd();

static double oneOverLn2 = 	0x3FF71547652B82FE; /* 1.4426950408889634074 */
static double ln2 = 		0x3FE62E42FEFA39EE; /* 0.6931471805599453094172321 */
static double p0 = 			0x3FCFFFFFFFFFFFFF; /* 0.249999999999999993 */
static double p1 = 			0x3F7C70E46FB3F6DF; /* 0.694360001511792852e-2 */
static double p2 = 			0x3EF152A46F58DC1D; /* 0.165203300268279130e-4 */
static double q0 = 			0x3FE0000000000000; /* 0.5 */
static double q1 = 			0x3FAC718E714251B2; /* 0.555538666969001188e-1 */
static double q2 = 			0x3F403F996FDE3809; /* 0.495862884905441294e-3 */
static double dot5 = 		0x3FE0000000000000; /* 0.5 */

double exp(x)
double x;
	{
	int n;
	double xn, g, z, pz, qz;

	if(x > BIGX || x < SMALLX)
		{
		errno = ERANGE;
		return -XMAX;
		}
	if(fabs(x) < EPS)
  		return 1.0;

	xn = n = _intrnd(x * oneOverLn2);
	g = x - xn * ln2;
	z = g * g;
	pz = ((p2 * z + p1) * z + p0) * g;
	qz =  (q2 * z + q1) * z + q0;
	return ldexp((dot5 + pz / (qz - pz)), n+1);
	}
