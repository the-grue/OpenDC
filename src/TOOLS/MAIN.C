/*
/* main.c: command line expansion for wildcard filenames.
/*
/*  This main program takes the standard input list and
/*    converts the wildcard filename requests into
/*    filenames.  The new entries are stored on the stack.
/*    Finally, the routine main1 is called with the new
/*    argument list.  A maximum of 4KB will be used
/*    for the argument list.
/*
/*	written by Michael Ouye, 7/1/84
*/

#define MAX_SIZE	4096

static struct {
	char dta_reserved[21];
	char dta_attribute;
	short int dta_time;
	short int dta_date;
	long int dta_file_size;
	char dta_name[13];
	} DTA;

static int find_first(ptr)
	char *ptr; {

#asm
	mov		dx,[bp+4]			; ptr
	mov		cx,	0
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

static int find_next() {

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

static char msg[] = {"too many arguments\n"};

main(argc, argv)
	short int argc; char *argv[]; {
	short int *arg_vector;		/* pointer to the next entry in the argv array */
	char *last_str;				/* pointer to the last string entered */
	short int new_count, pathcount, filecount;
	char *cptr, *new_ptr, *old;
	char buffer[MAX_SIZE], buf[128];

	arg_vector = buffer;
	last_str = &buffer[MAX_SIZE];
	new_count = 0;

	/* set the Data Transfer Address to the local block */
#asm
	mov		dx, offset DTA_
	mov		ah, 01AH			; set disk transfer address
	int		21H
#

	while(argc--) {
		cptr = *argv;
		/* check the argument for wildcard characters '*' or '?' */
		if (*cptr == '\"') {
			/* beginning copying arguments into buf until a second " is found */
			buf[0] = '\0';
			cptr++; /* beyond the " */
			old = buf;
			while (argc) {
				strcat(buf, cptr);
				/* now search for a " */
				while (*old != '\"' && *old != '\0') old++;
				if (*old == '\"') {
					/* string with " found, terminate on top of the " */
					*old = '\0';
					break;
					}
				old += strlen(cptr);
				*old++ = ' '; /* add a blank back */
				*old = '\0';
				argc--;
				argv++;
				}
			last_str -= strlen(buf)+1;
			_move(strlen(buf)+1, buf, last_str);
			*arg_vector++ = last_str;
			new_count++;
			argv++;
			continue;
			}
		else {
			while (*cptr) {
				if (*cptr == '*' || *cptr == '?') break;
				cptr++;
				}
			}
		if (*cptr == '\0') {
			/* no expansion characters found, just save this pointer */
			if (arg_vector+1 > last_str) {
				puts(msg);
				exit(1);
				}

			*arg_vector++ = *argv;
			new_count++;
			}
		else {
			/* must expand the filename */
			while( *(++cptr));		/* advance to the end of the name */
			while ( cptr > *argv && *cptr != '/' && *cptr != '\\' && *cptr != ':') cptr--;
			if (cptr == *argv) pathcount = 0; /* no path to copy */
			else pathcount = cptr - *argv + 1;

			if (!find_first(*argv)) {
				argv++;
				continue;
				}
			do {
				filecount = strlen(DTA.dta_name) + 1;
				if ( (last_str - filecount - pathcount) <= (arg_vector+1)) {
					puts(msg);
					exit(1);
					}

				last_str -= filecount;
				_move(filecount, DTA.dta_name, last_str);
				last_str -= pathcount;
				_move(pathcount, *argv, last_str);
				*arg_vector++ = last_str;
				new_count++;
				} while (find_next());
			}
		argv++;
		}

	main1(new_count, buffer);
	exit(0);
	}
