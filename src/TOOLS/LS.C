/*
/*  ls: a unix-styled directory listing program.
/*
/*	written by Michael Ouye, 7/1/84
*/


struct {
	char dta_reserved[21];
	char dta_attribute;
	short int dta_time;
	short int dta_date;
	long int dta_file_size;
	char dta_name[13];
	} DTA;

struct d_entry{
	char		e_attribute;
	short int	e_time;
	short int	e_date;
	long int	e_file_size;
	char		e_name[13];
	} entry[1000];


int entries;

int find_first(ptr)
	char *ptr; {

#asm
	mov		dx,[bp+4]			; ptr
	mov		cx,	0FFH
	mov		ah, 04EH
	int		21H
	mov		ax,0
	jc		ff_over
	inc		ax
ff_over:
	pop		bp
	ret
#
	}

int find_next() {

#asm
	mov		ah, 04FH			; find next
	int		21H
	mov		ax,0
	jc		fn_over
	inc		ax
fn_over:
	pop		bp
	ret
#
	}


char null_arg[] = "*.*";
int long_format = 0;
int time_sort = 0;
int reverse_order = 0;
int single_column = 0;

int opposite(val)
	int val; {

	if (reverse_order)
		return -val;
	return val;
	}


int comp(ptr1, ptr2)
	struct d_entry *ptr1, *ptr2; {
	long *time1, *time2;
	int val;

	val = 0;
	if (time_sort) {
		time1 = &ptr1->e_time;
		time2 = &ptr2->e_time;
		if (*time1 > *time2) val = -1;
		else if (*time1 < *time2) val = 1;
		return opposite(val);
		}
	else {
		return opposite(strcmp(ptr1->e_name, ptr2->e_name, 13));
		}
	}


main(argc, argv)
	short int argc; char *argv[]; {
	char *cptr, device_arg[85];
	short int i, args, arg_count, has_wildcards;

	/* set the Data Transfer Address to the local block */
#asm
	mov		dx, offset DTA_
	mov		ah, 01AH			; set disk transfer address
	int		21H
#

	if (argc == 1) *argv = null_arg;
	entries = 0;
	args = 0;
	arg_count = argc-1;
	while(argc--) {
		has_wildcards = 0;
		if (**argv == '-') {
			args++;
			cptr = *argv;
			while (*cptr) {
				switch(*cptr) {
					case 'l': long_format = 1; single_column = 1; break;
					case 't': time_sort = 1; break;
					case 'r': reverse_order = 1; break;
					case '1': single_column = 1; break;
					case '?':
						puts("  syntax: ls [ -ltr1?] <path> ...\n");
						puts("     -l: long format listing\n");
						puts("     -t: sort by modification time\n");
						puts("     -r: reverse the sort order\n");
						puts("     -1: single column listing\n");
						puts("     -?: this message\n");
						puts("\n  written by Michael Ouye, 8/21/84\n");
						exit(0);
						break;
					}
				cptr++;
				}
			argv++;
			}
		else {
			cptr = *argv;
			while (*cptr) {
				if (*cptr == '*' || *cptr == '?') break;
				cptr++;
				}
			if (*cptr != '\0') has_wildcards = 1;
			strcpy(device_arg, *argv++);
			if ((device_arg[0] == '\\' || device_arg[0] == '/') &&
			      device_arg[1] == '\0')
				strcat(device_arg, null_arg);
			else if (device_arg[1] == ':' && (device_arg[2] == '\0' || device_arg[2] == ' '))
				strcat(device_arg, null_arg);
			if (!find_first(device_arg)) continue;
			if (!has_wildcards) {
				/* check for directory listing */
				if ((DTA.dta_attribute & 0x10) != 0) {
					cptr = &device_arg[strlen(device_arg)-1];
					if (*cptr != '\\' && *cptr != '/')
						strcat(device_arg, "\\");
					strcat(device_arg, null_arg);
					if (!find_first(device_arg)) continue;
					}
				}
			do {
				_move(22, &DTA.dta_attribute, &entry[entries++]);
				} while (find_next());
			}
		if (args >= arg_count && entries == 0) {
			*--argv = null_arg;
			argc++;
			}
		}

	qsort(&entry, entries, 22, comp);
	print_entries();
	exit(0);
	}


print_entries() {
	int i, reps, max_reps, hour, pm;
	char e_buf[40];
	long int total;
	char access_attr, type_attr;

	max_reps = (single_column) ? 1: 5;
	reps = 0;
	total = 0L;

	if (long_format) {
		printf("name             attrib     size      time       date\n");
		}

	for (i = 0; i < entries; i++) {
		*e_buf = '\0';
		if (entry[i].e_attribute & 0x10)
			strcat(e_buf, "[");
		strcat(e_buf, entry[i].e_name);
		if (entry[i].e_attribute & 0x10)
			strcat(e_buf, "]");
		strcat(e_buf, "                    ");
		e_buf[14] = '\0';
		if (long_format) {
			hour = 	(entry[i].e_time >> 11);
			if (hour > 12) {
				hour -= 12;
				pm = 1;
				}
			else pm = 0;
			if (entry[i].e_attribute & 0x01)
				access_attr = '-';
			else access_attr = 'w';

			if (entry[i].e_attribute & 0x10)
				type_attr = 'd';
			else if (entry[i].e_attribute & 0x02)
				type_attr = 'h';
			else if (entry[i].e_attribute & 0x04)
				type_attr = 's';
			else
				type_attr = '-';

			printf("%14s    %c%c%c    %8D    %2d:%1d%1d%c    %d-%d-%d", 
					e_buf, 
					type_attr, 'r', access_attr,
					entry[i].e_file_size,
					hour,
					((entry[i].e_time >> 5) & 0x003F) / 10,
					((entry[i].e_time >> 5) & 0x003F) % 10,
					(pm) ? 'p': 'a',
					(entry[i].e_date >> 5) & 0x000F,
					(entry[i].e_date & 0x001F),
					(entry[i].e_date >> 9) + 1980);
			}
		else
			puts(e_buf);
		reps++;
		if (reps >= max_reps) {
			puts("\n");
			reps = 0;
			}
		total += entry[i].e_file_size;
		}
	if (reps != 0) puts("\n");
	printf("\n%d entries with %D total bytes\n", entries, total);
	}

