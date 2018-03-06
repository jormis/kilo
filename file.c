/*** file.c ***/

#include <stdlib.h>
#include <string.h>
#include "data.h"

extern struct editor_config *E;

char *
editor_rows_to_string(int *buflen) {
	int totlen = 0; 
	int j; 
	char *buf; 
	char *p; 

	for (j = 0; j < E->numrows; j++)
		totlen += E->row[j].size + 1; 

	*buflen = totlen; 

	buf = malloc(totlen); 
	p = buf; 

	for (j = 0; j < E->numrows; j++) {
		memcpy(p, E->row[j].chars, E->row[j].size);
		p += E->row[j].size; 
		*p = '\n';
		p++;
	}

	return buf; 
}

char *
editor_basename(char *path) {
	char *s = strrchr(path, '/');
	if (s == NULL) {
		return strdup(path);
	} else {
    	   return strdup(s + 1);
        }
}

