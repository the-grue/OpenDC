#include <graphics.h>

main()
	{
	FORM *fp, *createForm();
	char *cp;
	int i, j;

	screen(2);
	circle(50,50,25,1);
	paint(50,50,1,1);

	if((fp = createForm(100,100)) == 0)
		{
		puts("cannot open FORM\n");
		exit(1);
		}

	get(0, 0, 99, 99, fp);
	put(100, 100, fp);
	scr_ci();			/* wait for key to proceed */

	screen(1);
	circle(50,50,25,2);
	paint(50,50,1,2);

	if((fp = createForm(160,100)) == 0)
		{
		puts("cannot open FORM\n");
		exit(1);
		}

	get(0, 0, 159, 99, fp);
	put(100, 100, fp);
	scr_ci();

	screen(0, Burst(1));
	width(80)