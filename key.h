#ifndef KEY_H
#define KEY_H
/**
        key.h
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "terminal.h"
#include "row.h"

/**
The CTRL_KEY macro bitwise-ANDs a character with the value 00011111, in binary. 
In other words, it sets the upper 3 bits of the character to 0. 
This mirrors what the Ctrl key does in the terminal: it strips the 6th and 7th 
bits from whatever key you press in combination with Ctrl, and sends that. 
The ASCII character set seems to be designed this way on purpose. 
(It is also similarly designed so that you can set and clear the 6th bit 
to switch between lowercase and uppercase.)
*/
#define CTRL_KEY(k) ((k) & 0x1f)

enum editor_key {
	BACKSPACE = 127, 
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DEL_KEY,
	HOME_KEY,              /* Ctrl-A */
	END_KEY,               /* Ctrl-E */
	PAGE_UP,               /* Esc-V */
	PAGE_DOWN,             /* Ctrl-V */
	REFRESH_KEY,           /* Ctrl-L TODO */
	QUIT_KEY,              // 1010, Ctrl-Q
	SAVE_KEY,              /* Ctrl-S */
	FIND_KEY,              /* Ctrl-F */
	CLEAR_MODIFICATION_FLAG_KEY, /* M-c */

	/* These are more like commands.*/
        MARK_KEY,               /* Ctrl-Space */
        KILL_LINE_KEY,          /* Ctrl-K (like nano) */
	COPY_REGION_KEY,        /* Esc-W */
	KILL_REGION_KEY,        /* Ctrl-W */
	YANK_KEY,               /* Ctrl-Y */
	COMMAND_KEY,            /* Esc-x */
	COMMAND_UNDO_KEY,       /* Ctrl-u */
	COMMAND_INSERT_NEWLINE, /* 1022 Best undo for deleting newline (backspace in the beginning of row ...*/
        ABORT_KEY,              /* TODO NOT IMPLEMENTED */
        GOTO_LINE_KEY,          /* Ctrl-G */
        NEXT_BUFFER_KEY,        /* Esc-N */
        PREVIOUS_BUFFER_KEY,    /* Esc-P */
        NEW_BUFFER_KEY,         /* Ctrl-N */
        OPEN_FILE_KEY,          /* Ctrl-O */
        GOTO_BEGINNING_OF_FILE_KEY, /* Esc-A */
        GOTO_END_OF_FILE_KEY,   /* Esc-E */
};

int editor_read_key();
int editor_normalize_key(int c);
void editor_move_cursor(int key); 

#endif
