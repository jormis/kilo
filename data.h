#ifndef DATA_H
#define DATA_H
/**
 * data.h
 *
 * Contains data structures for 
 * - text row
 * - editor config
 * - editor (buffer/file) syntax
 */
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
	int numrows;
	erow *row; 
	int dirty; 
	char *filename; 
	char *absolute_filename; 
	char *basename; 
	char statusmsg[256];
	time_t statusmsg_time; 
	struct editor_syntax *syntax; 
	int is_new_file; 
	int is_banner_shown; /* If shown once do not show again. */
	int is_soft_indent; 
	int is_auto_indent; 
	int tab_stop;  
	int debug; 
        /* Set by COMMAND_MARK. Default values -1. */
        int mark_x, mark_y; 
        int ascii_only; 
};


struct editor_syntax {
        char *filetype;                // The Mode name
        char **filematch;              // A list of file extensions.
        char **executables;            // The basename executable in #!
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
