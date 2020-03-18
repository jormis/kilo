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
int is_syntax_mode_set(); // a wrapper for E->syntax != NULL
int syntax_set_mode_by_name(char *name, int silent); // An alias for syntax_select_hightlight(mode)
int syntax_set_mode_by_filename_extension(int silent); // Note: E->filename already set.
int syntax_select_highlight(char *mode, int silent);

#endif

