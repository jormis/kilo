#include "clipboard.h"

#include <stdlib.h>
#include <string.h>

struct clipboard C; /* Defined here, declared in clipboard.h. */

extern struct editor_config *E;

void
clipboard_clear() {
        int i; 
	if (C.row != NULL) { /* C.row is an array of clipboard_rows containing char *row. */
                for (i = 0; i < C.numrows; i++) {
                        if (C.row[i].row != NULL) {
                                free(C.row[i].row);
                        }
                }         
		free(C.row); 
	}

        C.row = NULL; 
	C.numrows = 0;
	C.is_full = 0; 
}

void
clipboard_add_line_to_clipboard() {
	if (E->cy < 0 || E->cy >= E->numrows)
		return; 

	if (C.is_full) {
		clipboard_clear();
	}

	erow *row = &E->row[E->cy];	

	// Append to the end.
	C.row = realloc(C.row, sizeof(clipboard_row) * (C.numrows + 1));
	C.row[C.numrows].row = malloc(row->size);
	memcpy(C.row[C.numrows].row, row->chars, row->size);
	C.row[C.numrows].size = row->size;
	C.row[C.numrows].orig_x = E->cx;
	C.row[C.numrows].orig_y = E->cy;
	C.row[C.numrows].is_eol = 1; 
	C.numrows++;		

	editor_del_row(E->cy);
}

// XXX
void
clipboard_add_region_to_clipboard(int command) { // FIXME command out of here.
        /* Depending on whether the mark if before or after C-W or M-W either
        the first or last row may not be whole. So, a partial row is added / extracted. */

/*
        if (command == COMMAND_COPY_REGION) {
                // Don't delete copied region.
        } else {
                // Delete/kill.
        }
        
*/
}

/* An easy command to implement. */
void 
command_mark(char *success /*struct command_str *c*/) {
        E->mark_x = E->cx;
        E->mark_y = E->cy;
        editor_set_status_message(/*c->*/success); 
}

/* mode: Esc-W, Ctrl-W. First one only copies, second one also deletes the marked region. 
   mode == c->command. */
void
command_copy_from_mark() { // char *success, char *error/*struct command_str *c*/) {
        /* No explicit mark, no kill from mark. */
        if ((E->mark_x == -1 && E->mark_y == -1)
                || (E->mark_x == E->cx && E->mark_y == E->cy)) {
                //editor_set_status_message(c->error_status);
                return;
        }

        // TODO
        // from_x_&_y to to_x_&_y
}
void
command_kill_from_mark() {
        //command_copy_from_mark(command_get_by_key(COMMAND_KILL_REGION));
}

void
clipboard_yank_lines(char *success /*struct command_str *c*/) { 
	int j = 0; 
	for (j = 0; j < C.numrows; j++) {
		editor_insert_row(E->cy++, C.row[j].row, C.row[j].size);		
	}

	editor_set_status_message(/*c->*/success);
}

void
undo_clipboard_kill_lines(struct clipboard *copy) { 
	int j = 0; 

	if (copy == NULL) {
		editor_set_status_message("Unable to undo kill lines.");
		return;
	}
	
	for (j = 0; j < copy->numrows; j++) {
		editor_insert_row(E->cy++, copy->row[j].row, copy->row[j].size);		
	}

	E->cx = copy->row[copy->numrows-1].orig_x;
	E->cy = copy->row[copy->numrows-1].orig_y; 

	editor_set_status_message("Undo kill lines!");
}

struct clipboard *
clone_clipboard() {
	int i = 0;
	struct clipboard *copy = malloc(sizeof(struct clipboard));
	if (copy == NULL) {
		editor_set_status_message("Failed to create clipboard to the undo stack.");
		return NULL; 
	}

	copy->is_full = 1; 
	copy->numrows = C.numrows;
	copy->row = malloc(C.numrows * sizeof(struct clipboard_row));
	if (copy->row == NULL) {
		editor_set_status_message("Failed to create clipboard rows to the undo stack.");
		return NULL; 
	}

	for (i = 0; i < C.numrows; i++) {
		clipboard_row *from = &C.row[i]; 
		clipboard_row *to = &copy->row[i];
		to->row = malloc(from->size);
		memcpy(to->row, from->row, from->size);
		to->size = from->size; 
		to->orig_x = from->orig_x;
		to->orig_y = from->orig_y;
		to->is_eol = from->is_eol;
	}

	return copy; 
}
