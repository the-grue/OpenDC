/* The following are for use with Gimpel LINT ...			*/
/* -------------------------------------------------------------------- */
/*lint	+fcu		char-is-unsigned				*/
/*lint	+fsu		string-is-unsigned				*/
/*lint	+fzu		sizeof-is-unsigned				*/
/*lint	-library	don't worry about unreferenced or undefined	*/

#include <stdio.h>

static void	fib() ;
static long	time() ;

extern void	_doint() ;

/*lint +fva	Gimpel LINT: printf() may have a variable # arguments	*/
extern int	printf() ;

void main()
	{
	int i, n1, n2 ;
	long start, stop ;

	n1 = 30000 ;
	n2 = 30000 ;
	printf("Fib(%d,%d):", n1, n2) ;
	start = time() ;

	for (i = 1 ; i < n2 ; i++)
		{
		fib(n1) ;
		}

	stop = time() ;
	printf(" Time=%ld.%02ld secs", 
		(stop - start)/100L,
		(stop - start)%100L) ;
	}

static void fib(n)
int n ;
	{
	int a, b, c ;

	a = 0 ;
	b = 1 ;
	while (b < n)
		{
		c = b ;
		b = a + b ;
		a = c ;
		}
	}

static long time()
	{
	extern unsigned int _rax,  _rcx, _rdx ;

	_rax = 0x2c00 ;
	_doint(0x21) ;
	return 6000L * (long) (_rcx & 0xFF) +
		100L * (long) (_rdx >> 8) +
		(long) (_rdx & 0xFF) ;
	}

