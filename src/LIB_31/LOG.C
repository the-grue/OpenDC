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
 *  log returns the natural logarithm of "x".
 */

#include "machar.h"
#include "errno.h"

extern double frexp();

static double a0 = 	0xC05007FF12B3B59A; /* -0.64124943423745581147e2 */
static double a1 = 	0x4030624A2016AFEC; /* 0.16383943563021534222e2 */
static double a2 = 	0xBFE94415B356BD29; /* -0.78956112887491257267 */
static double b0 = 	0xC0880BFE9C0D9079; /* -0.76949932108494879777e3 */
static double b1 = 	0x40738083FA15267F; /* 0.31203222091924532844e3 */
static double b2 = 	0xC041D5804B67CE0E; /* -0.35667977739034646171e2 */
static double c0 = 	0x3FE6A09E667F3BCC; /* 0.70710678118654752440 */
static double c1 = 	0x3FE6300000000000; /* 355.0 / 512.0 */
static double c2 = 	0xBF2BD0105C610CA7; /* -2.121944400546905827679e-4 */
static double dot5 =0x3FE0000000000000; /* 0.5 */

double log(x)
double x;
	{
	int n;
	double f, znum, zden, z, w, aw, bw, rz;

	if(x <= 0.0)
		{
    	errno = EDOM;
		return -XMAX;
		}

	f = frexp(x, &n);
	znum = f - dot5;
	if(f > c0)
		{
		znum -= dot5;
		zden  = f * dot5 + dot5;
		}
	else
		{
		n--;
		zden = znum * dot5 + dot5;
		}
	z = znum / zden;
	w = z * z;
	bw = ((w + b2) * w + b1) * w + b0;
	aw = (a2 * w + a1) * w + a0;
	rz = z + z * (w * aw / bw);
	return (c2 * n + rz) + c1 * n;
	}
