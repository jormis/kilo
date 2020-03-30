#include "token.h"
#include <string.h>
#include <ctype.h>

/**
 * token.c
 *
 */


/*** row operations ***/

/* 
        is_indent(row, triggers) 
        Note: Erlang's "->" is reduced to ">" 

        row     the row the cursor is on
        triggers char[] of single triggers eg, ">" or ":" or "}"
*/

int
is_indent(char *row, char *triggers, int line_len) {
        int i;
        unsigned int j; 
        if (row == NULL || triggers == NULL || strlen(triggers) == 0) 
                return 0;
                
        for (i = line_len-1; i >= 0; i--) {
                /* Check if the char in row belongs to trigger chars. */
                for (j = 0; j < strlen(triggers); j++) {
                        if (row[i] == triggers[j])
                                return 1; 
                }
                
                /* Not a trigger char. Continue only if white space. */
                if (!isspace(row[i])) {
                        return 0; 
                }
        }
        
        return 0; 
} 

int
is_whitespace_between(char *start, char *p) {
        while (start < p && isspace(*start))
                start++;
        
        return start == p; 
}

/* */
int
is_whitespace_to_end(char *p) {
        while (isspace(*p))
                p++;
                
        return *p == '\0';
}

/**
 * Fails with ^(WHITESPACE*)<< comment >>(WHITESPACE*)token 
 */
int 
is_first_token(char *row, char *token) { 
        if (row == NULL || token == NULL)
                return 0; 
                 
        char *p = strstr(row, token); 
        if (p == NULL) 
                return 0;         

        char *start = row; 
        while (start < p && isspace(*start))
                start++;
        
        return start == p;
}

int
is_last_token(char *row, char *token) {
        if (row == NULL || token == NULL)
                return 0; 
                 
        char *p = strstr(row, token);
        if (p == NULL) 
                return 0; 
                
        p += strlen(token);
        
        return is_whitespace_to_end(p);
}
