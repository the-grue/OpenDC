/*
 *	"DHRYSTONE" Benchmark Program
 *
 *	Version:	C/1
 *	Date:		12/01/84
 *	Author:		Reinhold P. Weicker, CACM Vol 27, No 10, 10/84 pg.1013
 *			Translated from ADA by Rick Richardson
 *			Every method to preserve ADA-likeness has been used,
 *			at the expense of C-ness.
 *	Compile:	c88 drystone (DeSmet has no register option)
 *	Defines:	Defines are provided for old C compiler's
 *			which don't have enums, and can't assign structures.
 *
 * MACHINE	MICROPROCESSOR	OPERATING	COMPILER	DHRYSTONES/SEC
 * TYPE				SYSTEM				NO REG	REGS
 * --------------------------	------------	-----------	--------------
 * IBM PC/XT	8088-4.77Mhz	PCDOS 2.1	CI-C86 2.1	????	????
 * IBM PC/XT	8088-4.77Mhz	PC/IX		cc		????	????
 * IBM PC/XT	8088-4.77Mhz	VENIX/86 2.0	cc		 297	 324
 * IBM PC/AT	80286-6Mhz	PCDOS 3.0	CI-C86 2.1	 666	 684
 * ATT 3B2/300	MAC32-?Mhz	UNIX 5.0.2	cc		 735	 806
 * IBM PC/AT	80286-6Mhz	VENIX/86 2.1	cc		 961	1000
 * Sun2/120	68010-10Mhz	Sun 4.2BSD	cc		1136	1219
 * PDP 11/70	-		UNIX 5.2	cc		????	????
 * VAX 11/780	-		UNIX 5.2	cc		????	????
 * Macintosh	68000-??Mhz	????		??		????	????
 * Lisa		68000-??Mhz	????		??		????	????
 *
 *
 **************************************************************************
 *
 *	The following program contains statements of a high-level programming
 *	language (C) in a distribution considered representative:
 *
 *	assignments			53%
 *	control statements		32%
 *	procedure, function calls	15%
 *
 *	100 statements are dynamically executed.  The program is balanced with
 *	respect to the three aspects:
 *		- statement type
 *		- operand type (for simple data types)
 *		- operand access
 *			operand global, local, parameter, or constant.
 *
 *	The combination of these three aspects is balanced only approximately.
 *
 *	The program does not compute anything meaningfull, but it is
 *	syntactically and semantically correct.
 *
 */

/* The following are for use with Gimpel LINT ...			*/
/* -------------------------------------------------------------------- */
/*lint	+fcu		char-is-unsigned				*/
/*lint	+fsu		string-is-unsigned				*/
/*lint	+fzu		sizeof-is-unsigned				*/
/*lint	-library	don't worry about unreferenced or undefined	*/
/*lint	-e550		BoolLoc is intentionally not referenced		*/

#define	structassign(d, s)	d = s

typedef enum	{Ident1, Ident2, Ident3, Ident4, Ident5} Enumeration;

typedef int	OneToThirty;
typedef int	OneToFifty;
typedef char	CapitalLetter;
typedef char	String30[31];
typedef int	Array1Dim[51];
typedef int	Array2Dim[51][51];

struct	Record
{
	struct Record		*PtrComp;
	Enumeration		Discr;
	Enumeration		EnumComp;
	OneToFifty		IntComp;
	String30		StringComp;
};

typedef struct Record 	RecordType;
typedef RecordType *	RecordPtr;
typedef int		boolean;

#define	NULL		0
#define	TRUE		1
#define	FALSE		0

#ifndef REG
#define	REG
#endif

void		Proc0() ;
void		Proc1() ;
void		Proc2() ;
void		Proc3() ;
void		Proc4() ;
void		Proc5() ;
void		Proc6() ;
void		Proc7() ;
void		Proc8() ;
Enumeration	Func1();
boolean		Func2();
boolean		Func3();
long		time() ;

extern char	*malloc() ;
extern char	*strcpy() ;
extern int	strcmp() ;
extern void	_doint() ;

/*lint +fva	Gimpel LINT: printf() may have a variable # arguments	*/
extern int	printf() ;

void main()
{
	Proc0();
}

/*
 * Package 1
 */
int		IntGlob;
boolean		BoolGlob;
char		Char1Glob;
char		Char2Glob;
Array1Dim	Array1Glob;
Array2Dim	Array2Glob;
RecordPtr	PtrGlob;
RecordPtr	PtrGlobNext;

void Proc0()
{
	OneToFifty		IntLoc1;
	REG OneToFifty		IntLoc2;
	OneToFifty		IntLoc3;
	REG char		CharIndex;
	REG Enumeration	 	EnumLoc;
	String30		String1Loc;
	String30		String2Loc;

#define LOOPS		10000
long			time();
long			starttime;
long			benchtime;
long			nulltime;
register unsigned int	i;
starttime = time();
for (i = 0; i < LOOPS; ++i)
	{
	}
nulltime = time() - starttime;

	PtrGlobNext = (RecordPtr) malloc(sizeof(RecordType));
	PtrGlob = (RecordPtr) malloc(sizeof(RecordType));
	PtrGlob->PtrComp = PtrGlobNext;
	PtrGlob->Discr = Ident1;
	PtrGlob->EnumComp = Ident3;
	PtrGlob->IntComp = 40;
	strcpy(PtrGlob->StringComp, "DHRYSTONE PROGRAM, SOME STRING");

/*****************
-- Start Timer --
*****************/
starttime = time();
for (i = 0; i < LOOPS; ++i)
{

	Proc5();
	Proc4();
	IntLoc1 = 2;
	IntLoc2 = 3;
	strcpy(String2Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
	EnumLoc = Ident2;
	BoolGlob = ! Func2(String1Loc, String2Loc);
	while (IntLoc1 < IntLoc2)
	{
		IntLoc3 = 5 * IntLoc1 - IntLoc2;
		Proc7(IntLoc1, IntLoc2, &IntLoc3);
		++IntLoc1;
	}
	Proc8(Array1Glob, Array2Glob, IntLoc1, IntLoc3);
	Proc1(PtrGlob);
	for (CharIndex = 'A'; CharIndex <= Char2Glob; ++CharIndex)
		if (EnumLoc == Func1(CharIndex, 'C'))
			Proc6(Ident1, &EnumLoc);
	IntLoc3 = IntLoc2 * IntLoc1;
	IntLoc2 = IntLoc3 / IntLoc1;
	IntLoc2 = 7 * (IntLoc3 - IntLoc2) - IntLoc1;
	Proc2(&IntLoc1);

/*****************
-- Stop Timer --
*****************/
}
benchtime = time() - starttime - nulltime;
printf("Elapsed time for %ld passes = %ld.%02ld\n", (long) LOOPS, 
	benchtime/100L, benchtime%100L);
printf("This machine benchmarks at %ld dhrystones/second\n",
	((long) LOOPS) / (benchtime/100L));
}

void Proc1(PtrParIn)
REG RecordPtr	PtrParIn;
{
#define	NextRecord	(*(PtrParIn->PtrComp))

	structassign(NextRecord, *PtrGlob);
	PtrParIn->IntComp = 5;
	NextRecord.IntComp = PtrParIn->IntComp;
	NextRecord.PtrComp = PtrParIn->PtrComp;
	Proc3(&(NextRecord.PtrComp));
	if (NextRecord.Discr == Ident1)
	{
		NextRecord.IntComp = 6;
		Proc6(PtrParIn->EnumComp, &NextRecord.EnumComp);
		NextRecord.PtrComp = PtrGlob->PtrComp;
		Proc7(NextRecord.IntComp, 10, &NextRecord.IntComp);
	}
	else
		structassign(*PtrParIn, NextRecord);

}

void Proc2(IntParIO)
OneToFifty	*IntParIO;
{
	REG OneToFifty		IntLoc;
	REG Enumeration		EnumLoc;

	IntLoc = *IntParIO + 10;
	for(;;)
	{
		if (Char1Glob == 'A')
		{
			--IntLoc;
			*IntParIO = IntLoc - IntGlob;
			EnumLoc = Ident1;
		}
		if (EnumLoc == Ident1)
			break;
	}
}

void Proc3(PtrParOut)
RecordPtr	*PtrParOut;
{
	if (PtrGlob != NULL)
		*PtrParOut = PtrGlob->PtrComp;
	else
		IntGlob = 100;
	Proc7(10, IntGlob, &PtrGlob->IntComp);
}

void Proc4()
{
	REG boolean	BoolLoc;

	BoolLoc = Char1Glob == 'A';
	BoolLoc |= BoolGlob;
	Char2Glob = 'B';
}

void Proc5()
{
	Char1Glob = 'A';
	BoolGlob = FALSE;
}

void Proc6(EnumParIn, EnumParOut)
REG Enumeration	EnumParIn;
REG Enumeration	*EnumParOut;
{
	*EnumParOut = EnumParIn;
	if (! Func3(EnumParIn) )
		*EnumParOut = Ident4;
	switch (EnumParIn)
	{
	case Ident1:	*EnumParOut = Ident1; break;
	case Ident2:	if (IntGlob > 100) *EnumParOut = Ident1;
			else *EnumParOut = Ident4;
			break;
	case Ident3:	*EnumParOut = Ident2; break;
	case Ident4:	break;
	case Ident5:	*EnumParOut = Ident3;
	}
}

void Proc7(IntParI1, IntParI2, IntParOut)
OneToFifty	IntParI1;
OneToFifty	IntParI2;
OneToFifty	*IntParOut;
{
	REG OneToFifty	IntLoc;

	IntLoc = IntParI1 + 2;
	*IntParOut = IntParI2 + IntLoc;
}

void Proc8(Array1Par, Array2Par, IntParI1, IntParI2)
Array1Dim	Array1Par;
Array2Dim	Array2Par;
OneToFifty	IntParI1;
OneToFifty	IntParI2;
{
	REG OneToFifty	IntLoc;
	REG OneToFifty	IntIndex;

	IntLoc = IntParI1 + 5;
	Array1Par[IntLoc] = IntParI2;
	Array1Par[IntLoc+1] = Array1Par[IntLoc];
	Array1Par[IntLoc+30] = IntLoc;
	for (IntIndex = IntLoc; IntIndex <= (IntLoc+1); ++IntIndex)
		Array2Par[IntLoc][IntIndex] = IntLoc;
	++Array2Par[IntLoc][IntLoc-1];
	Array2Par[IntLoc+20][IntLoc] = Array1Par[IntLoc];
	IntGlob = 5;
}

Enumeration Func1(CharPar1, CharPar2)
CapitalLetter	CharPar1;
CapitalLetter	CharPar2;
{
	REG CapitalLetter	CharLoc1;
	REG CapitalLetter	CharLoc2;

	CharLoc1 = CharPar1;
	CharLoc2 = CharLoc1;
	if (CharLoc2 != CharPar2)
		return (Ident1);
	else
		return (Ident2);
}

boolean Func2(StrParI1, StrParI2)
String30	StrParI1;
String30	StrParI2;
{
	REG OneToThirty		IntLoc;
	REG CapitalLetter	CharLoc;

	IntLoc = 1;
	while (IntLoc <= 1)
		if (Func1(StrParI1[IntLoc], StrParI2[IntLoc+1]) == Ident1)
		{
			CharLoc = 'A';
			++IntLoc;
		}
	if (CharLoc >= 'W' && CharLoc <= 'Z')
		IntLoc = 7;
	if (CharLoc == 'X')
		return(TRUE);
	else
	{
		if (strcmp(StrParI1, StrParI2) > 0)
		{
			IntLoc += 7;
			return (TRUE);
		}
		else
			return (FALSE);
	}
}

boolean Func3(EnumParIn)
REG Enumeration	EnumParIn;
{
	REG Enumeration	EnumLoc;

	EnumLoc = EnumParIn;
	if (EnumLoc == Ident3) return (TRUE);
	return (FALSE);
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
