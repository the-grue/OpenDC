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
/*	OVERLAY.C	--	Load an Overlay.	*/

	/*	header for overlays. at start of .OV file. followed by code,data
		for each overlay except 0 (root).	*/
	/*	ovlen is length of data, code and reserved for each overlay.	*/

#define MAXOVER	40
	static unsigned ovlen[MAXOVER][3];
	static int  fid=-1;
	static char num_overlay;
	extern char _msdos2;

/*	OVERLAY_INIT	--	Initilize for Overlays.	*/
/*						The name is the name of the overlay file.	*/

overlay_init(name)
	char *name; {
	char path[65];
	if ((fid=findfile("PATH", name, path)) == 0) return -1;
	fid = open(path, 0);
	return read(fid,ovlen,sizeof(ovlen)) == sizeof(ovlen) ? 1: -1;
	}


/*	OVERLAY	--	load in the overlay	*/

overlay(ov)
	int  ov; {
	int  i;
	unsigned dataat,codeat,buflen,need,nread,*ptr,bufferat;
	long seekto;
	char buffer[128];

	if (ov == num_overlay) return 1;
	if (ov <= 0 || ov >= MAXOVER || fid == -1 ||
		ovlen[ov][0]+ovlen[ov][1]+ovlen[ov][2] == 0) return -1;

	/*	get code and data base addresses	*/

	dataat=ovlen[0][0]+ovlen[0][2];	/*	base of overlay daya	*/
	codeat=ovlen[0][1];				/* base of overlay code	*/
	seekto=sizeof(ovlen);
	for (i=1; i < ov; i++) {
		seekto+=ovlen[i][0];
		seekto+=ovlen[i][1];
		}
	lseek(fid,seekto,0);
	if (ovlen[ov][1]) {		/* read in the code	*/
		bufferat=dataat;
		buflen=_memory()-dataat;
		if (buflen > 512) buflen&=0xfe00;	/* read 512's	*/
		else if (buflen < 128) {
			bufferat=buffer;
			buflen=sizeof(buffer);
			}

		need=ovlen[ov][1];
		do {
			nread=need > buflen ? buflen: need;
			if (read(fid,bufferat,nread) != nread) return -1;
			_lmove(nread,bufferat,_showds(),codeat,_showcs());
			codeat+=nread;
			need-=nread;
			}
		while (need);
		}

	if (ovlen[ov][0]) {		/* read in the data	*/
		if (read(fid,dataat,ovlen[ov][0]) != ovlen[ov][0]) return -1;
		}

	if (ovlen[ov][2]) {		/* clear reserved	*/
		ptr=dataat+ovlen[ov][0];
		i=(ovlen[ov][2]+1)>>1;
		while (i--) *ptr=0;
		}
	num_overlay=ov;
	return 1;
	}

/*	OVERLAY_CLOSE  --	close the overlay file.		*/

overlay_close() {
	int temp_fid;

	temp_fid=fid;
	fid=-1;
	return close(temp_fid);
	}
