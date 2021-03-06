/*
        kilo -- lightweight editor

        Based on the kilo project (https://github.com/antirez/kilo):

        Copyright (c) 2016, Salvatore Sanfilippo <antirez at gmail dot com>

        All rights reserved.

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions are met:

        * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        * Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.

        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
        ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
        WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
        DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
        ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
        (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
        LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
        ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
        (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
        SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "kilo.h"
#include "output.h"
#include "options.h"

/** buffers **/

void
handle_resize(int dummy) {
        char msg[80];
        
	if (get_window_size(&TERMINAL.screenrows, &TERMINAL.screencols) == -1)
		die("get_window_size@handle_resize");
        
        TERMINAL.screenrows -= 2; /* status & message bars */
        
        sprintf(msg, "Resized to %d rows and %d columns.", 
                TERMINAL.screenrows, TERMINAL.screencols);
                        
        editor_scroll();
        editor_set_status_message(msg);
        editor_refresh_screen();
}

void 
open_argument_files(int argc, char **argv, int index) {
        struct command_str *c = command_get_by_key(COMMAND_OPEN_FILE);

        if (index >= argc)
                return; 
                
        /* We have already called init_buffer() once before argument parsing. */
        command_open_file(argv[index]);
        
        while (++index < argc) {
                (void) create_buffer(BUFFER_TYPE_FILE, 0, c->success, COMMAND_NO_CMD);
                command_open_file(argv[index]);
        }
}


void
parse_options(int argc, char **argv) {
        int file_index = 0; // Start index of file names.
         
        Option *list = options_parse(argc, argv, "version|v,help|h,debug|d:i,ascii|a", &file_index);
        
        while (list != NULL) { // options_parse can return NULL
                if (list->is_set) {
                        if (! strcmp(list->long_option, "debug") 
                                || ! strcmp(list->short_option, "d")) {
                                E->debug = list->value.numeric; 
                        } else if (! strcmp(list->long_option, "ascii")
                                || ! strcmp(list->short_option, "a")) {
                                E->ascii_only = 1;                
                        } else if (! strcmp(list->long_option, "version") 
                                || ! strcmp(list->short_option, "v")) {
                                print_version();
                                exit(0);
                        } else if (! strcmp(list->long_option, "help") 
                                || ! strcmp(list->short_option, "h")) {
                                display_help();
                                exit(0);
                        }
                }
                list = list->next; 
        }
                
       if (file_index < argc)         
                open_argument_files(argc, argv, file_index);
}       

#if 0
void
old_parse_options(int argc, char **argv) {
        struct command_str *c = command_get_by_key(COMMAND_OPEN_FILE);
	if (argc >= 3) {
		if (!strcmp(argv[1], "-d") || !strcmp(argv[1], "--debug")) {
			E->debug = atoi(argv[2]);

                        /* TODO multiple files: create buffers & open. */
			if (argc >= 4) {
                                open_argument_files(3, argc, argv, c->success);
                        }
		}  else {
                        open_argument_files(1, argc, argv, c->success);
                }
	} else if (argc >= 2) {
		if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
                        print_version();
			exit(0);
		} else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
			display_help();
			exit(0);
                } else if (!strcmp(argv[1], "-a") || !strcmp(argv[1], "--ascii")) {
                        E->ascii_only = 1; 
		} else {
                        open_argument_files(1, argc, argv, c->success);

		}
	}
}
#endif

int 
main(int argc, char **argv) {
	enable_raw_mode();
	       
        buffer = create_buffer(BUFFER_TYPE_FILE, 0, "", COMMAND_NO_CMD);
	
        init_editor();
	parse_options(argc, argv); // Also opens file.

	editor_set_status_message(WELCOME_STATUS_BAR);

        signal(SIGWINCH, handle_resize);

	while (1) {
		editor_refresh_screen();
		editor_process_keypress();
	}

	return 0;
}
