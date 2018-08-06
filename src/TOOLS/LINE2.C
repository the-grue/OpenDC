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

char *cur2_line;
int  line2_number;

static char input2_buf[BUF_SIZE+REC_SIZE];

static int in2_file  = -1;
static int bytes2_read = 0;
static char *next2_start;		/* pointer to the start of the next line */


static char *next2_line(ptr)
	char *ptr; {
	char *stop_ptr;

	if (ptr >= &input2_buf[bytes2_read]) return (0);

	while (*ptr != CR && *ptr != LF) {
		ptr++;
		if (ptr >= &input2_buf[bytes2_read]) return (0);
		}
	stop_ptr = ptr;
	while (*ptr != LF) {
		ptr++;
		if (ptr >= &input2_buf[bytes2_read]) return (0);
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

int line2_start(fname, position, first_line)
	char *fname; long position; int first_line; {

	/* make sure the file exists */
	in2_file = open(fname, 3);
	if (in2_file < 0) return (FALSE);

	lseek(in2_file, position, 0);
	line2_number = first_line;
	cur2_line = input2_buf;
	*cur2_line = 0;
	bytes2_read = read(in2_file, input2_buf, BUF_SIZE);
	next2_start = next2_line(cur2_line);

	if (next2_start == 0) {
		input2_buf[bytes2_read++] = CR;
		input2_buf[bytes2_read++] = LF;
		next2_start = next2_line(cur2_line);
		}
	return (TRUE);
	}



/* line_next(): insures that cur2_line points to the next input line.  
	buf_get handles the switching between stacked files.  FALSE is
	returned if no more input lines are available.  Returned lines are
	always terminated by 0, not CR or LF.
*/

int line2_next() {

	if (next2_start == 0) return (0);
	cur2_line = next2_start;
	next2_start = next2_line(next2_start);
	if (next2_start == 0) {
		if (cur2_line < &input2_buf[bytes2_read]) {
			_move(&input2_buf[bytes2_read] - cur2_line, cur2_line, input2_buf);
			bytes2_read = &input2_buf[bytes2_read] - cur2_line;
			}
		else
			bytes2_read = 0;
	
		bytes2_read += read(in2_file, &input2_buf[bytes2_read], BUF_SIZE);
		cur2_line = input2_buf;
		if (bytes2_read == 0) return (FALSE);

		next2_start = next2_line(cur2_line);
		if (next2_start == 0) {
			input2_buf[bytes2_read++] = CR;
			input2_buf[bytes2_read++] = LF;
			next2_start = next2_line(cur2_line);
			}
		}
	
	line2_number++;
	return (TRUE);
	}


/* line_stop(): close the input file.
*/
int line2_stop() {
	close(in2_file);
	}
