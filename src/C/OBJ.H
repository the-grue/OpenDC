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
/*	OBJ.H			object record types	*/

/*	P = found in pipe.	*/
/*	O = found in .O file. */

#define OEOF		0	/* PO		end of file. */
#define OPUBLIC		1	/* O		name,0, number. */
#define ORESERVE	2	/* O		number, # of bytes. */
#define OLOCAL		3	/* PO		number (,segment, offset in .O). */
						/* the above 3 must preceed the rest in .O files */
#define ODSEG		4	/* O		following in DSEG. */
#define OCSEG		5	/* O		following in CSEG. */
#define ONAMEREL	6	/* PO		number of extrn or local. */
#define OJUMPREL	7	/* PO		number of extrn or local. */
#define OJUMP		8	/* P		jump type, label number. */
#define OEVEN		9	/* P		even allign */
#define OLIST		10	/* L		OLIST+n bytes of object follow. */
#define OPTYPE		11	/* O		name of public, type. */
#define OLTYPE		12	/* O		name of local, type, offset from BP. */
#define OMTYPE		13	/* O		name of public, type, offset from start. */
#define ONAME		14	/* O		file name. file is now open.*/
#define OLNAME		15	/* O		name of function for locals, type. */
#define OLINE		16	/* PO		line number. also location in .CHK. */
#define OOV			17	/*			overlay number. in .CHK only.	*/
#define OSTATIC		18	/* O		name,0, number. */

#define	OESEG		19	/* O		following in ESEG */
#define OBIG		20	/* O		BIG object file */
#define OLNAMEREL	21	/* PO		number of extrn of local. need 4 byte fixup. */
#define OSEGPTR		22	/* O		number of extrn or local. need segment
									fixup. */
#define OPTR		23	/* O		number of extrn or local. need offset of
									the 4 byte pointer that is in dseg. */
#define ODFIX		24	/* O        data segment fixup needed. */
#define OCFIX		25	/* O        code segment fixup needed. */

/*			129 to 228	   PO		bytes of contents -128, contents. */
