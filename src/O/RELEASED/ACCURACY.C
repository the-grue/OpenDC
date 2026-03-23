/*
 * ACCURACY.C: double precision math accuracy tester, v 1.7
 *     (c) 1987, PC Tech Journal and Ziff Communications Co.
 *                    written by Jim Roberts.
 *
 * The strings COMPIL and MACHIN should be set for each system.
 *
 * Format is not ideal C style, because it is meant to be 
 *  easily convertible among several programming languages.
*/

#define  NOTANSI           1  /* old or nonconforming compilers */

#if NOTANSI
#else
#define  LINT_ARGS         1  /* arg chk in Microsoft C headers */
#define  NO_EXT_KEYS       1  /* no non-ANSI keywords in MS C   */
#define  __STDC__          1  /* ANSI C in Turbo C headers      */
#endif  /* NOTANSI */

#include <stdio.h>
#include <math.h>

char *COMPIL  = "C-Ware/DeSmet C88 V 2.6" ;
char *MACHIN  = "IBM AT, 8 Mhz, w/80287"  ;

#define   MINERR          1.0E-17    /* for 64 bit reals         */
#define   LOGMIN         17.0        /* -LOG10(MINERR)           */

#define   N              10          /* size of matrix           */
#define   Y               1.0        /* const element of matrix  */
#define   STEP            0.2        /* for function tests       */
#define   ITERTRIG        5          /* for trig tests           */
#define   ITER           20          /* for other function tests */

#if NOTANSI
#define   LOG10E          0.434294481903251828

#define   PI		((double) 0x400921FB54442D18)
#ifndef   PI
#define   PI              3.141592653589793238
#endif

#define   PIO2            1.570796326794896619
#define   ROOT2           1.414213562373095049
#define   ROOT3           1.732050807568877293
#define   SQRTO2          0.707106781186547524
#else
#ifndef   PI
#define   PI              3.14159265358979323846
#endif       /* PI */
#define   PIO2            1.57079632679489661923
#define   LOG10E          0.43429448190325182765
#define   ROOT2           1.4142135623730950488
#define   ROOT3           1.7320508075688772935
#define   SQRTO2          0.7071067811865475244
#endif  /* NOTANSI */

#define   osgn(n)         ( (n==2*(n/2)) ? 1 : -1 )
                          /* -1 if n int & odd */
double  a[N][N], b[N][N], c[N][N], sum, X ;

int     i, j, k, l, m, ntest;
double  th[6], val[6], err[6], logerr[6], diverr[6], funct[6] ;
double  testerr[11], toterr;
double  xx, zz, quot ;
double  a0, a1, d0, d1, frac ;
double  p, p2 ;

long time()
	{
	extern unsigned int _rax,  _rcx, _rdx ;

	_rax = 0x2c00 ;
	_doint(0x21) ;
	return 6000L * (long) (_rcx & 0xFF) +
		100L * (long) (_rdx >> 8) +
		(long) (_rdx & 0xFF) ;
	}

void filla()
{
int     i, j;

   for (i = 0; i < N ; i++)
       for (j = 0 ; j < N ; j++)
           if (i != j) a[i][j] = Y ;
               else    a[i][j] = X + Y ;
}

void fillb()
{
int     i, j ;
double  f, d ;

   f = X + N*Y ;
   d = 1.0 / (X * f)   ;
   for (i = 0 ; i < N ; i++)
       for (j = 0 ; j < N ; j++)
         if (i != j)    b[i][j] =  -Y * d ;
               else     b[i][j] =  (-Y + f) * d ;
}

void fillc()
{
int     i, j;

   for (i = 0 ; i < N ; i++)
       for (j = 0 ; j < N ; j++)
           c[i][j] = 0.0;
}

void matmult()
{
int     i, j, k;

   for (i = 0 ; i < N ; i++)
       for (j = 0 ; j < N ; j++)
           {
           sum = 0.0;
           for (k = 0 ; k < N ; k++)
               sum +=  a[i][k] * b[k][j];
           c[i][j] = sum;
           }
}

void sumit()
{
int     i, j;

   sum = 0.0;
   for (i = 0 ; i < N ; i++)  c[i][i] -= 1.0 ;
   for (i = 0 ; i < N ; i++)
       for (j = 0 ; j < N ; j++)
           sum += fabs(c[i][j]) ;
}

void header()
{
   printf("ACCURACY: double precision reals tester: ");
   printf("%s; %s.\n", COMPIL, MACHIN) ;
   printf("V 1.7 (c) 1987, PC Tech Journal and Ziff Communications Co.\n");
   printf("           written by Jim Roberts.\n");
   printf("Test 1 checks multiplication and addition, ");
   printf("then division and subtraction.\n");
   printf("Test 2 measures the accuracy of the trig functions ");
   printf("sin(), tan(), and atan().\n");
   printf("Test 3 finds the truncation error in some ");
   printf("exponential and sqrt identities.\n");
   printf("ACCURACY is the rounded negative log of error.  ");
   printf("Program may exit abnormally.\n");
   printf("NOTE: an increase of 1 in the rating means ");
   printf("a factor of TEN less accurate.\n");
   printf("Interpretation  <0.0 - 0.5 => Excellent     ");
   printf("1.0 - 1.5 => Fair\n");
   printf("  of RATING:     0.5 - 1.0 => Good          ");
   printf("1.5 <     => Poor\n");
   printf("\n") ;
   printf("      TESTS                      ACCURACY            ");
   printf("RATING          \n");
}

void arith()
{
/*TEST 1: well-conditioned combinatorial matrix times its inverse.*/
   zz = 0.30 ;   /*factor used to control decrease of condition   */
   for (l=0;l<5;l++)
      {
      xx = (double)(zz*(2-l)) ;
      X = pow(10.0,xx) ;  /* slowly decreases condition */
      filla() ; fillb() ; fillc() ;
      matmult() ; sumit() ;
      err[l] = sum/((double)(N*N));
                  /* error is average absolute per element */
      if (err[l] > MINERR) logerr[l] = -log(err[l]) * LOG10E;
                      else logerr[l] = LOGMIN;
      testerr[1] += LOGMIN - logerr[l] ; 
      }
   testerr[1] /= 5.0 ;

   printf("#1a: 10x10 matrix       ");
   for (l=0;l<5;l++) printf("% 5.1f",logerr[l]) ;
   printf("   %6.2f\n",testerr[1]);

/* TEST 2: infinite product and continued fraction */

/* infinite product for 1-delt: run in reverse to test division */
   sum = 0.0 ;
   for (l=0;l<5;l++)
      {
      xx = (double)(-l) / 4.0;
      zz = pow(10.0, xx-2.0);
                   /* increases number of factors for convergence */
      xx = 1.0 - zz ; /* lose about 2 significant figures here    */
/*
 *     The following formula for the number of factors is designed
 *     to give sufficient accuracy, while avoiding underflow
 *     in the powers of xx.  It gives a more uniform computation
 *     from compiler to compiler.
 */
      m  = 13+l ;
      quot = 1.0 ;
      for (k=1;k<=m;k++) {
         quot /= (1.0 + xx) ;
         xx *= xx ;
         }
      err[l] = fabs(1.0 - quot/zz)*0.01 ;
      /* factor of 0.01 compensates for cancellation error above */
      if (err[l] > MINERR) diverr[l] = -log(err[l]) * LOG10E ;
                      else diverr[l] = LOGMIN ;
      sum += LOGMIN - diverr[l] ;
      logerr[l] = diverr[l] ;        /* needed for later average */
      }
   sum /= 5.0 ;

   printf("#1 : infinite product   ");
   for (i=0;i<5;i++) printf("% 5.1f",diverr[i]) ;
   printf("   %6.2f\n",sum);
/*
 * continued fraction for tan() compared to actual values
 * for five angles: this is a test of division and subtraction,
 * not of the tangent.
 */
   th[0] = PI/12.0 ;
   th[1] = PI/6.0 ;
   th[2] = PI/4.0 ;
   th[3] = PI/3.0 ;
   th[4] = 5.0*PI/12.0 ;
   val[0] = 2.0 - ROOT3 ;
   val[1] = 1.0 / ROOT3 ;
   val[2] = 1.0         ;
   val[3] = ROOT3 ;
   val[4] = 2.0 + ROOT3 ;
   sum = 0.0 ;
   m = 8 ;     /* number of iterations, gives sufficient accuracy */
   for (l=0;l<5;l++)
      {
      a0 = 2.0 * m + 1.0 ;
      p2 = th[l] ;
      p  = p2*p2 ;
      d0 = a0 - p / (a0 + 2.0) ;
      for (k=0;k<m;k++)
         {
         a1 = a0 - 2.0 ;
         d1 = a1 - p / d0 ;
         a0 = a1 ;
         d0 = d1 ;
         }
      frac = p2 / d0 ;
      funct[l] = frac ;
      }

   for (l=0;l<5;l++)
      {
      err[l] = fabs(1.0 - val[l]/funct[l]) ;
      if (err[l] > MINERR) diverr[l] = -log(err[l]) * LOG10E;
                      else diverr[l] = LOGMIN ;
      sum += LOGMIN - diverr[l] ;
      }
   sum /= 5.0 ;

   printf("#1 : continued fraction ");
   for (i=0;i<5;i++) printf("% 5.1f",diverr[i]) ;
   printf("   %6.2f\n",sum);

   printf("#1b: division average   ");
   for(i=0;i<5;i++) {
      logerr[i] = 0.5 * (logerr[i] + diverr[i]) ;
      testerr[2] += LOGMIN - logerr[i] ;
      }
   testerr[2] /= 5.0 ;
   for (i=0;i<5;i++) printf("% 5.1f",logerr[i]) ;
   printf("   %6.2f\n",testerr[2]);
}


void trig()
/*TEST 2: first, truncation in some sine identities */
{
   for (l=0;l<5;l++) logerr[l] = 0.0 ;
   for (j=0;j < ITERTRIG;j++)
      {
      th[0] = PI/12.0 + j*PI ;
      th[1] = PI/6.0 + j*PI ;
      th[2] = PI/4.0 + j*PI ;
      th[3] = PI/3.0 + j*PI ;
      th[4] = 5.0*PI/12.0 + j*PI ;
      val[0] = osgn(j)*ROOT2*(ROOT3-1.0)*0.25 ;
      val[1] = osgn(j)*0.5 ;
      val[2] = osgn(j)*SQRTO2 ;
      val[3] = osgn(j)*0.5*ROOT3 ;
      val[4] = osgn(j)*ROOT2*(ROOT3+1)*0.25 ;
      for (l=0;l<5;l++) funct[l] = sin(th[l]) ;
      for (l=0;l<5;l++)
         {
         err[l] = fabs(1.0 - val[l]/funct[l]) ;
         if (err[l] > MINERR) logerr[l] -= log(err[l]) * LOG10E;
                         else logerr[l] += LOGMIN ;
         }
      }
   for (l=0;l<5;l++) logerr[l] /= (double)ITERTRIG ;
   for (l=0;l<5;l++) testerr[3] += LOGMIN - logerr[l] ;
   testerr[3] /= 5.0 ;

   printf("#2a: sin()              ");
   for (i=0;i<5;i++) printf("% 5.1f",logerr[i]) ;
   printf("   %6.2f\n",testerr[3]);

/* compare tan() with exact values */
   for (l=0;l<5;l++) logerr[l] = 0.0 ;
   for (j=0;j < ITERTRIG;j++)
      {
      th[0] = PI/12.0 + j*PI ;
      th[1] = PI/6.0 + j*PI ;
      th[2] = PI/4.0 + j*PI ;
      th[3] = PI/3.0 + j*PI ;
      th[4] = 5.0*PI/12.0 + j*PI ;
      val[0] = 2.0 - ROOT3 ;
      val[1] = 1.0 / ROOT3 ;
      val[2] = 1.0         ;
      val[3] = ROOT3 ;
      val[4] = 2.0 + ROOT3 ;
      for (l=0;l<5;l++) funct[l] = tan(th[l]) ;
      for (l=0;l<5;l++)
         {
         err[l] = fabs(1.0 - val[l]/funct[l]) ;
         if (err[l] > MINERR) logerr[l] -= log(err[l]) * LOG10E;
                         else logerr[l] += LOGMIN ;
         }
      }
   for (l=0;l<5;l++) logerr[l] /= (double)ITERTRIG  ;
   for (l=0;l<5;l++) testerr[4] += LOGMIN - logerr[l] ;
   testerr[4] /= 5.0 ;

   printf("#2b: tan()              ");
   for (i=0;i<5;i++) printf("% 5.1f",logerr[i]) ;
   printf("   %6.2f\n",testerr[4]);

/* compare atan() with tan() for consistency */
   for (l=0;l<5;l++) logerr[l] = 0.0 ;
   for (j=0;j < ITER;j++)
      {
      for (l=0;l<5;l++) th[l] = (5*j+l+1)*PIO2/(5*ITER+1) ;
      for (l=0;l<5;l++) val[l] = tan(th[l]) ;
      for (l=0;l<5;l++) funct[l] = atan(val[l]) ;
      for (l=0;l<5;l++)
         {
         err[l] = fabs(1.0 - th[l]/funct[l]) ;
         if (err[l] > MINERR) logerr[l] -= log(err[l]) * LOG10E;
                         else logerr[l] += LOGMIN ;
         }
      }
   for (l=0;l<5;l++) logerr[l] /= (double)ITER ;
   for (l=0;l<5;l++) testerr[5] += LOGMIN - logerr[l] ;
   testerr[5] /= 5.0 ;

   printf("#2c: atan()             ");
   for (i=0;i<5;i++) printf("% 5.1f",logerr[i]) ;
   printf("   %6.2f\n",testerr[5]);

}

/* TEST 3: log() against exp() for consistency */

void transc()
{
   for (l=0;l<5;l++) logerr[l] = 0.0 ;
   for (j=0;j<ITER;j++)
      {
      for (l=0;l<5;l++)  th[l] = (5*j+l+1)*STEP ;
      for (l=0;l<5;l++)  val[l] = exp(th[l]) ;
      for (l=0;l<5;l++)  funct[l] = log(val[l]) ;
      for (l=0;l<5;l++)
         {
         err[l] = fabs(1.0 - th[l]/funct[l]) ;  /* unnormalized */
         if (err[l] > MINERR) logerr[l] -= log(err[l]) * LOG10E ;
			 else logerr[l] += LOGMIN ;
			}
		}

	for (l=0;l<5;l++) logerr[l] /= (double)ITER ;
	for (l=0;l<5;l++) testerr[6] += LOGMIN - logerr[l] ;
	testerr[6] /= 5.0 ;

	printf("#3a: log() & exp()      ") ;
	for(i=0;i<5;i++) printf("% 5.1f",logerr[i]) ;
	printf("   %6.2f\n",testerr[6]) ;
	}

/* sqrt() identities */
void roots()
{
	for (l=0;l<5;l++) logerr[l] = 0.0 ;
	for (j=0;j<ITER;j++)
		{
		for (l=0;l<5;l++) th[l] = (5*j+l+1)*STEP ;
		for (l=0;l<5;l++) val[l] = sqrt(th[l]) ;
		for (l=0;l<5;l++) funct[l] = val[l]*val[l] ;
		for (l=0;l<5;l++)
			{
			err[l] = fabs(1.0 - th[l]/funct[l]) ; /* unnormalized */
			if (err[l] > MINERR) logerr[l] -= log(err[l]) * LOG10E ;
								 else logerr[l] += LOGMIN ;
			}
		}
	for (l=0;l<5;l++) logerr[l] /= (double)ITER ;
	for (l=0;l<5;l++) testerr[7] += LOGMIN - logerr[l] ;
	testerr[7] /= 5.0 ;

	printf("#3b: sqrt               ") ;
	for (i=0;i<5;i++) printf("% 5.1f", logerr[i]) ;
	printf("   %6.2f\n", testerr[7]) ;
	}


main()
	{
	long time(), start, stop ;

	start = time() ;
	header() ;
	for (i = 0; i < 10; i++) testerr[i] = 0.0 ;
	arith() ;
	trig() ;
	transc() ;
	roots() ;
	ntest = 7 ;
	toterr = 0.0 ;
	for (i = 1; i <= ntest; i++) toterr += testerr[i] ;
	toterr /= (double) ntest ;
	printf("Overall rating: %6.2f\n", toterr) ;
	stop = time() ;
	printf("Time=%ld.%02ld secs\n", 
		(stop - start)/100L,
		(stop - start)%100L) ;
	return(0) ;
	}

      