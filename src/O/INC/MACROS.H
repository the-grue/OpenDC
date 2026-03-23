/*
 *  Released under the GNU GPL.  See http://www.gnu.org/licenses/gpl.txt
 *
 *  This program is part of the DeSmet C Compiler
 *
 *  DeSmet C is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundatation; either version 2 of the License, or any
 *  later version.
 *
 *  DeSmet C is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 */
/*
			        File: MACROS.H
			        --------------

			      Copyright (c) 1987

				      by

			    Key Software Products
		    440 Ninth Avenue, Menlo Park, CA 94025
				(415) 364-9847

			    (All rights reserved)

This file contains some well-known macros that can be used to
replace certain library functions by in-line code that runs faster,
but eats up more code segment space.

*/

/* ---------------------------------------------------- */
/* The following temporary variable is used to prevent	*/
/* expansions of expressions used as arguments.		*/
/* ---------------------------------------------------- */
#ifndef _c
static unsigned char _c ;
#endif


/* ---------------------------------------------------- */
/* The following is here to intentionally trigger	*/
/* a compiler error message if any of the temporary	*/
/* macros defined below are already defined...		*/
/* ---------------------------------------------------- */
#ifdef	_isupper
	ERROR MACROS.H
#endif
#ifdef	_islower
	ERROR MACROS.H
#endif
#ifdef	_isdigit
	ERROR MACROS.H
#endif
#ifdef	_iscntrl
	ERROR MACROS.H
#endif
#ifdef	_isprint
	ERROR MACROS.H
#endif
#ifdef	isdigit
	ERROR MACROS.H
#endif
#ifdef	isupper
	ERROR MACROS.H
#endif
#ifdef	islower
	ERROR MACROS.H
#endif
#ifdef	isascii
	ERROR MACROS.H
#endif
#ifdef	iscntrl
	ERROR MACROS.H
#endif
#ifdef	isprint
	ERROR MACROS.H
#endif
#ifdef	isalpha
	ERROR MACROS.H
#endif
#ifdef	isalnum
	ERROR MACROS.H
#endif
#ifdef	ispunct
	ERROR MACROS.H
#endif
#ifdef	CASE_SHIFT
	ERROR MACROS.H
#endif
#ifdef	tolower
	ERROR MACROS.H
#endif
#ifdef	toupper
	ERROR MACROS.H
#endif

/* ---------------------------------------------------- */
/* These macros access the above temporary variable	*/
/* instead of the original argument.  They were created	*/
/* merely to facilitate the definition of the real	*/
/* macros that follow.  DO NOT USE THESE IN YOUR CODE!	*/
/* ---------------------------------------------------- */
#define	_isupper(c)	(c >= 'A' && c <= 'Z')
#define	_islower(c)	(c >= 'a' && c <= 'z')
#define	_isdigit(c)	(c >= '0' && c <= '9')
#define	_iscntrl(c)	(c == 0x7F || c < 0x20)
#define	_isprint(c)	(c >= ' ' && c <= '~')


/* ---------------------------------------------------- */
/* Here are the macros that you use in your code...	*/
/* ---------------------------------------------------- */
#define	isdigit(c)	((_c = (c)), _isdigit(_c))
#define	isupper(c)	((_c = (c)), _isupper(_c))
#define	islower(c)	((_c = (c)), _islower(_c))
#define	isascii(c)	(((c) & 0x80) == 0)
#define	iscntrl(c)	((_c = (c)), _iscntrl(_c))
#define	isprint(c)	((_c = (c)), _isprint(_c))
#define	isalpha(c)	((_c = (c)), (_isupper(_c) || _islower(_c)))
#define	isalnum(c)	(isalpha(c) || _isdigit(_c))
#define	ispunct(c)	(!isalnum(c) && !_iscntrl(_c))

#define	CASE_SHIFT	('A' - 'a')

#define	tolower(c)	(isupper(c) ? ((_c) - CASE_SHIFT) : (_c))
#define	toupper(c)	(islower(c) ? ((_c) + CASE_SHIFT) : (_c))


		     /* End of file: MACROS.H */
