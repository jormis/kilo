#ifndef SYNTAX_H
#define SYNTAX_H
/**
        syntax.h
*/
#include "data.h"
#include "filetypes.h"

int is_separator(char c);
void syntax_update(erow *row);
int syntax_to_colour(int hl);
void syntax_set(struct editor_syntax *syntax);
int syntax_select_highlight(char *mode);

#endif

