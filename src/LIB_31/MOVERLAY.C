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
/*	MOVERLAY.C	--	Load an Overlay from Memory.	*/

	/*	header for overlays. at start of .OV file. followed by code,data
		for each overlay except 0 (root).	*/
	/*	ovlen is length of data, code and reserved for each overlay.	*/

#define MAXOVER	40
	static unsigned ovlen[MAXOVER][3];
	static char num_overlay;

/*	MOVERLAY	--	load in the overlay	*/

static get_mem();
moverlay(ov)
	int  ov; {
	int  i;
	unsigned dataat,codeat,buflen,need,nread,*ptr,bufferat;
	long seekto;

	if (ov == num_overlay) return 1;
	get_mem(sizeof(ovlen),0l,ovlen,_showds());

	if (ov <= 0 || ov >= MAXOVER ||
		ovlen[ov][0]+ovlen[ov][1]+ovlen[ov][2] == 0) return -1;

	/*	get code and data base addresses	*/

	dataat=ovlen[0][0]+ovlen[0][2];	/*	base of overlay daya	*/
	codeat=ovlen[0][1];				/* base of overlay code	*/
	seekto=sizeof(ovlen);
	for (i=1; i < ov; i++) {
		seekto+=ovlen[i][0];
		seekto+=ovlen[i][1];
		}
	if (ovlen[ov][1]) {		/* read in the code	*/
		get_mem(ovlen[ov][1],seekto,codeat,_showcs());
		}

	if (ovlen[ov][0]) {		/* read in the data	*/
		get_mem(ovlen[ov][0],seekto+ovlen[ov][1],dataat,_showds());
		}

	if (ovlen[ov][2]) {		/* clear reserved	*/
		ptr=dataat+ovlen[ov][0];
		i=(ovlen[ov][2]+1)>>1;
		while (i--) *ptr=0;
		}
	num_overlay=ov;
	return 1;
/*	need a int 3 for d88	*/
#asm
		public	_movint3_
_movint3_:	int	3
#
	}


/*	GET_MEM  --	Copy memory down for MEMOVERLAY	*/

static get_mem(size,into_mem,to_off,to_seg)
	int size;
	long into_mem;
	unsigned to_off,to_seg; {
	unsigned *wp,from_seg,from_off;

	wp=4;			/* start of overlay stuff is at ds:[4] */
	into_mem+=*wp;
	from_seg=_showcs();
	from_seg+=into_mem>>4;
	from_off=into_mem & 15;
	_lmove(size,from_off,from_seg,to_off,to_seg);
	}

