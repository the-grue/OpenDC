/*
	Buffered input module:

	This file contains the routines for handling buffered input through
	1024 byte chunks.  The input granularity is a line, maximum 255
	characters long.  The exported variable cur_line always points to the
	beginning of a complete line after the line_next function is called.



	Interface routines:

		line_start(fname, postition, first_line): input file from position
		line_next(): advances cur_line to the next line.
		line_stop(): close the input file
*/

#define	TRUE		1
#define FALSE		0

#define BUF_SIZE	1024			/* buffer size */
#define REC_SIZE	255				/* maximum line length */

#define	LF			0x0A
#define CR			0x0D

char *cur_line;
int  line_number;
long line_location;					/* file offset to cur_line */

char input_buf[BUF_SIZE+REC_SIZE];

int in_file  = -1;
int bytes_read = 0;
char *next_start;		/* pointer to the start of the next line */
int cur_len;			/* length of the current line */

char *next_line(ptr)
	char *ptr; {
	char *stop_ptr;

	cur_len = 0;
	if (ptr >= &input_buf[bytes_read]) return (0);

	while (*ptr != CR && *ptr != LF) {
		ptr++;
		cur_len++;
		if (ptr >= &input_buf[bytes_read]) return (0);
		}
	stop_ptr = ptr;
	while (*ptr != LF) {
		ptr++;
		cur_len++;
		if (ptr >= &input_buf[bytes_read]) return (0);
		}
	*stop_ptr = 0;	 /* store the terminator */
	return (ptr+1);
	}

/*  Interface routines */

/* line_start(fname, position, first_line): opens the file for input
   and sets up the first buffer of text.   cur_line is valid unless
   false is returned, implying no such file.  position is the location
   in the file to start the input and first_line is the line number to
   be used for the first line read.
*/

int line_start(fname, position,	first_line)
	char *fname; long position; int first_line; {

	/* make sure the file exists */
	in_file = open(fname, 3);
	if (in_file < 0) return (FALSE);

	lseek(in_file, position, 0);
	line_location = position;
	line_number = first_line;
	cur_line = input_buf;
	*cur_line = 0;
	bytes_read = read(in_file, input_buf, BUF_SIZE);
	next_start = next_line(cur_line);

	if (next_start == 0) {
		input_buf[bytes_read++] = CR;
		input_buf[bytes_read++] = LF;
		next_start = next_line(cur_line);
		}
	return (TRUE);
	}



/* line_next(): insures that cur_line points to the next input line.  
	buf_get handles the switching between stacked files.  FALSE is
	returned if no more input lines are available.  Returned lines are
	always terminated by 0, not CR or LF.
*/

int line_next() {

	if(next_start == 0) return (FALSE);
	line_location += cur_len;
	cur_line = next_start;
	next_start = next_line(next_start);
	if (next_start == 0) {
		if (cur_line < &input_buf[bytes_read]) {
			_move(&input_buf[bytes_read] - cur_line, cur_line, input_buf);
			bytes_read = &input_buf[bytes_read] - cur_line;
			}
		else
			bytes_read = 0;
	
		bytes_read += read(in_file, &input_buf[bytes_read], BUF_SIZE);
		cur_line = input_buf;
		if (bytes_read == 0) return (FALSE);

		next_start = next_line(cur_line);
		if (next_start == 0) {
			input_buf[bytes_read++] = CR;
			input_buf[bytes_read++] = LF;
			next_start = next_line(cur_line);
			}
		}
	
	line_number++;
	return (TRUE);
	}


/* line_stop(): close the input file.
*/
int line_stop() {
	close(in_file);
	}
