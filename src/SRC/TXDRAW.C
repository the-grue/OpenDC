#include	<graphics.h>

char A[]="M+2,-1M+2,-1M+3,-1R7";		/* U L CORNER */
char B[]="R3M+2,1M+1,1D2";				/* U R CORNER */
char C[]="M-1,1M-2,2M-2,1M-3,1M-3,1L4"; /* L R CORNER */
char D[]="L3M-3,-1M-2,-1M-1,-1U1M+1,-1";/* L L CORNER */
char L[]="NM320,182C0NM320,182C1";		/* LASER ORIGIN POINT */
char LL[]="NM320,182NU4ND5NL10NR10NE4NF5NG5NH4C0NM320,182NU4ND5NL10NR10NE4NF5NG5NH4C1"; /* SPARKLER ON END OF LASER */

main()
	{
	screen(2);
	cls();
	locate(25,17);
	scr_printf("(C) Copyright COMPAQ Computer Corporation 1982, 83");

/* "C" */
	draw("BM300,2;X",LL,"R21;X",L,"M-7,7;X",L,"L17;X",L,"M-10,10;X",L,
		 "R17;X",L,"M-6,6;X",L,"L18;X",D,L,"M+15,-15;X",L,A,L,
		 "R20M-7,7L17M-10,10R17M-6,6L18;X",D,"M+15,-15;X",A,0L);

/* "O" */
	draw("BM334,2",LL,"R21",L,B,L,"M-13,13",L,C,L,
		 "L17",L,D,L,"M+15,-15",L,A,L,"BM-3,6",L,
		 "R11",L,"M-11,11",L,"L11",L,"M+11,-11",L,
		 "M-11,11R11M+11,-11L11BM334,2R21",B,"M-13,13",C,"L17",D,
		 "M+15,-15",A,0L);

/* "M" */
	draw("BM379,2",LL,"R37",L,B,L,"M-19,19",L,"L14",L,
		 "M+17,-17",L,"L7",L,"M-17,17",L,"L14",L,"M+17,-17",L,
		 "NM-17,17L7",L,"M-17,17",L,"L14",L,"M+20,-20",L,A,
		 "R35",B,"M-19,19L14M+17,-17L7M-17,17L14M+17,-17L7M-17,17L14",0L);

/* "P" */
	draw("BM440,2",LL,"R20",L,B,L,"M-7,7",L,C,L,
		 "L11",L,"M+5,-5",L,"R7",L,"M+6,-6",L,"L12",L,"M-17,17",L,
		 "L14",L,"M+20,-20",L,A,L,0L);

/* REDRAW UPPER "P" */
	draw("BM440,2R20",B,"M-7,+7",C,
		 "L11M+5,-5R7M+6,-6L12M-17,17L14M+20,-20",A,0L);

/* "A" */
	draw("BM485,2",LL,"R17",L,"M-6,23",L,"L14",L,"M+1,-4L9",L,
		 "M+7,-5",L,"R4",L,"M+2,-6",L,"M-23,+15",L,"L17",L,
		 "M+37,-23",L,"M-37,23R17M+23,-15M-2,6L4M-7,5R9;",0L);

/* "Q" */
	draw("BM530,2",LL,"R20",L,B,L,"M-13,13",L,"R10",L,
		 "M-6,6",L,"L36",L,D,L,"M+15,-15",L,
		 "M-15,15BM+15,-15",A,L,"BM-3,6",L,"R11",L,"M-11,11",L,
		 "L11",L,"M+11,-11",L,"M-11,11BM530,2R20",B,
		 "M-13,13R10M-6,6L36",D,0L);

/* UNDERLINE */
	draw("BM140,30",LL,"R402",LL,"L402R402M-6,6",LL,
		 "M+6,-6M-6,6L402",LL,"M+6,-6",LL,"BM140,30;R402M-6,6L402M+6,-6;",0L);

/* PAINT TOP PART OF LOGO */
	paint(291,6,1,1);
	paint(354,6,1,1);
	paint(400,6,1,1);
	paint(460,6,1,1);
	paint(488,6,1,1);
	paint(488,6,1,1);
	paint(550,6,1,1);
	paint(450,34,1,1);

/* TRADEMARK */
	draw("BM587,03",LL,0L);
	locate(1,73);
	scr_printf("T");
	draw("BM590,03",LL,0L);
	locate(1,73);
	scr_printf("TM");
	ci();
	screen(0);
	width(80);
	}
