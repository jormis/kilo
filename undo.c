#include "undo.h"
#include "buffer.h"
#include "clipboard.h"

extern struct buffer_str *current_buffer;
extern struct buffer_str *buffer;

/*** undo ***/
struct undo_str *
init_undo_stack(int default_command) {
	struct undo_str *undo_stack = malloc(sizeof(struct undo_str));
	if (undo_stack == NULL)
		die("undo stack");

	undo_stack->undo_command_key = default_command; //COMMAND_NO_CMD;
	undo_stack->command_key = default_command; //COMMAND_NO_CMD; // The terminus, new stack entries on top of this.
	undo_stack->cx = 0;
	undo_stack->cy = 0; 
	undo_stack->orig_value = 0;
	undo_stack->clipboard = NULL; 
	undo_stack->next = NULL; 

        return undo_stack; 
}

void
undo_debug_stack() {
	struct undo_str *p = current_buffer->undo_stack;
	char *debug = NULL; 
	int i = 1; 
	int currlen = 0;
	int from = 0;

	if (!(E->debug & DEBUG_UNDOS))
		return;

	while (p != NULL) {
		char *entry = malloc(80);
		int len = sprintf(entry, "|%d={%d,%d,%d}", i++, p->command_key, p->undo_command_key, p->orig_value);
		
		debug = realloc(debug, currlen + len + 1);
		memcpy(&debug[currlen], entry, len);
		debug[currlen + len] = '\0';
		currlen += len; 
		free(entry);	

		p = p->next; 
	} 

	if (currlen > TERMINAL.screencols)
		from = currlen - TERMINAL.screencols; 
	else
		from = 0; 

	editor_set_status_message(&debug[from]);
}

struct undo_str *
alloc_and_init_undo(int command_key) {
	struct undo_str *undo = malloc(sizeof(struct undo_str));
	if (undo == NULL)
		die("alloc_and_init_undo");

	undo->cx = E->cx;
	undo->cy = E->cy; 
	undo->command_key = command_key; 
        undo->clipboard = NULL; 
	undo->next = current_buffer->undo_stack;
	current_buffer->undo_stack = undo; 

	return undo; 
}

/* no args for commands. */
void
undo_push_simple(int command_key, int undo_command_key) {
//	if (current_buffer->undo_stack == NULL)
//		current_buffer->undo_stack = init_undo_stack(); 

	struct undo_str *undo = alloc_and_init_undo(command_key);
	undo->undo_command_key = undo_command_key; 
	undo->orig_value = -1; 
	undo_debug_stack();
}

void
undo_push_one_int_arg(int command_key, int undo_command_key, int orig_value) {
	struct undo_str *undo = alloc_and_init_undo(command_key);
	undo->undo_command_key = undo_command_key; 
	undo->orig_value = orig_value; 
	undo_debug_stack(); 
}

void
undo_push_clipboard() {
	struct clipboard *copy = clone_clipboard();
	struct undo_str *undo = alloc_and_init_undo(COMMAND_KILL_LINE);
	undo->undo_command_key = COMMAND_YANK_CLIPBOARD; 
	undo->clipboard = copy; 
	undo->orig_value = -999; 
	undo_debug_stack(); 
}

void
undo() {
	if (current_buffer->undo_stack == NULL) {
		current_buffer->undo_stack = init_undo_stack(COMMAND_NO_CMD);
		return;
	}

	if (current_buffer->undo_stack->command_key == COMMAND_NO_CMD)
		return; 

	struct undo_str *top = current_buffer->undo_stack; 	
	current_buffer->undo_stack = top->next; 

	undo_debug_stack();

	switch(top->undo_command_key) {
	case COMMAND_MOVE_CURSOR_UP:
		key_move_cursor(ARROW_UP);
		break;
	case COMMAND_MOVE_CURSOR_DOWN:
		key_move_cursor(ARROW_DOWN);
		break;
	case COMMAND_MOVE_CURSOR_LEFT:
		key_move_cursor(ARROW_LEFT);
		break;
	case COMMAND_MOVE_CURSOR_RIGHT:
		key_move_cursor(ARROW_RIGHT);
		break;
	case COMMAND_SET_TAB_STOP:
		E->tab_stop = top->orig_value;
		break;
	case COMMAND_SET_HARD_TABS:
		E->is_soft_indent = 0;
		break;
	case COMMAND_SET_SOFT_TABS:
		E->is_soft_indent = 1; 
		break;
	case COMMAND_SET_AUTO_INDENT:
		E->is_auto_indent = top->orig_value;
		break;
	case COMMAND_DELETE_CHAR:
		editor_del_char(1);
		break;
	case COMMAND_INSERT_CHAR:
		if (top->orig_value == '\r')
			editor_insert_newline(); 
		else
			editor_insert_char(top->orig_value);
		break; 
	case COMMAND_DELETE_INDENT_AND_NEWLINE: {
		// Undoes command_insert_newline().
		int del = top->orig_value; 
		while (del-- > 0)
			editor_del_char(1);
		break; 
		}
	case COMMAND_YANK_CLIPBOARD:
		undo_clipboard_kill_lines(top->clipboard);
		free(top->clipboard->row);
		free(top->clipboard);
                break;
        case COMMAND_GOTO_LINE: 
                E->cy = top->orig_value;
                command_refresh_screen(); 
                break; 
        case COMMAND_NEXT_BUFFER:
                command_next_buffer();
                break;
        case COMMAND_PREVIOUS_BUFFER:
                command_previous_buffer();
                break; 
	default:
		break; 
	}

	top->next = NULL; 
	free(top); 
}

