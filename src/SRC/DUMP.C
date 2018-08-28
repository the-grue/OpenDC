/*		dump.c		core style dump of a file	*/

/*		usage: A>DUMP B:BLIP.O					*/

# include <stdio.h>

char buffer[4096];

main(argc,argv)
	int  argc;
	char *argv[]; {
	unsigned i,numin,tot,file;
	int  cfrom;

	if (argc < 2) {
		puts("Missing Filename");
		exit(1);
		}

	tot=0;
	if ((file=open(argv[1],0)) == -1) {
		fputs("Cannot Open ", stdout);
		puts(argv[1]);
		exit(1);
		}

/*	read and dump 4k at a time	*/

	do {
		numin=read(file,buffer,4096);
		if (numin == -1) {
			fputs("Cannot Read ", stdout);
			puts(argv[1]);
			exit(1);
			}
		cfrom=0;
		while (cfrom < numin) {

/*	print the offset in hex	*/

			ohw(cfrom+tot);
			putc(' ', stdout);

/*	print 16 bytes in hex	*/

			for (i=0; i < 16; i++) {
				putc(i == 8 ? '-' :' ', stdout);
				ohb(buffer[cfrom++]);
				}
			cfrom-=16;
			putc(' ', stdout);
			putc('*', stdout);

/*	print the bytes in ascii	*/

			for (i=0; i < 16; i++) {
				putc((buffer[cfrom] >= ' ' && buffer[cfrom] < 0x7f)
					 ? buffer[cfrom]: '.', stdout);
				cfrom++;
				}
			puts("*");
			}
		tot+=numin;
		}
	while (numin == 4096);
	}

/*	print a word in hex	*/

ohw(wrd)
	unsigned wrd; {
	ohb(wrd>>8);
	ohb(wrd);
	}

/*	print a byte in hex	*/

ohb(byt)
	char byt; {
	onib(byt>>4);
	onib(byt);
	}

/*	print a nibble as a hex character	*/

onib(nib)
	char nib; {

	nib&=15;
	putc((nib >= 10) ? nib-10+'A': nib+'0', stdout);
	}
