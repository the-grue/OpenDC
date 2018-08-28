#include "graphics.h"

Point minHand[360], hrHand[360], ctr, arcPoint();

int facer,						/* clock face radius */
	markr,						/* clock mark radius */
	hrr,						/* hour hand radius */
	hrs,mins, secs,				/* current time */
	hndx, mndx, sndx;			/* time index 0..359 */

char ctime[9];
char blanks[] = "        ";

showtime(){
	char ntime[9];
	int nhrs, nmins, nsecs, nhndx, nmndx, nsndx;

	do
		times(ntime);
	while(strcmp(ntime, ctime) == 0);

	if((nhrs = atoi(ntime)) > 12)
		nhrs -= 12;
	nmins = atoi(&ntime[3]);
	nsecs = atoi(&ntime[6]);

	nhndx = (nhrs * 30) + (nmins >> 1);
	nmndx = (nmins * 6) + (nsecs / 10);
	nsndx = nsecs * 6;

	if(nsndx != sndx)
		line(ctr, minHand[sndx], 0);
	line(ctr, minHand[nsndx], 1);

	if(nmndx != mndx)
		line(ctr, minHand[mndx], 0);
	line(ctr, minHand[nmndx], 2);

	if(nhndx != hndx)
		line(ctr, hrHand[hndx], 0);
	line(ctr, hrHand[nhndx], 3);

	hndx = nhndx;
	mndx = nmndx;
	sndx = nsndx;
	strcpy(ctime, ntime);
	locate(1,1);
	scr_puts(ntime);
	}

main(){
	int i, a;

	screen(1);
	color(0,1);
	color(1,0);
	Fg(1);
	PSET;

	facer = scr_maxy >> 1;
	markr = (facer * 90) / 100;
	hrr = facer >> 1;

	ctr.x = scr_maxx >> 1;
	ctr.y = scr_maxy >> 1;

	for(i = 359, a = 91; i >= 0; i--, a++) {
		minHand[i] = arcPoint(ctr, markr, a);
		hrHand[i] = arcPoint(ctr, hrr, a);
		}
	arc(ctr, facer, 1, 0, 360);

	for(a = 0; a < 360; a += 30)
		line(arcPoint(ctr, markr, a), arcPoint(ctr, facer, a), 1);

	times(ctime);
	
	if((hrs = atoi(ctime)) > 11)
		hrs -= 12;
	mins = atoi(&ctime[3]);
	secs = atoi(&ctime[6]);

	hndx = (hrs * 30) + (mins >> 1);
	mndx = (mins * 6) + (secs / 10);
	sndx = secs * 6;

	line(ctr, minHand[sndx], 1);
	line(ctr, minHand[mndx], 2);
	line(ctr, hrHand[hndx], 3);

	while(csts() == 0)
		showtime();
	screen(0);
	width(80);
	}

