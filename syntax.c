#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "const.h"
#include "syntax.h"
#include "output.h"
#include "highlight.h"
#include "filetypes.h"

/* Defined in config.h */
extern struct editor_config *E;

/* Defined in filetypes.h */
extern struct editor_syntax HLDB[];

/*** syntax highlighting ***/

int
is_separator(char c) {
	return isspace(c) || c == '\0' || strchr(",.(){}+-/*=~%<>[];", c) != NULL;
}

void
editor_update_syntax(erow *row) {
	int i = 0; 
	int prev_sep = 1; 
	int in_string = 0; 
	int in_comment = 0; //(row->idx > 0 && E.row[row->idx - 1].hl_open_comment); 
	char prev_char = '\0'; /* JK */
	char *scs; 
	char *mcs;
	char *mce;
	int mcs_len;
	int mce_len; 
	int scs_len; 
	char **keywords; // = E.syntax->keywords; 
	int changed; 

	row->hl = realloc(row->hl, row->rsize);
	memset(row->hl, HL_NORMAL, row->rsize);

	if (E->syntax == NULL)
		return; 

	in_comment = (row->idx > 0 && E->row[row->idx - 1].hl_open_comment); 
	keywords = E->syntax->keywords; 

	scs = E->syntax->singleline_comment_start;
	mcs = E->syntax->multiline_comment_start;
	mce = E->syntax->multiline_comment_end;

	scs_len = scs ? strlen(scs) : 0; 
	mcs_len = mcs ? strlen(mcs) : 0; 
	mce_len = mce ? strlen(mce) : 0; 

	while (i < row->rsize) {
		char c = row->render[i];
		unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

		if (scs_len && !in_string && !in_comment) {
			if (!strncmp(&row->render[i], scs, scs_len)) {
				memset(&row->hl[i], HL_COMMENT, row->rsize - i); 
				break; 
			}
		}

		/* multiline comment end found while in mlcomment */
		if (mcs_len && mce_len && !in_string) {
			if (in_comment) {
				row->hl[i] = HL_MLCOMMENT;
				if (!strncmp(&row->render[i], mce, mce_len)) {
					memset(&row->hl[i], HL_MLCOMMENT, mce_len);
					i += mce_len;
					in_comment = 0;
					prev_sep = 1;
					continue;  
				} else {
					i++; 
					continue; 
				}
			} else if (!strncmp(&row->render[i], mcs, mcs_len)) {
				memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
				i += mcs_len;
				in_comment = 1; 
				continue; 
			}
		}

		if (E->syntax->flags & HL_HIGHLIGHT_STRINGS) {
			if (in_string) {
				row->hl[i] = HL_STRING;

				if (c == '\\' && i+ 1 < row->rsize) {
					row->hl[i + 1] = HL_STRING; 
					i += 2; 
					continue; 
				}

				if (c == in_string) /* Closing quote char. */
					in_string = 0; 
				i++;
				prev_sep = 1; 
				continue; 
			} else {
				if (c == '"' || c == '\'') {
					in_string = c; 
					row->hl[i] = HL_STRING; 
					i++; 
					continue; 

				}
			}
		}

		if (E->syntax->flags & HL_HIGHLIGHT_NUMBERS) {
			if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) 
					|| (c == '.' && prev_hl == HL_NUMBER)) {
				row->hl[i] = HL_NUMBER; 
				i++;
				prev_sep = 0;
				prev_char = c; 
				continue; 
			}
		}

		if (prev_sep) {
			int j; 
			for (j = 0; keywords[j]; j++) {
				int klen = strlen(keywords[j]);
				int is_keyword2 = keywords[j][klen - 1] == '|';

				if (is_keyword2) 
					klen--;

				if (!strncmp(&row->render[i], keywords[j], klen)
					&& is_separator(row->render[i + klen])) {
					memset(&row->hl[i], is_keyword2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
					i += klen;
					break;
				}
			}
			if (keywords[j] != NULL) {
				prev_sep = 0;
				continue; 
			}
		}

		prev_sep = is_separator(c);

		if (isspace(c) && i > 0 && prev_char == '.' && prev_hl == HL_NUMBER)
			row->hl[i - 1] = HL_NORMAL; /* Denormalize sentence ending colon. */
		prev_char = c; 
			
		i++;
	}

	changed = (row->hl_open_comment != in_comment); 
	row->hl_open_comment = in_comment; 
	if (changed && row->idx + 1 < E->numrows) {
		editor_update_syntax(&E->row[row->idx + 1]);
	}
}

int
editor_syntax_to_colour(int hl) {
	switch(hl) {
		case HL_COMMENT: 
		case HL_MLCOMMENT: return 36; 
		case HL_KEYWORD1: return 33;
		case HL_KEYWORD2: return 32; 
		case HL_NUMBER: return 31; 
		case HL_STRING: return 35; 
		case HL_MATCH: return 34; 
		default: return 37; 
	}
}

void 
editor_set_syntax(struct editor_syntax *syntax) {
	int filerow; 

	E->syntax = syntax; 
	E->tab_stop = E->syntax->tab_stop; // TODO refactor E->tab_stop away
	E->is_soft_indent = ! (E->syntax->flags & HARD_TABS); 
	E->is_auto_indent = E->syntax->is_auto_indent;

	for (filerow = 0; filerow < E->numrows; filerow++) {
		editor_update_syntax(&E->row[filerow]); 
	}
}

int
editor_select_syntax_highlight(char *mode) {
	unsigned int j; 
        int entries = hldb_entries();
	int mode_found = 0; 
	char *p = NULL ;
	E->syntax = NULL;

	if (E->filename == NULL && mode == NULL)
		return 0;  

	for (j = 0; j < entries; j++) {
		struct editor_syntax *s = &HLDB[j];
		unsigned int i = 0;

		/* Set explicitely based on cmd line option or M-x set-command-mode (& another prompt for the mode). */
		if (mode != NULL) {
			if (s->filetype) {
				if (! strcasecmp(mode, s->filetype)) {
					editor_set_syntax(s);

					mode_found = 1; 
					editor_set_status_message("Mode set to '%s'", s->filetype);
					return 0; 
				}
			}
		} else { /* mode == NULL, set it based on the filematch. */

			while (s->filematch[i]) {
				p = strstr(E->filename, s->filematch[i]); 
				if (p != NULL) {
					int patlen = strlen(s->filematch[i]); 
					if (s->filematch[i][0] != '.' || p[patlen] == '\0') {
						editor_set_syntax(s);
						return 0; 
					}
				}
				i++;
			} 
		}
	}

	if (mode != NULL && ! mode_found) {
		editor_set_status_message("Unknown mode '%s'", mode);
		return -1; 
	}

	return 0; 
}

