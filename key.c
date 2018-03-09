#include "key.h"

/* Defined in config.h */
extern struct editor_config *E;

/** key_read() */
int 
key_read() {
  	int nread;
  	char c;

	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN) die("read");
	}

  	if (c == '\x1b') {
  		char seq[3];
  		
  		if (read(STDIN_FILENO, &seq[0], 1) != 1) return c; //'\x1b'; /* vy!c?*/
  	
  		if (seq[0] == 'v' || seq[0] == 'V') { 
  			return PAGE_UP; 
  		} else if (seq[0] == 'c' || seq[0] == 'C') {
  			return CLEAR_MODIFICATION_FLAG_KEY; 
  		} else if (seq[0] == 'x' || seq[0] == 'X') {
  			return COMMAND_KEY; 
  		} else if (seq[0] == 'n' || seq[0] == 'N') {
                        return NEXT_BUFFER_KEY;
                } else if (seq[0] == 'p' || seq[0] == 'P') {
                        return PREVIOUS_BUFFER_KEY; 
                } else if (seq[0] == 'w' || seq[0] == 'W') {
                        return COPY_REGION_KEY; 
                } else if (seq[0] == 'a' || seq[0] == 'A') {
                        return GOTO_BEGINNING_OF_FILE_KEY;
                } else if (seq[0] == 'e' || seq[0] == 'E') {
                        return GOTO_END_OF_FILE_KEY;
                }
                
  		if (read(STDIN_FILENO, &seq[1], 1) != 1) return c; //'\x1b'; /*ditto*/

  		if (seq[0] == '[') {
  			if (seq[1] >= '0' && seq[1] <= '9') {
  				if (read(STDIN_FILENO, &seq[2], 1) != 1) return c; 
  				if (seq[2] == '~') { // <esc>5~ and <esc>6~ 
  					switch (seq[1]) {
  						case '1': return HOME_KEY;
  						case '3': return DEL_KEY;
  						case '4': return END_KEY;
  						case '5': return PAGE_UP;
  						case '6': return PAGE_DOWN;
  						case '7': return HOME_KEY;
  						case '8': return END_KEY;
  					}
  				}
  			} else {
  				switch (seq[1]) {
  					case 'A': return ARROW_UP;
  					case 'B': return ARROW_DOWN;
  					case 'C': return ARROW_RIGHT;
  					case 'D': return ARROW_LEFT;
  					case 'H': return HOME_KEY;
  					case 'F': return END_KEY;
  				}
  			}

  		} else if (seq[0] == 'O') {
  			switch (seq[1]) {
  				case 'H': return HOME_KEY;
  				case 'F': return END_KEY;
  			}
  		} 
  	} 

  	return c;
}

/**
*/
int 
key_normalize(int c) {
	if (c == CTRL_KEY('v'))
		c = PAGE_DOWN;
	else if (c == CTRL_KEY('y'))
		c = YANK_KEY;
	else if (c == CTRL_KEY('k'))
		c = KILL_LINE_KEY; 
	else if (c == CTRL_KEY('e'))
		c = END_KEY; 
	else if (c == CTRL_KEY('a'))
		c = HOME_KEY; 
	else if (c == CTRL_KEY('q'))
		c = QUIT_KEY;
	else if (c == CTRL_KEY('s'))
		c = SAVE_KEY;
	else if (c == CTRL_KEY('f'))
		c = FIND_KEY;
	else if (c == CTRL_KEY('u'))
		c = COMMAND_UNDO_KEY;
	else if (c == CTRL_KEY('g'))
		c = GOTO_LINE_KEY;
        else if (c == CTRL_KEY(' '))
                c = MARK_KEY;
        else if (c == CTRL_KEY('w')) 
                c = KILL_REGION_KEY; // M-W = COPY_REGION_KEY 
        else if (c == CTRL_KEY('l'))
                c = REFRESH_KEY;
        else if (c == CTRL_KEY('n'))
                c = NEW_BUFFER_KEY;
        else if (c == CTRL_KEY('o'))
                c = OPEN_FILE_KEY;
	return c; 
}

void 
key_move_cursor(int key) {
	int rowlen;
	erow *row = (E->cy >= E->numrows) ? NULL : &E->row[E->cy];

  	switch (key) {
        case ARROW_LEFT:
            	if (E->cx != 0) {
      		        E->cx--;
      		        if (E->coloff > 0)
      			       E->coloff--;

                } else if (E->cy > 0) {
        	       E->cy--;
       		       E->cx = E->row[E->cy].size;
                }
                break;
        case ARROW_RIGHT:
                //if (E.cx != E.screencols - 1)
                if (row && E->cx < row->size) {
      		        E->cx++;
                } else if (row && E->cx == row->size) {
        	       E->cy++;
        	       E->cx = 0;
                }
                break;
        case ARROW_UP:
                if (E->cy != 0)
      		        E->cy--;
                break;
        case ARROW_DOWN:
                if (E->cy < E->numrows)
      		        E->cy++;
                break;
        default:
                break;
  	}

        row = (E->cy >= E->numrows) ? NULL : &E->row[E->cy];
  	rowlen = row ? row->size : 0;
  	if (E->cx > rowlen) {
                E->cx = rowlen;
  	}
}

