/*
 *  Released under the GNU GPL.  See http://www.gnu.org/licenses/gpl.txt
 *
 *  This program is part of the DeSmet C Compiler
 *
 *  DeSmet C is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundatation; either version 2 of the License, or any
 *  later version.
 *
 *  DeSmet C is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 */
			  /*  Start of file CC.C  */

/*
		 O88 Optimizer for DeSmet (C-Ware) C-Compiler
		 --------------------------------------------

			      Copyright (c) 1986

				      by

			    Key Software Products
		    440 Ninth Avenue, Menlo Park, CA 94025
				(415) 364-9847

			    (All rights reserved)


*/

unsigned char _cc ;

#include <stdio.h>
#include "inc\o88.h"
#include "inc\version.h"

typedef	long FAR_PTR ;

typedef struct DTA
	{
	char rsvd[21] ;         /* Reserved for use by DOS */
	char attb ;             /* Attribute of file       */
	long stamp ;            /* date and time stamp     */
	long size ;             /* size of file in bytes   */
	char name[13] ;         /* filename and extension  */
	char xtra[85] ;         /* filled out to 128 bytes */
	} DTA ;


/* PUBLIC Library variables...						*/
/* -------------------------------------------------------------------- */
PUBLIC unsigned _rax, _rbx, _rcx, _rdx, _rsi, _rdi, _res, _rds, _carryf ;

#ifdef	_lint

#ifdef	DEBUG
VOID getchar(VOID) ;
VOID printf(CHAR *, ...) ;
#endif

unsigned exec(CHAR *, CHAR *) ;
VOID putchar(CHAR) ;
VOID puts(CHAR *) ;
int strcmp(CHAR *, CHAR *) ;
FILE *fopen(CHAR *, CHAR *) ;
CHAR *fgets(CHAR *, unsigned, FILE *) ;
int fputs(CHAR *, FILE *) ;
VOID fclose(FILE *) ;
BOOLEAN isspace(CHAR) ;

PRIVATE VOID Ctl_Brk(VOID) ;
PRIVATE VOID Delete_File(CHAR *) ;
PRIVATE BOOLEAN Directory(CHAR *) ;
PRIVATE CHAR *Full_Path_Name(CHAR *) ;
PRIVATE VOID Get_Cur_Path(CHAR *) ;
PRIVATE FAR_PTR Get_DTA(VOID) ;
PRIVATE CHAR *Get_Env_Str(CHAR *) ;
PRIVATE CHAR *Get_Obj_Name(VOID) ;
PRIVATE CHAR *Get_O88_Input(VOID) ;
PRIVATE CHAR *Get_O88_Output(VOID) ;
PRIVATE CHAR *Get_Src_Name(CHAR *) ;
PRIVATE unsigned My_Exec(CHAR *, CHAR *) ;
PRIVATE VOID Process_Args(CHAR *) ;
PRIVATE unsigned Process_Cmd_Line(unsigned, CHAR *[]) ;
PRIVATE VOID Process_Options(CHAR *) ;
PRIVATE unsigned Run_ASM(VOID) ;
PRIVATE unsigned Run_C88(unsigned, CHAR *[]) ;
PRIVATE unsigned Run_O88(unsigned, CHAR *[]) ;
PRIVATE unsigned Run_Bind(long, BOOLEAN) ;
PRIVATE BOOLEAN Search_First(CHAR *, unsigned) ;
PRIVATE BOOLEAN Search_Next(VOID) ;
PRIVATE VOID Set_DTA(FAR_PTR) ;
PRIVATE VOID Sign_On(VOID) ;
PRIVATE VOID Build_File_List(CHAR *, CHAR *) ;
PRIVATE CHAR *Indirect(CHAR *) ;
PRIVATE VOID Record_OFile(CHAR *) ;

#else

unsigned exec() ;
VOID putchar() ;
VOID puts() ;
int strcmp() ;
FILE *fopen() ;
CHAR *fgets() ;
int fputs() ;
VOID fclose() ;
BOOLEAN isspace() ;

PRIVATE VOID Ctl_Brk() ;
PRIVATE VOID Delete_File() ;
PRIVATE BOOLEAN Directory() ;
PRIVATE CHAR *Full_Path_Name() ;
PRIVATE VOID Get_Cur_Path() ;
PRIVATE FAR_PTR Get_DTA() ;
PRIVATE CHAR *Get_Env_Str() ;
PRIVATE CHAR *Get_Obj_Name() ;
PRIVATE CHAR *Get_O88_Input() ;
PRIVATE CHAR *Get_O88_Output() ;
PRIVATE CHAR *Get_Src_Name() ;
PRIVATE unsigned My_Exec() ;
PRIVATE VOID Process_Args() ;
PRIVATE unsigned Process_Cmd_Line() ;
PRIVATE VOID Process_Options() ;
PRIVATE unsigned Run_ASM() ;
PRIVATE unsigned Run_C88() ;
PRIVATE unsigned Run_O88() ;
PRIVATE unsigned Run_Bind() ;
PRIVATE BOOLEAN Search_First() ;
PRIVATE BOOLEAN Search_Next() ;
PRIVATE VOID Set_DTA() ;
PRIVATE VOID Sign_On() ;
PRIVATE VOID Build_File_List() ;
PRIVATE CHAR *Indirect() ;
PRIVATE VOID Record_OFile() ;

#endif

#ifndef	OLD_ASM88
PRIVATE BOOLEAN superset_enabled = TRUE ;
#endif

#define	FILESPACE 4000
#define	TEMPFILE  "$$TEMP"

PRIVATE CHAR filelist[FILESPACE] ;
PRIVATE CHAR tempfile[15] = TEMPFILE ;
PRIVATE BOOLEAN update = FALSE ;
PRIVATE BOOLEAN display = FALSE ;
PRIVATE CHAR *exe_file = NULL ;
PRIVATE BOOLEAN interrupts = FALSE ;
PRIVATE BOOLEAN o88_enabled = TRUE ;
PRIVATE CHAR c88_args[200], o88_args[100], asm88_args[100] ;
PRIVATE CHAR *b_switch, *t_switch, *o_switch, *a_switch, *c_switch, *m_switch;
PRIVATE CHAR *c_file, *o_file, *o88_inp, *o88_out ;
PRIVATE unsigned o88_arg1, o_arg ;
PRIVATE BOOLEAN wildcards ;
PRIVATE CHAR obj_ext[5] ;

PRIVATE CHAR *msgs[] =
{
"No (matching) source code files!\7\n",
"C88's -O switch must specify directory when multiple .C files are used!\7\n",
"CC terminated by CTL-C or CTL-BREAK...\n",
"No command line arguments!\n",
"Too many source files!\7\n",
"Can't open @indirect file!\7\n",
"Can't create temporary indirect file for BIND!\7\n"
} ;

#define	SRC_ERR	msgs[0]
#define	OBJ_ERR	msgs[1]
#define	CTL_BRK	msgs[2]
#define	ARG_ERR	msgs[3]
#define	SPC_ERR msgs[4]
#define	IND_ERR msgs[5]
#define	TMP_ERR msgs[6]

main(argc, argv)
unsigned argc ;
CHAR *argv[] ;
	{
	unsigned arg, rtn_code = 0 ;
	BOOLEAN new_ofile ;
	DTA dta ;
	unsigned dta_adr[2] ;
	long c_stamp, o_stamp, o_newer ;

	if (argc == 1)
		{
		Sign_On() ;
		puts(ARG_ERR) ;
		exit(ERRORS) ;
		}

	/* We'll need the unused argv[0] to split 1 command line arg */
	_move(--argc * sizeof(CHAR *), &argv[1], &argv[0]) ;

	Process_Args(Get_Env_Str("O88.1")) ;		/* 3rd priority */
	Process_Args(Get_Env_Str("O88")) ;		/* 2nd priority */
	argc = Process_Cmd_Line(argc, argv) ;		/* 1st priority */

	/* ------------------------------------- */
	/* Disable optimizer if -A is specified. */
	/* ------------------------------------- */
	if (a_switch != NULL) o88_enabled = FALSE ;

	wildcards = index(argv[0], '?')!=NULL ||
		    index(argv[0], '*')!=NULL ||
		    index(argv[0], ',')!=NULL ||
		    index(argv[0], '@')!=NULL ||
		    Directory(argv[0]) ;

	if (wildcards) Sign_On() ;

	/* ---------------------------------- */
	/* If more than one source file, then */
	/* o_switch must be a subdirectory.   */
	/* ---------------------------------- */
	if (wildcards && o_switch!=NULL && !Directory(&o_switch[1]))
		{
		Sign_On() ;
		puts(OBJ_ERR) ;
		exit(ERRORS) ;
		}

	/* Set-up for ^C or Ctl-Break */
	Ctl_Brk() ;

	/* --------------------------------------------- */
	/* Perform everything on the files that we find! */
	/* --------------------------------------------- */
	Build_File_List(argv[0], filelist) ;
	o_newer = -1L ;
	new_ofile = FALSE ;

	for (c_file = filelist; *c_file != '\0'; c_file += strlen(c_file) + 1)
		{
		/* Prepare filenames */
		o_file  = Get_Obj_Name() ;
		o88_inp = Get_O88_Input() ;
		o88_out = Get_O88_Output() ;

		/* Switch to local DTA */
		dta_adr[0] = (unsigned) &dta ;
		dta_adr[1] = _showds() ;
		Set_DTA(*((FAR_PTR *) dta_adr)) ;

		c_stamp = Search_First(c_file, 0x00) ? dta.stamp : 0L ;
		o_stamp = Search_First(o_file, 0x00) ? dta.stamp : 0L ;

		if (o_stamp > o_newer) o_newer = o_stamp ;

		if (exe_file != NULL) Record_OFile(o_file) ;

		if ((update || display) && (c_stamp < o_stamp)) continue ;
		new_ofile = TRUE ;

		if (wildcards || display)
			{
			puts("CC ") ;
			puts(c_file) ;
			puts(" -o") ;
			puts(o_file) ;
			for (arg = 1; arg < argc; arg++)
				{
				if (arg != o_arg)
					{
					putchar(' ') ;
					puts(argv[arg]) ;
					}
				}
			putchar('\n') ;

			if (display) continue ;
			}

		/* Exec programs */
		if ((rtn_code = Run_C88(argc, argv)) > 1) break ;
		if ((rtn_code = Run_O88(argc, argv)) > 1) break ;
		if ((rtn_code = Run_ASM()) > 1) break ;

		if (wildcards || (exe_file != NULL)) putchar('\n') ;
		}

	/* Delete temp files in case we broke out of while */
	Delete_File(o88_inp) ;
	Delete_File(o88_out) ;

	/* Kill .O file if any errors */
	if (rtn_code > 1) Delete_File(o_file) ;
	else if (exe_file != NULL) rtn_code = Run_Bind(o_newer, new_ofile) ;

	if (interrupts) puts(CTL_BRK) ;

	exit(rtn_code) ;
	}

PRIVATE CHAR *Get_O88_Output()
	{
	static CHAR o88_out[100] ;

	if (t_switch != NULL)
		{
		o88_out[0] = t_switch[1] ;
		o88_out[1] = ':' ;
		strcpy(&o88_out[2], "$$TEMP.A") ;
		}
	else strcpy(o88_out, "$$TEMP.A") ;
	return o88_out ;
	}

PRIVATE CHAR *Get_O88_Input()
	{
	static CHAR o88_inp[100] ;

	strcpy(o88_inp, c_file) ;
	strcpy(rindex(o88_inp, '.'), ".A") ;
	return o88_inp ;
	}

PRIVATE CHAR *Get_Obj_Name()
	{
	static CHAR o_file[100] ;
	CHAR *ptr ;
	unsigned last ;

	if (o_switch != NULL)
		{
		strcpy(o_file, &o_switch[1]) ;
		if (Directory(&o_switch[1]))
			{
			last = strlen(o_file) - 1 ;
			if (o_file[last]!=':' && o_file[last]!='\\')
				{
				strcat(o_file, "\\") ;
				}
			if ((ptr = rindex(c_file, '\\')) == NULL)
				{
				ptr = c_file - 1 ;
				}
			strcat(o_file, &ptr[1]) ;
			}
		}
	else strcpy(o_file, c_file) ;
		
	if ((ptr = rindex(o_file, '\\')) == NULL) ptr = o_file ;
	if ((ptr = index(ptr, '.')) != NULL) *ptr = '\0' ;
	strcat(o_file, obj_ext) ;

	return o_file ;
	}

PRIVATE unsigned Run_C88(argc, argv)
unsigned argc ;
CHAR *argv[] ;
	{
	unsigned arg, rtn_code ;
	CHAR *ptr ;
	CHAR del_file[100] ;

	/* ------------------------------------------ */
	/* Build name of .O file from name of .C file */
	/* ------------------------------------------ */
	o_file = Get_Obj_Name() ;

	if (argc < 1)
		{
		Sign_On() ;
		puts(SRC_ERR) ;
		exit(ERRORS) ;
		}

	/* -------------------------------------------- */
	/* Build (most) command line arguments for C88. */
	/* -------------------------------------------- */
	strcpy(c88_args, c_file) ;

	if (!o88_enabled)
		{
		strcat(c88_args, " -O") ;
		strcat(c88_args, o_file) ;
		/* Remove .O extension (C88 will add its own regardless!) */
		*rindex(c88_args, '.') = '\0' ;
		}

	for (arg = 1; arg < o88_arg1; arg++)
		{
		ptr = argv[arg] ;
		if (*ptr == '+') continue ;
		if (*ptr == '-') ptr++ ;
		if (toupper(*ptr) != 'O')
			{
			strcat(c88_args, " ") ;
			strcat(c88_args, argv[arg]) ;
			}
		}

	/* --------------------------------- */
	/* Exec C88 alone if O88 is disabled */
	/* --------------------------------- */
	if (!o88_enabled)
		{
#ifdef	DEBUG
		printf("C88 %s\n", c88_args) ;
#endif
		return My_Exec("C88.EXE", c88_args) ;
		}

	/* --------------------------- */
	/* Add the -A and -C switches. */
	/* --------------------------- */
	strcat(c88_args, " -A") ;
	if (c_switch == NULL) strcat(c88_args, " -C") ;

	/* -------------------------------- */
	/* Exec C88 to generate the .A file */
	/* -------------------------------- */
#ifdef	DEBUG
	printf("C88 %s\n", c88_args) ;
#endif
	rtn_code = My_Exec("C88.EXE", c88_args) ;

	/* Delete extraneous .O file produced with -C,-A options */
	strcpy(del_file, c_file) ;
	ptr = rindex(del_file, '.') ;
	strcpy(ptr, ".O") ;
	Delete_File(del_file) ;
	strcpy(ptr, ".OBJ") ;
	Delete_File(del_file) ;

	putchar('\n') ;
	return rtn_code ;
	}


PRIVATE unsigned Run_O88(argc, argv)
unsigned argc ;
CHAR *argv[] ;
	{
	unsigned arg, rtn_code ;

	if (!o88_enabled) return 0 ;

	/* ------------------------------------- */
	/* Build command line options for O88:   */
	/* Use T and B switches if used for C88, */
	/* and append -C to disable comments.    */
	/* ------------------------------------- */
	strcpy(o88_args, "{") ;
	strcat(o88_args, o88_inp) ;
	strcat(o88_args, " }") ;
	strcat(o88_args, o88_out) ;
	for (arg = o88_arg1; arg < argc; arg++)
		{
		strcat(o88_args, " ") ;
		strcat(o88_args, argv[arg]) ;
		}
	if (t_switch != NULL)
		{
		strcat(o88_args, " ") ;
		strcat(o88_args, t_switch) ;
		strcat(o88_args, ":") ;
		}
	if (b_switch != NULL) strcat(o88_args, " +B") ;
	else strcat(o88_args, " -B") ;
	strcat(o88_args, " -C") ;

#ifdef	DEBUG
	printf("O88 %s\n", o88_args) ;
#endif
	/* ------------------------------------------------- */
	/* Exec O88 using undocumented "{inp_file }out_file" */
	/* ------------------------------------------------- */
	rtn_code = My_Exec("O88.EXE", o88_args) ;
	Delete_File(o88_inp) ;
	return rtn_code ;
	}


PRIVATE unsigned Run_ASM()
	{
	unsigned rtn_code ;

	if (!o88_enabled) return 0 ;

	/* ---------------------------------- */
	/* Build command line args for ASM88. */
	/*				      */
	/* #ifndef OLD_ASM88		      */
	/* If +1 option enabled, then pass    */
	/* to ASM88 as ".1"		      */
	/* #endif			      */
	/* ---------------------------------- */
	strcpy(asm88_args, o88_out) ;
	strcat(asm88_args, " -O") ;
	strcat(asm88_args, o_file) ;
#ifndef	OLD_ASM88
	if (superset_enabled) strcat(asm88_args, " -.1") ;
#endif
	if (t_switch != NULL)
		{
		strcat(asm88_args, " -") ;
		strcat(asm88_args, t_switch) ;
		}
	if (b_switch != NULL) strcat(asm88_args, " -B") ;
	if (m_switch != NULL) strcat(asm88_args, " -M") ;

#ifdef	DEBUG
	printf("ASM88 %s\n", asm88_args) ;
#endif
	/* ---------------------------- */
	/* Exec ASM88 to finish the job */
	/* ---------------------------- */
	rtn_code = My_Exec("ASM88.EXE", asm88_args) ;
	Delete_File(o88_out) ;
	return rtn_code ;
	}


PRIVATE unsigned Run_Bind(o_newer, new_ofile)
long o_newer ;
BOOLEAN new_ofile ;
	{
	unsigned rtn_code ;
	long x_stamp ;
	DTA dta ;
	unsigned dta_adr[2] ;
	CHAR bind_args[200] ;
	CHAR *p ;
	CHAR bind_pgm[13] ;

	if (m_switch != NULL) strcpy(bind_pgm, "LINK") ;
	else if (b_switch != NULL) strcpy(bind_pgm, "BBIND") ;
	else strcpy(bind_pgm, "BIND") ;

	Record_OFile(NULL) ;

	rtn_code = 0 ;

	/* Switch to local DTA */
	dta_adr[0] = (unsigned) &dta ;
	dta_adr[1] = _showds() ;
	Set_DTA(*((FAR_PTR *) dta_adr)) ;

	for (p = exe_file; *p != '\0'; p++) *p = toupper(*p) ;

	strcpy(bind_args, "-O") ;
	strcat(bind_args, exe_file) ;
	if ((p = rindex(bind_args, '.')) != NULL) *p = '\0' ;
	strcat(bind_args, ".EXE") ;

	x_stamp = Search_First(&bind_args[2], 0x00) ?  dta.stamp : 0L ;

	if ((update || display) && !new_ofile && (o_newer < x_stamp))
		{
		goto done ;
		}

	*rindex(bind_args, '.') = '\0' ;

	if (wildcards || display)
		{
		puts(bind_pgm) ;
		if (m_switch != NULL)
			{
			puts(" ") ;
			puts(Full_Path_Name("C.OBJ")) ;
			puts("+<obj-files>,") ;
			puts(exe_file) ;
			puts(",NUL,") ;
			puts(Full_Path_Name("C88.LIB")) ;
			puts(";\n") ;
			}
		else
			{
			puts(" <obj-files> -o") ;
			puts(&bind_args[2]) ;
			putchar('\n') ;
			}
		if (display) goto done ;
		}

	if (m_switch != NULL)
		{
		strcpy(bind_args, Full_Path_Name("C.OBJ")) ;
		strcat(bind_args, "+@") ;
		strcat(bind_args, tempfile) ;
		strcat(bind_args, ",") ;
		strcat(bind_args, exe_file) ;
		strcat(bind_args, ",NUL,") ;
		strcat(bind_args, Full_Path_Name("C88.LIB")) ;
		strcat(bind_args, ";") ;
		}
	else
		{
		strcat(bind_args, " -F") ;
		strcat(bind_args, tempfile) ;
		}

	strcat(bind_pgm, ".EXE") ;
	if ((rtn_code = My_Exec(bind_pgm, bind_args)) > 1)
		{
		strcpy(bind_args, exe_file) ;
		if ((p = rindex(bind_args, '.')) != NULL) *p = '\0' ;
		strcat(bind_args, ".EXE") ;
		Delete_File(bind_args) ;
		}

done:
	Delete_File(tempfile) ;
	return rtn_code ;
	}

PRIVATE unsigned Process_Cmd_Line(argc, argv)
unsigned argc ;
CHAR *argv[] ;
	{
	unsigned arg, i ;
	CHAR *ptr, *p ;

	/* -------------------------------------------- */
	/* Separate O88 options from C88 options,	*/
	/* eliminate leading minus on options, look	*/
	/* for big model (B), ramdisk (T), doslink (M), */
	/* and output file (O) switches.  Update the	*/
	/* optimize enable switch (+O) fm command line.	*/
	/* -------------------------------------------- */
	if (toupper(argv[0][1])=='U' && (argv[0][0]=='+' || argv[0][0]=='-'))
		{
		if (argv[0][0]=='+') update  = TRUE ;
		if (argv[0][0]=='-') display = TRUE ;
		for (arg = 1; arg < argc ; arg++)
			{
			argv[arg - 1] = argv[arg] ;
			}
		argc-- ;
		}

	if ((ptr = index(argv[0], '=')) != NULL)
		{
		exe_file = argv[0] ;
		*ptr = '\0' ;
		argv[0] = ptr + 1 ;
		}

	o88_arg1 = argc ;
	a_switch = NULL ;
	b_switch = NULL ;
	t_switch = NULL ;
	o_switch = NULL ;
	c_switch = NULL ;
	m_switch = NULL ;
	strcpy(obj_ext, ".O") ;
	o_arg = 0 ;
	for (arg = 0; arg < argc; arg++)
		{
		if ((p = index(ptr = argv[arg], '/')) != NULL)
			{
			*p++ = '\0' ;
			for (i = argc - 1; i > arg; i--)
				{
				argv[i+1] = argv[i] ;
				}
			argv[o88_arg1 = arg + 1] = p ;
			argc++ ;
			}

		if (arg == 0) continue ;

		if (o88_arg1 == argc)
			{
			if (*ptr == '-') ptr++ ;
			switch (toupper(*ptr))
				{
				case 'B':
					b_switch = ptr ;
					break ;
				case 'T':
					t_switch = ptr ;
					break ;
				case 'O':
					o_arg = arg ;
					o_switch = ptr ;
					break ;
				case 'A':
					a_switch = ptr ;
					break ;
				case 'C':
					c_switch = ptr ;
					break ;
				case 'M':
					m_switch = ptr ;
					strcpy(obj_ext, ".OBJ") ;
					break ;
				}
			}
		else Process_Args(ptr) ;
		}
	return argc ;
	}


PRIVATE VOID Process_Args(ptr)
CHAR *ptr ;
	{
	while (*ptr != '\0')
		{
		if (*ptr=='+' || *ptr=='-') Process_Options(ptr) ;

		/* Find end of current substring */
		do
			{
			ptr++ ;
			} while (*ptr!='\0' && *ptr!=' ') ;
		}
	}


PRIVATE VOID Process_Options(ptr)
CHAR *ptr ;
	{
	CHAR ch ;
	BOOLEAN on ;

	while ((ch = toupper(*ptr++)) != '\0')
		{
		if (ch == '+') on = TRUE ;
		else if (ch == '-') on = FALSE ;
		else if (ch == 'O') o88_enabled = on ;
#ifndef	OLD_ASM88
		else if (ch == '1') superset_enabled = on ;
#endif
		}
	}


PRIVATE unsigned My_Exec(program, args)
CHAR *program, *args ;
	{
	unsigned status ;
	CHAR *path ;

	if ((path = Full_Path_Name(program)) != NULL)
		{
		interrupts = 0x00 ;
		status = exec(path, args) ;
		return (status | interrupts) ;
		}
	return 0xFF ;
	}


PRIVATE CHAR *Full_Path_Name(program)
CHAR *program ;
	{
	static CHAR full_path_name[200] ;
	CHAR cur_path[200] ;
	CHAR *dir, *ptr ;
	unsigned len ;

	/* look in current directory first */
	strcpy(full_path_name, program) ;
	if (Search_First(full_path_name, 0x00)) return full_path_name ;

	Get_Cur_Path(cur_path) ;
	for (dir = cur_path; *dir; dir = index(dir, ';') + 1)
		{
		strcpy(full_path_name, dir) ;
		if ((ptr = index(full_path_name, ';')) != NULL) *ptr = '\0' ;
		len = strlen(full_path_name) ;
		if (!len) continue ;
		ptr = &full_path_name[len] ;
		if ((ptr[-1] != '\\') && (ptr[-1] != ':')) *ptr++ = '\\' ;
		strcpy(ptr, program) ;
		if (Search_First(full_path_name, 0x00)) return full_path_name;
		}

	return NULL ;
	}


PRIVATE VOID Get_Cur_Path(cur_path)
CHAR *cur_path ;
	{
	CHAR *ptr ;

	*cur_path = '\0' ;
	if ((ptr = Get_Env_Str("PATH")) != NULL) strcpy(cur_path, ptr) ;
	}


PRIVATE CHAR *Get_Env_Str(var)
CHAR *var ;
	{
	unsigned offset = 0 ;
	CHAR *ptr, ch ;
	static CHAR bfr[200] ;
	extern unsigned _pcb ;	/* Pgm Ctl Blk: contents is segment of PSP */
	union
		{
		unsigned seg ;	/* environment segment */
		CHAR byte[2] ;	/* unpacked form */
		} env ;

	/* retrieve paragraph address of environment strings */
	env.byte[0] = _peek(0x2C, _pcb) ;
	env.byte[1] = _peek(0x2D, _pcb) ;

	while (_peek(offset, env.seg))
		{
		ptr = bfr ;
		do /* copy string */
			{
			ch = toupper(_peek(offset++, env.seg)) ;
			} while ((*ptr++ = ch) != '\0') ;
		while ((ptr = index(bfr, ' ')) != NULL) strcpy(ptr, ptr + 1) ;
		*(ptr = index(bfr, '=')) = '\0' ;
		if (!strcmp(bfr, var)) return ptr + 1 ;
		}
	return NULL ;
	}


PRIVATE FAR_PTR Get_DTA()
	{
	unsigned words[2] ;

	_rax = 0x2F00 ;
	_doint(0x21) ;
	words[0] = _rbx ;
	words[1] = _res ;
	return *((FAR_PTR *) words) ;
	}

PRIVATE VOID Set_DTA(off_seg)
FAR_PTR off_seg ;
	{
	unsigned words[2] ;

	*((FAR_PTR *) words) = off_seg ;

	_rdx = words[0] ;
	_rds = words[1] ;
	_rax = 0x1A00 ;
	_doint(0x21) ;
	}

PRIVATE BOOLEAN Search_First(file, attb)
CHAR *file ;
unsigned attb ;
	{
	_rax = 0x4E00 ;
	_rdx = (unsigned) file ;
	_rds = _showds() ;
	_rcx = attb ;
	_doint(0x21) ;
	return (_carryf == 0) ;
	}

PRIVATE BOOLEAN Search_Next()
	{
	_rax = 0x4F00 ;
	_doint(0x21) ;
	return (_carryf == 0) ;
	}

PRIVATE VOID Delete_File(filename)
CHAR *filename ;
	{
#ifdef	DEBUG
	printf("Delete_File(%s)\n", filename) ;
#endif
	if (filename[0] != '\0')
		{
		_rax = 0x4100 ;	/* Delete file */
		_rds = _showds() ;
		_rdx = (unsigned) filename ;
		_doint(0x21) ;
		}
	}

PRIVATE VOID Build_File_List(arg1, filelist)
CHAR *arg1 ;
CHAR *filelist ;
	{
	CHAR *spec, *filespec, *filename, *list, *p ;
	unsigned len, ttl_len ;
	
	ttl_len = 0 ;
	*(list = filelist) = '\0' ;
	while (*arg1 != '\0')
		{
		if ((p = index(spec = arg1, ',')) != NULL)
			{
			*p = '\0' ;
			arg1 = p + 1 ;
			}
		else arg1 = "" ;

		while ((filespec = Indirect(spec)) != NULL)
			{
			spec = NULL ;

			/* Force filespec to uppercase for uniform matching */
			/* ------------------------------------------------ */
			for (p = filespec ; *p != '\0'; p++)
				{
				*p = toupper(*p) ;
				}

			while ((filename = Get_Src_Name(filespec)) != NULL)
				{
				filespec = NULL ;

				/* Check for duplicates ... */
				/* ------------------------ */
				for (p=filelist; *p!='\0'; p+=strlen(p)+1)
					{
					if (strcmp(filename, p) == 0) break ;
					}

				if (*p == '\0')
					{
					len = strlen(filename) + 1 ;
					if ((ttl_len + len) >= FILESPACE)
						{
						Sign_On() ;
						puts(SPC_ERR) ;
						exit(ERRORS) ;
						}
					strcpy(list, filename) ;
					ttl_len += len ;
					list += len ;
					*list = '\0' ;
					}
				}
			}
		}

	}


PRIVATE CHAR *Get_Src_Name(c_file_spec)
CHAR *c_file_spec ;
	{
	static CHAR full_spec[80] ;
	static CHAR pattern[80] ;
	static DTA dta ;
	CHAR *p ;
	unsigned last ;
	FAR_PTR old_dta ;
	unsigned dta_adr[2] ;

	/* Save current DTA */
	old_dta = Get_DTA() ;

	/* Switch to local DTA */
	dta_adr[0] = (unsigned) &dta ;
	dta_adr[1] = _showds() ;
	Set_DTA(*((FAR_PTR *) dta_adr)) ;

	if (c_file_spec != NULL)	/* First entry? */
		{
		/* Work with a local copy of file_spec */
		strcpy(pattern, c_file_spec) ;

		/* Trim any trailing '\' from filespec ... */
		last = strlen(pattern) - 1 ;
		if (pattern[last] == '\\') pattern[last--] = '\0' ;

		/* If this is the name of a subdirectory */
		/* then append "\*" to end of pattern.  */
		if (Directory(pattern))
			{
			if (pattern[last] == ':') strcat(pattern, "*") ;
			else strcat(pattern, "\\*") ;
			}

		/* Be sure pattern has a trailing ".C" */
		if ((p = rindex(pattern, '\\')) == NULL) p = pattern ;
		if ((p = index(p, '.')) != NULL) *p = '\0' ;
		strcat(pattern, ".C") ;
		if (!Search_First(pattern, 0x00))
			{
			Sign_On() ;
			puts(SRC_ERR) ;
			exit(ERRORS) ;
			}
		}
	else if (!Search_Next()) return NULL ;

	/* Restore old DTA */
	Set_DTA(old_dta) ;

	/* Build full src file spec from directory and name */

	strcpy(full_spec, pattern) ;
	if ((p = rindex(full_spec, '\\')) == NULL)
		{
		p = full_spec - 1 ;
		/* drive spec? */
		if (p[2] == ':') p += 2 ; /* Skip over it */
		}
	strcpy(p + 1, dta.name) ;

	return full_spec ;
	}

PRIVATE BOOLEAN Directory(name)
CHAR *name ;
	{
	unsigned last ;
	CHAR copy[80] ;
	DTA dta ;
	FAR_PTR old_dta ;
	unsigned dta_adr[2] ;
	BOOLEAN found ;


	/* Wildcards not allowed in directory names */
	if (index(name, '?') != NULL) return FALSE ;
	if (index(name, '*') != NULL) return FALSE ;

	/* Work with local copy */
	strcpy(copy, name) ;

	/* Remove trailing backslash (if any) */
	last = strlen(copy) - 1 ;
	if (copy[last] == '\\') copy[last--] = '\0' ;
	if (copy[last] == ':') return TRUE ;

	/* Save current DTA */
	old_dta = Get_DTA() ;

	/* Use local DTA */
	dta_adr[0] = (unsigned) &dta ;
	dta_adr[1] = _showds() ;
	Set_DTA(*((FAR_PTR *) dta_adr)) ;

	/* Is this the name of a subdirectory? */
	found = Search_First(copy, 0x10) ;

	/* Restore old DTA */
	Set_DTA(old_dta) ;

	return (found && (dta.attb & 0x10)!=0) ;
	}


PRIVATE VOID Ctl_Brk()
	{
#ifndef	_lint
#asm
	jmp	Set_Up

Int_23:
	push	es
	mov	es,cs:save_ds
	mov	es:interrupts_,0FFh
	pop	es
	stc
	lret

save_ds	dw	0

Set_Up:
	mov	cs:save_ds,ds
	push	ds
	mov	ax,cs
	mov	ds,ax
	mov	dx,OFFSET Int_23
	mov	ax,2523h	; set ctl brk int vec
	int	21h
	pop	ds
#end
#endif
	}


PRIVATE VOID Sign_On()
	{
	static BOOLEAN virgin = TRUE ;

	if (virgin) puts(CC) ;
	virgin = FALSE ;
	}

PRIVATE CHAR *Indirect(spec)
CHAR *spec ;
	{
	static FILE *fp = NULL ;
	static CHAR bfr[100] ;
	CHAR *p ;

	if (spec!=NULL && *spec=='@')
		{
		if ((fp = fopen(spec + 1, "r")) == NULL)
			{
			Sign_On() ;
			puts(IND_ERR) ;
			exit(ERRORS) ;
			}
		}

	if (fp != NULL)
		{
		if (fgets(bfr, sizeof(bfr), fp) != NULL)
			{
			/* Strip whitespace */
			for (spec = bfr ; isspace(*spec) ; spec++) ;
			for (p = spec ; !isspace(*p) ; p++) ;
			*p = '\0' ;
			}
		else
			{
			/* End of file */
			fclose(fp) ;
			fp = NULL ;
			spec = NULL ;
			}
		}

	return spec ;
	}

PRIVATE VOID Record_OFile(o_file)
CHAR *o_file ;
	{
	static FILE *fp = NULL ;
	int sts ;

	if (o_file == NULL)
		{
		if ((sts = fputs("\n", fp)) == ERR)
			{
			Sign_On() ;
			puts(TMP_ERR) ;
			exit(ERRORS) ;
			}
		fclose(fp) ;
		return ;
		}

	sts = 0 ;
	if (fp == NULL)
		{
		if (t_switch != NULL)
			{
			tempfile[0] = t_switch[1] ;
			tempfile[1] = ':' ;
			strcpy(&tempfile[2], TEMPFILE) ;
			}

		strcat(tempfile, TEMPFILE) ;
		if ((fp = fopen(tempfile, "w")) == NULL) sts = ERR ;
		if (sts != ERR) sts = fputs(o_file, fp) ;
		}
	else
		{
		if (sts != ERR && m_switch != NULL) sts = fputs("+", fp) ;
		if (sts != ERR) sts = fputs("\n", fp) ;
		if (sts != ERR) sts = fputs(o_file, fp) ;
		}

	if (sts==ERR)
		{
		Sign_On() ;
		puts(TMP_ERR) ;
		exit(ERRORS) ;
		}
	}

			   /*  End of file CC.C  */

