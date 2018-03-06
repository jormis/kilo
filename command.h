#ifndef COMMAND_H
#define COMMAND_H

#include <stddef.h>
#include "const.h"
#include "key.h"
#include "undo.h"
#include "clipboard.h"
#include "buffer.h"

/**
* Everything is a command. 
*/
enum command_key {
	COMMAND_NO_CMD = 0,
	COMMAND_SET_MODE = 1,
	COMMAND_SET_TAB_STOP,
	COMMAND_SET_SOFT_TABS,
	COMMAND_SET_HARD_TABS,
	COMMAND_SET_AUTO_INDENT,
	COMMAND_SAVE_BUFFER, 
	COMMAND_SAVE_BUFFER_AS,  
	COMMAND_OPEN_FILE, // TODO
	COMMAND_MOVE_CURSOR_UP,
	COMMAND_MOVE_CURSOR_DOWN, // cmd_key = 10
	COMMAND_MOVE_CURSOR_LEFT,
	COMMAND_MOVE_CURSOR_RIGHT,
	COMMAND_MOVE_TO_START_OF_LINE,
	COMMAND_MOVE_TO_END_OF_LINE,
	COMMAND_KILL_LINE,
	COMMAND_YANK_CLIPBOARD,
	COMMAND_UNDO,
	COMMAND_INSERT_CHAR, /* for undo */
	COMMAND_DELETE_CHAR, /* for undo */
	COMMAND_DELETE_INDENT_AND_NEWLINE, /* for undo only */
        COMMAND_GOTO_LINE,
        COMMAND_GOTO_BEGINNING_OF_FILE,
        COMMAND_GOTO_END_OF_FILE,
        COMMAND_REFRESH_SCREEN, /* Ctrl-L */
        COMMAND_CREATE_BUFFER,
        COMMAND_SWITCH_BUFFER,
        COMMAND_DELETE_BUFFER,
        COMMAND_NEXT_BUFFER, /* Esc-N */
        COMMAND_PREVIOUS_BUFFER, /* Esc-P */
        COMMAND_MARK,
        COMMAND_COPY_REGION,
        COMMAND_KILL_REGION
};

enum command_arg_type {
	COMMAND_ARG_TYPE_NONE = 0,
	COMMAND_ARG_TYPE_INT,
	COMMAND_ARG_TYPE_STRING
};

struct command_str {
	int command_key;
	char *command_str;
	int command_arg_type;
	char *prompt; 
	char *success;
	char *error_status;
};

/** prototypes */
int command_entries(); 

void editor_del_char(int undo);
void editor_process_keypress();
void command_open_file(char *filename);
void editor_save(int command_key);
void command_debug(int command_key);
struct command_str *command_get_by_key(int command_key);
void command_insert_char(int character);
void command_delete_char();
void command_insert_newline();
int editor_get_command_argument(struct command_str *c, int *ret_int, char **ret_string);
void command_move_cursor(int command_key);
void command_goto_line();
void command_goto_beginning_of_file();
void command_goto_end_of_file();
void command_refresh_screen();
void exec_command(); 

#endif
