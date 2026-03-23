/*
 WHETSTONE BENCHMARK PROGRAM

 This program uses a carefully chosen mix of instructions typical of
 scientific (floating point) calculations.

 See H.J. Curnow and B.A. Wichmann,
 "A Synthetic Benchmark", Computer J., V19 #1, Feb. 1976, pp. 43-49.

 Table of times for various computers in <info-ibmpc>whetst.answers
 compiled by Richard Gillmann (GILLMANN@ISIB)

Whetstone Fortran Benchmark
(I=10, optimization off, CPU seconds)

DEC       1.1 sec   DECsystem 2060 (TOPS-20 v4, F66)
PR1ME     1.4 sec   PR1ME 750 (PRIMOS v18.1, F66)
PR1ME     1.5 sec   PR1ME 750 (PRIMOS v18.1, F77)
DEC       2.1 sec   VAX 11/780 (Unix, F77)
Apollo    6.2 sec   10 MHz MC68000 w/hardware float. point (AEGIS v4.0, F77)
Apollo   13.1 sec   10 MHz MC68000 w/software float. point (AEGIS v4.0, F77)
Intel    16.0 sec   8086/8087 (286WD Micro Development System,Intel FORTRAN)
IBM      16.0 sec   4.77 MHz 8088 PC w/8087 (DOS 2, Microsoft F77/3.10)
Z80     124.0 sec   4 MHz Z80 with Microsoft Fortran, CP/M
IBM     268.9 sec   4.77 MHz 8088 PC ($NODEBUG) (DOS 1, Microsoft F77/1.0)
Intel   390.0 sec   8086 alone (286WD Micro Development System,Intel FORTRAN)

Table compiled by Richard Gillmann (Gillmann@ISIB).
*/

/* The following are for use with Gimpel LINT ...			*/
/* -------------------------------------------------------------------- */
/*lint	+fcu		char-is-unsigned				*/
/*lint	+fsu		string-is-unsigned				*/
/*lint	+fzu		sizeof-is-unsigned				*/
/*lint	-library	don't worry about unreferenced or undefined	*/
/*lint	-e550		isave, n12, n5 are intentionally not referenced	*/

#include <stdio.h>

long		time() ;
double		p3() ;
void		pout() ;
void		pa() ;
void		p0() ;

extern double	atan() ;
extern double	sin() ;
extern double	cos() ;
extern double	sqrt() ;
extern double	exp() ;
extern double	log() ;
extern void	_doint() ;

/*lint +fva	Gimpel LINT: printf can have a variable # arguments */
extern int	printf() ;

double t, t1, t2, e1[5] ;
int j, k, l ;

void main()
	{
	long start, stop ;
	double x1, x2, x3, x4, x, y, z ;
	int i, isave, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12 ;

	start = time() ;

/* I=10 CORRESPONDS TO ONE MILLION WHETSTONE INSTRUCTIONS */

      i = 10 ;
      t1 = 050025000 ;
      t = 0.499975000 ;
      t2 = 2.0000 ;
      isave = i ;
      n1 = 0 ;
      n2 = 12 * i ;
      n3 = 14 * i ;
      n4 = 348 * i ;
      n5 = 0 ;
      n6 = 210 * i ;
      n7 = 32 * i ;
      n8 = 899 * i ;
      n9 = 516 * i ;
      n10 = 0 ;
      n11 = 93 * i ;
      n12 = 0 ;
      x1 = 1.0 ;
      x2 = -1.0 ;
      x3 = -1.0 ;
      x4 = -1.0 ;
	for (i = 1; i <= n1; i++)
		{
		x1 = (x1+x2+x3-x4)*t ;
		x2 = (x1+x2-x3+x4)*t ;
		x4 = (-x1+x2+x3+x4)*t ;
		x3 = (x1-x2+x3+x4)*t ;
		}

      pout(n1,n1,n1,x1,x2,x3,x4) ;
      e1[1] = 1.0 ;
      e1[2] = -1.0 ;
      e1[3] = -1.0 ;
      e1[4] = -1.0 ;

      for (i = 1; i <= n2; i++)
		{
		e1[1] = (e1[1]+e1[2]+e1[3]-e1[4])*t ;
		e1[2] = (e1[1]+e1[2]-e1[3]+e1[4])*t ;
		e1[3] = (e1[1]-e1[2]+e1[3]+e1[4])*t ;
		e1[4] = (-e1[1]+e1[2]+e1[3]+e1[4])*t ;
		}

      pout(n2,n3,n2,e1[1],e1[2],e1[3],e1[4]) ;
      for (i = 1 ; i <= n3 ; i++)
		{
		pa(e1) ;
		}
      pout(n3,n2,n2,e1[1],e1[2],e1[3],e1[4]) ;
      j = 1 ;
      for (i = 1 ; i <= n4 ; i++)
		{
		j = (j-1) ? 3 : 2 ;
		j = (j-2 < 0) ? 0 : 1 ;
		j = (j-1 < 0) ? 1 : 0 ;
		}
      pout(n4, j, j, x1, x2, x3, x4) ;
      j = 1 ;
      k = 2 ;
      l = 3 ;
      for (i = 1 ; i <= n6 ; i++)
		{
		j = j*(k-j)*(l-k) ;
		k = l*k-(l-j)*k ;
		l = (l-k)*(k+j) ;
		e1[l-1] = j+k+l ;
		e1[k-1] = j*k*l ;
		}
      pout(n6,j,k,e1[1],e1[2],e1[3],e1[4]) ;
      x = 0.5 ;
      y = 0.5 ;

      for (i = 1 ; i <= n7 ; i++)
		{
		x = t * atan(t2* sin(x)* cos(x) /
		    ( cos(x+y)+ cos(x-y)-1.0  )) ;
                y = t * atan(t2* sin(y)* cos(y) /
		    ( cos(x+y)+ cos(x-y)-1.0  )) ;
		}

      pout(n7, j, k, x, x, y, y) ;
      x = 1.0 ;
      y = 1.0 ;
      z = 1.0 ;

      for (i = 1 ; i <= n8 ; i++)
		{
		z = p3(x, y) ;
		}
      pout(n8, j, k, x, y, z, z) ;
      j = 1 ;
      k = 2 ;
      l = 3 ;
      e1[1] = 1.0 ;
      e1[2] = 2.0 ;
      e1[3] = 3.0 ;

      for (i = 1 ; i <= n9 ; i++)
		{
		p0() ;
		}

      pout(n9, j, k, e1[1], e1[2], e1[3], e1[4]) ;
      j = 2 ;
      k = 3 ;

      for (i = 1 ; i <= n10 ; i++)
		{
		j = j + k ;
		k = j + k ;
		j = j - k ;
		k = k - j - j ;
		}

      pout(n10, j, k, x1, x2, x3, x4) ;
      x = 0.75 ;

      for (i = 1 ; i <= n11 ; i++)
		{
		x =  sqrt( exp( log(x) / t1)) ;
		}

	pout(n11,j,k,x,x,x,x) ;

	stop = time() ;
	printf("Elapsed time = %ld.%02ld seconds\n",
		(stop - start)/100L,
		(stop - start)%100L) ;
	}

void pa(e)
double e[] ;
	{
	for (j = 0 ; j < 6 ; j++)
		{
		e[1] = (e[1] + e[2] + e[3] - e[4]) * t ;
		e[2] = (e[1] + e[2] - e[3] + e[4]) * t ;
		e[3] = (e[1] - e[2] + e[3] + e[4]) * t ;
		e[4] = (-e[1] + e[2] + e[3] + e[4]) / t2 ;
		}
	}

void p0()
	{
	e1[j] = e1[k] ;
	e1[k] = e1[l] ;
	e1[l] = e1[j] ;
	}

double p3(x, y)
double x, y ;
	{
	x = t * (x + y) ;
	y = t * (x + y) ;
	return (x + y) / t2 ;
	}

void pout(n, j, k, x1, x2, x3, x4)
int n, j, k ;
double x1, x2, x3, x4 ;
	{
	printf(" %7d %7d %7d %12.4e %12.4e %12.4e %12.4e\n",
		n, j, k, x1, x2, x3, x4) ;
	}

long time()
	{
	extern unsigned int _rax,  _rcx, _rdx ;

	_rax = 0x2c00 ;
	_doint(0x21) ;
	return 6000L * (long) (_rcx & 0xFF) +
		100L * (long) (_rdx >> 8) +
		(long) (_rdx & 0xFF) ;
	}
