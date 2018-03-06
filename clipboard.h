#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "data.h"
#include "row.h"
#include "output.h"

/* row (E.row in phase 1; from E.cx' to E.cx'' in phase 2)
   is_eol = the whole line (phase 1; also phase 2 with double Ctrl-K or C-' ' & C/M-w)
*/

typedef struct clipboard_row {
	char *row; 
	int size; 
	int orig_x; /* TODO for emacs */
	int orig_y; /* TODO for undo */
	int is_eol; 
} clipboard_row; 

struct clipboard {
	/* 
	Fill clipboard with successive KILL_KEY presses. After the first non-KILL_KEY set C.is_full = 1
	as not to add anything more. The clipboard contents stay & can be yanked as many times as needed, 
        UNTIL the next KILL_KEY which clears clipboard and resets C.is_full to 0 if it was 1. 
	*/
	int is_full; 
	int numrows;
	clipboard_row *row; 
}; 

extern struct clipboard C; /* TODO static or extern in other source files? */

void clipboard_clear();
void clipboard_add_line_to_clipboard();
void clipboard_add_region_to_clipboard(int command); // FIXME get rid of "int command"
void clipboard_yank_lines(char *success); 
struct clipboard *clone_clipboard();
void undo_clipboard_kill_lines(struct clipboard *copy);

// FIXME create proper command_*() functions to commands.c & rename these to clipboard_* (command_*s will call these).
void command_mark(char *success /*struct command_str *c*/); // FIX most definitely get rid of struct command_str *
void command_copy_from_mark(); 
void command_kill_from_mark();

#endif

