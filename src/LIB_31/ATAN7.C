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
#define	SQRT3	1.732050807568877293
#define	EPS		2.328306436e-10

static double A[] =	{
					0.0,
					0.52359877559829887308,
					1.57079632679489661923,
					1.04719755119659774615
					};

static double f, result;

double fabs();

double atan(X)
double X;
	{
	int N;

	f = fabs( X );

	if ( f > 1.0 )
		{
		f = 1.0 / f;
		N = 2;
		}
	else
		N = 0;

	if ( f > ( 2.0 - SQRT3 ))
		{
		f = (((( SQRT3 - 1.0 ) * f - 0.5 ) - 0.5 ) + f ) / ( SQRT3 + f );
		N++;
		}

	if ( fabs( f ) < EPS )
		result = f;
	else {
#asm
	FLD QWORD f_
	FLD1
	FPATAN
	FST QWORD result_
#
	}
	if ( N > 1 )
		result = -result;
	result += A[N];

	if ( X < 0.0 )
		result = -result;

	return ( result );
	}
