/*
/* rm <file> ...
/*
/*  remove files.  The file list may contain wild card delimeters for
/*		files.
*/

main1(argc, argv)
	int argc; char *argv[]; {
	int index;
	int prompted, listed, recursive;
	char answer, *ptr;

	if (argc <= 1) {
		puts("syntax: rm [-i] [-l] file ...\n");
		puts("   options: -i : interactive\n");
		puts("            -l : list files as deleted\n");
		exit(1);
		}

	prompted = listed = recursive = 0;

	argv++;
	while (--argc) {
		if (**argv == '-') {
			ptr = (*argv) + 1;
			if (tolower(*ptr) == 'i') prompted = 1;
			else if (tolower(*ptr) == 'l') listed = 1;
			}
		else {
			if (prompted) {
				puts("  remove file: ");
				puts(*argv);
				puts("? (y/n): ");
				answer = ci();
				puts("\n");
				if (toupper(answer) != 'Y') {
					argv++;
					continue;
					}
				}
			if (listed) {
				puts("  removing ");
				puts(*argv);
				puts("\n");
				}
			unlink(*argv);
			}
		argv++;
		}
	exit(0);
	}

