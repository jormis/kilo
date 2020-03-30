#ifndef ROW_H
#define ROW_H

#include <ctype.h>
#include "data.h"
#include "syntax.h"

/** 
        row.h
        A row of text. 
        TODO Implement a more efficient data structure. 
*/

int editor_row_cx_to_rx(erow *row, int cx);
int editor_row_rx_to_cx(erow *row, int rx);
void editor_update_row(erow *row);
void editor_insert_row(int at, char *s, size_t len);
void editor_free_row(erow *row);
void editor_del_row(int at);
int editor_row_insert_char(erow *row, int at, char c);
void editor_row_append_string(erow *row, char *s, size_t len);
int editor_row_del_char(erow *row, int at);

/** editor operations, maybe an own source file? */
void editor_insert_char(int c);
int calculate_indent(erow *row);
int editor_insert_newline();
void editor_del_char(int undo); /** XXX command delete char */

#endif
