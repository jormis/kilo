
#include <stddef.h>
#include <fcntl.h>
#include "command.h"
#include "row.h"
#include "buffer.h"
#include "clipboard.h"
#include "file.h"
#include "find.h"

extern struct clipboard C;

/* M-x "command-str" <ENTER> followed by an optional argument (INT or STRING). */ 
struct command_str COMMANDS[] = {
        {
                COMMAND_CREATE_BUFFER,
                "create-buffer",
                COMMAND_ARG_TYPE_NONE,
                NULL,
                "Buffer created.",
                NULL
        },
        {
                COMMAND_SWITCH_BUFFER,
                "switch-buffer",
                COMMAND_ARG_TYPE_STRING,
                "Buffer: %s",
                "Buffer created.",
                "Failed to switch to buffer: '%s'"
        },
        {
                /* DELETE CURRENT BUFFER */
                COMMAND_DELETE_BUFFER,
                "delete-buffer",
                COMMAND_ARG_TYPE_NONE,
                NULL,
                "The current buffer deleted.",
                "Unsaved changes. Save buffer or clear modification bit."
        },
        {
                COMMAND_NEXT_BUFFER,
                "next-buffer",
                COMMAND_ARG_TYPE_NONE,
                NULL,
                "Switched to next buffer.",
                NULL
        },
        {
                COMMAND_PREVIOUS_BUFFER,
                "previous-buffer",
                COMMAND_ARG_TYPE_NONE,
                NULL,
                "Switched to previous buffer.",
                NULL
        },      
	{
		COMMAND_SET_MODE,
		"set-mode",
		COMMAND_ARG_TYPE_STRING,
		"Mode: %s",
		"Mode set to: '%s'",
		"Failed to set mode to '%s'"
	},
	{
		COMMAND_SET_TAB_STOP,
		"set-tab-stop",
		COMMAND_ARG_TYPE_INT,
		"Set tab stop to: %s", // always %s
		"Tab stop set to: %d", // based on ARG_TYPE: %d or %s
		"Invalid tab stop value: '%s' (range 2-)"
	},
	{
		COMMAND_SET_SOFT_TABS,
		"set-soft-tabs",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"Soft tabs in use.",
		NULL,
	},
	{
		COMMAND_SET_HARD_TABS,
		"set-hard-tabs",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"Hard tabs in use",
		NULL
	},
	{
		COMMAND_SET_AUTO_INDENT,
		"set-auto-indent",
		COMMAND_ARG_TYPE_STRING,
		"Set auto indent on or off: %s",
		"Auto indent is %s",
		"Invalid auto indent mode: '%s'"
	},
	{
		COMMAND_SAVE_BUFFER,
		"save-buffer",
		COMMAND_ARG_TYPE_STRING, /* Prompt for new file only. */
		"Save as: %s",
		"%d bytes written successfully to %s", // Special handling: %d and %s
		"Can't save, I/O error: %s" // %s = error
	},
	{
                // Open file for reading, save current buffer to it.
		COMMAND_SAVE_BUFFER_AS,
		"save-buffer-as",
		COMMAND_ARG_TYPE_STRING,
		"Save buffer as: %s",
		"%d bytes written successfully to %s", // Special handling: %d and %s
		"Can't save, I/O error: %s" // %s = error
	},
        {       
                COMMAND_OPEN_FILE,
                "open-file",
                COMMAND_ARG_TYPE_STRING,
                "Open file: %s",
                "%s opened.",
                "Cannot open file: %s"
        },       
	{
		COMMAND_MOVE_CURSOR_UP,
		"move-cursor-up",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_MOVE_CURSOR_DOWN,
		"move-cursor-down",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_MOVE_CURSOR_LEFT,
		"move-cursor-left",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_MOVE_CURSOR_RIGHT,
		"move-cursor-right",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
        {
                COMMAND_GOTO_BEGINNING_OF_FILE,
                "goto-beginning",
                COMMAND_ARG_TYPE_NONE,
                NULL,
                "",
                NULL
        },
        {
                COMMAND_GOTO_END_OF_FILE,
                "goto-end",
                COMMAND_ARG_TYPE_NONE,
                NULL,
                "",
                NULL
        },
        {
                COMMAND_REFRESH_SCREEN,
                "refresh",
                COMMAND_ARG_TYPE_NONE,
                NULL,
                "",
                NULL
        },
        
	{
		COMMAND_KILL_LINE,
		"kill-line",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_YANK_CLIPBOARD,
		"yank",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_UNDO,
		"undo",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_INSERT_CHAR,
		"insert-char",
		COMMAND_ARG_TYPE_STRING,
		"Character: %s",
		"Inserted '%c'",
		"Failed to insert a char:"
	},
	{
		COMMAND_DELETE_CHAR,
		"delete-char",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"Deleted",
		"No characters to delete!"
	},
        { 
                COMMAND_GOTO_LINE,
                "goto-line",
                COMMAND_ARG_TYPE_INT,
                "Goto line: %s",
                "Jumped",
                "Failed to goto line: "
        },
        {
                COMMAND_MARK,
                "mark",
                COMMAND_ARG_TYPE_NONE,
                "",
                "Mark set",
                "Failed to set a mark."
        },
        {
                COMMAND_COPY_REGION,
                "copy-region",
                COMMAND_ARG_TYPE_NONE,
                "",
                "Region copied.",
                "No region to copy."
        },
        {
                COMMAND_KILL_REGION,
                "kill-region",
                COMMAND_ARG_TYPE_NONE,
                "",
                "Region killed.",
                "No region to kill."
        }        
};


int
command_entries() {
        return sizeof(COMMANDS) / sizeof(COMMANDS[0]);
}

/**
 * The functionality ought to be in command_del_char() because undo*()s are also here.
 * TODO FIXME XXX
 * @param undo (1=called from undo(); 0=normal operation) 
 */

void
editor_del_char(int undo) {
	erow *row;

	if (E->cy == E->numrows) {
		if (E->cy > 0) {
			if (undo)
				key_move_cursor(ARROW_LEFT);
			else
				command_move_cursor(COMMAND_MOVE_CURSOR_LEFT);
		}
		return; 
	}

	if (E->cx == 0 && E->cy == 0) 
		return;

	row = &E->row[E->cy];
	if (E->cx > 0) {
		int orig_cx = E->cx; 
		int char_to_be_deleted = row->chars[E->cx];
		int len = editor_row_del_char(row, E->cx - 1);

		if (len > 0) {
			int current_cx = E->cx;
			E->cx = orig_cx;

                        // FIXME undo as a single undo command
                        // as currently we can only pop one from the stack.
                        // Should do it, though.

			while (current_cx < E->cx) {
				E->cx = current_cx; 
				if (!undo) {
					if (current_cx == orig_cx) { // ??
						undo_push_one_int_arg(COMMAND_DELETE_CHAR, COMMAND_INSERT_CHAR, 
							char_to_be_deleted);
					} else {
						undo_push_one_int_arg(COMMAND_DELETE_CHAR, COMMAND_INSERT_CHAR, 
							E->is_soft_indent ? (int) ' ' : (int) '\t');
					}
				} 
				current_cx++;
			}

		}

		E->cx = orig_cx - len;

                if (E->coloff > 0) {
                        E->coloff -= len;
                        if (E->coloff < 0)
                                E->coloff = 0;
                }

	} else { 
                if (!undo) {
			undo_push_one_int_arg(COMMAND_DELETE_CHAR, COMMAND_INSERT_CHAR, '\r');
                } else {
                        // FIXME undo insert_newline.
                }  
		E->cx = E->row[E->cy - 1].size; 
		editor_row_append_string(&E->row[E->cy - 1], row->chars, row->size); 
		editor_del_row(E->cy);
		E->cy--; 
	}
}

/**
        editor_process_keypress()
*/
void 
editor_process_keypress() {
	static int quit_times = KILO_QUIT_TIMES; 
	static int previous_key = -1; 
                
	int c = key_normalize(key_read());

        // Cut and paste fix?
        if (previous_key != '\r' && c == '\n')
                c = '\r';

	/* Clipboard is deemed full after the first non-KILL_LINE_KEY. */
	if (previous_key == KILL_LINE_KEY && c != KILL_LINE_KEY) {
		C.is_full = 1; 
		undo_push_clipboard();
	}

	previous_key = c; 

	E->is_banner_shown = 1; // After the first keypress, yes. 

	switch (c) {
	case '\r':
		command_insert_newline(); 
		break;
	case QUIT_KEY:
		if (E->dirty && quit_times > 0) {
			editor_set_status_message(UNSAVED_CHANGES_WARNING, quit_times);
			quit_times--;
			return; 
		}

		/* Clear the screen at the end. */
		write(STDOUT_FILENO, "\x1b[2J", 4);
      		write(STDOUT_FILENO, "\x1b[H", 3);
		exit(0);
		break;
	case SAVE_KEY:
		editor_save(COMMAND_SAVE_BUFFER);
		break; 
	case HOME_KEY:
		E->cx = 0;
		break;
	case END_KEY:
		if (E->cy < E->numrows)
			E->cx = E->row[E->cy].size; 
		break;
	case FIND_KEY:
		editor_find();
		break; 
	case BACKSPACE:
	case CTRL_KEY('h'):
	case DEL_KEY:
		if (c == DEL_KEY) 
			command_move_cursor(COMMAND_MOVE_CURSOR_RIGHT);
		command_delete_char();
		break; 
	case PAGE_DOWN:
	case PAGE_UP: { 
		int times = 0;
		if (c == PAGE_UP) {
			E->cy = E->rowoff;
			times = TERMINAL.screenrows; 
		} else if (c == PAGE_DOWN) {
			E->cy = E->rowoff + TERMINAL.screenrows - 1;

			if (E->cy <= E->numrows) {
				times = TERMINAL.screenrows;
			} else {
				E->cy = E->numrows; 
				times = E->numrows - E->rowoff; 
			}
		}
		while (times--)
			command_move_cursor(c == PAGE_UP 
			? COMMAND_MOVE_CURSOR_UP : COMMAND_MOVE_CURSOR_DOWN);
		break;
		}
	case ARROW_UP:
		command_move_cursor(COMMAND_MOVE_CURSOR_UP);
		break;
	case ARROW_LEFT:
		command_move_cursor(COMMAND_MOVE_CURSOR_LEFT);
		break;
	case ARROW_RIGHT:
		command_move_cursor(COMMAND_MOVE_CURSOR_RIGHT);
		break;
        case ARROW_DOWN:
		command_move_cursor(COMMAND_MOVE_CURSOR_DOWN);
		break;
        case GOTO_BEGINNING_OF_FILE_KEY:
                command_goto_beginning_of_file();
                break;
        case GOTO_END_OF_FILE_KEY:
                command_goto_end_of_file();
                break;
	case REFRESH_KEY: /* Ctrl-L */
    	case '\x1b':
                command_refresh_screen(); 
      		break;
      	case KILL_LINE_KEY:
      		clipboard_add_line_to_clipboard();
      		break;
      	case YANK_KEY:
        {
                struct command_str *c = command_get_by_key(COMMAND_YANK_CLIPBOARD);
      		clipboard_yank_lines(c->success);
      		break;
        }
      	case CLEAR_MODIFICATION_FLAG_KEY:
      		if (E->dirty) {
      			editor_set_status_message("Modification flag cleared.");
      			E->dirty = 0;
      		}
      		break;
      	case COMMAND_KEY:
      		exec_command();
      		break; 
      	case COMMAND_UNDO_KEY:
      		undo();
      		break;
        case GOTO_LINE_KEY:
                command_goto_line();
                break;
        case NEXT_BUFFER_KEY:
                command_next_buffer();
                break;
        case PREVIOUS_BUFFER_KEY:
                command_previous_buffer(); 
                break;
        case MARK_KEY:
        {
                struct command_str *c = command_get_by_key(COMMAND_MARK);
                command_mark(c->success);
                break;
        } 
        case COPY_REGION_KEY:
                command_copy_from_mark(command_get_by_key(COMMAND_COPY_REGION));
                break;
        case KILL_REGION_KEY:
                command_kill_from_mark(); 
                break; 
        case NEW_BUFFER_KEY:
        {
                struct command_str *c = command_get_by_key(COMMAND_CREATE_BUFFER);
                (void) create_buffer(BUFFER_TYPE_FILE, 0, c->success, COMMAND_NO_CMD);
                break;
        }
        case OPEN_FILE_KEY:
                command_open_file(NULL);
                break; 
	default:
		command_insert_char(c);
		break; 
	}

	quit_times = KILO_QUIT_TIMES; 
}

/**
 M-x open-file; also used when starting the editor to open files
 specified in the command line.
*/
void
command_open_file(char *filename) {
 	char *line = NULL;
  	size_t linecap = 0;
  	ssize_t linelen;
        struct stat stat_buffer; 
        FILE *fp = NULL;
        int free_filename = 0; 
        int int_arg;
        char *char_arg; 

        if (filename == NULL) {
                struct command_str *c = command_get_by_key(COMMAND_OPEN_FILE);
                int rc = editor_get_command_argument(c, &int_arg, &char_arg);
                if (rc == 1) {
                        filename = strdup(char_arg);
                        free(char_arg);
                        free_filename = 1; 
                } else if (rc == 0) {
                        editor_set_status_message(STATUS_MESSAGE_ABORTED);
                        return;
                } else {
                        editor_set_status_message(c->error_status);
                        return;
                }                
        }

        if (E->dirty > 0  
                || E->numrows > 0 || E->cx > 0 || E->cy > 0 
                ||  E->filename != NULL) {
                /* This buffer is in use. Create a new one & use it. */
                (void) create_buffer(BUFFER_TYPE_FILE, 0, "FIXME: new buffer", COMMAND_NO_CMD); 
        }

        E->filename = strdup(filename); 
        syntax_select_highlight(NULL);

	E->absolute_filename = realpath(filename, NULL); 
	E->basename = editor_basename(filename);

  	if (stat(E->filename, &stat_buffer) == -1) {
  		if (errno == ENOENT) {
  			E->is_new_file = 1; 
  			E->dirty = 0; 
                        if (free_filename)
                                free(filename);
  			return; 
  		} else {
  			die("stat");
  		}
  	}

 	fp = fopen(E->absolute_filename, "r");
  	if (!fp) {
  		die("fopen");
 	}

  	while ((linelen = getline(&line, &linecap, fp)) != -1) {
                if (linelen > 0 && (line[linelen - 1] == '\n' 
                        || line[linelen - 1] == '\r'))
      		        linelen--;

                editor_insert_row(E->numrows, line, linelen);
	}

	free(line);
	fclose(fp);
        if (free_filename)
                free(filename); 
                
	E->dirty = 0; 
}


/**
 * rc = 0 OK
 * rc = -1 error
 * rc = 1 aborted
 */
void
editor_save(int command_key) {
	int len; 
	char *buf; 
	int fd; 
	char *tmp; 

	struct command_str *c = command_get_by_key(command_key);
	if (c == NULL) {
		editor_set_status_message("Unknown command! Cannot save!?!");
		return;
	}

	if (command_key == COMMAND_SAVE_BUFFER_AS || E->filename == NULL) {
		tmp = editor_prompt(c->prompt, NULL); 
		if (tmp != NULL) {
			free(E->filename);
			free(E->absolute_filename);
			free(E->basename);

			E->filename = strdup(tmp); 
			E->absolute_filename = strdup(E->filename); // realpath(E->filename, NULL) returns NULL.; 
			E->basename = editor_basename(E->filename);
                        syntax_select_highlight(NULL);
                        free(tmp);

		} else {
			editor_set_status_message(STATUS_MESSAGE_ABORTED); // TODO ABORT message in conf.
			return; 
		}
	}

	buf = editor_rows_to_string(&len);


        if (buf == NULL || buf[0] == '\0') {
                if (! E->dirty) { 
                        editor_set_status_message("Empty buffer -- not saved.");
                        return;
                } else {
                        // Add a newline and re-buf.
                        editor_insert_newline();
                        buf = editor_rows_to_string(&len);                                 
                }
        }

	fd = open(E->filename, O_RDWR | O_CREAT, 0644);
	if (fd != -1) {
		if (ftruncate(fd, len) != -1) {
			if (write(fd, buf, len) == len) {
                
                                close(fd);
                                free(buf);
                                E->dirty = 0;
                                E->is_new_file = 0;  

                                // if (strlen(abs) + strlen(success) > TERMINAL.screencols (not 100% acc)
                                // then cut X 
                                if (E->absolute_filename
                                        && (strlen(E->absolute_filename) + strlen(c->success) > TERMINAL.screencols)) {
                                        char *status_filename = malloc(TERMINAL.screencols + 1);

                                        int truncate_len = strlen(E->absolute_filename) + strlen(c->success) 
                                                - TERMINAL.screencols + 3; // "..."
                                                
                                        memset(status_filename, '\0', TERMINAL.screencols + 1);
                                        strncpy(status_filename, "...", 3);
                                        strncpy(status_filename+3, E->absolute_filename+truncate_len, 
                                                strlen(E->absolute_filename)-truncate_len);
                                        status_filename[TERMINAL.screencols] = '\0';        
                                        editor_set_status_message(c->success, // TODO Special case: both %d and %s
                                                len, status_filename); // ? E->absolute_filename : E->filename);
                                        free(status_filename);
                                } else {
                                        editor_set_status_message(c->success, len, E->absolute_filename ? E->absolute_filename : E->filename);
                                }

				return; // fd closed, buf freed.
			} // if write ok
			syntax_select_highlight(NULL); 
		}
		close(fd);
	}
	free(buf);

	editor_set_status_message(c->error_status, strerror(errno));
	return; 
} /* editor_save -> Acommand_save ... */


/*** M-x command ***/

void
command_debug(int command_key) {
	if (!(E->debug & DEBUG_COMMANDS))
		return;

	struct command_str *c = command_get_by_key(command_key);
	if (c != NULL)
		editor_set_status_message(c->success);
}

/**
 * Return command_str by command_key 
 */
struct command_str *
command_get_by_key(int command_key) {
	unsigned int i;
        int entries = command_entries();
	for (i = 0; i < entries; i++) {
		if (COMMANDS[i].command_key == command_key) 
			return &COMMANDS[i];
	}
	return NULL;
}

void
command_insert_char(int character) {
	if (character <= 31 && character != 9) // TODO 9 = TABKEY
		return;

	undo_push_simple(COMMAND_INSERT_CHAR, COMMAND_DELETE_CHAR);
	editor_insert_char(character);
}

void 
command_delete_char() {
	editor_del_char(0);
	undo_debug_stack(); 
	command_debug(COMMAND_DELETE_CHAR);
}

void 
command_insert_newline() {
	int indent_len = editor_insert_newline();
	if (E->is_auto_indent 
		&& (indent_len % E->tab_stop == 0)) { // 1 for newline
		indent_len = indent_len / E->tab_stop; // no of tabs, soft or hard.
	}

	// Delete indented+1 chars at once.
	undo_push_one_int_arg(COMMAND_INSERT_CHAR, COMMAND_DELETE_INDENT_AND_NEWLINE, 
		indent_len + 1); // newline here

	if (E->debug & DEBUG_COMMANDS)
		editor_set_status_message("Inserted newline");
}

/**
  Return:
  0 = no argument gotten
  1 = one argument gotten.
  -1 = argument conversion error. 
 */ 
int
editor_get_command_argument(struct command_str *c, int *ret_int, char **ret_string) {
	char *raw_str = NULL;
	char *original_raw_string = NULL; 
	int raw_int = 0;
	int rc; 

	if (c == NULL)
		return 0; 

	/* FIXME now assume existence of %s in c->prompt. */
	raw_str = editor_prompt(c->prompt, NULL);
	if (raw_str == NULL)
		return 0; /* Aborted. */

	original_raw_string = strdup(raw_str); 

	if (c->command_arg_type == COMMAND_ARG_TYPE_STRING) {
		*ret_string = original_raw_string; 
		free(raw_str);
		return 1; // One argument 
	} else if (c->command_arg_type == COMMAND_ARG_TYPE_INT) {
		*ret_string = original_raw_string; /* For the error status message */

		// convert
		if (strlen(raw_str) > 8) {
			raw_str[8] = '\0';
		}

		rc = sscanf(raw_str, "%d", &raw_int); /* strtoimax(raw_str, NULL, 10); */

		free(raw_str);

		if (rc == 0) { 
			return -1; 
		}

		*ret_int = raw_int;
		return 1; 
	}

	free(raw_str);
	return 0; 
} // editor_get_command_argument

void
command_move_cursor(int command_key) {
	switch(command_key) {
	case COMMAND_MOVE_CURSOR_UP:
		/* Start macro */
		undo_push_simple(COMMAND_MOVE_CURSOR_UP, COMMAND_MOVE_CURSOR_DOWN);
		key_move_cursor(ARROW_UP);
		/* End macro */
		break;
	case COMMAND_MOVE_CURSOR_DOWN:
		undo_push_simple(COMMAND_MOVE_CURSOR_DOWN, COMMAND_MOVE_CURSOR_UP);
		key_move_cursor(ARROW_DOWN);
		break;
	case COMMAND_MOVE_CURSOR_LEFT:
		undo_push_simple(COMMAND_MOVE_CURSOR_LEFT, COMMAND_MOVE_CURSOR_RIGHT);
		key_move_cursor(ARROW_LEFT);
		break;
	case COMMAND_MOVE_CURSOR_RIGHT:
		undo_push_simple(COMMAND_MOVE_CURSOR_RIGHT, COMMAND_MOVE_CURSOR_LEFT);
		key_move_cursor(ARROW_RIGHT);
		break;
	default:
		break;		
	}
}

void
command_goto_line() {
        int int_arg = 0;
        char *char_arg = NULL;
        int current_cy = E->cy;
        
	struct command_str *c = command_get_by_key(COMMAND_GOTO_LINE);
	if (c == NULL)
                return;
                        
        int rc = editor_get_command_argument(c, &int_arg, &char_arg);
        if (rc == 0) {
                editor_set_status_message(STATUS_MESSAGE_ABORTED);
                free(char_arg);
                return;
        } else if (rc == -1) {
                editor_set_status_message(c->error_status, char_arg);
                free(char_arg); 
                return;
        }
        
        if (int_arg > 0 && int_arg < E->numrows) { 
                E->cy = int_arg - 1; 
                command_refresh_screen();        
        }
        free(char_arg); 
        undo_push_one_int_arg(COMMAND_GOTO_LINE, COMMAND_GOTO_LINE, current_cy);
}

void
command_goto_beginning_of_file() {
        E->cy = 0;
        E->cx = 0;       
}

void
command_goto_end_of_file() {
        E->cy = E->numrows;
        E->cx = 0;       
}

void
command_refresh_screen() {
        E->rowoff = E->cy - (TERMINAL.screenrows / 2);
        if (E->rowoff < 0)
                E->rowoff = 0;
}


void
exec_command() {
	char *command = NULL; 
        int entries; 
	unsigned int i = 0;
	int int_arg = 0;
	char *char_arg = NULL;
	int found = 0; 

	char *tmp = editor_prompt("Command: %s", NULL);
	if (tmp == NULL) {
		editor_set_status_message(STATUS_MESSAGE_ABORTED);
		return; 
	}

	command = strdup(tmp);
	free(tmp);

        entries = command_entries();        
	for (i = 0; i < entries; i++) {
		struct command_str *c = &COMMANDS[i];

		if (!strncmp(c->command_str, command, strlen(command))) {
			found = 1; 
			if ((c->command_arg_type == COMMAND_ARG_TYPE_INT 
				|| c->command_arg_type == COMMAND_ARG_TYPE_STRING)
				/* FIXME remove exception... */
				&& (c->command_key != COMMAND_SAVE_BUFFER 
					&& c->command_key != COMMAND_SAVE_BUFFER_AS)) {
				/* should do union. */

				// rc=1 is good: it's the number of successfully parsed arguments.
				int rc = editor_get_command_argument(c, &int_arg, &char_arg);
				if (rc == 0) { // Aborted
					editor_set_status_message(STATUS_MESSAGE_ABORTED);
					free(char_arg);
					free(command);
					return; 
				} else if (rc == -1) {
					editor_set_status_message(c->error_status, char_arg);
					free(char_arg);
					free(command);
					return;
				}
			}

			switch (c->command_key) {
			case COMMAND_SET_MODE:
				if (syntax_select_highlight(char_arg) == 0) { 
					editor_set_status_message(c->success, char_arg);
				} else {
					editor_set_status_message(c->error_status, char_arg);
				}
				break;
			case COMMAND_SET_TAB_STOP:
				if (int_arg >= 2) { 
					undo_push_one_int_arg(COMMAND_SET_TAB_STOP, COMMAND_SET_TAB_STOP, E->tab_stop);
					E->tab_stop = int_arg; 
					editor_set_status_message(c->success, int_arg);
				} else {
					editor_set_status_message(c->error_status, char_arg);
				}
				break;
			case COMMAND_SET_AUTO_INDENT: {
				int auto_indent_set = 0;
				if (!strcasecmp(char_arg, "on") 
					|| !strcasecmp(char_arg, "t") || !strcasecmp(char_arg, "true")) {
					E->is_auto_indent = 1;
					auto_indent_set = 1;
				} else if (!strcasecmp(char_arg, "off") 
					|| !!strcasecmp(char_arg, "f") || strcasecmp(char_arg, "false")) {
					E->is_auto_indent = 0;
					auto_indent_set = 1;
				}
				if (auto_indent_set)
					editor_set_status_message(c->success, E->is_auto_indent ? "on" : "off"); 
				else
					editor_set_status_message(c->error_status, char_arg);
				break;
			}
			case COMMAND_SET_HARD_TABS:
				E->is_soft_indent = 0;
				editor_set_status_message(c->success);
				break;
			case COMMAND_SET_SOFT_TABS:
				E->is_soft_indent = 1;
				editor_set_status_message(c->success);
				break;
			case COMMAND_SAVE_BUFFER:
			case COMMAND_SAVE_BUFFER_AS:
				editor_save(c->command_key);
				break;
			case COMMAND_MOVE_CURSOR_UP:
			case COMMAND_MOVE_CURSOR_DOWN:
			case COMMAND_MOVE_CURSOR_LEFT:
			case COMMAND_MOVE_CURSOR_RIGHT:
				command_move_cursor(c->command_key); // away (ref. helper f.)
				break;
			case COMMAND_UNDO:
				undo();
				break; 
			case COMMAND_INSERT_CHAR:
				if (strlen(char_arg) > 0) {
					int character = (int) char_arg[0];
					command_insert_char(character);
				} else {
					editor_set_status_message(c->error_status);
				}
				break; 
			case COMMAND_DELETE_CHAR:
				command_delete_char();
				break;	
			case COMMAND_INSERT_NEWLINE:
				command_insert_newline();
				break;
                        case COMMAND_GOTO_LINE:
                                if (int_arg >= 0 && int_arg < E->numrows) {
                                        undo_push_one_int_arg(COMMAND_GOTO_LINE, COMMAND_GOTO_LINE, E->cy);
                                        E->cy = int_arg;
                                }
                                break;
                        case COMMAND_GOTO_BEGINNING_OF_FILE:
                                undo_push_one_int_arg(COMMAND_GOTO_BEGINNING_OF_FILE, COMMAND_GOTO_LINE, E->cy); /* TODO E->cx */
                                command_goto_beginning_of_file(); 
                                break;
                        case COMMAND_GOTO_END_OF_FILE:
                                undo_push_one_int_arg(COMMAND_GOTO_BEGINNING_OF_FILE, COMMAND_GOTO_LINE, E->cy); /* TODO E->cx */
                                command_goto_end_of_file();
                                break;
                        case COMMAND_REFRESH_SCREEN:
                                command_refresh_screen();
                                break;
                        case COMMAND_CREATE_BUFFER: 
                        {
                                /* Note: for *Help* and *Compile* we set the type differently
                                   and the buffer creation is not done here. */
                                struct command_str *c = command_get_by_key(COMMAND_CREATE_BUFFER);
                                (void) create_buffer(BUFFER_TYPE_FILE, 1, c->success, COMMAND_NO_CMD);
                                break;
                        }
                        case COMMAND_DELETE_BUFFER:
                                delete_current_buffer();
                                break;
                        case COMMAND_NEXT_BUFFER:
                                command_next_buffer();
                                break;
                        case COMMAND_PREVIOUS_BUFFER:
                                command_previous_buffer();
                                break;
                        case COMMAND_OPEN_FILE:
                                /* Note: is current buffer is empty, open into it. 
                                Otherwise, create new buffer and open the file it.*/
                                command_open_file(char_arg);
                                break;
                        case COMMAND_MARK:
                                command_mark(c->success);
                                break;
                        case COMMAND_COPY_REGION:
                                command_copy_from_mark(c); 
                                break;
                        case COMMAND_KILL_REGION:
                                command_kill_from_mark(); // calls copy_from with *KILL* 
                                break;
			default:
				editor_set_status_message("Got command: '%s'", c->command_str);
				break;
			} 

                        if (char_arg != NULL)
			     free(char_arg);

			free(command);
			return;
		} /* if !strncasecmp */ 
	} /* for */

	if (!found) {
		editor_set_status_message("Unknown command: '%s'");
	}

	free(command);
	return;
}

