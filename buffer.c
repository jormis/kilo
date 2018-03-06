#include "buffer.h"

/* 	Multiple buffers. Editor config and undo stack are buffer specific; 
   	clipboard and editor syntax aren't. 

	Buffer types: file (like now), readonly (*Help), command (M-x compile, M-x repl?)

   	TODO: macros to handle buffer->E....blaa. ?
	TODO commands: open-file (C-x C-f), create-buffer, switch-buffer, 
	close-buffer(-and-save?), kill-buffer (?).
 */


/* create_buffer() - Becomes the current buffer. Cannot be undone. */
struct buffer_str *
create_buffer(int type, int verbose, char *success, int initial_undo_command) {
        struct editor_config *old_E = E; 
        //struct command_str *c = command_get_by_key(COMMAND_CREATE_BUFFER);
	struct buffer_str *p = malloc(sizeof(struct buffer_str));
	if (p == NULL)
		die("new buffer");

        p->type = type;
        p->undo_stack = init_undo_stack(initial_undo_command);
        p->prev = NULL; 
        p->next = NULL; 
        init_config(&p->E);

        if (current_buffer != NULL) {
                while (current_buffer->next != NULL)
                        current_buffer = current_buffer->next; 
                                
                current_buffer->next = p;
                p->prev = current_buffer;  
                current_buffer = p; 
                if (verbose)
                        editor_set_status_message(success);
                
        } else {
                // This is a call to init_buffer(); 
                current_buffer = p; 
        }
        
        E = &current_buffer->E; 
        if (old_E != NULL)
                E->debug = old_E->debug;
        return current_buffer;        
}

/** to command.c */
void
command_next_buffer() {
        if (current_buffer->next != NULL) {
                struct command_str *c = command_get_by_key(COMMAND_NEXT_BUFFER);
                current_buffer = current_buffer->next;
                E = &current_buffer->E;
                
                editor_scroll();

                undo_push_simple(COMMAND_NEXT_BUFFER, COMMAND_PREVIOUS_BUFFER);
                if (c != NULL)
                        editor_set_status_message(c->success);
        }
}

void
command_previous_buffer() {
        if (current_buffer->prev != NULL) {
                struct command_str *c = command_get_by_key(COMMAND_PREVIOUS_BUFFER);
                current_buffer = current_buffer->prev;
                E = &current_buffer->E;
                
                editor_scroll();
                
                undo_push_simple(COMMAND_PREVIOUS_BUFFER, COMMAND_NEXT_BUFFER);
                if (c != NULL)
                        editor_set_status_message(c->success);
        }
}

/* Cannot be undone.*/
void
delete_current_buffer() {
        struct buffer_str *new_current = NULL;
        struct undo_str *u = NULL;
        struct undo_str *n = NULL; 
        struct command_str *c = command_get_by_key(COMMAND_DELETE_BUFFER);
        
        if (current_buffer == NULL)
                return;
                
        if (current_buffer->prev == NULL && current_buffer->next == NULL) {
                editor_set_status_message("Only buffer -- cannot be deleted.");
                return; // Cannot delete the only buffer.    
        }

        // Not so fast: are there any unsaved changes? 
        // Ok, this is a quick and dirty solution for this buffer only.
        if (E->dirty) {
                editor_set_status_message(c->error_status);
                return; 
        }

        // Free undo_stack.
        u = current_buffer->undo_stack;
        while (u != NULL) {
                if (u->clipboard != NULL) { /* clipboard */
                        if (u->clipboard->row != NULL) {
                                int i = 0;
                                for (i = 0; i < u->clipboard->numrows; i++) {
                                        struct clipboard_row *r = &u->clipboard->row[i];
                                        if (r->row != NULL)
                                                free(r->row);
                                }
                                
                                free(u->clipboard->row); // clipboard_row * (a realloc'd chunk)
                        }
                        free(u->clipboard);
                } 
                
                n = u->next;
                free(u);
                u = n;
        }

        if (current_buffer->prev != NULL) {
                current_buffer->prev->next = current_buffer->next;
                new_current = current_buffer->prev; /* Prev has priority. */
        } 
        
        if (current_buffer->next != NULL) {
                current_buffer->next->prev = current_buffer->prev;
                if (new_current == NULL) /* If no prev the set current to next. */
                        new_current = current_buffer->next;
        }

        if (new_current == NULL)
                die("new current");
                
        free(current_buffer);        
        current_buffer = new_current;
        E = &current_buffer->E; 
        editor_set_status_message(c->success);
}                

