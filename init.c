#include "init.h"

void
init_config(struct editor_config *cfg) {
        cfg->cx = 0;
        cfg->cy = 0;
        cfg->rx = 0;
        cfg->numrows = 0;
        cfg->rowoff = 0;
        cfg->coloff = 0;
        cfg->dirty = 0;
        cfg->row = NULL; 
        cfg->filename = NULL; 
        cfg->absolute_filename = NULL; 
        cfg->basename = NULL; 
        cfg->statusmsg[0] = '\0';
        cfg->statusmsg_time = 0; 
        cfg->syntax = NULL; 
        cfg->is_new_file = 0;
        cfg->is_banner_shown = 0; 
        cfg->tab_stop = DEFAULT_KILO_TAB_STOP;
        cfg->is_soft_indent = 0;
        cfg->is_auto_indent = 0;
        cfg->debug = 0;
        cfg->mark_x = -1; 
        cfg->mark_y = -1; 
}


/*
void
init_buffer(int init_command_key) {
	buffer = create_buffer(BUFFER_TYPE_FILE, 0, init_command_key); // FIXME -- get rid of init_buffer()
}
*/
void
init_editor() {
        //init_buffer(); // Side effect: sets E.
	init_clipboard(); // C

        /* XXX TODO Need global terminal settings for new buffer config initialization. */
	if (get_window_size(&TERMINAL.screenrows, &TERMINAL.screencols) == -1)
		die("get_window_size");

        /* TODO BACK TO current_buffer->E at some point as E will be tied to a buffer, TERMINAL is global */
	TERMINAL.screenrows -= 2; /* Room for the status bar & status messages. */
}

void
init_clipboard() {
	clipboard_clear();
}


