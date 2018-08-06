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
/*	CHSIZE.C	--	change the size of a file	*/

chsize(file,size)
	char *file;
	long size; {
	int  fil,i,err;
	char zero[128];
	long  location;

	fil=_open("X",2);
	if (fil == -1) return -1;
	location=_lseek(fil,0l,2);
	if (location < size) {
		for (i=0; i < 128; i++)
			zero[i]=0;
		while (location < size) {
			if (size-location > 128) i=128;
			else i=size-location;
			err=_write(fil,zero,i);
			location+=i;
			}
		}
	else {
		_lseek(fil,location,0);
		err=_write(fil,0,0);
		}
	return err;
	}
		
main() {
	int  fil;

	fil=open("x");
	chsize(fil,100l);
	
	}
