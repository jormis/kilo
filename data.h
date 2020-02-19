#ifndef DATA_H
#define DATA_H

#include <ctype.h>
#include <time.h>

/* From row.h */
typedef struct erow {
	int idx;
	int size; 
	int rsize; 
	char *chars;
	char *render; 
	unsigned char *hl; 
	int hl_open_comment; 
} erow;


struct editor_config {
	int cx, cy; 
	int rx; 
	int rowoff;
	int coloff; 
	//int screenrows; TODO reintroduce
	//int screencols; TODO reintroduce
	int numrows;
	erow *row; 
	int dirty; 
	char *filename; 
	char *absolute_filename; 
	char *basename; 
	char statusmsg[256];
	time_t statusmsg_time; 
	struct editor_syntax *syntax; 
	//struct termios orig_termios;
	int is_new_file; 
	int is_banner_shown; /* If shown once do not show again. */
	int is_soft_indent; 
	int is_auto_indent; 
	int tab_stop;  
	int debug; 
        /* Set by COMMAND_MARK. Default values -1. */
        int mark_x, mark_y; 
};


struct editor_syntax {
	char *filetype; 
	char **filematch; 
	char **keywords; 
	char *singleline_comment_start; 
	char *multiline_comment_start; 
	char *multiline_comment_end; 
	int flags; // HARD_TAB here
	int tab_stop; 
	int is_auto_indent; 
}; 

/* Defined here. Points to current_buffer->E. */
struct editor_config *E;

#endif
