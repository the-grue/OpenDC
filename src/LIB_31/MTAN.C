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
#define	RANGE	7.853981634e-2 /* PI/2 * 0.05 */

/*
	Series Expansion of tan(x)  |x| < PI/2

	Abramowitz and Stegun 4.3.67
*/

static double tantab[] =
	{
	1.0,
	0.333333333,
	0.133333333,
	5.230769231e-2,
	2.186948854e-2,
	8.863235529e-3,
	5.131611482e-3,
	1.455834387e-3,
	5.900274409e-4,
	2.391291142e-4
	};

static double itab[] =
	{
	0.0,
	7.87017068e-2,
	1.58384440e-1,
	2.40078759e-1,
	3.24919696e-1,
	4.14213562e-1,
	5.09525449e-1,
	6.12800788e-1,
	7.26542528e-1,
	8.54080685e-1
	};

double _tan(z)
double z;
	{
	double m,n,r,x;
	int i,j;

	j = 0;
	while ( z > RANGE)
		{
		j++;
		z -= RANGE;
		}
	m = z;
	n = m * m;
	r = 0;
	for ( i=0; i < 10; i++)
		{
		x = tantab[i] * m;
		r += x;
		m *= n;
		if ( x < 1.0e-12 ) break;
		}
	return( ( r + itab[j] ) / ( 1.0 - r * itab[j] ) );
	}
