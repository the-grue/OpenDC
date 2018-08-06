/*
 *  Released under the GNU GPL.  See http://www.gnu.org/licenses/gpl.txt
 *
 *  This program is part of the SEE editor
 *
 *  SEE is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundatation; either version 2 of the License, or any
 *  later version.
 *
 *  SEE is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 */
/*
	view.c

	(c) Copyright 1983-1989 Michael Ouye, All Rights Reserved

	This module maintains the multiple views used in the SEE editor.
*/

#include "world.h"
#include "extern.h"

void view_init(void) {
	int i;
	int top, left, rows, cols;

	for (i = 0; i < max_views; i++) {
		views[i].scr_id = -1;
		views[i].buffer_id = -1;
	}

	scr_info(0, &top, &left, &rows, &cols);
	views[0].scr_id = scr_open(-1, top, left, rows, cols);
	views[0].buffer_id = active_buffer;
	scr_pick(views[0].scr_id);
	active_view = 0;

	views[FILE_VIEW].scr_id = -1;
	views[FILE_VIEW].buffer_id = FILE_BUFFER;
}

void view_select(which)
	int which; {

	if (which == active_view) return 0;
	if (which >= max_windows) return -1;
	if (views[which].scr_id == -1) return -1;

	active_view = which;
	scr_pick(views[which].scr_id);
}

void view_refresh(void) {
	int i;
	int active;
	int top,left,rows,cols;

	active = active_view;
	active_view = -1;
	for (i = 0; i < max_windows; i++) {
		if (views[i].scr_id != -1) {
			view_select(i);
			switch_buffer(views[i].buffer_id);
			scr_info(views[i].scr_id, &top,&left,&rows,&cols);
			scr_open(views[i].scr_id,top,left,rows,cols); /* set the color */
			scr_clr();
			review(3);
			message(in_file);
		}
	}
	view_select(active);
}

int view_next(void) {
	int i;

	for (i = (active_view+1) % max_views; i != active_view; i = (i+1) % max_views) {
		if (views[i].scr_id != -1) return i;
	}
	return active_view;
}

int view_count(void) {
	int i, count;

	for (i = 0, count = 0; i < max_views; i++) {
		if (views[i].scr_id != -1) count++;
	}
	return count;
}

/* split the current view in half & order the view list */
int view_new(void) {
	int top, left, rows, cols;
	char leftover;

	if (views[max_views-1].scr_id != -1) return -1;

	scr_info(views[active_view].scr_id, &top, &left, &rows, &cols);
	leftover = (rows / 2)+1;
	if (leftover < 6) return -1;

	/* open a hole at active view + 1 */
	_move(((max_views-active_view)-2) * sizeof(view_struct),
				&views[active_view+1], &views[active_view+2]);

	scr_open(views[active_view].scr_id, top, left, leftover, cols);
	views[active_view+1].scr_id = scr_open(-1, top+leftover-1, left, rows-leftover+1, cols);
	views[active_view+1].buffer_id = active_buffer;
	return active_view+1;
}

void view_set_height(which, new_height)
	int which, new_height; {
	int top, left, rows, cols;
	int delta;
	int i;

	scr_info(views[which].scr_id, &top, &left, &rows, &cols);
	delta = new_height - rows;
	scr_open(views[which].scr_id, top, left, new_height, cols);

	/* adjust the following views to account for the new height */
	for (i = which+1; i < max_windows; i++) {
		if (views[i].scr_id != -1) {
			scr_info(views[i].scr_id, &top, &left, &rows, &cols);
			top += delta;
			scr_open(views[i].scr_id, top, left, rows, cols);
		}
	}

	scr_clr();
	view_refresh();
}

