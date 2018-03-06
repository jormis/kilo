#ifndef UNDO_H
#define UNDO_H

#include "const.h"
#include "terminal.h"
#include "output.h"
#include "clipboard.h"

struct undo_str {
	int command_key; // orginal command
	int undo_command_key; // undo command (if need to be run)
	int cx; 
	int cy;
	int orig_value; // set-tabs, auto-indent
	struct clipboard *clipboard; // kill, yank
	struct undo_str *next; // Because of stack.
};

struct undo_str *init_undo_stack(int init_command_key); // no more COMMAND_NO_KEY (ie command.h) any more.
void undo_debug_stack();
struct undo_str *alloc_and_init_undo(int command_key);
void undo_push_simple(int command_key, int undo_command_key);
void undo_push_one_int_arg(int command_key, int undo_command_key, int orig_value);
void undo_push_clipboard();
void undo();

#endif

