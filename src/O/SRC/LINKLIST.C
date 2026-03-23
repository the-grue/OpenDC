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
		       /*  Start of file LINKLIST.C  */

/*
		 O88 Optimizer for DeSmet (C-Ware) C-Compiler
		 --------------------------------------------

			      Copyright (c) 1986

				      by

			    Key Software Products
		    440 Ninth Avenue, Menlo Park, CA 94025
				(415) 364-9847

			    (All rights reserved)


*/

#include	"inc\o88.h"

LINKLIST *LL_Free(head, off_cond, off_xtra)
CHAR *head ;
unsigned off_cond ;	/* Stop if this member NULL	*/
unsigned off_xtra ;	/* Delete this member too	*/
	{
	LINKLIST *next ;

	while (head != NULL)
		{
		if (off_cond != 0 &&
		    (*((CHAR **) (head + off_cond)) != NULL)) break ;
		next = ((LINKLIST *) head)->next ;
		if (off_xtra != 0)
			{
			My_Free(*((CHAR **) (head + off_xtra))) ;
			}
		My_Free(head) ;
		head = (CHAR *) next ;
		}
	return (LINKLIST *) head ;
	}

LINKLIST *LL_Search(head, off, str)
LINKLIST **head ;
unsigned off ;
CHAR *str ;
	{
	LINKLIST *entry, *prev ;

	if (*head == NULL) return NULL ;

	prev = (LINKLIST *) head ;
	for (entry = prev->next; entry!=NULL; entry = (prev=entry)->next)
		{
		if (Equal(((CHAR *) entry) + off, str)) break ;
		}

	if ((entry != NULL) && (prev != ((LINKLIST *) head)))
		{
		/*
			Move entry to the beginning of the list!
		*/
		prev->next = entry->next ;
		entry->next = *head ;
		*head = entry ;
		}

	return entry ;
	}

			/*  End of file LINKLIST.C  */

