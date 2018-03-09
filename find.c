/**
        find.c
*/

#include "output.h"
#include "key.h"
#include "find.h"


void
editor_find_callback(char *query, int key) {
	static int last_match = -1; 
	static int direction = 1; 
	static int saved_hl_line; 
	static char *saved_hl; 

	if (saved_hl) {
		memcpy(E->row[saved_hl_line].hl, saved_hl, E->row[saved_hl_line].rsize);
		free(saved_hl);
		saved_hl = NULL; 
	}

	int current; 
	erow *row; 
	char *match; 
	int i; 

	if (key == '\r' || key == '\x1b') {
		last_match = -1; 
		direction = 1; 
		return; 
	} else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
		direction = 1; 
	} else if (key == ARROW_DOWN || key == ARROW_UP) {
		direction = -1; 
	} else {
		last_match = -1;
		direction = 1; 
	}

	if (last_match == -1)
		direction = 1; 
	current = last_match; 

	for (i = 0; i < E->numrows; i++) {		
		current += direction; 
		if (current == -1)
			current = E->numrows - 1; 
		else if (current == E->numrows)
			current = 0; 

		row = &E->row[current];
		match = strstr(row->render, query); 
		if (match) {
			last_match = current; 
			E->cy = current; 
			E->cx = editor_row_rx_to_cx(row, match - row->render); 
			E->rowoff = E->numrows; 

			saved_hl_line = current; 
			saved_hl = malloc(row->rsize);
			memcpy(saved_hl, row->hl, row->rsize);
			memset(&row->hl[match - row->render], HL_MATCH, strlen(query));

			break; 
		}
	}
}

void
editor_find() {
        char *query = editor_prompt(DEFAULT_SEARCH_PROMPT, editor_find_callback); 
	if (query) {
		free(query);
	} 
}

char *
editor_prompt(char *prompt, void (*callback) (char *, int)) {
	size_t bufsize = 128; 
	char *buf = malloc(bufsize); 
	size_t buflen = 0; 
	int c; 

	buf[0] = '\0';

	while (1) {
		editor_set_status_message(prompt, buf); 
		editor_refresh_screen();

		c = key_read();
		
		if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
			if (buflen != 0) 
				buf[--buflen] = '\0';
		} else if (c == '\x1b') {
			editor_set_status_message("");
			
			if (callback)
				callback(buf, c); 

			free(buf);
			return NULL; 
		} else if (c == '\r') {
			if (buflen != 0) {
				editor_set_status_message("");

				if (callback)
					callback(buf, c); 

				return buf; 
			}
		} else if (!iscntrl(c) && c < 128) {
			if (buflen == bufsize - 1) {
				bufsize *= 2; 
				buf = realloc(buf, bufsize); 
			}
			buf[buflen++] = c; 
			buf[buflen] = '\0';
		}

		if (callback)
			callback(buf, c); 
	}
}
