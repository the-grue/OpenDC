
#include	<graphics.h>
#define scr_setyx(y, x)	(scr_lry = (y), scr_lrx = (x))

#define scr_setact(act)	(scr_action = (act))

#define scr_setattr(attr)	(scr_attr = (attr))

#define scr_setpage(page)	(scr_page = (page))

static char special[] = 
	{
	1, 2, 3, 4, 5, 6, 7, 8, ' ','t',
	9, 't', 10, 'x', 11, 'x', 'x', 13, 't', 9,
	14, 15, 16,	17, 18, 19, 20, 21, 22, 23, 24,
	25, 26, 27, 28, 29, 30, 31, 0
	};

static char lbr[] = 
	{ 0x0C, 0x08, 0x10, 0x08, 0x0C };

static char rbr[] = 
	{ 0x0C, 0x04, 0x02, 0x04, 0x0C };

static char two[] = 
	{ 0x0C, 0x02, 0x0C, 0x10, 0x1E };

static char five[] = 
	{ 0x1C, 0x10, 0x1C, 0x02, 0x1C };

static char dol[] = 
	{ 0x04, 0x0E, 0x08, 0x0E, 0x04 };

static char and[] = 
	{ 0x0C, 0x10, 0x08, 0x13, 0x0C };

main()
	{
	int xb, yb, xe, ye, c, len;
	char t, *fontp, *scr_setch();

	screen(1);
	cls();
	color(1,0);
	color(0,3 | 0x10);

	beep();
	scr_setact(0);
	Fg(1);
	scr_setyx(24,12);
	scr_puts("0123456789\n");
	Fg(2);
	scr_puts("abcdefghijklmnopqrstuvwxyz");
	Fg(3);
	scr_puts("\nzyxwvutsrqpomnlkjihgfedcba\n");
	Fg(1);
	scr_puts(" !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");
	scr_setyx(48,18);
	Fg(2);
	scr_puts("The quick, sly fox jumped over the lazy brown dog.");
	scr_setyx(54,18);
	scr_puts("The quick, sly fox jumped over the lazy brown dog.");
	Fg(2);
	scr_puts("012345678911234567892123456789312345678941234567895123456789");

	fontp = scr_setch(0);
	_move(5,lbr,fontp + 5 * 'a');
	_move(5,rbr,fontp + 5 * ('a' + '}' - '{'));
	_move(5,five,fontp + 5 * '5');
	_move(5,two,fontp + 5 * '2');
	Fg(1);
	scr_puts("\n{52}\n");
	scr_printf("this is a zero %03d and this isn't %d\n", 0, -156);
	Fg(3);
	scr_puts(special);

	chekin();
	scr_setyx(48,0);
	scr_cline();
	chekin();
	scr_setyx(48,14);
	Fg(1);
	scr_puts("The quick, sly fox jumped over the lazy brown dog.");

	chekin();
	scr_putch(12);
	scr_putch('a');

	circle(159,99,50, 2);
		chekin();

	paint(159, 99, 1, 2);
		chekin();

	line(10,10,190,10,2);
		chekin();

	line(190,10,190,190,2);
		chekin();

	line(190,190,10,190,2);
		chekin();

	line(10,190,10,10,2);
		chekin();

	line(10,10,190,190,1);
		chekin();

	line(190,10,10,190,3);

	while ( 1 )
		{
		chekin();
		xb = rand() % 320;
		yb = rand() % 200;
		xe = rand() % 320;
		ye = rand() % 200;

		line( xb, yb, xe, ye, (++c & 3) );
		}
	}

sexit()
	{
	screen(0);
	width(80);
	exit(0);
	}

chekin()
	{
	int t;
	while((t = ci()) != ' ')
		{
		switch(t)
			{
			case 'b':
				color(0,ci());
				break;
			case 'p':
				color(1,ci());
				break;
			case 'q':
				sexit();
			}
		}
	}
