#ifndef FIND_H
#define FIND_H

/**
        find.h
*/

void editor_find_callback(char *query, int key);
void editor_find();
char *editor_prompt(char *prompt, void (*callback) (char *, int));

#endif
