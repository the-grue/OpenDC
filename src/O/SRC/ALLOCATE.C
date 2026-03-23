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
			   FILE: ALLOCATE.C

This set of three routines provides a VERY high-speed memory allocation
scheme when you have an application in which most or all of the allocated
blocks are either (1) all about the same size, or (b) smaller than some
(small) maximum size.

It achieves it's speed by maintaining these blocks as a one-way linked
list.  When you need to allocate a block, it is simply removed from the 
head of the list; when you free a block it is simply inserted at the
head, and will thus be the next block that is allocated.  This avoids the
need to search through the heap (as done by malloc), looking for a free
block large enough to satisfy the request.  The tradeoff (of course) is
that the implementation requires that all blocks be of the same size, and
this requires that the requested size is always less than or equal to the
allocation size.

This implementation allows for larger blocks by reserving a portion of the
heap so that these blocks can be allocated by malloc() in the traditional
manner.  Note, however, that performance will suffer if malloc() is called
frequently.

I have added a useful debugging feature that checks to see that a block is
allocated before it can be freed.  For the fixed-size blocks, this amounts
to checking to see if the block pointer points into the array of fixed-size
blocks;  for malloc'd blocks, I rely on the fact that the DeSmet 
implementation of free() returns 1 on success and 0 on failure (block was not
freed because it had not been allocated).  Note that other compilers do not
necessarily implement this feature of free().  One notable case is Microsoft.

This debugging tool can help discover that you have accidentally written over
malloc's control information that resides immediately before the data.  It has
saved me more than once!

I strongly suggest that before you implement this allocation scheme, that you
add code to your existing program to gather statistics on the number and size
of malloc() requests.  That way, you can intelligently set the #define for
BLKSIZ below, which is the size of the fixed-size blocks that will be used.

The #define "STKSIZ" specifies how many bytes of stack space will be used;
more to the point, this establishes an upper limit for malloc().  The value
of 1000 will have to be increased if you have any large non-static arrays
declared inside of functions - these are always allocated on the stack. 
Perhaps a better solution would be to make them static and leave STKSIZ 
at 1000.

The #define "MLCSIZ" specifies how much room is left in the heap for malloc()
calls.  Hopefully you won't be doing much of this, so a reasonably small value
would be appropriate.  Of course it will also depend on how large your large
requests are going to be.

			  Copyright (c) 1987

			   Daniel W. Lewis

			Key Software Products
			   440 Ninth Avenue
			 Menlo Park, CA 94025
			    (415) 364-9847

*/

#include "inc\o88.h"

#define	BLKSIZ	80	/* Size of allocation block	*/
#define	MLCSIZ	3000	/* Bytes reserved for malloc()	*/

#define	SIGNATURE	0x5A	/* Identifies allocated block	*/

typedef	union BLOCK
	{
	CHAR	*link ;
	CHAR	data[BLKSIZ] ;
	} BLOCK ;

#ifdef	SPEEDY
PRIVATE	CHAR	*btm_of_heap ;
PRIVATE CHAR	*top_of_heap ;
PRIVATE BLOCK	*heap ;
#endif

VOID Init_Heap(stack_size)
unsigned stack_size ;
	{
#ifdef	SPEEDY
	unsigned *ptr ;
	unsigned size ;
	unsigned blk ;
	unsigned ttl_blks ;

	freeall(stack_size) ;		/* Reserve a stack of 500 words */
	ptr = _memory() + 1 ;		/* Get pointer to heap size	*/
	size = *ptr ;			/* Get size of heap		*/
	size -= MLCSIZ ;		/* Keep 3K for malloc()		*/
	ttl_blks = size/sizeof(BLOCK) ;	/* # available blocks		*/
	size = sizeof(BLOCK)*ttl_blks ;	/* Use an even multiple		*/
	btm_of_heap = malloc(size) ;	/* Reserve the blocks		*/

	heap = btm_of_heap ;		/* Form the free list		*/
	for (blk = 0 ; blk < ttl_blks - 1; blk++)
		{
		heap->link = heap + 1 ;
		heap++ ;
		}
	heap->link = NULL ;		/* Establish end marker		*/
	heap = btm_of_heap ;		/* Restore head pointer		*/
	top_of_heap = &heap[ttl_blks] ;	/* 1st malloc() block		*/
#else
	freeall(stack_size) ;
#endif
	}


VOID My_Free(ptr)
CHAR *ptr ;
	{
#ifdef	SPEEDY
	if (ptr < btm_of_heap)
		{
		goto error ;
		}

	if (ptr < top_of_heap)	/* Block is from my heap	*/
		{
		CHAR *p = ptr - 1 ;

		if ((*p = SIGNATURE) != NULL)  /* Back up & check sig.	*/
			{
			((BLOCK *) p)->link = heap ;
			heap = p ;
			return ;
			}
		}
#endif
	if (free(ptr) != 0)	/* Block is from C-Ware's heap	*/
		{
		return ;
		}

#ifdef	SPEEDY
error:
#endif
	Errs("\r\nAttempt to free unallocated memory!\r\n\7") ;
	exit(ERRORS) ;
	}


CHAR *Allocate(bytes)
unsigned bytes ;
	{
	CHAR *ptr ;

#ifdef	SPEEDY
	if (bytes < BLKSIZ)		/* Block will fit in my size	*/
		{
		if (heap != NULL)	/* Any free blocks?		*/
			{
			ptr = heap ;
			heap = heap->link ;
			*ptr++ = SIGNATURE ;
			return ptr ;
			}
		}
#endif

	if ((ptr = malloc(bytes)) == NULL) /* Won't fit or none left!	*/
		{
		Errs("\r\nInsufficient memory for heap!\r\n\7") ;
		exit(ERRORS) ;
		}

	return ptr ;
	}


