#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "const.h"
#include "data.h"
#include "row.h"
#include "terminal.h"
#include "highlight.h"

struct abuf {
	char *b;
	int len; 
};

#define ABUF_INIT { NULL, 0 }

void ab_append(struct abuf *ab, const char *s, int len);
void ab_free(struct abuf *ab);

/* TODO editor -> output */
void editor_scroll();
void editor_draw_rows(struct abuf *ab);
void editor_refresh_screen();
void editor_set_status_message(const char *fmt, ...);
void editor_draw_message_bar(struct abuf *ab);
void editor_draw_status_bar(struct abuf *ab);
void editor_draw_rows(struct abuf *ab);
void debug_cursor(); /* TODO maybe in debug.[ch] */

#endif

