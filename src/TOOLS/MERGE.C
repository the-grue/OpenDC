/*
/*  merge <file>
/*		Utility for merging source files with the assembler language
/*		file produced by the DeSmet C Compiler with the -A and -C
/*		options.
*/

extern char *cur_line, *cur2_line;
extern int line_number, line2_number;

main(argc, argv)
	int argc; char *argv[]; {
	int i, l_file, to_line;
	char c_name[80], a_name[80], l_name[80];

	if (argc <= 1) {
		printf("usage is:  merge <file>\n");
		exit(1);
		}

	i = 1;
	while (argc > 1) {
		strcpy(c_name, argv[1]);
		strip(c_name);
		strcpy(a_name, c_name);
		strcpy(l_name, c_name);
		strcat(c_name, ".c");
		strcat(a_name, ".a");
		strcat(l_name, ".l");

		if (!line_start(c_name, 0L, 1)) {
			printf("cannot open file: %s\n", c_name);
			exit(1);
			}
		if (!line2_start(a_name, 0L, 1)) {
			printf("cannot open file: %s\n", a_name);
			exit(1);
			}

		if ((l_file = creat(l_name)) < 0) {
			printf("cannot create file: %s\n", l_name);
			exit(1);
			}

		/* skip to a line number statement in the asm file */
		while (1) {
			to_line = line_no(cur2_line);
			if (to_line < 0) {
				if (!line2_next()) break;
				}
			else break;
			}

		if (to_line < 0) {
			printf("use the 'c' switch when compiling\n");
			exit(1);
			}
		line2_stop();
		line2_start(a_name, 0L, 1);

		while(1) {
			/* now write all the C lines up to and including to_line */

			while (line_number <= to_line) {
				fprintf(l_file, ";%4d %s\n", line_number, cur_line);
/*				printf("%4d %s\n", line_number, cur_line);*/
				if (!line_next()) goto next;
				}

			/* skip to a line number statement in the asm file */
			while (1) {
				to_line = line_no(cur2_line);
				if (to_line < 0) {
					if (*cur2_line == ' ' || *cur2_line == 9)
						fprintf(l_file, "\t%s\n", cur2_line);
					else
						fprintf(l_file, "%s\n", cur2_line);

/*					printf("     %s\n", cur2_line); */
					if (!line2_next()) break;
					}
				else {
					line2_next();
					break;
					}
				}
			fprintf(l_file, "\n");
			if (to_line < 0) to_line = 32767; /* set to all lines */
			}
next:		
		line_stop();
		line2_stop();
		close(l_file);
		argc--;
		i++;
		}
	exit(0);
	}


strip(ptr)
	char *ptr; {			/* remove any suffix */
	char *start;

	start = ptr;

	while (*ptr) ptr++;		/* advance to the end of the string */
	while (*ptr != '.' && ptr > start) ptr--;
	if (*ptr == '.') *ptr = 0;
	}

int line_no(ptr)
	char *ptr; {
	int this_num;

	if (*ptr == 0) return (-1);
	ptr++;		/* skip the first character */
	while (*ptr == ' ') ptr++;
	if (strncmp(ptr, "LINE", 4) == 0) {
		while (*ptr != ' ') ptr++;
		while (*ptr == ' ') ptr++;		/* advance to the number */
		this_num = atoi(ptr);
		return (this_num);
		}
	return (-1);
	}
