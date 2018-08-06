/*
/* grep: generalized regular expresion file search utility.
/*
/*    similar to the grep utility under UNIX except for
/*     a simplifed version of the regular expression syntax (only * and ?)
/*       on a given line.
/*
/*    grep [- y] <search string> file ...
/*
/*     written by Michael Ouye, 9/17/84
*/

extern char *cur_line;
extern int line_number;
int ignore_case = 0;

char casing(c)
	char c; {
	
	if (ignore_case)
		return toupper(c);
	return (c);
	}


int wildcmp(template, str)
	char *template, *str; {
	int temp_len, old_len;
	char *old_template, *match_start, startrange, endrange;

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
		else if (*template == '[') {
			template++;
			temp_len--;
			while (1) {
				if (*template == ']' || *template == 0) goto miscompare;
				if (casing(*template) == casing(*str)) break;
				startrange = casing(*template++);
				temp_len--;
				if (*template == '-') {
					/* check if in range */
					template++;
					temp_len--;
					endrange = casing(*template++);
					temp_len--;
					if (startrange > endrange) goto miscompare;
					if (casing(*str) >= startrange && casing(*str) <= endrange) break;
					}
				}
			while (*template != ']' && *template != 0) {
				template++;
				temp_len--;
				}
			if (*template == ']') {
				template++;
				temp_len--;
				}
			}
		else if (*template == '\\') {
			template++;
			temp_len--;
			if (casing(*template) != casing(*str)) goto miscompare;
			template++;
			temp_len--;
			}
		else if (casing(*template) != casing(*str)) {
			/* restart the search from the miscompare */
miscompare:
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


main1(argc, argv)
	int argc; char *argv[]; {
	char pattern[128];

	ignore_case = 0;
	if (argc <= 1) {
		puts("  syntax is:  grep [ -y ] <search pattern> file ...\n");
		puts("    -y: letter case ignored\n");
		puts("    <search pattern> may contain wildcard characters '*' and '?'\n");
		puts("      but the pattern must be enclosed by \"  \"\n");
		puts("                  written by Michael Ouye, 9/17/84\n");
		exit(0);
		}
	argc--;
	argv++;
	while (**argv == '-') {
		if (*(*argv+1) == 'y')  ignore_case = 1;
		argc--;
		argv++;
		}
	if (**argv != '*') {
		strcpy(pattern, "*");
		strcat(pattern, *argv);
		}
	else
		strcpy(pattern, *argv);
	argc--;
	argv++;

	while(argc-- > 0) {
		if (line_start(*argv, 0L, 1)) {
			do {
/*				printf("searching: %s\n", cur_line); */
				if (wildcmp(pattern, cur_line)) {
					printf("%s  [%5d] %s\n", *argv, line_number, cur_line);
					}
				} while (line_next());
			line_stop();
			}
		else {
			printf("cannot open file: %s\n", *argv);
			}
		argv++;
		}
	}

