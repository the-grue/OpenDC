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
Quicker sort algorithm (from C. A. R. Hoare)
This uses linear insertion sort for number of elements
smaller than M in some partitioning.
 */

static char	*median();
static int linsert(), xchng();

#define	M	10

qsort(base, nel, width, compar)
char *base;
unsigned nel, width;
int (*compar)();
	{
	char *bot, *top;

	bot = base;
	top = base + nel*width;
	if (nel < M)
		{
		if (nel > 1)
			linsert(bot, nel, width, compar);
		return;
		}
	xchng(bot, median(compar, bot, bot+(nel/2)*width, top-width), width);
	for (;;)
		{
		while ((*compar)(base, bot+=width)>=0 && bot<top)
			;
		while ((*compar)(top-=width, base)>=0 && top>base)
			;
		if (bot < top)
			xchng(bot, top, width);
		else
			break;
	}
	xchng(top, base, width);
	qsort(base,(unsigned) ((top-base)/width), width, compar);
	qsort(bot, nel - (unsigned)((bot-base)/width), width, compar);
	}

/*
Exchange two records of `width' bytes pointed to by `p1' and `p2'.
 */

static xchng(p1, p2, width)
char *p1, *p2;
unsigned width;
	{
	char save;

	if (width)
		do {
			save = *p1;
			*p1++ = *p2;
			*p2++ = save;
		} while (--width);
	}

/*
Determine the median of the first, middle, and last elements.
 */

static char * median(comp, a, b, c)
int (*comp)();
char *a, *b, *c;
	{
	char *bmin, *bmax;

	if ((*comp)(a, b) < 0)
		{
		bmin = a;
		bmax = b;
		}
	else
		{
		bmin = b;
		bmax = a;
		}
	if ((*comp)(bmax, c) < 0)
		return (bmax);
	return ((*comp)(bmin, c) < 0 ? c : bmin);
	}

/*
Linear insertion sort used to speed up final sorts when parititions get small.
 */

static linsert(base, nel, width, compar)
char *base;
unsigned nel;
unsigned width;
int (*compar)();
{
	char *min, *mbase;
	unsigned n;

	--nel;
	do {
		n = nel;
		min = base;
		mbase = min+width;
		do {
			if ((*compar)(mbase, min) < 0)
				min = mbase;
			mbase += width;
		} while (--n);
		xchng(min, base, width);
		base += width;
	} while (--nel);
}
