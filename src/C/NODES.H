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
/*	TYPES GO INTO THE DTYP FIELD OF NODES THAT NEED A TYPE */


#define CCHAR		1
#define CINT		2
#define CUNSG		3
#define CLONG		4
#define CFLOAT		5
#define CDOUBLE		6
#define	CSCHAR		7
#define CSHORT		8
#define	CULONG		9
#define CLABEL		10
#define CSTRUCT		11	/* FOLLOWED BY POINTER TO STAG IN SYMBOL TABLE.	*/
#define BITS		12	/* dtype is BITS+(#bits-1)*16+bit offset */

/*	THE FOLLOWING ARE THE TYPE MODIFIERS	*/
#define	FPTR2		251
#define	HPTR2		252
#define FUNCTION	253
#define ARRAY		254	/* FOLLOWED BY DIMENSION */
#define PTRTO		255

/*	Types of initilization records	*/

#define	INITEND	10
#define	INITDB	11	/* int number of bytes to zero */
#define	INITRB	12	/* int number of bytes to RB */
#define	INITVAL 13	/* type byte, node to keep, nodes needed, start node */
#define	INITSTR	14	/* byte number of bytes that follow */
#define INITFUN 15	/* nothing follows - not even INITEND */
/*	Define Node Types	*/

struct nod{
	char ntyp;
	char dtyp;
	int  subn1;
	int  subn2;
	char onext;
	char pat; };

#define OPND	0	/* operand.			DTYPE, BP offset or ordinal,
										structure offset, zero. */
#define CONST	1	/* constant.		DTYPE (1, 2 or 4), value. */
										/* value 2 words if CLONG */
#define LAB		2	/* label.			node, ordinal of label. */
#define CAST	3	/* cast.			DTYPE, operand. */
#define TA		4	/* take address (&).operand. */
#define IND		5	/* indirect(*).		DTYPE, operand. */
#define IFE		6	/* if then else.	test, then stmt, else stmt or 0 */
#define WHIL	7	/* while.			test, stmt, 0, 0, 0, 0 */
#define DOW		8	/* do while.		stmt, test, 0, 0, 0, 0 */
#define SWIT	9	/* switch.			test, stmt. */
#define FORS	10	/* for.				pre, test, post, stmt, 0, 0, 0, 0*/
#define RET		11	/* return.			value or zero. */
#define GOTOS	12	/* goto.			label node. */
#define CONT	13	/* continue.		*/
#define BRK		14	/* break.			*/
#define LST		13	/* nodes 15 to 19 are list nodes with 2 to 6 subn */
#define CAS		20	/* case label.		node, value. */
#define DFLT	21	/* default label	node. */
#define CALL	22	/* call.			dtype,function ord, arguments or 0,
										return structure length or 0. */
#define TEST	23	/* conditional(? :).dtype, test, true val, false val. */
#define ADD		24	/* add.				left node, right node. */
#define SUB		25	/* subtract.		left node, right node. */
#define MUL		26	/* multiply.		left node, right node. */
#define DIV		27	/* divide.			left node, right node. */
#define MOD		28	/* modulo.			left node, right node. */
#define SHR		29	/* shift right.		left node, right node. */
#define SHL		30	/* shift left.		left node, right node. */
#define AND		31	/* binary and (&).	left node, right node. */
#define OR		32	/* binary or (|).	left node, right node. */
#define XOR		33	/* exclusive or.	left node, right node. */
#define LOR		34	/* logical or (||).	left node, right node. */
#define LAND	35	/* logical and (&&).left node, right node. */
#define EQ		36	/* equality (==).	left node, right node. */
#define NE		37	/* not equal (!=).	left node, right node. */
#define GE		38	/* greater or equal.left node, right node. */
#define LT		39	/* less than.		left node, right node. */
#define GT		40	/* greater than.	left node, right node. */
#define LE		41	/* less or equal.	left node, right node. */
#define NEG		42	/* negate.			node. */
#define NOT		43	/* not.				node. */
#define COMP	44	/* compliment.		node. */
#define PREI	45	/* pre increment(++).  node, add value. */
#define PRED	46	/* post decrement(++). node, subtract value. */
#define POSTI	47	/* pre increment(--).  node, add value. */
#define POSTD	48	/* pre decrement(--).  node, subtract value. */

/*	THE FOLLOWING ARE THE ASSIGNMENT NODES */

#define ASGN	49	/* assignment.		left node, right node. */
#define AADD	50	/* add (+=).		left node, right node. */
#define ASUB	51	/* subtract (-=).	left node, right node. */
#define AMUL	52	/* multiply (*=).	left node, right node. */
#define ADIV	53	/* divide (/=).		left node, right node. */
#define AMOD	54	/* modulo (%=).		left node, right node. */
#define ASHR	55	/* shift right (>>=). left node, right node. */
#define ASHL	56	/* shift left (<<=).left node, right node. */
#define AAND	57	/* binary and (&=).	left node, right node. */
#define AOR		58	/* binary or (|=).	left node, right node. */
#define AXOR	59	/* exclusive or.	left node, right node. */

#define NULL	60	/* used as null type in generation */
#define OUTASM	61	/* assembler follows. ends with byte=255. */
#define STMT	62	/* line number follows	*/
#define MOVE	63	/* move structure.   left,right,length. right=0 for push.*/
