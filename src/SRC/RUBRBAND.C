#include <graphics.h>

#define UpChar 30
#define DownChar 31
#define LeftChar 29
#define RightChar 28
#define InsChar 206
#define DelChar 207


static
cursor(x, y, c)
int x, y, c;
	{
	line(x, y-1, x, y-3, c);
	line(x, y+1, x, y+3, c);
	line(x-3, y, x-1, y, c);
	line(x+1, y, x+3, y, c);
	}

main()
	{
	int xb, yb, xc, yc;
	char c;

	scr_setup();
	scr_cursoff();
	scr_setmode( 4 );
	scr_clr();
	scr_color(0,1);
	scr_color(1,0);

	cursor(3, 3, 1);

	xb = yb = xc = yc = 3;

	while(( c = scr_ci()) != ' ')
		{
		switch(c)
			{
			case UpChar:
				if ( xb != xc || yb != yc)
					line(xb, yb, xc, yc, 0x82);
				cursor(xc, yc, 0x81);
				if ((yc-=8) < 3)
					yc = 3;
				line(xb, yb, xc, yc, 0x82);
				cursor(xc, yc, 0x81);
				break;
			case DownChar:
				if ( xb != xc || yb != yc)
					line(xb, yb, xc, yc, 0x82);
				cursor(xc, yc, 0x81);
				if ((yc+=8) > 195)
					yc = 195;
				line(xb, yb, xc, yc, 0x82);
				cursor(xc, yc, 0x81);
				break;
			case LeftChar:
				if ( xb != xc || yb != yc)
					line(xb, yb, xc, yc, 0x82);
				cursor(xc, yc, 0x81);
				if ((xc-=8) < 4)
					xc = 4;
				line(xb, yb, xc, yc, 0x82);
				cursor(xc, yc, 0x81);
				break;
			case RightChar:
				if ( xb != xc || yb != yc)
					line(xb, yb, xc, yc, 0x82);
				cursor(xc, yc, 0x81);
				if ((xc+=8) > 315)
					xc = 315;
				line(xb, yb, xc, yc, 0x82);
				cursor(xc, yc, 0x81);
				break;
			case InsChar:
				cursor(xc, yc, 0x81);
				line(xb, yb, xc, yc, 1);
				cursor(xc, yc, 0x81);
				xb = xc;
				yb = yc;
				break;
			default:
				beep();
			}
		}

	scr_setmode( 3 );
	scr_curson();
	scr_clr();
	}
