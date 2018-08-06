#define DEBUG 1
/*********************************************************************

                          P C - M A K E

                             V 1 . 2

         A makefile generator for the IBM Personal Computer

                      (c) 1983, 1984 Michael Ouye

  PCmake is a program loosely modeled on the make program under UNIX.
  This is a very basic program management tool which allows the
  specification of dependencies for files.  All dependencies are
  based on the date of the affected files.

  Invocation syntax:

    pcmake [ <item> ] [-f<make file>] [-o<bat file>] [-a] [-d] [-D] [-c ]

       where <item> is the name of the dependency to be made
               (the first dependency is used by default).
             -f<make file> gets input from the given filename.
               (makefile is used by default)
             -o<bat file> directs the output to the given file
               (makeit.bat) is used by default)
             -a requests all file be remade
               (defaults to only modified files)
             -d writes the dependency list to the console
               (defaults to no dependency list)
             -i ignores errors from executed programs
               (default is to generate comparisons)
             -D turns on the debug display output
               (defaults to no debugging information written)

  makefile syntax:

    <dependent> [     <dependencies> ]
                      <constructor>

        where  <dependent> is the name of the file which is under examination
               <dependencies> is the list of files <dependent> is
                              dependent on.  This is a multi-line list
                              which is terminated by the right bracket.
               <constructor> is one or more lines of commands used to
                             update the dependent file (a blank line
                             delimits the end of the constructor).

	Macros are names which are prefixed with a '$' character (e.g. $FOO).
	Macros are defined by beginning a line with the macro name followed by
	the definition of the macro (e.g.  $FOO aa.o bb.o ).  Macros may be
	used almost anywhere in the makefile and are expanded recursively.
	(You can't use them at the beginning of a line since this constitutes
	a definition).  Note that for a macro definition line, the FIRST character
	in the line must be the '$' character.

	Makefile lines which begin with the '#' character are considered comments
	and are ignored.

	Certain extensions are recognized by the program, .exe, .o, .c and .a.
	If a dependent file is not given any dependecies (i.e. no brackets
	follow the filename) then the extension is checked for one of the
	aforementioned extensions.  If the extension is .exe, a dependency
	on <file>.o is automatically generated.  If the extension is .o, a
	depenedency on <file>.c, if it exists, or <file>.a if it exists, is
	generated.

	If no construction lines are given, then one is generated depending
	on the extension.  If the extension is .exe,
	the construction line :
		$BIND88 <dependencies> $BIND88FLAGS
	is generated and expanded. If the extension is .o and the
	extension of the FIRST dependency is .c then the line
		$C88 <file>.c $C88FLAGS
	is generated and expanded.  If the extension is .o and the
	extension of the FIRST dependency is .a, then the line
		$ASM88 <file>.a $ASM88FLAGS
	is generated and expanded.
	Otherwise, no line is generated.

	The defaults for the macros are
		$BIND88  bind
		$BIND88FLAGS <none>
		$C88  c88
		$C88FLAGS  <none>
		$ASM88 asm88
		$ASM88FLAGS <none>
	Any of these macro definitions may be altered by redefining the
	name.

******************************************************************************/

#define CR '\r'
#define LF '\n'
#define control_Z 26
#define MAX_NAME 132
#define STACK_SIZE 8000

/* node typedef */

typedef struct c_line {
	struct c_line *next_line;	/* pointer to the next construction line */
	char command[0];					 /* the command line */
	} construct;

typedef struct n_entry {
	struct n_entry *next_name;	/* pointer to the next name */
	char *ne_depend;			/* pointer to the dependency tree */
	long int ne_modified;		/* last modification date, 0 if no file */
	char name[0];				/* the depency name string */
	} name_entry;

typedef struct d_ptr {
	struct d_ptr *next_block;	/* pointer to the next block of pointers */
	char *db_depend[5];
	} depend_block;

typedef struct a_node {
	char visited;				/* cycle stopping visited flag */
	name_entry *name_ptr;		/* pointer to the filename */
	depend_block *depend_ptr;	/* pointer to the dependency list */
	construct *list;			/* pointer to the construct list */
	} node;

char *free_ptr;		/* free space pointer */
char *free_limit;	/* end of free space */

node *depend_list; /* pointer to the first dependent node */

name_entry *hash_table[64] =   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/* I/O buffer */
char buffer[1024];
char *buf_ptr;	/* next character pointer */
char input_line[4096];
char *line_ptr;

char makefile[80] = {"makefile"};
char makeout[80]	= {"makeit.bat"};
short int in_file, out_file;

short int current_line = 0;
short int read_chars = 0;

/* flags */
short int make_all = 0;
short int show_depend = 0;
short int debug = 0;
short int ignore_error = 0;

short int indent = 0;

char make_arg[80] = {0};

/* default definitions for $C88 $BIND88 and $ASM88 */
char *bind_mac = {"$BIND88 bind"};
char *c88_mac = {"$C88 c88"};
char *asm88_mac = {"$ASM88 asm88"};


/************************************************************************/
main(argc, argv)
	short int argc; char *argv[]; {
	short int arg_index; 
	long int do_depend();
	char *arg_ptr;

	printf("PCmake V1.2, (c) 1984 Michael Ouye\n");
	arg_ptr = argv[1];
	arg_index = 1;
	if (argc >= 2 && *arg_ptr != '-') {
		strcpy(make_arg, argv[1]); /* get the dependent name */
		arg_ptr = argv[2];
		argc--;
		arg_index++;
		}

	/* process any flags */
	argc--; /* get rid of the first argument counted (null name) */
	while (argc > 0) {
		arg_ptr = argv[arg_index];
		if (*arg_ptr == '-') {
			switch(*(++arg_ptr)) {
				case 'a': /* make sure all files are compiled */
					make_all = 1;
					break;

				case 'd': /* turn on the dependency trace listing */
					show_depend = 1;
					break;

				case 'D':
					debug = 1;
					break;

				case 'f': /* redirect the input file */
					strcpy(makefile, ++arg_ptr);
					break;

				case 'o': /* redirect the BAT file output */
					strcpy(makeout, ++arg_ptr);
					break;

				case 'i': /* ignore errors */
					ignore_error = 1;
					break;
				}
			}
		argc--;
		arg_index++;
		}

#if DEBUG
		if (debug)
			printf("selection of '%s'\n", argv[1]);
#endif
	in_file = open(makefile, 0); /* open for input */
	if (in_file < 0) {
		printf("can't open the makefile %s\n", makefile);
		exit(1);
		}

	out_file = creat(makeout, 1); /* create the output file */
	if (out_file < 0) {
		printf("can't open the output file %s\n", makeout);
		exit(1);
		}

	free_ptr = _memory(); /* a pointer to the first free memory bytes */
	free_limit = _showsp() - STACK_SIZE;
	depend_list = 0;

	define_macro(bind_mac);
	define_macro(c88_mac);
	define_macro(asm88_mac);

#if DEBUG
	if (debug)
		printf("beginning make file processing . . .\n");
#endif
	/* read in the makefile and build the dependency trees */
	if (build_depend()) {
		printf("errors in the makefile, no makeit.bat file created\n");
		exit(1);
		}

	if (show_depend) {
		printf("\nD E P E N D E N C Y   T R E E\n");
		print_depend(depend_list);
		}

	/* walk through the dependency tree and output the command file */
	do_depend(depend_list, 0x7FFFFFFFl);
	if (!ignore_error)
		fprintf(out_file, ":stop\n");
	close(out_file);
	close(in_file);
	exit(0);
	}

/************************************************************************
	alloc:	allocate a chunk of free memory, if full print error and exit
************************************************************************/
char *alloc(size)
	short int size; {
	char *temp_ptr;

	if (free_ptr+size > free_limit) {
		/* memory is full, sorry */
		printf("%d: Out of Memory, reduce the size of the makefile\n", current_line);
		exit(1);
		}
#if DEBUG
	if (debug)
		printf("allocate %d bytes at %xH\n", size, free_ptr);
#endif
	temp_ptr = free_ptr;
	free_ptr += size;
	return (temp_ptr);
	}



/************************************************************************
	collect_line:	read a line of text from the input file and return
		the length of the line (-1 if no more lines).
************************************************************************/
short int collect_line() {
	int len, mac_len;
	char last_char;

	while(1) {
		len = 0;
		line_ptr = input_line;
		*line_ptr = '\0';
		while (1) {
			/* collect an input line */
			if (buf_ptr >= buffer + read_chars) {
				/* empty buffer, read next */
				read_chars = read(in_file, buffer, sizeof(buffer));
				if (read_chars == 0) {
					buffer[0] = control_Z;
					read_chars = 1;
					}
				buf_ptr = buffer;
				}

			if (*buf_ptr != CR) { /* ignore carriage returns */
				*line_ptr = toupper(*buf_ptr);
				if (*line_ptr == LF || *line_ptr == control_Z) {
					buf_ptr++; /* increment passed the end */
					break;
					}
				len++;
				if (line_ptr < input_line + sizeof(input_line)) line_ptr++;
				}
			buf_ptr++;
			}
		last_char = *line_ptr;
		*line_ptr = 0; /* set terminator */
		len++;
		current_line++;
		line_ptr = input_line; /* beginning of the line */
	
#if DEBUG
		if (debug)
			printf("#%d:	'%s'\n", current_line, input_line);
#endif

		if (*input_line == '#')
			continue;		/* comment line, ignore it */

		if (*input_line == '$') {
			/* macro definition */
			define_macro(input_line);
			continue; /* next line */
			}
		if (last_char == control_Z) return(-1);
		all_expand(input_line, sizeof(input_line));
		return(strlen(input_line));
		}
	}




/************************************************************************
	skip_blanks:	skip over white space in the line.
************************************************************************/
skip_blanks() {

	while (isspace(*line_ptr))
		line_ptr++;
	}




/************************************************************************
	depend_name: input the characters up to the next blank and hash
		it into the dictionary.
************************************************************************/
name_entry *depend_name(special)
	char *special;	{
	name_entry *entry_ptr;
	short int hash, n_len;
	char name_buffer[MAX_NAME], *n_ptr, *d_ptr, *use_ptr;

	if (special) use_ptr = special;
	else use_ptr = line_ptr;
	hash = 0;
	n_len = 0;
	n_ptr = name_buffer;

	/* collect the name up to the next blank, 0, [, or ] */
	while(1) {
		if (isspace(*use_ptr) || (*use_ptr == 0) || (*use_ptr == '[') || (*use_ptr == ']')) {
			*n_ptr = 0;
			break;
			}
		if (n_len < sizeof(name_buffer)-1) {
			/* ignore character beyond the max_length string */
			*n_ptr++ = toupper(*use_ptr);
			hash = (hash << 1) + toupper(*use_ptr);
			n_len++;
			}
		use_ptr++;
		}

	hash = hash & 0x003F;	/* hash value mod 64 */
	if (hash_table[hash] == 0) { /* just enter it into the table */
		hash_table[hash] = entry_ptr = alloc(sizeof(name_entry)+n_len+1);
		entry_ptr->next_name = 0;
		entry_ptr->ne_depend = 0;
		entry_ptr->ne_modified = 0xFFFFFFFFl;
		strcpy(entry_ptr->name, name_buffer);
		if (!special) line_ptr = use_ptr;
		return (entry_ptr);
		}
	else {
		entry_ptr = hash_table[hash];
		while(1) {
			if (strcmp(entry_ptr->name, name_buffer) == 0) {
				if (!special) line_ptr = use_ptr;
				return (entry_ptr); /* match was found */
				}
			if (entry_ptr->next_name == 0) {
				/* enter the name */
				entry_ptr->next_name = alloc(sizeof(name_entry)+n_len+1);
				entry_ptr = entry_ptr->next_name;
				entry_ptr->next_name = 0;
				entry_ptr->ne_depend = 0;
				entry_ptr->ne_modified = 0xFFFFFFFFl;
				strcpy(entry_ptr->name, name_buffer);
				if (!special) line_ptr = use_ptr;
				return (entry_ptr);
				}
			entry_ptr = entry_ptr->next_name; /* next list element */
			}
		}
	}

/************************************************************************
	define_macro: use the first token as the macro name( including the $)
		and install it in the dictionary.  Collect the rest of the line
		as the expansion characters.
************************************************************************/
define_macro(line_ptr)
	char *line_ptr; {
	char *ptr;
	int hash, n_len;
	name_entry *entry_ptr;

	ptr = line_ptr;
	hash = 0;
	n_len = 0;
	while (*ptr != ' ' && *ptr != 9 && *ptr != '\0') {
		hash = (hash << 1) + toupper(*ptr++);
		n_len++;
		}
	if (*ptr == '\0') return; /* just a name */
	*ptr++ = '\0';

	hash = hash & 0x003F;	/* hash value mod 64 */
	if (hash_table[hash] == 0) { /* just enter it into the table */
		hash_table[hash] = entry_ptr = alloc(sizeof(name_entry)+n_len+1);
		strcpy(entry_ptr->name, line_ptr);
		entry_ptr->next_name = 0;
		entry_ptr->ne_depend = alloc(strlen(ptr)+1);
		strcpy(entry_ptr->ne_depend, ptr);
		return;
		}
	else {
		entry_ptr = hash_table[hash];
		while(1) {
			if (strcmp(entry_ptr->name, line_ptr) == 0) {
				/* match, throw out old definition */
				entry_ptr->ne_depend = alloc(strlen(ptr)+1);
				strcpy(entry_ptr->ne_depend, ptr);
				return;
				}
			if (entry_ptr->next_name == 0) {
				/* enter the name */
				entry_ptr->next_name = alloc(sizeof(name_entry)+n_len+1);
				entry_ptr = entry_ptr->next_name;
				strcpy(entry_ptr->name, line_ptr);
				entry_ptr->next_name = 0;
				entry_ptr->ne_depend = alloc(strlen(ptr)+1);
				strcpy(entry_ptr->ne_depend, ptr);
				return;
				}
			entry_ptr = entry_ptr->next_name; /* next list element */
			}
		}
	}

/************************************************************************
	expand_macro:  look up a macro name and copy the string that is
		associated with it.  If the name is not found, -1 is returned,
		otherwise the length of the string is returned.
************************************************************************/
int expand_macro(mac_name, buffer)
	char *mac_name, *buffer; {
	char *ptr;
	name_entry *entry_ptr;
	int hash;

	ptr = mac_name;
	hash = 0;
	while (*ptr) {
		hash = (hash << 1) + toupper(*ptr);
		ptr++;
		}

	hash = hash & 0x003F;	/* hash value mod 64 */
	if (hash_table[hash] == 0)  /* not found */
		return (0);
	else {
		entry_ptr = hash_table[hash];
		while(1) {
			if (strcmp(entry_ptr->name, mac_name) == 0) {
				if (entry_ptr->ne_depend == 0) return (0);
				*buffer++ = ' ';
				strcpy(buffer, entry_ptr->ne_depend);
				return(strlen(buffer)+1);
				}
			if (entry_ptr->next_name == 0) {
				/* not found */
				return(0);
				}
			entry_ptr = entry_ptr->next_name; /* next list element */
			}
		}
	}

/************************************************************************
	all_expand: expand all macros contained in the line, including
		macros called by macros.
************************************************************************/
all_expand(line_ptr, line_len)
	char *line_ptr; int line_len; {
	char *ptr, *endptr, *mptr;
	int mac_len, max_len;

	ptr = line_ptr;
	while (*ptr != '\0') {
		if (*ptr == '$') {
			mptr = ptr;
			/* move beyond the macro name */
			while (*ptr != ' ' && *ptr != 9 && *ptr != '\0') ptr++;
			/* move every thing following ptr to the end of the buffer */
			endptr = line_ptr + line_len - strlen(ptr) -1;
			_move(strlen(ptr) + 1, ptr, endptr);
			*ptr = '\0';
			mac_len = expand_macro(mptr, mptr);
			/* now put the line back together */
			_move(strlen(endptr)+1, endptr, mptr+mac_len);
			ptr = mptr;
			}
		else 
			ptr++;
		}
	}


#define	no_suffix	0
#define o_suffix	1
#define	a_suffix	2
#define c_suffix	3
#define exe_suffix	4


/************************************************************************
	get_suffix: return an indicator as to the suffix used in the name.
************************************************************************/
int get_suffix(ptr)
	char *ptr; {

	while (*ptr != '.' && *ptr != 0) ptr++;
	if (*ptr == '.') {
		ptr++;
		if (toupper(*ptr) == 'O') return o_suffix;
		else if (toupper(*ptr) == 'A' && *(ptr+1) == '\0') return a_suffix;
		else if (toupper(*ptr) == 'C'&& *(ptr+1) == '\0') return c_suffix;
		else if (toupper(*ptr) == 'E' &&
					toupper(*(ptr+1)) == 'X' &&
					toupper(*(ptr+2) == 'E' && *(ptr+3) == '\0') )
			return exe_suffix;
		}
	return no_suffix;
	}

change_ext(name, new_ext)
	char *name, *new_ext; {

	while (*name != '.' && *name != '\0') name++;
	strcpy(name, new_ext);
	}

/************************************************************************
	build_depend:	read in the makefile and build the dependency tree.
		all errors are reported, with the line number to the screen.
************************************************************************/
short int build_depend() {
	int read_chars, len, d_index, s_type;
	construct *this_list;
	name_entry *this_name, *dep_name;
	node *this_node, *dep_node;
	depend_block *dep_ptr;
	char filename[81];
	int file;

	buf_ptr = buffer;
	read_chars = 0;
	current_line = 0;

	for(;;) { /* for all lines */

		if (collect_line() < 0) { /* end of the file */
			if (make_arg[0] != 0) { /* find the dependent requested */
				strcpy(input_line, make_arg);
#if DEBUG
				if (debug)
					printf("searching for dependency '%s'\n", input_line);
#endif
				line_ptr = input_line;
				depend_list = depend_name(0);
				}
			return(0);
			}

		skip_blanks();
		if (*line_ptr != 0) {
			/* process a dependent */
			this_name = depend_name(0); /* finds/puts the name in the dictionary */
			if (this_name->ne_depend == 0) {
				/* allocate a node */
				this_node = this_name->ne_depend = alloc(sizeof(node));
				if (depend_list == 0)
					depend_list = this_name; /* store the first dependency name pointer */
				this_node->visited = 0;
				this_node->name_ptr = this_name;
				this_node->depend_ptr = dep_ptr = alloc(sizeof(depend_block));
				this_node->list = 0;
				_setmem(dep_ptr, sizeof(depend_block), 0); /* store all zeros */
				d_index = 0;
				}
			else {
				/* this node already has a dependency list ! */
				printf("error: %d: %s already has a dependency list\n", current_line, this_name.name);
				continue; /* skip the rest of the line */
				}
			skip_blanks();

			if (*line_ptr != '[') {
				s_type = get_suffix(this_name->name);
				if (s_type == exe_suffix) {
					strcpy(filename, this_name->name);
					change_ext(filename, ".O");
					dep_name = depend_name(filename);
#if DEBUG
					if (debug)
						printf("dependency %s, left[%s]\n", dep_name->name, line_ptr);
#endif
					dep_ptr->db_depend[0] = dep_name; /* store the pointer to the name */
					continue;
					}
				else if (s_type == o_suffix) {
					/* check for .c or .a file existence */
					strcpy(filename, this_name->name);
					change_ext(filename, ".C");
					file = open(filename, 0);
					if (file < 0) {
						change_ext(filename, ".A");
						file = open(filename, 0);
						}
					if (file > 0) {
						/* install the filename as a dependent */
						close(file);
						dep_name = depend_name(filename);
#if DEBUG
						if (debug)
							printf("dependency %s, left[%s]\n", dep_name->name, line_ptr);
#endif
						dep_ptr->db_depend[0] = dep_name; /* store the pointer to the name */
						continue;
						}
					}
		        printf("%d[%c]: missing '[' for dependency list\n", current_line, *line_ptr);
				continue;
				}
			else
				line_ptr++; /* skip beyond the [ */

			/* now process all of the dependencies */
			while (1) {
				skip_blanks();
				if (*line_ptr == ']') break;
				if (*line_ptr == 0) {
					/* use the next line too */
					if (collect_line() < 0) break;
					continue;
					}
				dep_name = depend_name(0);
#if DEBUG
				if (debug)
					printf("dependency %s, left[%s]\n", dep_name->name, line_ptr);
#endif
				if (d_index >= 5) {
					/* allocate another dependency block */
					dep_ptr->next_block = alloc(sizeof(depend_block));
					dep_ptr = dep_ptr->next_block;
					_setmem(dep_ptr, sizeof(depend_block), 0);
					d_index = 0;
					}
				dep_ptr->db_depend[d_index++] = dep_name; /* store the pointer to the name */
				}
			
			/* now input the construction lines up to a blank line */
			this_list = 0;
			while (1) {
				if ((len = collect_line()) < 0) break;
				skip_blanks();
				if (*line_ptr == 0) break;
#if DEBUG
				if (debug)
					printf("construct %s\n", line_ptr);
#endif
				if (this_list == 0) 
					this_node->list = this_list = alloc(sizeof(construct) + len + 1);
				else {
					this_list->next_line = alloc(sizeof(construct) + len + 1);
					this_list = this_list->next_line;
					}
				this_list->next_line = 0;
				strcpy(this_list->command, line_ptr);
				}
			}
		}
	}



/************************************************************************
	print_depend: print the dependency tree.	Recursive calls for each level.
************************************************************************/
print_depend(n_ptr)
	name_entry *n_ptr; {
	node *node_ptr;
	depend_block *d_ptr;
	short int save, dep_index, i;
	construct *con_line;

	if (n_ptr == 0) return;
	save = indent;
	for (i = 0; i < indent/2; i++)
		printf("	"); /* write out the indent */

	printf("%s\n", n_ptr->name);
	node_ptr = n_ptr->ne_depend;
	if (node_ptr == 0) return; /* no dependencies */
	con_line = node_ptr->list;
	while (con_line != 0) {
		for (i = 0; i < indent/2; i++)
			printf("	"); /* write out the indent */
		printf("--->%s\n", con_line->command);
		con_line = con_line->next_line;
		}
	indent += 2;
	/* now process each of the dependencies */
	d_ptr = node_ptr->depend_ptr;
	dep_index = 0;
	while (1) {
		if (dep_index >= 5) {
			/* move to the next dependency block, if any */
			if (d_ptr->next_block == 0) break;
			d_ptr = d_ptr->next_block;
			dep_index = 0;
			}
		if (d_ptr->db_depend[dep_index] == 0) break;
		print_depend(d_ptr->db_depend[dep_index++]);
		}
	indent = save;
	}



/************************************************************************
	do_depend: process the dependency tree and build the appropriate batch file.
		This routine is called recursively to process each dependency level.
************************************************************************/
long int do_depend(n_ptr, last_date) 
	name_entry *n_ptr; long int last_date; {
	node *node_ptr;
	depend_block *d_ptr;
	int dep_index, i, must_gen;
	long int was_modified, get_date(), todays_date();
	construct *con_line;
	int suffix;
	name_entry *nmptr;

	if (n_ptr->ne_modified == 0xFFFFFFFFl) 
		n_ptr->ne_modified = get_date(n_ptr->name);

#if DEBUG
	if (debug)
		printf("%s == %ld\n", n_ptr->name, n_ptr->ne_modified);
#endif

	must_gen = 0;
	node_ptr = n_ptr->ne_depend;
	if (node_ptr == 0) {
		if (n_ptr->ne_modified == 0) {
			printf("error: '%s' does not exist\n", n_ptr->name);
			return (0x7FFFFFFFl);
			}
		return (n_ptr->ne_modified); /* return the last modification date */
	}

	if (node_ptr->visited)
		return (n_ptr->ne_modified); /* already processed */
 
	node_ptr->visited = 1;
	/* now process each of the dependencies */
	d_ptr = node_ptr->depend_ptr;
	dep_index = 0;
	while (1) {
		if (dep_index >= 5) {
			/* move to the next dependency block, if any */
			if (d_ptr->next_block == 0) break;
			d_ptr = d_ptr->next_block;
			dep_index = 0;
			}
		if (d_ptr->db_depend[dep_index] == 0) break;
		was_modified = do_depend(d_ptr->db_depend[dep_index++], n_ptr->ne_modified);

#if DEBUG
		if (debug)
			printf("depends on %ld\n", was_modified);
#endif

		if (was_modified >= n_ptr->ne_modified) 
			must_gen = 1;
		}
	if (must_gen || make_all) {
		/* generate this node */
#if DEBUG
		if (debug)
			printf("generating %s\n", n_ptr->name);
#endif
		n_ptr->ne_modified = todays_date();		 /* give this node a new date */
		/* write out the construction lines */
		con_line = node_ptr->list;
		if (con_line == 0) {
			/* try for a suffix generated line */
			suffix = get_suffix(n_ptr->name);
			if (suffix != no_suffix) {
				if (suffix == exe_suffix) { /* .O file */
					strcpy(input_line, "$BIND88 ");
					dep_index = 0;
					d_ptr = node_ptr->depend_ptr;
					while (1) {
						if (dep_index >= 5) {
							/* move to the next dependency block, if any */
							if (d_ptr->next_block == 0) break;
							d_ptr = d_ptr->next_block;
							dep_index = 0;
							}
						if (d_ptr->db_depend[dep_index] == 0) break;
						nmptr = d_ptr->db_depend[dep_index];
						strcat(input_line, nmptr->name);
						strcat(input_line, " ");
						dep_index++;
						}
					strcat(input_line, "$BIND88FLAGS");
					}
				else if (suffix == o_suffix) {
					d_ptr = node_ptr->depend_ptr;
					nmptr = d_ptr->db_depend[0];
					suffix = get_suffix(nmptr->name);
					if (suffix == c_suffix) { /* .C file */
						strcpy(input_line, "$C88 ");
						strcat(input_line, nmptr->name);
						strcat(input_line, " $C88FLAGS");
						}
					else if (suffix == a_suffix) { /* .A file */
						strcpy(input_line, "$ASM88 ");
						strcat(input_line, nmptr->name);
						strcat(input_line, " $ASM88FLAGS");
						}
					}
				all_expand(input_line, sizeof(input_line));
				fprintf(out_file, "%s\n", input_line);
				if (!ignore_error)
					fprintf(out_file, "IF ERRORLEVEL 1 GOTO stop\n");
#if DEBUG
				if (debug)
					printf("execute: %s\n  from %s\n", input_line, con_line->command);
#endif
				}
			}

		while (con_line != 0) {
			strcpy(input_line, con_line->command);
			all_expand(input_line, sizeof(input_line));
			fprintf(out_file, "%s\n", input_line);
			if (!ignore_error)
				fprintf(out_file, "IF ERRORLEVEL 1 GOTO stop\n");
#if DEBUG
			if (debug)
				printf("execute: %s\n  from %s\n", input_line, con_line->command);
#endif
			con_line = con_line->next_line;
			}
		}
	return (n_ptr->ne_modified);
	}


char name_buffer[80];
short int file = 0;
long new_time = 0;

long int get_date(name_ptr) {
	strcpy(name_buffer, name_ptr);
	file = 1;
#asm
	mov		dx, offset name_buffer_
	mov		al, 0
	mov		ah, 3DH
	int		21H						; try to open the file for read
	mov		file_,ax
	jnc		over
	mov		file_, -1
over:
#
	if (file < 0) {
#if DEBUG
		if (debug)
			printf("can't find file [%d] %s\n", file, name_ptr);
#endif
		return (0L); /* oldest date if no such file */
		}
	else {
#asm
	mov		al,0
	mov		bx, file_
	mov		ah, 57H
	int		21H						; get modification date and time
	mov		word new_time_, cx
	mov		word new_time_+2, dx

	mov		bx, file_
	mov		ah, 3EH
	int		21H						; close the file
#
	
#if DEBUG
		if (debug)
			printf("found file %s: %ld\n", name_ptr, new_time);
#endif
		return (new_time);
		}
	}

long int todays_date() {

	return (0x7FFFFFFFL);
	}
