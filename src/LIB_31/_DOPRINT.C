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
#define MAXLINE 258	/* Longest line of input expected from the console */
char toupper(), isdigit();
char *ecvt(), *fcvt();
/*
	General formatted output conversion routine, used by
	fprintf and sprintf..."line" is where the output is
	written, and "fmt" is a pointer to an argument list 
	which must consist of a format string pointer and
	subsequent list of (optional) values. Having arguments
	passed on the stack works out a heck of a lot neater
	than it did before when the args were passed via an
	absolute vector in low memory!
*/

/*
	Internal routine used by "_spr" to perform ascii-
	to-decimal conversion and update an associated pointer:
*/

static int _gv2(sptr)
char **sptr;
{
	int n;
	n = 0;
	while (isdigit(**sptr)) n = 10 * n + *(*sptr)++ - '0';
	return n;
}

static _tohex(char ** ptr, unsigned val) {
	int v;

	v = (val >> 12) & 15;
	*(*ptr)++ = (v < 10) ? v + '0' : v + 55 ;
	v = (val >> 8) & 15;
	*(*ptr)++ = (v < 10) ? v + '0' : v + 55 ;
	v = (val >> 4) & 15;
	*(*ptr)++ = (v < 10) ? v + '0' : v + 55 ;
	v = val & 15;
	*(*ptr)++ = (v < 10) ? v + '0' : v + 55 ;
	}

/*
	Internal function which converts n into an ASCII
	base `base' representation and places the text
	at the location pointed to by the pointer pointed
	to by `string'. Yes, you read that correctly.
*/

static char _uspr(string, n, base, c)
char **string, c;
unsigned n;
{
	char length;
	if (n<base) {
		*(*string)++ = (n < 10) ? n + '0' : n + ((c == 'X') ? 55 : 87);
		return 1;
	}
	length = _uspr(string, n/base, base, c);
	_uspr(string, n%base, base, c);
	return length + 1;
}

/*
	Same as above except for longs.	*/

static char _duspr(string, n, base, c)
char **string, c;
long n;
int  base;
{
	char length;
	if (n >= 0 && n < base) {
		*(*string)++ = (n < 10) ? n + '0' : n + ((c == 'X') ? 55 : 87);
		return 1;
		}
	if (base == 8) {
		length = _duspr(string, n>>3, base, c);
		_duspr(string, n & 7, base, c);
		}
	else if (base == 16) {
		length = _duspr(string, n>>4, base,c);
		_duspr(string, n & 15, base,c);
		}
	else {
		if (n < 0) {
			length='0';
			while (n < 0 || n > 1000000000) {
				n-=1000000000;
				length++;
				}
			*(*string)++=length;
			return _duspr(string,n,base,c)+1;
			}
		length = _duspr(string, n/base, base,c);
		_duspr(string, n%base, base,c);
		}
	return length + 1;
	}

union {long dword; double dval; char bytes[8];};

#define min(x,y) ((x)<(y) ? (x) : (y))

#define	MAXCVT	18

_doprint(int (* put)(), char * f, char ** fmt) {
	char c, base, *sptr, *format, *bp;
	char wbuf[MAXLINE], ecvb[32], *wptr;
	char df, lf, pf, ljflag, zfflag, plusf, blankf, sharpf, rzflag;
	int width, prec,  *args, siz, nchars = 0;
	int dpt, sgn;

	format = *fmt++;    /* fmt first points to the format string	*/
	args = fmt;	    /* now fmt points to the first arg value	*/

	while (c = *format++)
	  if (c == '%') {
	    wptr = wbuf;
	    ljflag = df = lf = pf = zfflag = plusf = blankf = sharpf = rzflag = 0;
   		prec = 6;

		while(1) {
			switch(c = *format) {
				case '-':	ljflag++;
							format++;
							continue;
		    	case '+':	plusf++;
		    				blankf=0;
							format++;
		    				continue;
		    	case ' ':	if(!plusf) blankf++;
							format++;
		    				continue;
				case '#':	sharpf++;
							format++;
							continue;
				case '0':	zfflag++;
							format++;
							continue;
				}
			break;
		    }

	    if(isdigit(c))
	    	width = _gv2(&format);
	    else if(c == '*') {
	    	width = *args++;
	    	if(width < 0) {
	    		ljflag++;
	    		width = -width;
	    		}
	    	format++;
	    	}
	    else
	    	width = 0;

	    if ((c = *format++) == '.') {
	    	df++;
		    if(isdigit(c = *format))
		    	prec = _gv2(&format);
	    	else if(c == '*') {
		    	if((prec = *args++) < 0)
		    		prec = -prec;
		    	format++;
		    	}
		    pf++;
		    if(prec > MAXLINE - 1)
		    	prec = MAXLINE - 1;
			c=*format++;
			}

		if (c == 'l' || c == 'L') {
			lf++;
		    c=*format++;
	     	}

	    switch(c) {

		case 'i':
		case 'd':	if (!lf && *args < 0) {
						*wptr++ = '-';
						*args = -*args;
						width--;
			    		}
					else if (lf && ((void*)args)->dword < 0) {
						*wptr++ = '-';
						((void*)args)->dword = -((void*)args)->dword;
						width--;
			    		}
			    	else if(blankf) {
			    		*wptr++ = ' ';
			    		width--;
			    		}
			    	else if(plusf) {
			    		*wptr++ = '+';
			    		width--;
			    		}

		case 'u':	base = 10; goto val;

		case 'x':
		case 'X':	base = 16;
					if(sharpf) {
						*wptr++ = '0';
						*wptr++ = c;
						width -= 2;
						}
					goto val;

		case 'o':	base = 8;  /* note that arbitrary bases can be
				         added easily before this line */
					if(sharpf) {
						*wptr++ = '0';
						width--;
						}
		     val:	if(df)
		     			zfflag++;
		     		if(lf) {
						width -= _duspr(&wptr,((void*)args)->dword,base,c);
						args+=2;
						}
		     		else 
			     	   width -= _uspr(&wptr,*args++,base,c);
				  	goto pad;

		case 'n':	**(int **)args++=nchars;
					continue;

		case 'p':	width=0;

#ifdef LARGE_CASE
					_tohex(&wptr,*(args+1));
					*wptr++=':';
					_tohex(&wptr,*args++);
					args++;
# else
					_tohex(&wptr,*args++);
#endif
					goto pad;

		case 'E':
		case 'e':
					/*
					 * E-format.  The general strategy
					 * here is fairly easy: we take
					 * what ecvt gives us and re-format it.
					 */

					/* Develop the mantissa */
					bp = ecvt(((void*)args)->dval, min(prec + 1, MAXCVT), &dpt, &sgn, ecvb);

					/* Determine the prefix */
e_merge:
					if (sgn)
						*wptr++ = '-';
					else if (plusf)
						*wptr++ = '+';
					else if (blankf)
						*wptr++ = ' ';

					/* Place the first digit in the buffer*/
					*wptr++ = (*bp != '\0') ? *bp++ : '0';

					/* Put in a decimal point if needed */
					if (prec != 0 || sharpf) {
						*wptr++ = '.';

					/* Create the rest of the mantissa */
						siz = prec;
						for ( ; siz > 0 && *bp; --siz)
							*wptr++ = *bp++;
						while(siz--)
							*wptr++ = '0';
						}

					/* Create the exponent */
					*wptr++ = c;
					dpt--;
					if (dpt < 0) {
						dpt = -dpt;
						*wptr++ = '-';
						}
					else
						*wptr++ = '+';
					for (siz = 1000; siz != 0; siz /= 10)
						if (siz <= dpt || siz <= 10) /* force 2 digits */
							*wptr++ = (dpt / siz) % 10 + '0';
fcom:				*wptr++ = 0;
					width-=strlen(wbuf);
					args+=4;
					goto pad2;
		case 'f':
					/*
					 * F-format floating point.  This is a
					 * good deal less simple than E-format.
					 * The overall strategy will be to call
					 * fcvt, reformat its result into buf,
					 * and calculate how many trailing
					 * zeroes will be required.  There will
					 * never be any leading zeroes needed.
					 */

f_merge:
					/* Do the conversion */
					bp = fcvt(((void*)args)->dval, min(prec, MAXCVT), &dpt, &sgn, ecvb);

					/* Determine the prefix */
					if (sgn && dpt > -prec && *bp != '0')
						*wptr++ = '-';
					else if (plusf)
						*wptr++ = '+';
					else if (blankf)
						*wptr++ = ' ';

					{ 	int mm = 0, nn = dpt;

					/* Emit the digits before the decimal point */
						siz = 0;
						do {
							*wptr++ = (nn <= 0 || *bp == '\0' || siz >= MAXCVT) ?
					    		'0' : (siz++, *bp++);
							} while (--nn > 0);

						/* Decide whether we need a decimal point */
						if (sharpf || prec > 0) {
							*wptr++ = '.';

							/* Digits (if any) after the decimal point */
							nn = min(prec, MAXCVT);
							if (prec > nn)
								mm = prec - nn;
							while (--nn >= 0)
								*wptr++ = (++dpt <= 0 || *bp == '\0' ||
							   	    siz >= MAXCVT) ? '0' : (siz++, *bp++);
							while(mm--)
								*wptr++='0';
							}
						}
					goto fcom;
		case 'g':
		case 'G':
					/*
					 * g-format.  We play around a bit
					 * and then jump into e or f, as needed.
					 */
		
					if (prec == 0)
						prec = 1;

					bp = ecvt(((void*)args)->dval, min(prec + 1, MAXCVT), &dpt, &sgn, ecvb);
					if (((void*)args)->dval == 0)
						dpt = 1;

					{ 	int n, kk = prec;
						if (!sharpf) {
							n = strlen(bp);
							if (n < kk)
								kk = n;
							while (kk >= 1 && bp[kk-1] == '0')
								--kk;
						}
				
						if (dpt < -4 || dpt > prec) {
							prec = kk - 1;
							if(ecvb[kk] >= '5' && ecvb[kk] <= '9') {
								ecvb[kk] = '0';
								for(kk = prec; kk >= 0; kk--)
									if(ecvb[kk] == '9')
										ecvb[kk] = '0';
									else {
										ecvb[kk]++;
										break;
										}
								if(kk == 0 && ecvb[0] == '0') {
									ecvb[0] = '1';
									dpt++;
									}
								}
							c -= 2;
							goto e_merge;
							}
						prec = kk - dpt;
						goto f_merge;
						}

		case 'c':  *wptr++ = *args++;
				   width--;
				   goto pad;

		case 's':  	if (!pf)
						prec = MAXLINE - 1;
			   		sptr = *((char **)args)++;
			   		while (*sptr && prec) {
						*wptr++ = *sptr++;
						prec--;
						width--;
			    		}

		     pad:  *wptr = '\0';
		     pad2:	wptr = wbuf;
				   if (!ljflag) {
				   		if( zfflag 
				   			&& (*wptr == '+' || *wptr == '-' || *wptr == ' ')) {
				   				nchars++;
				   				if((*put)(*wptr++, f) == -1)
				   					return -1;
				   				}
						while (width-- > 0) {
			   				nchars++;
			   				if((*put)(zfflag ? '0' : ' ', f) == -1)
			   					return -1;
							}
						}

				   while (*wptr) {
		   				nchars++;
		   				if((*put)(*wptr++, f) == -1)
		   					return -1;
		   				}

				   if (ljflag)
					while (width-- > 0) {
		   				nchars++;
		   				if((*put)(' ', f) == -1)
		   					return -1;
		   				}
				   break;

		 default:  nchars++;
		   		   if((*put)(c, f) == -1)
		   		   	return -1;
	     }
	  }
	  else { nchars++; if((*put)(c, f) == -1) return -1; }
	return nchars;
}
