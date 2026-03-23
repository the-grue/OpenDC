/* The following are for use with Gimpel LINT ...			*/
/* -------------------------------------------------------------------- */
/*lint	+fcu		char-is-unsigned				*/
/*lint	+fsu		string-is-unsigned				*/
/*lint	+fzu		sizeof-is-unsigned				*/
/*lint	-library	don't worry about unreferenced or undefined	*/

#include <stdio.h>

static int	ack() ;
static long	time() ;

extern void	_doint() ;

/*lint +fva	Gimpel LINT: printf() may have a variable # arguments	*/
extern int	printf() ;


main()
	{
	int n1, n2, result ;
	long start, stop ;

	n1 = 3 ;
	n2 = 8 ;
	printf("Ack(%d,%d)=", n1, n2) ;
	start = time() ;
	result = ack(n1, n2) ;
	stop = time() ;
	printf("%d, Time=%ld.%02ld secs", 
		result,
		(stop - start)/100L,
		(stop - start)%100L) ;
	}

static int ack(a, b)
int a, b ;
	{
	if (a == 0) return b + 1 ;
	if (b == 0) return ack(a-1, 1) ;
	return ack(a-1, ack(a, b-1)) ;
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

