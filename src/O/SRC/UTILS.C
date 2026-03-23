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
			/*  Start of file UTILS.C  */

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

#include "inc\o88.h"
#include "inc\version.h"

/* Functions PRIVATE to UTILS.C						*/
/* -------------------------------------------------------------------- */
#ifdef	_lint

VOID		Usage_Line(SWITCH *) ;
VOID		Postpone(PARTS *) ;
PARTS		*Make_Parts(CHAR *, unsigned) ;
CHAR		Process_Options(CHAR *) ;
CHAR		Process_Args(CHAR *) ;
CHAR		Process_Cmd_Line(unsigned, CHAR *[]) ;
VOID		Usage_Line(SWITCH *) ;
PARTS		*Push_Pop(PARTS *, OPERAND *) ;
PARTS		*Avoid_Stack(PARTS *, OPERAND *) ;
BOOLEAN		Push_Reg(PARTS *) ;
BOOLEAN		Push_Mem(PARTS *) ;
PARTS		*Get_Level2(PARTS *) ;
PARTS		*Get_Level1(PARTS *) ;
PARTS		*Get_Level0(PARTS *) ;
VOID		Call_Replace(PARTS *) ;
VOID		Combine(PARTS *) ;
BOOLEAN		PP_Load(PARTS *, PARTS *) ;
VOID		Option_Err(CHAR *, CHAR) ;
BOOLEAN		Adjust_Offsets(PARTS *, int, FLAGS) ;
VOID		Absorb_Constant(PARTS *) ;
PARTS		*Find_Level0(PARTS *, PARTS *(*)()) ;

#else

VOID		Usage_Line() ;
VOID		Postpone() ;
PARTS		*Make_Parts() ;
CHAR		Process_Options() ;
CHAR		Process_Args() ;
CHAR		Process_Cmd_Line() ;
PARTS		*Push_Pop() ;
PARTS		*Avoid_Stack() ;
BOOLEAN		Push_Reg() ;
BOOLEAN		Push_Mem() ;
PARTS		*Get_Level2() ;
PARTS		*Get_Level1() ;
PARTS		*Get_Level0() ;
VOID		Call_Replace() ;
VOID		Combine() ;
BOOLEAN		PP_Load() ;
VOID		Option_Err() ;
BOOLEAN		Adjust_Offsets() ;
VOID		Absorb_Constant() ;
PARTS		*Find_Level0() ;

#endif


PRIVATE CHAR *scratch_dir = null ;
PRIVATE REG *ndxreg ;


VOID Error_Msg(label, file)
CHAR *label, *file ;
	{
	static CHAR bfr[] = "00000" ;
	static CHAR *err_table[] =
		{
		"Disk full",				/*0*/
		null,					/*1*/
		"File not found",			/*2*/
		"Path not found",			/*3*/
		"Too many open files",			/*4*/
		"Access denied",			/*5*/
		"Invalid handle",			/*6*/
		null,					/*7*/
		null,					/*8*/
		null,					/*9*/
		null,					/*10*/
		null,					/*11*/
		"Invalid access code",			/*12*/
		null,					/*13*/
		null,					/*14*/
		null,					/*15*/
		null,					/*16*/
		"Not same device",			/*17*/
		null					/*18*/
		} ;
#	define	ERROR_MSGS	(sizeof(err_table)/sizeof(CHAR *))

	Errs("\r\n*** ") ;
	Errs(label) ;
	Errs(file) ;
	Errs(" Error #") ;
	Errs(Convert(err_code, &bfr[4])) ;
	Errs(" (") ;
	Errs((err_code < ERROR_MSGS) ? err_table[err_code] : "UNKNOWN") ;
	Errs(") ***\r\n\7") ;
	exit(ERRORS) ;
	}


VOID Output(parts)
PARTS *parts ;
	{
	CHAR line[100] ;
	CHAR *ptr, *cmt ;
	BOOLEAN err, cmts, code ;

	err = (parts->attb & ERRMSG) != 0 ;
	cmts = err || option.comments.enabled ;

	if (!cmts && *parts->label==';') return ;

	if (option.superset.enabled) Assemble(parts) ;

	*(ptr = line) = '\0' ;
	code = (*parts->label != '\0') ;
	if (code) ptr = Append(ptr, parts->label) ;

	if (*parts->mnemonic != '\0')
		{
		*ptr = ' ' ;
		ptr = Append(ptr + 1, parts->mnemonic) ;
		code = TRUE ;
		}

	if (parts->dst.len != 0)
		{
		*ptr++ = ' ' ;
		_move(parts->dst.len, parts->dst.orig, ptr) ;
		ptr += parts->dst.len ;
		}

	if (parts->src.len != 0)
		{
		*ptr++ = ',' ;
		_move(parts->src.len, parts->src.orig, ptr) ;
		ptr += parts->src.len ;
		}

	if (*parts->third != '\0')
		{
		*ptr = ',' ;
		ptr = Append(ptr + 1, parts->third) ;
		}

	cmt = NULL ;
	if (cmts && *parts->comment!='\0')
		{
		*ptr++ = ' ' ;
		cmt = ptr ;
		*ptr++ = ';' ;
		ptr = Append(ptr, parts->comment) ;
		}

	*ptr = '\0' ;

	ptr = line ;
	if (!code && cmt!=NULL) ptr = cmt ;

	if (err)
		{
		Errs(ptr) ;
		Errs("\7\r\n") ;
		exit_code = ERRORS ;
		}

	Put_Line(ptr, &code_bfr) ;
	}


VOID Get_Args(argc, argv)
unsigned argc ;
CHAR *argv[] ;
	{
	int arg ;
	CHAR *ptr ;
	CHAR bad1, bad2, bad3 ;

	/* Convert argument list to upper case and check for filenames */
	/* ----------------------------------------------------------- */
	for (arg = 1; arg < argc; arg++)
		{
		if (argv[arg][0] == '{')
			{
			src_file = &argv[arg][1] ;
			continue ;
			}
		if (argv[arg][0] == '}')
			{
			dst_file = &argv[arg][1] ;
			continue ;
			}
		for (ptr = argv[arg]; *ptr!='\0'; ptr++)
			{
			*ptr = toupper(*ptr) ;
			}
		}

	/* I/O Filenames determined - ok to issue output now */
	/* ------------------------------------------------- */
	Errs(OPENO88) ; /* Sign-On */
	Errs(O88) ; /* Sign-On */

	/* Process options specified by environment variables */
	/* -------------------------------------------------- */
	ptr = Get_Env_Str("O88.1") ;		/* 3rd priority */
	bad1 = (ptr != NULL) ? Process_Args(ptr) : '\0' ;

	ptr = Get_Env_Str("O88") ;		/* 2nd priority */
	bad2 = (ptr != NULL) ? Process_Args(ptr) : '\0' ;


	/* Check for minimal DOS Version */
	/* ----------------------------- */
	if (DOS_Version() < 2)
		{
		Errs("\r\n*** O88 requires DOS 2.0 or higher! ***\7\r\n") ;
		Usage(ERRORS) ;
		}

	bad3 = Process_Cmd_Line(argc, argv) ;

	if (!option.numeric.enabled)	  option.init8087.enabled = FALSE ;

	if (bad1 != '\0') Option_Err("O88.1 environment string", bad1) ;
	if (bad2 != '\0') Option_Err("O88 environment string", bad2) ;
	if (bad3 != '\0') Option_Err("Command line argument", bad3) ;

	if (Equal(src_file, "STDIN") && No_Input_File()) Usage(PERFECT) ;

	option.optimize.enabled = TRUE ;	/* Go for it! */
	}


CHAR *Make_Filename()
	{
	static unsigned number = 1 ;
	static CHAR filename[] = "$O88TMP$.000" ;
	static CHAR filepath[100] ;

	Convert(number++, index(filename, '.') + 3) ;
	strcpy(filepath, scratch_dir) ;
	strcat(filepath, filename) ;
	return filepath ;
	}


PRIVATE CHAR Process_Args(ptr)
CHAR *ptr ;
	{
	CHAR err, ch ;

	err = '\0' ;
	while (*ptr != '\0')
		{
		switch (*ptr)
			{
			case ' ':	/* Blanks seperate substrings */
				*ptr = '\0' ;
				break ;

			case '+':	/* Turn options on */
			case '-':	/* Turn options off */
				ch = Process_Options(ptr) ;
				if (err == '\0') err = ch ;
				break ;

			case 'T':	/* Temporary directory prefix */
				scratch_dir = ptr + 1 ;
				break ;

			case 'S':	/* Clean stack filename */
				clean_stack = ptr + 1 ;
				break ;

			default:	/* Illegal substring argument */
				if (err == '\0') err = *ptr ;
			}

		/* Find end of current substring */
		do
			{
			ptr++ ;
			} while (*ptr!='\0' && *ptr!=' ') ;
		}

	return err ;
	}


PRIVATE CHAR Process_Options(ptr)
CHAR *ptr ;
	{
	CHAR err, ch ;
	SWITCH *opt ;
	int i ;
	BOOLEAN on ;

	err = '\0' ;
	while ((ch = *ptr++) != '\0')
		{
		if (ch == '+') on = TRUE ;
		else if (ch == '-') on = FALSE ;
		else
			{
			opt = (SWITCH *) &option;
			for (i = 0; i < OPTIONS; i++, opt++)
				{
				if (ch == opt->selector)
					{
					opt->enabled = on ;
					break ;
					}
				}
			if (err=='\0' && i==OPTIONS) err = ch ;
			}
		}

	return err ;
	}


PRIVATE VOID Option_Err(who, ch)
CHAR *who ;
CHAR ch ;
	{
	Errs("\r\n*** ") ;
	Errs(who) ;
	Errs(" contains bad option: ") ;
	Errc(ch) ;
	Errs(" ***\7\r\n") ;
	Usage(ERRORS) ;
	}


PRIVATE VOID Usage_Line(opt)
SWITCH *opt ;
	{
	Errs("  ") ;
	Errc((CHAR) (opt->enabled ? '+' : '-')) ;
	Errc(opt->selector) ;
	Errs("   ") ;
	Errs(opt->enabled ? opt->on_msg : opt->off_msg) ;
	Errs("\r\n") ;
	}

VOID Usage(exit_code)
unsigned exit_code ;
	{
	SWITCH *opt ;
	BOOLEAN printed ;
	int i ;

	Errs("\r\nCurrent default settings are:\r\n\r\n") ;

	Usage_Line(&option.optimize) ;
	Errs("\r\n") ;

	printed = FALSE ;
	opt = (SWITCH *) &option ;
	for (i = 0; i < OPTIONS; i++, opt++)
		{
		if (opt == &option.optimize) continue ;
		if (!opt->enabled) continue ;
		Usage_Line(opt) ;
		printed = TRUE ;
		}

	if (printed) Errs("\r\n") ;

	opt = (SWITCH *) &option ;
	for (i = 0; i < OPTIONS; i++, opt++)
		{
		if (opt == &option.optimize) continue ;
		if (opt->enabled) continue ;
		Usage_Line(opt) ;
		}

	exit(exit_code) ;
	}


PARTS *Get_Next(stmt)
PARTS *stmt ;
	{
	OPCODE *opc ;

	if (stmt!=NULL && stmt->next!=NULL) stmt = stmt->next ;
	else
		{
		stmt = Get_Level2(stmt) ;
		if (stmt!=NULL && stmt->label[0]=='\0')
			{
			if ((opc = stmt->opcode) == MOV)
				{
				if (stmt->dst.reg != NULL)
					{
					(VOID) Push_Reg(stmt) ;
					}
				}
			else if (opc==LEA || opc==LES) (VOID) Push_Reg(stmt) ;
			else if (opc==PUSH) (VOID) Push_Mem(stmt) ;
			if (stmt->label[0] == '\0') Postpone(stmt) ;
			}
		}

	return stmt ;
	}


PRIVATE PARTS *Get_Level2(stmt)
PARTS *stmt ;
	{
	if (stmt!=NULL && stmt->next!=NULL) stmt = stmt->next ;
	else
		{
		stmt = Get_Level1(stmt) ;

		if (stmt!=NULL && stmt->label[0]=='\0' && stmt->opcode==CALL)
			{
			Call_Replace(stmt) ;
			}
		}

	return stmt ;
	}


PRIVATE PARTS *Get_Level1(stmt)
PARTS *stmt ;
	{
	if (stmt!=NULL && stmt->next!=NULL) stmt = stmt->next ;
	else
		{
		stmt = Get_Level0(stmt) ;
		if (stmt!=NULL && stmt->label[0]=='\0' &&
		    stmt->src.reg==NULL && CONSTANT(stmt->src.attb) &&
		    stmt->dst.reg!=NULL && INDEX(stmt->dst.reg))
		    	{
			Absorb_Constant(stmt) ;
			}
		}

	return stmt ;
	}


PRIVATE PARTS *Get_Level0(stmt)
PARTS *stmt ;
	{
	static BOOLEAN cseg = FALSE ;
	PARTS *new ;
	unsigned len ;
	CHAR line[200] ;
	BOOLEAN public ;

	if (stmt!=NULL && stmt->next!=NULL) return stmt->next ;

	while ((len = Get_Line(line, &inp_bfr)) != 0)
		{
		if (line[0]==' ' && EqualN(&line[2], "SEG", 3))
			{
			cseg = (line[1] == 'C') ;
			if (cseg) continue ;
			}

		public = EqualN(line, " PUBLIC", 7) ;

		if (cseg)
			{
			if (public)
				{
				Put_Line(line, &cs_pub_bfr) ;
				continue ;
				}
			break ; /* CSEG text! Return to caller. */
			}

		if (!public) Zero_One(line) ;

		Put_Line(line, &data_bfr) ;
		}

	if (len == 0) return NULL ; /* end of file */

	new = Make_Parts(line, len) ;
	if (stmt != NULL) stmt->next = new ;

	if (new->opcode==LINE) new->label = deleted ;
	else if (new->label[0]=='\0')
		{
		if (new->opcode==CALL)
			{
			if (option.numeric.enabled) NDP_Replace(new) ;
			}
		else if (new->opcode==MOV)
			{
			if (new->dst.reg!=NULL) Combine(new) ;
			}
		}

	return new ;
	}

PRIVATE PARTS *Make_Parts(str, len)
CHAR *str ;
unsigned len ;
	{
	PARTS *parts ;
	OPCODE *opc ;
	CHAR *dst ;

	if (EqualN(str, " MOV", 4) && str[8]=='@')
		{
		/*
		-------------------------------------------------
		                  11                  11111
		col:	012345678901        012345678901234
		         MOV ??,@xyz         MOV ??,@DWORD xyz
		-------------------------------------------------
		*/
		parts = (PARTS *) Allocate(sizeof(PARTS) + len + 6) ;
		_move(9, str, dst = parts->stmt) ;
		_move(6, "DWORD ", dst + 9) ;
		strcpy(dst + 15, Skip_Prefix(str + 9)) ;
		}
	else if (EqualN(str, " LEA", 4))
		{
		/*
		------------------------------------------------
		                  11                  11111
		col:	012345678901        012345678901234
		str:     LEA ??,src     =>   LEA ??,WORD src
		------------------------------------------------
		*/
		parts = (PARTS *) Allocate(sizeof(PARTS) + len + 5) ;
		_move(8, str, dst = parts->stmt) ;
		_move(5, "WORD ", &dst[8]) ;
		strcpy(&dst[13], Skip_Prefix(&str[8])) ;
		}

	else if (EqualN(str, " LES", 4))
		{
		/*
		------------------------------------------------
		                  11                  11111
		col:	012345678901        012345678901234
		str:     LES ??,src     =>   LES ??,DWORD src
		str:     LES ??,ES:src  =>   LES ??,ES:DWORD src
		------------------------------------------------
		*/
		parts = (PARTS *) Allocate(sizeof(PARTS) + len + 6) ;
		_move(8, str, dst = parts->stmt) ;
		dst += 8 ; str += 8 ;
		if (str[0]=='@')
			{
			*dst++ = '@' ;
			str++ ;
			}
		else if (str[2]==':')
			{
			_move(3, str, dst) ;
			dst += 3 ; str += 3 ;
			}
		_move(6, "DWORD ", dst) ;
		strcpy(dst + 6, Skip_Prefix(str)) ;
		}
	else
		{
		parts = (PARTS *) Allocate(sizeof(PARTS) + len) ;
		_move(len + 1, str, parts->stmt) ;
		}

	parts->id = id++ ;
	parts->next = NULL ;

	Parse(parts) ;

	opc = parts->opcode ;
	if (opc!=NULL && opc!=LINE) stats.cseg_ttl++ ;

	return parts ;
	}

PRIVATE VOID Call_Replace(stmt)
PARTS *stmt ;
	{
	/* NOTE: This routine is called within a recursive loop!
		 To prevent the stack from growing too large, I
		 made the following temporary variables static.
		 None of them need to be preserved during the
		 recursion.
	*/
	static CHAR *dst, mne[80] ;
	static PARTS *next ;
	static CHAR *ptr, **str ;
	static LIB *lib ;
	static FNC *fnc ;
#	define	CALLCOST	10	/* (A tradeoff!) */

	dst = stmt->dst.orig ;
	if (dst[0]=='_' && dst[1]!='F' &&
	    ((lib = (LIB *) Find(dst))) != NULL)
	    	{
		fnc = option.big_model.enabled ? &lib->large : &lib->small ;
		if ((str = fnc->code) == NULL) return ;
		if (!option.time.enabled && (fnc->cost > CALLCOST)) return ;
		stmt->label = replaced ;
		next = Get_Level1(stmt) ;
		if (next->dst.reg==SP)
			{
			if (next->opcode==MOV)
				{
				next->label = deleted ;
				stats.cseg_del++ ;
				}
			else if (next->opcode==ADD)
				{
				if (Value(next->src.orig, next->src.len) !=
					fnc->bytes)
					{
					stmt->label = arguments ;
					stmt->attb |= ERRMSG ;
					}
				else
					{
					next->label = deleted ;
					stats.cseg_del++ ;
					}
				}
			}
		else if (fnc->bytes != 0)
			{
			stmt->label = arguments ;
			stmt->attb |= ERRMSG ;
			}
		next = stmt ;
		while (*str != NULL)
			{
			dst = null ;
			if ((ptr = index(strcpy(mne, *str++), ',')) != NULL)
				{
				*ptr = '\0' ;
				dst = ptr + 1 ;
				}
			Emit(next, null, mne, dst, null) ;
			if (Equal(mne, "POP")) stmt->attb |= POPXST ;
			next = next->next ;
			stats.cseg_ins++ ;
			}
		stats.call_del++ ;
		stats.cseg_del++ ;
		}
	}


PRIVATE VOID Combine(stmt)
PARTS *stmt ;
	{
	/* NOTE: This routine is called within a recursive loop!
		 To prevent the stack from growing too large, I
		 made the following temporary variables static.
		 None of them need to be preserved during the
		 recursion.
	*/
	static REG *dreg1, *dreg2, *rx ;
	static PARTS *next ;
	static int imm1, imm2 ;
	static PACKED n ;
	static CHAR bfr[] = "65535" ;

	if (stmt->src.reg==NULL && CONSTANT(stmt->src.attb))
	    	{
		next = Get_Next(stmt) ;
		dreg1 = stmt->dst.reg ;
		if (next!=NULL && next->opcode==MOV &&
		    ((dreg2 = next->dst.reg) != NULL) &&
		    dreg1!=dreg2 && RX(dreg1)==RX(dreg2) &&
		    next->src.reg==NULL &&
		    CONSTANT(next->src.attb))
			{
			/*
				-----------------------------
				MOV r1,imm1      ###
				MOV r2,imm2  =>  MOV rX,imm16
				-----------------------------
			*/
			imm1 = Value(stmt->src.orig, stmt->src.len) ;
			if (WORD(dreg1->attb))
				{
				n.word = (unsigned) imm1 ;
				}
			else if (dreg1->name[1] == 'L')
				{
				n.byte.lsb = (CHAR) imm1 ;
				}
			else
				{
				n.byte.msb = (CHAR) imm1 ;
				}
			imm2 = Value(next->src.orig, next->src.len) ;
			if (WORD(dreg2->attb))
				{
				n.word = (unsigned) imm2 ;
				}
			else if (dreg2->name[1] == 'L')
				{
				n.byte.lsb = (CHAR) imm2 ;
				}
			else
				{
				n.byte.msb = (CHAR) imm2 ;
				}
			rx = RX(dreg1) ;
			Emit(next, null, mov, rx->name,
				Convert(n.word, &bfr[4])) ;
			next->label = combined ;
			stmt->label = combined ;
			stats.cseg_del++ ;
			}
		}
	}


PRIVATE CHAR Process_Cmd_Line(argc, argv)
unsigned argc ;
CHAR *argv[] ;
	{
	unsigned arg ;
	CHAR err, ch ;

	err = '\0' ;
	for (arg = 1; arg < argc; arg++)
		{
		if (argv[arg][0] == '{' || argv[arg][0] == '}') continue ;
		if (argv[arg][0] == 'X') continue ;
		ch = Process_Args(argv[arg]) ;
		if (err == '\0') err = ch ;
		if (ch != '\0') break ;
		}
	return err ;
	}


PRIVATE VOID Postpone(new)
PARTS *new ;
	{
	/* NOTE: This routine is called within a recursive loop!
		 To prevent the stack from growing too large, I
		 made the following temporary variables static.
		 None of them need to be preserved during the
		 recursion.
	*/
	static OPCODE *opc ;

	/* However, these variables must be 
	   unique for each incarnation!
	*/
	PARTS *stmt, *after, *next ;
	FLAGS flags ;

	flags = new->opcode->attb ;
	if ((flags & MOVABLE)==0 ||
	    new->dst.reg!=NULL || new->src.reg!=NULL ||
	    VARDST(new->dst.attb) || VARDST(new->src.attb))
		{
		return ;
		}

	flags &= (DEF_FLG|ZRO_FLG) ;
	for (next=Get_Next(after=new); (stmt=next) != NULL; after=stmt)
		{
		if (*stmt->label!='\0') break ;
		opc = stmt->opcode ;
		if (opc==NULL) break ;
		if ((opc->attb & MOVABLE)==0) break ;
		if (MEMREF(stmt->src.attb) || MEMREF(stmt->dst.attb)) break ;
		if (flags!=0 && (opc->attb & (DEF_FLG|ZRO_FLG|BAD_FLG))!=0 &&
		    Flags_Needed(stmt))
			{
			break ;
			}
		next = Get_Next(stmt) ;
		if (next==NULL || next->dst.reg==SP) break ;
		}

	if (after != new)
		{
		Emit(after, null, new->mnemonic, new->dst.orig,
			new->src.orig) ;
		new->label = postponed ;
		}
	}

PARTS *Find_Next(stmt)
PARTS *stmt ;
	{
	return Find_Level0(stmt, Get_Next) ;
	}


PRIVATE PARTS *Find_Level0(stmt, get_next)
PARTS *stmt ;
PARTS *((*get_next)()) ;
	{
	OPCODE *opc ;

	while (stmt != NULL)
		{
		if (stmt->next != NULL) stmt = stmt->next ;
		else stmt = (*get_next)(stmt) ;
		if (stmt != NULL)
			{
			if (stmt->label[0] == ';') continue ;
			opc = stmt->opcode ;
			if (*stmt->label!='\0' || islower(*stmt->mnemonic))
				{
				stmt = NULL ;
				}
			if (opc!=NULL) break ;
			}
		}

	return stmt ;
	}


CHAR *Get_Env_Str(var)
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

	while (_peek(offset, env.seg) != '\0')
		{
		ptr = bfr ;
		do /* copy string */
			{
			ch = toupper(_peek(offset++, env.seg)) ;
			} while ((*ptr++ = ch) != '\0') ;
		while ((ptr = index(bfr, ' ')) != NULL)
			{
			strcpy(ptr, ptr + 1) ;
			}
		*(ptr = index(bfr, '=')) = '\0' ;
		if (Equal(bfr, var))
			{
			return ptr + 1 ;
			}
		}
	return NULL ;
	}


PRIVATE BOOLEAN Push_Reg(first)
PARTS *first ;
	{
	static CHAR seg[100], off[100] ;
	CHAR *dst ;
	PARTS *second, *third ;

	if ((second = Find_Level0(first, Get_Level2)) == NULL) return FALSE ;
	if ((third = Find_Level0(second, Get_Level2)) == NULL) return FALSE ;

	if (second->opcode==PUSH)
		{
		if (third->opcode==PUSH && third->dst.reg==first->dst.reg)
			{
			if (first->opcode==LES)
				{
				if (second->dst.reg==ES)
					{
					Seg_Off(first->src.orig,
						first->src.len, off, seg) ;
					dst = first->dst.orig ;
					Emit(third, null, push, dst, null) ;
					Emit(third, null, mov, dst, off) ;
					Emit(third, null, push, dst, null) ;
					Emit(third, null, mov, dst, seg) ;
					first->label = replaced ;
					second->label = replaced ;
					third->label = replaced ;
					return TRUE ;
					}
				return FALSE ;
				}

			if (!REFERENCED(first->dst.reg, second))
				{
				if (PP_Load(first, third)) return TRUE ;
				}
			}

		if (second->dst.reg==first->dst.reg)
			{
			return PP_Load(first, second) ;
			}
		}

	return FALSE ;
	}


PRIVATE BOOLEAN PP_Load(load_stmt, push_stmt)
PARTS *load_stmt ;
PARTS *push_stmt ;
	{
	PARTS *pop_stmt ;

	if (push_stmt->dst.reg==load_stmt->dst.reg &&
	    (pop_stmt = Avoid_Stack(push_stmt, &load_stmt->src)) != NULL)
		{
		/*
			--------------------------------------
			MOV/LEA/LES r,src  =>  ###
			[PUSH SegReg]          [PUSH SegReg]
			PUSH r                 ###
			...                    ...
			POP dst                MOV/LEA dst,src
			--------------------------------------
		*/
		Emit(pop_stmt, null, load_stmt->mnemonic,
			pop_stmt->dst.orig, load_stmt->src.orig);
		pop_stmt->label = combined ;
		push_stmt->label = combined ;
		load_stmt->label = combined ;
		stats.cseg_del += 2 ;
		return TRUE ;
		}

	return FALSE ;
	}

PRIVATE BOOLEAN Push_Mem(stmt)
PARTS *stmt ;
	{
	PARTS *pop_stmt ;

	if ((pop_stmt = Avoid_Stack(stmt, &stmt->dst)) != NULL)
		{
		/*
			--------------------------
			PUSH src
			...
			POP dst   ==>  MOV src,dst
			--------------------------
		*/
		Emit(pop_stmt, null, mov, pop_stmt->dst.orig, stmt->dst.orig);
		pop_stmt->label  = combined ;
		stmt->label = combined ;
		stats.cseg_del++ ;
		return TRUE ;
		}
	return FALSE ;
	}


PRIVATE PARTS *Avoid_Stack(stmt, sop)
PARTS *stmt ;
OPERAND *sop ;
	{
	REG *dreg, *sreg ;
	FLAGS sattb ;

	sreg = sop->reg ;
	if ((stmt = Push_Pop(stmt, sop)) != NULL)
		{
		sattb = sop->attb ;
		if ((dreg = stmt->dst.reg) != NULL)
			{
			if (dreg==BP) stmt = NULL ;	/* BP <- src */
			else if (SEGREG(dreg) &&
			    ((sreg!=NULL && SEGREG(sreg)) ||
			     CONSTANT(sattb) || ADDRESS(sattb)))
			     	{
				stmt = NULL ;		/* ES <- imm */
				}
			}
		else if (sreg==NULL)
			{
			if (MEMREF(sattb & stmt->dst.attb))
				{
				stmt = NULL ;		/* mem <- mem */
				}
			}
		}

	return stmt ;
	}


BOOLEAN Invalidates(stmt, op)
PARTS *stmt ;
OPERAND *op ;
	{
	OPCODE *opc ;
	FLAGS opnd_att, opc_attb ;
	REG *r ;
	BOOLEAN result ;


	r = op->reg ;
	opnd_att = op->attb ;
	opc = stmt->opcode ;

	result = TRUE ;
	if (r==ES && opc==LES) goto out ;

	if (r==DI || (r==NULL && (opnd_att & _DEPDI)!=0))
		{
		if (opc==REP ||
		    opc==MOVSW || opc==MOVSB ||
		    opc==STOSW || opc==STOSB) goto out ;
		}

	if (r==SI || (r==NULL && (opnd_att & _DEPSI)!=0))
		{
		if (opc==REP ||
		    opc==MOVSW || opc==MOVSB) goto out ;
		}

	opc_attb = opc->attb ;

	if ((opc_attb & MDFY_AX)!=0 && r==AX) goto out ;
	if ((opc_attb & MDFY_DX)!=0 && r==DX) goto out ;

	if ((opc_attb & MDFYDST) != 0)
		{
		if (Equal(stmt->dst.orig, op->orig)) goto out ;
		else if (stmt->dst.reg != NULL)
			{
			if ((DEPEND_FLAG(stmt->dst.reg) & opnd_att)!=0)
				{
				goto out ;
				}
			}
		}
	    
	else if ((opc_attb & MDFYSRC) != 0)
		{
		if (Equal(stmt->src.orig, op->orig)) goto out ;
		else if (stmt->src.reg != NULL)
			{
			if ((DEPEND_FLAG(stmt->src.reg) & opnd_att)!=0)
				{
				goto out ;
				}
			}
		}

	result = FALSE ;
out:
	return result ;
	}


PRIVATE PARTS *Push_Pop(stmt, sop)
PARTS *stmt ;
OPERAND *sop ;
	{
	OPCODE *opc ;
	unsigned pops ;
	PARTS *next ;

	if (!option.move.enabled && !option.expand.enabled) return NULL ;

	/* Check 1st for downstream expanded CALL w/4 or less PUSH's */
	/* --------------------------------------------------------- */
	pops = 1 ;
	next = stmt ;
	while ((next = Get_Level2(next)) != NULL)
		{
		if ((opc = next->opcode) == LINE) return NULL ;
		if (next->label[0] == ';')
			{
			if (opc == CALL)
				{
				if ((next->attb & POPXST) == 0) return NULL ;
				else break ; /* All OK! */
				}
			continue ;
			}
		if (*next->label!='\0' || islower(*next->mnemonic) ||
		    opc==CALL || opc==JMP || opc==RET ||
		    (opc->attb & CONDJMP)!=0 || next->dst.reg==SP)
		    	{
			return NULL ;
			}
		if (opc==PUSH)
			{
			if (++pops > 5) return NULL ;
			}
		}

	/* Ok, Now try to replace PUSH-POP's by MOV's ... */
	/* ---------------------------------------------- */
	pops = 1 ;
	while ((stmt = Get_Level2(stmt)) != NULL)
		{
		if (pops == 6) break ;	/* Limit recursion to what's needed */
		if ((opc = stmt->opcode) == LINE) break ;
		if (stmt->label[0] == ';') continue ;
		if (*stmt->label!='\0') break ;
		if (islower(*stmt->mnemonic)) break ;
		if (opc==CALL || opc==JMP || opc==RET) break ;
		if (opc==POP)
			{
			if (--pops==0) return stmt ;
			}
		else if (opc==MOV)
			{
			if (stmt->dst.reg!=NULL)
				{
				if (Push_Reg(stmt)) continue ;
				}
			}
		else if (opc==LEA || opc==LES)
			{
			if (Push_Reg(stmt)) continue ;
			}
		else if (opc==PUSH)
			{
			if (!Push_Mem(stmt)) pops++ ;
			continue ;
			}

		if ((opc->attb & CONDJMP)!=0 || stmt->dst.reg==SP) break ;
		if (Invalidates(stmt, sop)) break ;
		}

	return NULL ;
	}


PRIVATE VOID Absorb_Constant(stmt1)
PARTS *stmt1 ;
	{
	PARTS *stmt2, *stmt3 ;
	int imm ;

	imm = Value(stmt1->src.orig, stmt1->src.len) ;

	stmt2 = Find_Level0(stmt1, Get_Level0) ;
	stmt3 = Find_Level0(stmt2, Get_Level0) ;

	ndxreg = stmt1->dst.reg ;
	if (stmt1->opcode==MOV)
		{
		if (stmt3!=NULL && stmt2->opcode==AND && stmt3->opcode==ADD &&
		    stmt2->dst.reg==ndxreg && stmt3->dst.reg==ndxreg &&
		    Equal(stmt2->src.orig, "255"))
		    	{
			/*
			---------------------------
			MOV rI,imm      ###
			AND rI,255  =>  ###
			ADD rI,src2     MOV rI,src2
			---------------------------
			*/
			if (Adjust_Offsets(stmt3, imm & 255, stmt1->src.attb))
				{
				Emit(stmt3, null, mov, stmt1->dst.orig,
					stmt3->src.orig);
				stmt3->label = replaced ;
				stmt2->label = deleted ;
				stmt1->label = absorbed ;
				stats.cseg_del += 2 ;
				}
			}

		else if (stmt2!=NULL && stmt2->opcode==ADD)
			{
			if (stmt2->dst.reg==ndxreg)
		    		{
				/*
				----------------------------
				MOV rI,imm       ###
				ADD rI,src2  =>  MOV rI,src2
				----------------------------
				*/
				if (Adjust_Offsets(stmt2, imm,
					stmt1->src.attb))
					{
					Emit(stmt2, null, mov,
						stmt1->dst.orig,
						stmt2->src.orig) ;
					stmt2->label = absorbed ;
					stmt1->label = absorbed ;
					stats.cseg_del++ ;
					}
				}
			}
		}

	else if (stmt1->opcode==ADD || stmt1->opcode==SUB)
		{
		if (stmt1->opcode==SUB) imm = -imm ;
		if (Adjust_Offsets(stmt1, imm, stmt1->src.attb))
			{
			/*
			-----------------------
			ADD/SUB rI,imm  =>  ###
			-----------------------
			*/
			stmt1->label = absorbed ;
			stats.cseg_del++ ;
			}
		}
	}


PRIVATE BOOLEAN Adjust_Offsets(stmt, imm, attb)
PARTS *stmt ;
int imm ;
FLAGS attb ;
	{
	OPCODE *opc ;
	FLAGS depends ;
	BOOLEAN fixed = FALSE ;


	depends = DEPEND_FLAG(ndxreg) ;
	while ((stmt = Get_Level0(stmt)) != NULL)
		{
		if (stmt->opcode == LINE) break ;
		if (stmt->label[0] != '\0') continue ;
		if (islower(stmt->mnemonic[0])) break ;
		if ((stmt->src.attb & depends) != 0)
			{
			Emit(stmt, null, stmt->mnemonic, stmt->dst.orig,
				Insert_String(stmt->src.orig, stmt->src.len,
				imm, NULL)) ;
			stmt->label = adjusted ;
			stmt = Get_Level0(stmt) ;
			fixed = TRUE ;
			}

		else if ((stmt->dst.attb & depends) != 0)
			{
			Emit(stmt, null, stmt->mnemonic,
				Insert_String(stmt->dst.orig, stmt->dst.len,
					imm, NULL), stmt->src.orig) ;
			stmt->label = adjusted ;
			stmt = Get_Level0(stmt) ;
			fixed = TRUE ;
			}

		if (stmt->src.reg == ndxreg) break ;

		opc = stmt->opcode ;
		if (stmt->dst.reg == ndxreg)
			{
			if (opc==ADD || opc==SUB || opc==INC || opc==DEC)
				{
				continue ;
				}
			else if (opc==SHL && !ADDRESS(attb) &&
			         CONSTANT(stmt->src.attb))
				{
				imm <<= 1 ;
				}
			else break ;
			}

		if (REDEFINED(ndxreg, stmt)) break ;
		if (opc == CALL) break ;

		}

	return fixed ;
	}


			 /*  End of file UTILS.C  */
