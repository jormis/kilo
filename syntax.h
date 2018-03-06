#ifndef SYNTAX_H
#define SYNTAX_H

#include "data.h"
#include "filetypes.h"

int is_separator(char c);
void editor_update_syntax(erow *row);
int editor_syntax_to_colour(int hl);
void editor_set_syntax(struct editor_syntax *syntax);
int editor_select_syntax_highlight(char *mode);

#endif

