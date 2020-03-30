#ifndef TOKEN_H
#define TOKEN_H

/**
 * is_indent
 *
 * return 1 is the line ends with (single chars) in triggers.
 * There can be whitespace after trigger char.
 *
 * Return 0 otherwise (trigger chars don't end the row.) 
 */
int is_indent(char *row, char *triggers, int line_len);

/* 
* [start, p[
*
* return 1 is only whitespace between start and p-1, 0 otherwise.
*/
int is_whitespace_between(char *start, char *p);

int is_whitespace_to_end(char *p);

/**
 * Fails with ^(WHITESPACE*)<< comment >>(WHITESPACE*)token 
 * 
 * Returns 1 is token is the first (whitespace separated) token
 * in the row? 
 *
 * Return 0 otherwise.
 */
int is_first_token(char *row, char *token);

/** Is token the last (whitespace separated) token in the row?
 *
 * Return 1 = yes; 0 = no
 */ 
int is_last_token(char *row, char *token);

#endif

