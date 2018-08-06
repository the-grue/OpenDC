/*
/*    more < file > ...
/*
/*      lists a file to the screen, pausing every 22 lines.
/*      if the space bar is typed, the next 22 lines are displayed,
/*      if the <return> key is typed, the next line is displayed,
/*      if 'q' or 'Q' is typed, the file is exited,
/*      if '/' is typed, more will request a search pattern and
/*        display the screen with the line which contains a
/*        matching pattern.
/*      
/*         written by Michael Ouye, 9/2/84
/*
*/

#define LINES	22
#define CR		0x0D

extern char *cur_line;
extern int line_number;
extern long line_location;

main1(argc, argv)
	int argc; char *argv[]; {
	int i, step, not_found, old_number, out_lines;
	char pattern[128], input[127], cc;
	long old_location;

	i = 1;
	pattern[0] = input[0] = 0;

	while (argc > 1) {
		if (!line_start(argv[i], 0L, 1)) {
			puts("cannot open file: ");
			puts(argv[i]);
			puts("\n");
			exit(1);
			}
		puts("file: ");
		puts(argv[i]);
		puts("\n");

		step = 0;
		out_lines = 0;
		do {
use_this_line:
			puts(cur_line);
			out_lines++;
			puts("\n");
just_wait:
			if (step || (out_lines >= LINES)) {
				step = 0;
				out_lines = 0;
				puts("-- MORE --");
				switch( cc = ci()) {
					case 'q':
					case 'Q':
						exit(0);
					case CR:
						step = 1;
						break;
					case '/':
					case 'n':
					case 'N':
						puts("\r          \r");
						old_location = line_location;
						old_number = line_number;
						not_found = 1;
						if (cc == '/') {
							scanf("%s", input);
							pattern[0] = '\0';	/* reinitialize if not empty */
							if (input[0] != '*')
								strcat(pattern, "*");
							strcat(pattern, input);
							}

						do {
							if (wildcmp(pattern, cur_line)) not_found = 0;
							} while (not_found && line_next());
						if (cc != '/') line_next();
						if (not_found) {
							/* rewind back to the old location */
							line_stop();
							line_start(argv[i], old_location, old_number);
							out_lines = 22;
							goto just_wait;
							}
						goto use_this_line;
					}
				puts("\r          \r");
				}
			} while (line_next());

		if ((line_number % LINES) != 0 && argc > 2) {
			puts("-- MORE [end of file]--");
			switch(ci()) {
				case 'q':
				case 'Q':
					exit(0);
				}
			puts("\n");
			}
		line_stop();
		argc--;
		i++;
		}
	exit(0);
	}

#define casing(c) c

int wildcmp(template, str)
	char *template, *str; {
	int temp_len, old_len;
	char *old_template, *match_start;

	old_len = temp_len = strlen(template);
/*	printf("wildcmp('%s', '%s'\n", template, str); */
	match_start = str;
	old_template = template;

	while(*str) {
/*		printf("temp_len: %d, *str = '%c'\n", temp_len, *str); */
		if (temp_len <= 0) return (1);
		if (*template == '*') {
			if ( casing(*(template+1)) == casing(*str)) {
/*				printf("beyond *, [%c] to [%c]\n", *(template+1), *str);*/
				if (template == old_template)
					match_start = str;	/* restart from first * */
				template += 2;
				temp_len -= 2;
				}
			else if (*(template+1) == '\0') {
				return (1);  /* * at the end of a template matches everything */
				}
			}
		else if (*template == '?') {
			template++;
			temp_len--;
			}
		else if (casing(*template) != casing(*str)) {
			/* restart the search from the miscompare */
			match_start = str;
			template = old_template;
			temp_len = old_len;
/*			printf("wildcmp('%s', '%s'\n", template, str); */
			str--; /* for the str++ at the end of the loop */
			}
		else {
			template++;
			temp_len--;
			}
		str++;
		}
	/* check to see if the template is exhausted */
	while (*template == '*') template++;
/*	printf("last template char[%d]\n", *template);*/
	if (*template != 0) return (0);
	return (1);
	}

