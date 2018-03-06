#ifndef TERMINAL_H
#define TERMINAL_H
/**
        terminal.h 

*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

/* Global terminal information. Separated from editor_config which is tied to buffer. */
struct term_config {
        int screenrows; 
        int screencols;
        struct termios orig_termios;
};

struct term_config TERMINAL; 

void die(const char *s);
void disable_raw_mode();
void enable_raw_mode();
int get_cursor_position(int *rows, int *cols);
int get_window_size(int *rows, int *cols);

#endif

