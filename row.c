#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "row.h"

extern struct editor_config *E; 

/*** row operations ***/

/* 
        is_indent(row, triggers) 
        Note: Erlang's "->" is reduced to ">" 

        row     the row the cursor is on
        triggers char[] of single triggers eg, ">" or ":" or "}"
*/

int
is_indent(erow *row, char *triggers) {
        int i;
        unsigned int j; 
        if (row == NULL || triggers == NULL || strlen(triggers) == 0) 
                return 0;
                
        for (i = E->cx-1; i >= 0; i--) {  
                /* Check if the char in row belongs to trigger chars. */
                for (j = 0; j < strlen(triggers); j++) {
                        if (row->chars[i] == triggers[j])
                                return 1; 
                }
                
                /* Not a trigger char. Continue only if white space. */
                if (!isspace(row->chars[i])) {
                        return 0; 
                }
        }
        
        return 0; 
} 


int
editor_row_cx_to_rx(erow *row, int cx) {
	int rx = 0;
	int j; 

	for (j = 0; j < cx; j++) {
		if (row->chars[j] == '\t') 
			rx += (E->tab_stop - 1) - (rx % E->tab_stop);
		rx++; 
	}
	return rx; 
}

int
editor_row_rx_to_cx(erow *row, int rx) {
	int cur_rx = 0;
	int cx; 

	for (cx = 0; cx < row->size; cx++) {
		if (row->chars[cx] == '\t')
			cur_rx = (E->tab_stop - 1) - (cur_rx % E->tab_stop);

		cur_rx++; 

		if (cur_rx > rx)
			return cx; 
	}

	return cx; 
}

void
editor_update_row(erow *row) {
	int j; 
	int idx = 0;
	int tabs = 0; 

	for (j = 0; j < row->size; j++) { // There may always be tabs.
		if (row->chars[j] == '\t') 
			tabs++;
	}

	free(row->render); 
	row->render = malloc(row->size + tabs * (E->tab_stop - 1) + 1);

	for (j = 0; j < row->size; j++) {
		if (row->chars[j] == '\t') {
			row->render[idx++] = ' ';
			while (idx % E->tab_stop != 0) 
				row->render[idx++] = ' ';
		} else {
			row->render[idx++] = row->chars[j];
		}
	}

	row->render[idx] = '\0';
	row->rsize = idx; 

	syntax_update(row);
}

void
editor_insert_row(int at, char *s, size_t len) {
	int j; 
	if (at < 0 || at > E->numrows > 0)
		return; 

	E->row = realloc(E->row, sizeof(erow) * (E->numrows + 1));
	memmove(&E->row[at + 1], &E->row[at], sizeof(erow) * (E->numrows - at));
	for (j = at + 1; j <= E->numrows; j++)
		E->row[j].idx++; 

	E->row[at].idx = at; 
  	E->row[at].size = len;
  	E->row[at].chars = malloc(len + 1);
  	memcpy(E->row[at].chars, s, len);
  	E->row[at].chars[len] = '\0';
  	E->row[at].rsize = 0;
  	E->row[at].render = NULL; 
  	E->row[at].hl = NULL;
  	E->row[at].hl_open_comment = 0; 

  	editor_update_row(&E->row[at]); 
  	
  	E->numrows++;
  	E->dirty++; 
}

void
editor_free_row(erow *row) {
	free(row->render);
	free(row->chars);
	free(row->hl);
}

void
editor_del_row(int at) {
	int j;
	if (at < 0 || at > E->numrows)
		return;

	editor_free_row(&E->row[at]);
	memmove(&E->row[at], &E->row[at + 1], sizeof(erow) * (E->numrows - at - 1));
	for (j = at; j < E->numrows - 1; j++)
		E->row[j].idx--;

	E->numrows--;
	E->dirty++;
}

int
editor_row_insert_char(erow *row, int at, char c) {
	int insert_len = 0;
	int i = 0; 
	int no_of_spaces = 0;

	if (at < 0 || at > row->size) 
		at = row->size; 

	if (c == '\t' && E->is_soft_indent) {
		/* 
		 * Calculate the number of spaces until the next tab stop. 
		 * Add E.tab_stop number of spaces if we are at the stop.
		 */
		no_of_spaces = E->tab_stop - (at % E->tab_stop);
		if (no_of_spaces == 0)
			no_of_spaces = E->tab_stop;

		insert_len = no_of_spaces;
		c = ' '; /* Tabs to spaces; swords to plows. */
	} else {
		/* Not a tab char or hard tabs set. */
		insert_len = 1; 
	}

	row->chars = realloc(row->chars, row->size + insert_len + 1); /* plus 1 is \0 */ 
	memmove(&row->chars[at + insert_len], &row->chars[at], row->size - at + 1); 

	for (i = 0; i < insert_len; i++) {
		row->size++;
		row->chars[at++] = c;
	} 

	editor_update_row(row); 	
	E->dirty++; 
	return insert_len;
}

void 
editor_row_append_string(erow *row, char *s, size_t len) {
	row->chars = realloc(row->chars, row->size + len + 1);
	memcpy(&row->chars[row->size], s, len);
	row->size += len; 
	row->chars[row->size] = '\0';
	editor_update_row(row);
	E->dirty++; 
}

/**
* at = E.cx - 1
*/
int
editor_row_del_char(erow *row, int at) {
	int len = 1; /* Default del len. */
	int i = 0; 
	int enough_spaces_to_the_left = 1; 

	if (at < 0 || at >= row->size)
		return 0;

	if (E->is_auto_indent) {
		if ((at+1) % E->tab_stop == 0) {
			/* There has to be at least E.tab_stop spaces to the left of 'at'.
				Note: start counting from TAB_STOP below & upwards. */
			for (i = at + 1 - E->tab_stop; enough_spaces_to_the_left && i >= 0 && i < at; i++) {
				if ((E->is_soft_indent && row->chars[i] != ' ')
					|| (!E->is_soft_indent && row->chars[i] != '\t')) {
					enough_spaces_to_the_left = 0; 
				}
			}

			if (enough_spaces_to_the_left)
				len = E->tab_stop;
		} else 
			enough_spaces_to_the_left = 0; 
	} 

	memmove(&row->chars[at + 1 - len], &row->chars[at + 1], row->size - at + 1); 
	row->size -= len;
	editor_update_row(row);
	E->dirty++; 

	return len; 
}

/*** editor operations ***/
void
editor_insert_char(int c) {
	if (E->cy == E->numrows) {
		editor_insert_row(E->numrows, "", 0); 
	}

	/* If soft_indent, we may insert more than one character. */
	E->cx += editor_row_insert_char(&E->row[E->cy], E->cx, c);  
}


/**
 * If auto_indent is on then we calculate the number of indents.
 * Here you can add language/mode specific indents. 
 */
int
calculate_indent(erow *row) {
	int iter = 0;
	int no_of_chars_to_indent = 0;
	int i = 0; 

	if (E->is_auto_indent) {
		iter = 1; 
		// Cutoff point is cursor == E.cx
		for (i = 0; iter && i < E->cx; i++) {
			if ((row->chars[i] == ' ' && E->is_soft_indent)	
				|| (row->chars[i] == '\t' && !E->is_soft_indent)) {
				no_of_chars_to_indent++;
			} else {
				iter = 0;
			}
		}

		if (E->is_soft_indent
			&& (no_of_chars_to_indent % E->tab_stop == 0)) {

			if (!strcasecmp(E->syntax->filetype, "Python")) { /* Little extra for Python mode. */
                                no_of_chars_to_indent += is_indent(row, ":\\") * E->tab_stop;
			} else if (!strcasecmp(E->syntax->filetype, "Erlang")) {
                                no_of_chars_to_indent += is_indent(row, ">") * E->tab_stop; // > not ->
			} else if (!strcasecmp(E->syntax->filetype, "Elm")) {
                                no_of_chars_to_indent += is_indent(row, "=") * E->tab_stop;
			} else if (!strcasecmp(E->syntax->filetype, "Bazel")) {
                                no_of_chars_to_indent += is_indent(row, "([") * E->tab_stop;
			} else if (!strcasecmp(E->syntax->filetype, "nginx")) {
                                no_of_chars_to_indent += is_indent(row, "{") * E->tab_stop;
                        } else if (!strcasecmp(E->syntax->filetype, "go")) {
                                no_of_chars_to_indent += is_indent(row, "{") * E->tab_stop;
                        }
		} else if (!E->is_soft_indent
		 	&& !strcasecmp(E->syntax->filetype, "Makefile")) {
                        // TODO like above 
			iter = 1; 
			for (i = 0; iter && i < E->cx; i++) {
				if (row->chars[i] == ':') { // target: dep
					no_of_chars_to_indent++;
					iter = 0;
				} 
			}
		}
	}

	return no_of_chars_to_indent; 
}

int
editor_insert_newline() {
	int no_of_chars_to_indent = 0;
	char *buf;
	erow *row; 

	if (E->cx == 0) {
		editor_insert_row(E->cy, "", 0); 
		no_of_chars_to_indent = 0; 
	} else {
		row = &E->row[E->cy];

		no_of_chars_to_indent = calculate_indent(row);

		if (no_of_chars_to_indent > 0) {
			/* # of new spaces + the end of row. */
			buf = malloc(no_of_chars_to_indent + row->size - E->cx + 1);
			if (no_of_chars_to_indent > 0) {
				memset(buf, E->is_soft_indent ? ' ' : '\t', no_of_chars_to_indent);
			}
			memcpy(&buf[no_of_chars_to_indent], &row->chars[E->cx], row->size - E->cx);
			buf[no_of_chars_to_indent + row->size - E->cx] = '\0';
			editor_insert_row(E->cy + 1, buf, strlen(buf));
			free(buf);
		} else {
			editor_insert_row(E->cy + 1, &row->chars[E->cx], row->size - E->cx); 
		}

		// Update the split upper row.
		row = &E->row[E->cy]; /* Reassign, because editor_insert_row() calls realloc(). */
		row->size = E->cx; 
		row->chars[row->size] = '\0'; 

		editor_update_row(row); 
	}
	
	E->cy++; 
	E->cx = no_of_chars_to_indent; // was: = 0 

	return no_of_chars_to_indent; 
}

