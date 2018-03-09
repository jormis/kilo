
#include "output.h"

void
ab_append(struct abuf *ab, const char *s, int len) {
	char *new = realloc(ab->b, ab->len + len);	
	if (new == NULL)
		return;

	memcpy(&new[ab->len], s, len); /* ! */
	ab->b = new;
	ab->len += len; 
}

void
ab_free(struct abuf *ab) {
	free(ab->b);
}

void
editor_scroll() {
	E->rx = 0; 

	if (E->cy < E->numrows) 
		E->rx = editor_row_cx_to_rx(&E->row[E->cy], E->cx);

	if (E->cy < E->rowoff)
		E->rowoff = E->cy;

	if (E->cy >= E->rowoff + TERMINAL.screenrows)
		E->rowoff = E->cy - TERMINAL.screenrows + 1; 
	
	if (E->rx < E->coloff) 
                E->coloff = E->rx;
  	
  	if (E->rx >= E->coloff + TERMINAL.screencols) 
                E->coloff = E->rx - TERMINAL.screencols + 1;
}

void
editor_draw_rows(struct abuf *ab) {
	int y;
	int filerow; 

	for (y = 0; y < TERMINAL.screenrows; y++) {
		filerow = y + E->rowoff; 

		if (filerow >= E->numrows) {
			if (!E->is_banner_shown && E->numrows == 0 && y == TERMINAL.screenrows / 3) {
				int padding = 0;
	      	        	char welcome[80];
                                int welcomelen = snprintf(welcome, sizeof(welcome),
        			     "%s", KILO_VERSION);
      			        if (welcomelen > TERMINAL.screencols) 
      				      welcomelen = TERMINAL.screencols;
      		
	      		        padding = (TERMINAL.screencols - welcomelen) / 2;
                                if (padding) {
                                        ab_append(ab, "~", 1);
	        		        padding--;
	      		        }

	      		        while (padding--) 
	         			ab_append(ab, " ", 1);

	      		        ab_append(ab, welcome, welcomelen);
	      	        } else { // / 3
			     ab_append(ab, "~", 1);
		        }
		} else {
			char *c; 
			unsigned char *hl; 
			int j; 
			int current_colour = -1; 
			int len = E->row[filerow].rsize - E->coloff;
			if (len < 0)
				len = 0; 

      		        if (len > TERMINAL.screencols) 
      			       len = TERMINAL.screencols;

      		        c  = &E->row[filerow].render[E->coloff];      		
      		        hl = &E->row[filerow].hl[E->coloff];

      		        for (j = 0; j < len; j++) {
      			        if (iscntrl(c[j])) {
      			                char sym = (c[j] <= 26) ? '@' + c[j] : '?';
                                        ab_append(ab, "\x1b[7m", 4);
      				        ab_append(ab, &sym, 1);
      				        ab_append(ab, "\x1b[m", 3);
      				        if (current_colour != -1) {
      					       char buf[16];
      					       int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_colour); 
      					       ab_append(ab, buf, clen);
                                        }

      			        } else if (hl[j] == HL_NORMAL) {
                                        if (current_colour != -1) {
                                                ab_append(ab, "\x1b[39m", 5); /* Text colours 30-37 (0=blak, 1=ref,..., 7=white. 9=reset*/
                                                current_colour = -1; 
      				        }
      				        ab_append(ab, &c[j], 1);
      				
      			        } else {
      				        int colour = syntax_to_colour(hl[j]);
                                        if (colour != current_colour) {
                                                char buf[16];
                                                int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", colour);
                                                ab_append(ab, buf, clen);
                                                current_colour = colour; 
                                        }

                                        ab_append(ab, &c[j], 1);
                                }
                        }

                        ab_append(ab, "\x1b[39m", 5); /* Final reset. */
                }

                ab_append(ab, "\x1b[K", 3); /* K = erase line */
                ab_append(ab, "\r\n", 2);
        }
}


#define ESC_PREFIX "\x1b["
#define ESC_PREFIX_LEN 2
#define APPEND_ESC_PREFIX(ab) (ab_append(ab, ESC_PREFIX, ESC_PREFIX_LEN))

enum esc_codes {
	ESC_BOLD = 1, 
	ESC_DIM = 2, 
	ESC_UNDERLINED = 4,
	ESC_BLINK = 5, 
	ESC_REVERSE = 7,
	ESC_HIDDEN = 8, 
	ESC_RESET_ALL = 0,
	ESC_RESET_BOLD = 21,
	ESC_RESET_DIM = 22,
	ESC_RESET_UNDERLINED = 24,
	ESC_RESET_BLINK = 25,
	ESC_RESET_REVERSE = 27,
	ESC_RESET_HIDDEN = 28
};

void
esc_invert(struct abuf *ab) {
	APPEND_ESC_PREFIX(ab);
	ab_append(ab, "7m", 2);
}

void
esc_bold(struct abuf *ab) {
	APPEND_ESC_PREFIX(ab);
	ab_append(ab, "1m", 2);
}

void
esc_reset_all(struct abuf *ab) {
	APPEND_ESC_PREFIX(ab);
	ab_append(ab, "0m", 2);
}

void
editor_draw_status_bar(struct abuf *ab) {
	int len = 0;
	int rlen = 0;
	char status[80], rstatus[80];

	//ab_append(ab, "\x1b[7m", 4); 
	esc_invert(ab);
	len = snprintf(status, sizeof(status), "-- %.16s %s - %d lines %s", 
		E->basename ? E->basename : "[No name]", E->is_new_file ? "(New file)" : "", E->numrows, 
		E->dirty ? "(modified)" : ""); 
	rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d", 
		E->syntax != NULL ? E->syntax->filetype : "no ft", E->cy + 1, E->numrows);

	if (len > TERMINAL.screencols)
		len = TERMINAL.screencols; 

	ab_append(ab, status, len); 

	while (len < TERMINAL.screencols) {
		if (TERMINAL.screencols - len == rlen) {
			ab_append(ab, rstatus, rlen);
			break; 
		} else {
			ab_append(ab, " ", 1);
			len++;
		}
	}

	esc_reset_all(ab);
	//ab_append(ab, "\x1b[m", 3); 
	ab_append(ab, "\r\n", 2); 
}


void 
debug_cursor() {
	char cursor[80];
	snprintf(cursor, sizeof(cursor), "cx=%d rx=%d cy=%d len=%d coloff=%d screencols=%d", 
		E->cx, E->rx, E->cy, E->row[E->cy].size, E->coloff, TERMINAL.screencols);
	editor_set_status_message(cursor); 
}

void
editor_draw_message_bar(struct abuf *ab) {
	int msglen; 
	ab_append(ab, "\x1b[K", 3); 

        if (E->debug & DEBUG_CURSOR) {
        	debug_cursor();
        }

	msglen = strlen(E->statusmsg); 
	if (msglen > TERMINAL.screencols)
		msglen = TERMINAL.screencols; 
	if (msglen && time(NULL) - E->statusmsg_time < 5) 
		ab_append(ab, E->statusmsg, msglen);

}

/**
	The first byte is \x1b, which is the escape character, 
	or 27 in decimal. (Try and remember \x1b, we will be using it a lot.) 
	The other three bytes are [2J.

	We are writing an escape sequence to the terminal. 
	Escape sequences always start with an escape character (27) 
	followed by a [ character. 

	Escape sequences instruct the terminal to do various text 
	formatting tasks, such as coloring text, moving the cursor around, 
	and clearing parts of the screen.

	We are using the J command (Erase In Display) to clear the screen. 
	Escape sequence commands take arguments, which come before the command. 
	In this case the argument is 2, which says to clear the entire screen. 
	<esc>[1J would clear the screen up to where the cursor is, 
	and <esc>[0J would clear the screen from the cursor up to the end of the screen. 
	Also, 0 is the default argument for J, so just <esc>[J by itself would also clear 
	the screen from the cursor to the end.
*/

void 
editor_refresh_screen() {
	char buf[32];
	struct abuf ab = ABUF_INIT;

	editor_scroll();

  	ab_append(&ab, "\x1b[?25l", 6); /* cursor off (l = reset mode) */ 
	ab_append(&ab, "\x1b[H", 3);

	editor_draw_rows(&ab);
	editor_draw_status_bar(&ab);
	editor_draw_message_bar(&ab);

  	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", 
  		(E->cy - E->rowoff) + 1, (E->rx + E->coloff) + 1);
  	ab_append(&ab, buf, strlen(buf));
 	ab_append(&ab, "\x1b[?25h", 6); /* cursor on (h = set mode) */

	write(STDOUT_FILENO, ab.b, ab.len);
	ab_free(&ab);
}

void
editor_set_status_message(const char *fmt, ...) {
	va_list ap; 
	va_start(ap, fmt);
	vsnprintf(E->statusmsg, sizeof(E->statusmsg), fmt, ap);
	va_end(ap);
	E->statusmsg_time = time(NULL);
}

