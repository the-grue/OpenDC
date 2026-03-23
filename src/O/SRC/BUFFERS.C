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
		       /*  Start of file BUFFERS.C  */

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

#include	"inc\o88.h"

PRIVATE CHAR open_input_err[] = "Error opening input file: " ;
PRIVATE CHAR open_output_err[] = "Error creating output file: " ;
PRIVATE CHAR close_input_err[] = "Error closing input file: " ;
PRIVATE CHAR close_output_err[] = "Error closing output file: " ;
PRIVATE CHAR write_err[] = "Error writing output file: " ;

VOID Init_Buffer(bfr, name, kbytes)
BUFFER *bfr ;
CHAR *name ;
unsigned kbytes ;
	{
	unsigned paragraphs ;

	if (kbytes > 63) kbytes = 63 ;

	bfr->size = kbytes << 10 ;
	bfr->seg = dos_base ;
	bfr->off = 0 ;

	paragraphs = kbytes << 6 ;
	dos_base += paragraphs ;
	dos_size -= paragraphs ;

	strcpy(bfr->filename, name) ;
	bfr->mode = 0 ;
	bfr->count = 0 ;
	}


VOID Reuse_Buffer(bfr, name)
BUFFER *bfr ;
CHAR *name ;
	{
	if ((bfr->mode & OPENED) != 0)
		{
		if ((bfr->mode & OUTPUT) != 0)
			{
			Flush_Buffer(bfr) ;
			Close_Output(bfr) ;
			}
		else Close_Input(bfr) ;
		}
	strcpy(bfr->filename, name) ;
	bfr->mode = 0 ;
	bfr->count = 0 ;
	bfr->off = 0 ;
	}


VOID Rewind_Buffer(bfr)
BUFFER *bfr ;
	{
	if ((bfr->mode & RESIDENT) != 0)
		{
		bfr->count = bfr->off ;
		bfr->off = 0 ;
		}
	else Close_Input(bfr) ;
	}


VOID Reverse_Buffer(bfr)
BUFFER *bfr ;
	{
	if ((bfr->mode & OPENED) != 0)
		{
		Flush_Buffer(bfr) ;
		Close_Output(bfr) ;
		}
	else bfr->mode |= RESIDENT ;
	bfr->off = 0 ;
	}


VOID Open_Input(bfr)
BUFFER *bfr ;
	{
#	define	DOS_READ	0x3D00

	if ((bfr->mode & OPENED) != 0) return ;
	bfr->handle = Open_File(bfr->filename, DOS_READ) ;
	if (bfr->handle == 0)
		{
		Error_Msg(open_input_err, bfr->filename) ;
		}
	bfr->mode |= (OPENED | INPUT) ;
	}


VOID Open_Output(bfr)
BUFFER *bfr ;
	{
#	define	DOS_CREAT	0x3C00

	if ((bfr->mode & OPENED) != 0) return ;
	bfr->handle = Open_File(bfr->filename, DOS_CREAT) ;
	if (bfr->handle == 0)
		{
		Error_Msg(open_output_err, bfr->filename) ;
		}
	bfr->mode |= (OPENED | OUTPUT) ;
	}


VOID Close_Input(bfr)
BUFFER *bfr ;
	{
	if ((bfr->mode & OPENED) == 0) return ;
	if (!Close_File(bfr->handle))
		{
		Error_Msg(close_input_err, bfr->filename) ;
		}
	bfr->mode = 0 ;
	}


VOID Close_Output(bfr)
BUFFER *bfr ;
	{
	if ((bfr->mode & OPENED) == 0) return ;
	if (!Close_File(bfr->handle))
		{
		Error_Msg(close_output_err, bfr->filename) ;
		}
	bfr->mode = 0 ;
	}


VOID Flush_Buffer(bfr)
BUFFER *bfr ;
	{
	unsigned written ;

	Open_Output(bfr) ;
	if (bfr->count != 0)
		{
		written = Write_File(bfr->handle, bfr->seg, bfr->count) ;
		if (written != bfr->count)
			{
			Error_Msg(write_err, bfr->filename) ;
			}
		}
	bfr->mode &= ~RESIDENT ;
	bfr->count = 0 ;
	bfr->off = 0 ;
	}


unsigned Get_Line(line, bfr)
CHAR line[] ;
BUFFER *bfr ;
	{
	unsigned cnt, off, seg ;
	CHAR ch, *ptr ;
	BOOLEAN txt ;

	ptr = line ;
	seg = bfr->seg ; off = bfr->off ;
	cnt = bfr->count ;
	txt = FALSE ;
	for (;;)
		{
		if (cnt == 0)
			{
			if ((bfr->mode & RESIDENT) != 0) return 0 ;
			Open_Input(bfr) ;
			cnt = Read_File(bfr->handle, seg, bfr->size) ;
			if (cnt == 0) return 0 ;
			off = 0 ;
			}
		if ((ch = _peek(off++, seg)) == CTLZ) 
			{
			cnt = 0 ;
			continue ;
			}
		cnt-- ;
		if (ch == CR) continue ;
		if (ch != LF)
			{
			if (ch == '\t') ch = ' ' ;
			else if (ch != ' ') txt = TRUE ;
			*ptr++ = ch ;
			}
		else
			{
			if (txt) break ;
			ptr = line ;
			}
		}

	*ptr = '\0' ;
	bfr->count = cnt ;
	bfr->off = off ;
	return (unsigned) (ptr - line) ;
	}


VOID Put_Line(str, bfr)
CHAR *str ;
BUFFER *bfr ;
	{
	Put_Str(str, bfr) ;
	Put_Str("\r\n", bfr) ;
	}


VOID Put_Str(str, bfr)
CHAR *str ;
BUFFER *bfr ;
	{
	unsigned wrote ;
	unsigned off, seg, cnt ;
	CHAR ch ;

	seg = bfr->seg ; off = bfr->off ;
	for (cnt = bfr->count; (ch = *str++) != '\0'; cnt++)
		{
		if (cnt == bfr->size)
			{
			Open_Output(bfr) ;
			wrote = Write_File(bfr->handle, seg, bfr->size) ;
			if (wrote != bfr->size)
				{
				Error_Msg(write_err, bfr->filename) ;
				}
			off = 0 ;
			cnt = 0 ;
			}
		_poke(ch, off++, seg) ;
		}
	bfr->count = cnt ;
	bfr->off = off ;
	}


VOID Copy_Buffer(in, out)
BUFFER *in ;
BUFFER *out ;
	{
	unsigned wrote ;
	unsigned space, move ;

	for (;;)
		{
		if ((in->mode & RESIDENT) == 0)
			{
			Open_Input(in) ;
			in->count = Read_File(in->handle, in->seg, in->size) ;
			}
		in->off = 0 ;
		if (in->count == 0) break ;
		while (in->count != 0)
			{
			if (out->count >= out->size)
				{
				Open_Output(out) ;
				wrote = Write_File(out->handle, out->seg,
						out->size) ;
				if (wrote != out->size)
					{
					Error_Msg(write_err, out->filename) ;
					}
				out->off = 0 ; out->count = 0 ;
				}
			space = out->size - out->count ;
			move = (in->count < space) ? in->count : space ;
			_lmove(move, in->off, in->seg, out->off, out->seg) ;
			in->off  += move ; in->count  -= move ;
			out->off += move ; out->count += move ;
			}
		}

	Close_Input(in) ;
	Delete_File(in->filename) ;
	}


			/*  End of file BUFFERS.C  */

