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
/*	random number generator		*/

static long _seed=7613941;
static long  xalo,leftlo, fhi, xhi, k, p=2147483647;



/*	frand returns a random floating point number in the range of 0 to 1. */

double frand() {
	return rand()/32767.;
	}



/*	rand returns an integer in the ranf 0 to 32767	*/

rand() {
	int  ans;

	xhi=_seed>>16;
	xalo=(_seed-(xhi<<16))*16807;
	leftlo=xalo>>16;
	fhi=xhi*16807+leftlo;
	k=fhi>>15;
	_seed=(((xalo-(leftlo<<16))-p)+((fhi-(k<<15))<<16))+k;
	if (_seed > p) _seed+=p;
	ans=_seed;
	return ans >= 0 ? ans: -ans;
	}

/*	set a random number _seed	*/

srand(news)
	int  news; {

	_seed=news;
	}
