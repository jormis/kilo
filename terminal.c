#include "terminal.h"

/**
        terminal.c
*/

void 
die(const char *s) {
	/* Clear the screen. */
 	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
  	perror(s);
  	exit(1);
}

void 
disable_raw_mode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &TERMINAL.orig_termios) == -1)
		die("tcsetattr");
}

void 
enable_raw_mode() {
	struct termios raw;

	if (tcgetattr(STDIN_FILENO, &TERMINAL.orig_termios) == -1)
		die("tcgetattr");

	atexit(disable_raw_mode);

	raw = TERMINAL.orig_termios;

	/* c_lflags are local flags. */ 
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);	
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 4; /* 4 = 400ms so we can catch <esc>-<key> better. */

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		die ("tcsetattr");
}

int 
get_cursor_position(int *rows, int *cols) {
	char buf[32];
  	unsigned int i = 0;
  	
  	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) 
  		return -1;	
  	/*
  		The reply is an escape sequence! It’s an escape character (27), 
  		followed by a [ character, and then the actual response: 24;80R, 
  		or similar.

  		We’re going to have to parse this response. But first, let’s read 
  		it into a buffer.  We’ll keep reading characters until we get to 
  		the R character.
  	*/
  	while (i < sizeof(buf) - 1) {
                if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
                if (buf[i] == 'R') break;
                i++;
  	}
  	
  	/*
  		When we print out the buffer, we don’t want to print the '\x1b' character, 
  		because the terminal would interpret it as an escape sequence and wouldn’t 
  		display it. So we skip the first character in buf by passing &buf[1] to 
  		printf().
  	*/
  	buf[i] = '\0';

  	if (buf[0] != '\x1b' || buf[1] != '[') 
  		return -1;
  	
  	if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) 
  		return -1;
  	
  	return 0;
}

int
get_window_size(int *rows, int *cols) {
	struct winsize ws;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
                if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) 
                        return -1;
		return get_cursor_position(rows, cols);
	} else {
		*cols = ws.ws_col; 
		*rows = ws.ws_row;
		return 0; 
	}
}

