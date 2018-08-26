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
/*	_doscan  --	FORMATTED INPUT.	*/

#define NULL (char*)0
#define EOF -1		/* Physical EOF returned by low level I/O functions */
#define ERROR -1	/* General "on error" return value */
#define OK 0		/* General purpose "no error" return value */
#define TRUE 1		/* general purpose true truth value	*/
#define FALSE 0		/* general purpose false truth value 	*/
#define	NCHARS	256	/* scanset size	*/

union {long dword; char byte;};

char toupper(), isdigit();

static int last = -1, (*cf)(), *cfa, eof, lch;

/*
	Internal function to get the next character
*/

static int iget() {

	if(eof) return EOF;
	if(last != -1) {
		char r = last;
		last = -1;
		return r;
		}
	if((lch=cf(cfa)) == EOF)
		eof++;
	return lch;
	}

/*
	Internal function to position the character
	pointer argument to the next non white-space
	character in the string:
*/

static int  _igs(){
	char c;
	while (isspace(c = iget()));
	return (c);
	}


/*
	Internal function to convert character c to value
	in base b , or return ERROR if illegal character for that
	base:
*/

static int _bc(c,b)
char c,b;{
	if (isalpha(c = toupper(c))) c -= 55;
         else  if (isdigit(c))  c -= 0x30;
	 else return ERROR;
	if (c > b-1) return ERROR;
		else return c;
	}

/*
	sets up the scanset table
*/

static char * setup(fmt, tab)
char *fmt, *tab; {
	int b, c, d, t = 1;

	if(*fmt == '^') {
		t--;
		fmt++;
		}

	_setmem(tab, NCHARS, !t);

	if((c = *fmt) == ']' || c == '-') { /* first char is special */
		tab[c] = t;
		fmt++;
		}

	while((c = *fmt++) != ']') {
		if(c == '\0')
			return(NULL); /* unexpected end of format */
		if(c == '-' && (d = *fmt) != ']' && (b = fmt[-2]) < d) {
			_setmem(&tab[b], d - b+1, t);
			fmt++;
			}
		else
			tab[c] = t;
		if(c == '\n')
			tab[0] = t;
		}
	return(fmt);
	}


/*
	General formatted input conversion routine. "line" points
	to a string containing ascii text to be converted, and "fmt"
	points to an argument list consisting of first a format
	string and then a list of pointers to the destination objects.

	Appropriate data is picked up from the text string and stored
	where the pointer arguments point according to the format string.
	See K&R for more info. The field width specification is not
	supported by this version.

	NOTE: the "%s" termination character has been changed
	from "any white space" to the character following the "%s"
	specification in the format string. That is, the call

		sscanf(string, "%s:", &str);

	would ignore leading white space (as is the case with all
	format conversions), and then read in ALL subsequent text
	(including newlines) into the buffer "str" until a COLON
	or null byte is encountered.

*/

int _doscan(int (*pf)(), int *pa, char ungetf, char **fmt) {
	char sf, longf, s, base, n, *sptr, *format, *fend, lchok;
	int c, sign, val, **args;
	unsigned curlen, maxlen;
	long dval;
	struct {char lobyte,hibyte;};
	union {double dftype; float ftype;};
	double tfloat, strtod();

	cf = pf;
	cfa = pa;
	last = -1;
	eof = 0;
	format = *fmt++;	/* fmt first points to the format string */
	args = fmt;		/* now it points to the arg list */

	n = 0;
	while (c = *format++) {
		if (isspace(c)) continue;	/* skip white space in format string */
		if (c != '%') {		/* if not %, must match text */
			if (c != _igs()) goto retn;
			lchok = 0;
			continue;
		    }
		else {		/* process conversion */
			lchok = sign = 1;
			base = 10;
			sf = longf = 0;
			if ((c = *format++) == '*') {
				sf++;		/* if "*" given, supress assignment */
				c = *format++;
				}
			curlen=0;
			maxlen=-1;		/* assume no maximum length	*/
			if (c >= '0' && c <= '9') {
				maxlen=c-'0';
				while (isdigit(c=*format++)) {
					maxlen*=10;
					maxlen+=c-'0';
					}
				}

			if (toupper(c) == 'L')
				c = toupper(*format++);
			else if (toupper(c) == 'H')
				c = tolower(*format++);
			else
				c = tolower(c);

			switch (c) {
				case 'x':	base = 16;
			    			goto doval;

				case 'o':	base = 8;
							goto doval;

				case 'i':
				case 'd':	if ((s=_igs()) == '-' || s == '+') {
								sign = s == '-' ? -1 : 1;
								curlen=1;
								}
							else if(eof)
									if(n == 0) return EOF;
									else goto retn;
							else last = s;

doval:			case 'u':	if(curlen++ >= maxlen ||
									(val = _bc(_igs(),base)) == ERROR)
								if(eof && n == 0) return EOF;
								else goto retn;
							while ((c = _bc(iget(),base)) != ERROR &&
									curlen++ < maxlen)
								val = val * base + c;
							last = lch;
							if (!sf) {
								**args++ = val * sign;
								n++;
								}
							break;

				case 'X':	base = 16; goto dodval;

				case 'O':	base = 8;  goto dodval;

				case 'I':
				case 'D':	if ((s=_igs()) == '-' || s == '+') {
								sign = s == '-' ? -1 : 1;
								curlen=1;
								}
							else if(eof)
									if(n == 0) return EOF;
									else goto retn;
							else
								last = s;

	  dodval:	case 'U':	longf=1;
							if (curlen++ >= maxlen ||
									(dval=_bc(_igs(),base)) == ERROR)
								if(eof && n == 0) return EOF;
								else goto retn;
							while ((c = _bc(iget(),base)) != ERROR &&
									 curlen++ < maxlen)
								dval = dval * base + c;
							last = lch;
							if (!sf) {
								((void*)(*args++))->dword = dval * sign;
								n++;
								}
							break;


				case 'e':
				case 'E':


				case 'f':
				case 'F':


				case 'g':
				case 'G':	if ((s = _igs()) == '-') {
								sign = -1;
								curlen=1;
								s = iget();
								}
							else if(s == '+') {
								sign = 1;
								curlen=1;
								s = iget();
								}
							else if(eof)
									if(n == 0) return EOF;
									else goto retn;
							{
								char buf[32], *cp = buf, *ep = &buf[30];
							
								while(isdigit(s)) {
									if(cp > ep) goto stopf;
									*cp++ = s;
									s = iget();
									}
								if(s == '.') {
									if(cp > ep) goto stopf;
									*cp++ = s;
									s = iget();
									}
								while(isdigit(s)) {
									if(cp > ep) goto stopf;
									*cp++ = s;
									s = iget();
									}
								if(s == 'E' || s == 'e') {
									if(cp > ep) goto stopf;
									*cp++ = s;
									s = iget();
									if(s == '+' || s == '-') {
										if(cp > ep) goto stopf;
										*cp++ = s;
										s = iget();
										}
									while(isdigit(s)) {
										if(cp > ep) goto stopf;
										*cp++ = s;
										s = iget();
										}
									}
stopf:							*cp = 0;
					 			if (cp == buf)
					 				if(eof && n == 0) return EOF;
									else goto retn;
					 			last = s;
								tfloat = strtod(buf, (char)0);
								if (sign < 0) tfloat=-tfloat;
								if (!sf) {
									if (islower(c))
										((void*)(*args++))->ftype=tfloat;
									else
										((void*)(*args++))->dftype=tfloat;
									n++;
									}
								break;
							}

				case 's':
				case 'S':	c = _igs();
							if(eof)
								if(n == 0) return EOF;
								else goto retn;
							sptr = *args;
							while (curlen++ < maxlen &&
									!isspace(c)) {
								if (c != '%' && c == *format) {
									format++;
									break;
									}
								if (!sf)
									*sptr++ = c;
								c = iget();
								if(eof)
									if(n == 0) return EOF;
							    	else break;
								}				
							last = c;
						    if (!sf) {
								n++;
								*sptr = '\0';
								args++;
						    	}
							if(eof) goto retn;
					    	break;

				case '[':	{
								char tab[NCHARS];

								if((format = setup(format, tab)) == NULL)
									return EOF;
								sptr = *args;
								while (curlen++ < maxlen && (c=iget()) != ERROR
										&& tab[c])
									if (!sf)
										*sptr++ = c;
								last = c;
							    if (!sf) {
									n++;
									*sptr = '\0';
									args++;
									}
								if(eof)
									if(n == 0) return EOF;
									else goto retn;
								break;
								}

				case 'c':
				case 'C':	if(maxlen == -1)
								maxlen = 1;
							sptr = *args;
							while (curlen++ < maxlen && (c=iget()) != ERROR)
								if (!sf)
									*sptr++ = c;
							if(eof)
								if(n == 0) return EOF;
								else goto retn;
							last = -1;
							last = iget();
							if (!sf) {
								n++;
								if(maxlen != 1)
									*sptr = '\0';
								args++;
								}
							break;

				default:	goto retn;
				}
			}
		if (last == ERROR) goto retn;	/* if end of input string, return */
		}
retn:	if(ungetf && lchok) ungetc(lch, pa);
	return n;
	}
