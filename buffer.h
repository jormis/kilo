#ifndef BUFFER_H
#define BUFFER_H

#include "data.h"
#include "terminal.h"
#include "clipboard.h"
#include "undo.h"
#include "output.h"
#include "init.h"
#include "command.h"

/** buffer */
enum buffer_type {
        BUFFER_TYPE_FILE = 0,
        BUFFER_TYPE_READONLY, 
        BUFFER_TYPE_COMMAND
};

struct buffer_str {
	int type; /* file, command, read (*Help*, for example) */
	struct editor_config E;
	struct undo_str *undo_stack;
        struct buffer_str *prev; 
	struct buffer_str *next; 
}; 

#define EDITOR_CONFIG &current_buffer->E

struct buffer_str *buffer;         /* Definition here. */
struct buffer_str *current_buffer; /* Definition here.*/

/** */
struct buffer_str *create_buffer(int type, int verbose, char *success, int init_undo_command);
void command_next_buffer(); /* TODO circular. */
void command_previous_buffer(); /* TODO circular. */
void delete_current_buffer();

#endif
