/*** includes ***/

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/*** defines ***/

/*
	2017-04-20

	Latest: 
		- Auto-indent (2017-04-20)
			- Python, Makefile: use of ':' for more proper auto-indenting

	TODO .kilorc (tab-width) (M-x set-tab-width)
	TODO M-x save-buffer-as 
	TODO M-x command buffer & context-sensitive parameter buffer.
	TODO- M-x TAB command completion
	TODO store last command argument context-sensitively
	TODO 	- Emacs style C-K or C-SPC & C/M-W
	TODO command line options (what would those be?) without getopts
	TODO *Help* mode
	TODO Multiple buffers
	TODO- *Command* or *Shell* buffer (think of REPL) 
	TODO M-x compile (based on Mode & cwd contents): like Emacs (output)
		- compile based on HL mode & working diretory: make, mvn build, ant ?
	
	TODO modes
	- Shell mode 
	- Javascript mode
	- Clojure mode
	- Forth mode
	- Perl mode
	- other modes 
	TODO Unicode support (from ncurses?)
	TODO Forth interpreter, this elisp... (also: M-x forth-repl)
*/

#define KILO_VERSION "0.1.1 Proper autoindenting."
#define DEFAULT_KILO_TAB_STOP 8
#define KILO_QUIT_TIMES 3

/**
	The CTRL_KEY macro bitwise-ANDs a character with the value 00011111, in binary. 
	In other words, it sets the upper 3 bits of the character to 0. 
	This mirrors what the Ctrl key does in the terminal: it strips the 6th and 7th 
	bits from whatever key you press in combination with Ctrl, and sends that. 
	The ASCII character set seems to be designed this way on purpose. 
	(It is also similarly designed so that you can set and clear the 6th bit 
	to switch between lowercase and uppercase.)
*/
#define CTRL_KEY(k) ((k) & 0x1f)

enum editor_key {
	BACKSPACE = 127, 
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DEL_KEY,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN,
	REFRESH_KEY,
	QUIT_KEY, 
	SAVE_KEY,
	FIND_KEY,
	CLEAR_MODIFICATION_FLAG_COMMAND, /* M-c */

	/* These are more like commands.*/
	MARK_KEY, /* Ctrl-Space */
	KILL_LINE_KEY, /* Ctrl-K (nano mode) */
	DEL_TO_THE_EOL_KEY, /* Ctrl-K (emacs mode)*/
	DEL_FROM_MARK_KEY, /* Ctrl-W */
	COPY_TO_CLIPBOARD_KEY, /* M-w */
	YANK_KEY, /* Ctrl-Y */
	COMMAND_KEY /* M-x */
};

/*
	editor_config.hl is an array of unsigned char values, meaning integers in the range of 0 to 255. 
	Each value in the array will correspond to a character in render, and will tell you whether that 
	character is part of a string, or a comment, or a number, and so on. Let’s create an enum containing 
	the possible values that the hl array can contain.
*/
enum editor_highlight {
	HL_NORMAL = 0,
	HL_COMMENT, 
	HL_MLCOMMENT,
	HL_KEYWORD1,
	HL_KEYWORD2,
	HL_STRING,
	HL_NUMBER,
	HL_MATCH
};

#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

/* Default: soft tabs. */
#define HARD_TABS (1<<2)

/*** data ***/

/* row of text */
typedef struct erow {
	int idx;
	int size; 
	int rsize; 
	char *chars;
	char *render; 
	unsigned char *hl; 
	int hl_open_comment; 
} erow;

struct editor_config {
	int cx, cy; 
	int rx; 
	int rowoff;
	int coloff; 
	int screenrows; 
	int screencols; 
	int numrows;
	erow *row; 
	int dirty; 
	char *filename; 
	char *absolute_filename; 
	char *basename; 
	char statusmsg[80];
	time_t statusmsg_time; 
	struct editor_syntax *syntax; 
	struct termios orig_termios;
	int is_new_file; 
	int is_banner_shown; /* If shown once do not show again. */
	/* TODO */
	int is_soft_indent; 
	int is_auto_indent; 
	int tab_stop;  
	int debug; 
};

struct editor_config E;

/**
	row (E.row in phase 1; from E.cx' to E.cx'' in phase 2)
	is_eol = the whole line (phase 1; also phase 2 with double Ctrl-K or C-' ' & C/M-w)
*/

typedef struct clipboard_row {
	char *row; 
	int size; 
	int orig_x; /* TODO for emacs */
	int orig_y; /* TODO for undo */
	int is_eol; 
} clipboard_row; 

struct clipboard {
	/* 
		Fill clipboard with successive KILL_KEY presses. After the first non-KILL_KEY set C.is_full = 1
	   	as not to add anything more. The clipboard contents stay & can be yanked as many times as needed, 
	   	UNTIL the next KILL_KEY which clears clipboard and resets C.is_full to 0 if it was 1. 
	*/
	int is_full; 
	int numrows;
	clipboard_row *row; 
}; 

struct clipboard C; 

struct editor_syntax {
	char *filetype; 
	char **filematch; 
	char **keywords; 
	char *singleline_comment_start; 
	char *multiline_comment_start; 
	char *multiline_comment_end; 
	int flags; // HARD_TAB here
	int tab_stop; 
	int is_auto_indent; 
}; 

/*** filetypes ***/

char *C_HL_extensions[] = { ".c", ".h", ".cpp", NULL }; 
char *C_HL_keywords[] = {
	"auto", "break", "case", "const", "continue", "default", "do", 
	"else", "enum", "extern", "for", "goto", "if", "register", 
	"return", "signed", "sizeof", "static", "struct", "switch",
	"typedef", "union", "unsigned", "volatile", "while",

	/* C99 */
	"restrict", "inline", "_Bool", "bool", "_Complex", "complex", 
	"_Imaginary", "imaginary", "_Pragma", 
	/* C11 */
	"_Alignas", "alignas", "_Alignof", "alignof", 
	"_Atomic", "atomic_bool", "atomic_int", 
	"_Generic", "_Noreturn", "noreturn", 
	"_Static_assert", "static_assert", 
	"_Thread_local", "thread_local",  /* C11 */

	/* "asm", "fortran", */

	/* Those C++(upto 11) keywords not in C(upto11). */ 
	"and", "and_eq", "asm", "atomic_cancel", "atomic_commit", 
	"atomic_noexcept", "bitand", "bitor", "catch", "char16_t", 
	"char32_t", "class", "compl", "concept", "constexpr", 
	"const_cast", "decltype", "delete", "dynamic_cast", "explicit", 
	"export", "false", "friend", "import", "module", "mutable", 
	"namespace", "new", "noexcept", "not", "not_eq", "nullptr", 
	"operator", "or", "or_eq", "private", "protected", "public",
	"register", "reinterpret_cast", "requires", "static_assert",
	"static_cast", "synchronized", "template", "this", "throw",
	"true", "try", "typeid", "typename", "using", "virtual",
	"wchar_t", "xor", "xor_eq",  /* C++ 11 */

	/* cpp */
	"#if", "#ifdef", "#ifndef",
	"#elif", "#else", "#endif",  
	"#define", "#defined", "#undef", 
	"#include", "#pragma", "#line", "#error",

	/* C types */
	"int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|", "void|", 
	NULL
};

char *Java_HL_extensions[] = { ".java", NULL };
char *Java_HL_keywords[] = {
	"abstract", "assert", "break", "case", "catch", "class", "const", 
	"continue", "default", "do", "else", "enum", "extends", "final",
	"finally", "for", "goto", "if", "implements", "import", "instanceof", 
	"native", "new", "package", "private", "protected", "public", "return",  
	"static", "strictfp", "super", "switch", "synchronized", "this", "throw",
	"transient", "try", "void", "volatile", "while", "false", "null", "true",

	"boolean|", "byte|", "char|", "double|", "float|", "int|", "long|", 
	"Boolean|", "Byte|", "Character|", "Double|", "Float|", "Integer|", "Long|",
	"short|", "Short|",

	"@Deprecated", "@Override", "@SuppressWarnings", "@SafeVarargs", 
	"@FunctionalInterface", "@Retention", "@Documented", "@Target", 
	"@Inherited", "@Repeatable",

	"@Test", // There will be more.

	NULL
};

char *Python_HL_extensions[] = { ".py", NULL };
char *Python_HL_keywords[] = {
	"False", "None", "True", 
	"and", "as", "assert", "break", "class", "continue", "def", "del", 
	"elif", "else", "except", "finally", "for", "from", "global", 
	"if", "import", "in", "is", "lambda", "nonlocal", "not", "or",
	"pass", "raise", "return", "try", "while", "with", "yield",

	"int|", "float|", "complex|", "decimal|", "fraction|", 
	"container|", "iterator|", "list|", "tuple|", "range|", 
	"bytes|", "bytearray|", 
	"set|", "frozenset|", 
	"dict|", 

	NULL
};

char *Text_HL_extensions[] = { ".txt", ".ini", ".cfg", NULL };
char *Text_HL_keywords[] = { NULL };

char *Makefile_HL_extensions[] = { "Makefile", "makefile", NULL };
char *Makefile_HL_keywords[] = { NULL };

struct editor_syntax HLDB[] = {
	{
		"Text",
		Text_HL_extensions,
		Text_HL_keywords,
		"#", 
		"", "", 
		HARD_TABS,
		DEFAULT_KILO_TAB_STOP, /* tab stop */
		0  /* auto */
	},
	{
		"Makefile",
		Makefile_HL_extensions,
		Makefile_HL_keywords, 
		"#",
		"", "", /* Comment continuation by backslash is missing. */
		HARD_TABS,
		DEFAULT_KILO_TAB_STOP,
		1 /* auto indent */
	},
	{
		"C", 
		C_HL_extensions, 
		C_HL_keywords,
		"//", 
		"/*", "*/",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
		DEFAULT_KILO_TAB_STOP,
		1, /* auto indent */
	},
	{
		"Java", 
		Java_HL_extensions, 
		Java_HL_keywords,
		"//", 
		"/*", "*/",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
		4,
		1
	},

	/* Python requires multiline strings, soft indent & auto indent. */
	{
		"Python",
		Python_HL_extensions,
		Python_HL_keywords,
		"#",
		"'''", "'''",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
		4,
		1
	}
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

enum command_key {
	COMMAND_SET_MODE = 1,
	COMMAND_SET_TAB_STOP,
	COMMAND_SET_SOFT_TABS,
	COMMAND_SET_HARD_TABS,
	COMMAND_SET_AUTO_INDENT,
	COMMAND_SAVE_AS,
	COMMAND_OPEN_FILE
};

enum command_arg_type {
	COMMAND_ARG_TYPE_NONE = 0,
	COMMAND_ARG_TYPE_INT,
	COMMAND_ARG_TYPE_STRING
};

struct command_str {
	int command_key;
	char *command_str;
	int command_arg_type;
	char *prompt; 
	char *success;
	char *error_status;
};


/* M-x "command-str" <ENTER> followed by an optional argument (INT or STRING). */ 
struct command_str COMMANDS[] = {
	{
		COMMAND_SET_MODE,
		"set-mode",
		COMMAND_ARG_TYPE_STRING,
		"Mode: %s",
		"Mode set to: '%s'",
		"Failed to set mode to '%s'"
	},
	{
		COMMAND_SET_TAB_STOP,
		"set-tab-stop",
		COMMAND_ARG_TYPE_INT,
		"Set tab stop to: %s", // always %s
		"Tab stop set to: %d", // based on ARG_TYPE: %d or %s
		"Invalid tab stop value: '%s' (range 2-)"

	},
	{
		COMMAND_SET_SOFT_TABS,
		"set-soft-tabs",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"Soft tabs in use.",
		NULL,
	},
	{
		COMMAND_SET_HARD_TABS,
		"set-hard-tabs",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"Hard tabs in use",
		NULL
	},
	{
		COMMAND_SET_AUTO_INDENT,
		"set-auto-indent",
		COMMAND_ARG_TYPE_STRING,
		"Set auto indent on or off: %s",
		"Auto indent in use.",
		"Invalid auto indent mode: '%s'"
	},
	{
		COMMAND_SAVE_AS,
		"save-buffer-as",
		COMMAND_ARG_TYPE_STRING,
		"Save buffer as: %s",
		"Buffer saved successfully to %s",
		"Failed to save buffer as '%s'"
	},
	{
		COMMAND_OPEN_FILE,
		"open-file",
		COMMAND_ARG_TYPE_STRING,
		"File to open: %s",
		"Opened %s",
		"Failed to open '%s'"
	}
};

#define COMMAND_ENTRIES (sizeof(COMMANDS) / sizeof(COMMANDS[0]))

/*** prototypes ***/

void editor_set_status_message(const char *fmt, ...);
void editor_refresh_screen();
char *editor_prompt(char *prompt, void (*callback) (char *, int));
void editor_move_cursor(int key);

/*** terminal ***/

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
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
		die("tcsetattr");
}

void 
enable_raw_mode() {
	struct termios raw;

	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
		die("tcgetattr");

	atexit(disable_raw_mode);

	raw = E.orig_termios;

	/* c_lflags are local flags. */ 
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);	
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 2; /* 1 -> 2 (200ms) so we can catch <esc>V better. */

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		die ("tcsetattr");
}

int 
editor_read_key() {
  	int nread;
  	char c;
  	/* char debug[6]; */

	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN) die("read");
	}

  	if (c == '\x1b') {
  		//write(STDERR_FILENO, "|ESC|", 6); 

  		char seq[3];
  		
  		if (read(STDIN_FILENO, &seq[0], 1) != 1) return c; //'\x1b'; /* vy!c?*/
  	
  		/*sprintf(debug, "<<%c>>", seq[0]);
  		write(STDERR_FILENO, debug, 6); */

  		/* if (key = editor_esc_keybindings(c)) return key; 
			TODO When I add more. Like <esc>F for search. */
  		if (seq[0] == 'v') { 
  			return PAGE_UP; 
  		} else if (seq[0] == 'c') {
  			return CLEAR_MODIFICATION_FLAG_COMMAND; 
  		} else if (seq[0] == 'x') {
  			return COMMAND_KEY; 
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

/*** syntax highlighting ***/

int
is_separator(char c) {
	return isspace(c) || c == '\0' || strchr(",.(){}+-/*=~%<>[];", c) != NULL;
}

void
editor_update_syntax(erow *row) {
	int i = 0; 
	int prev_sep = 1; 
	int in_string = 0; 
	int in_comment = 0; //(row->idx > 0 && E.row[row->idx - 1].hl_open_comment); 
	char prev_char = '\0'; /* JK */
	char *scs; 
	char *mcs;
	char *mce;
	int mcs_len;
	int mce_len; 
	int scs_len; 
	char **keywords; // = E.syntax->keywords; 
	int changed; 

	row->hl = realloc(row->hl, row->rsize);
	memset(row->hl, HL_NORMAL, row->rsize);

	if (E.syntax == NULL)
		return; 

	in_comment = (row->idx > 0 && E.row[row->idx - 1].hl_open_comment); 
	keywords = E.syntax->keywords; 

	scs = E.syntax->singleline_comment_start;
	mcs = E.syntax->multiline_comment_start;
	mce = E.syntax->multiline_comment_end;

	scs_len = scs ? strlen(scs) : 0; 
	mcs_len = mcs ? strlen(mcs) : 0; 
	mce_len = mce ? strlen(mce) : 0; 

	while (i < row->rsize) {
		char c = row->render[i];
		unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

		if (scs_len && !in_string && !in_comment) {
			if (!strncmp(&row->render[i], scs, scs_len)) {
				memset(&row->hl[i], HL_COMMENT, row->rsize - i); 
				break; 
			}
		}

		/* multiline comment end found while in mlcomment */
		if (mcs_len && mce_len && !in_string) {
			if (in_comment) {
				row->hl[i] = HL_MLCOMMENT;
				if (!strncmp(&row->render[i], mce, mce_len)) {
					memset(&row->hl[i], HL_MLCOMMENT, mce_len);
					i += mce_len;
					in_comment = 0;
					prev_sep = 1;
					continue;  
				} else {
					i++; 
					continue; 
				}
			} else if (!strncmp(&row->render[i], mcs, mcs_len)) {
				memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
				i += mcs_len;
				in_comment = 1; 
				continue; 
			}
		}

		if (E.syntax->flags & HL_HIGHLIGHT_STRINGS) {
			if (in_string) {
				row->hl[i] = HL_STRING;

				if (c == '\\' && i+ 1 < row->rsize) {
					row->hl[i + 1] = HL_STRING; 
					i += 2; 
					continue; 
				}

				if (c == in_string) /* Closing quote char. */
					in_string = 0; 
				i++;
				prev_sep = 1; 
				continue; 
			} else {
				if (c == '"' || c == '\'') {
					in_string = c; 
					row->hl[i] = HL_STRING; 
					i++; 
					continue; 

				}
			}
		}

		if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
			if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) 
					|| (c == '.' && prev_hl == HL_NUMBER)) {
				row->hl[i] = HL_NUMBER; 
				i++;
				prev_sep = 0;
				prev_char = c; 
				continue; 
			}
		}

		if (prev_sep) {
			int j; 
			for (j = 0; keywords[j]; j++) {
				int klen = strlen(keywords[j]);
				int is_keyword2 = keywords[j][klen - 1] == '|';

				if (is_keyword2) 
					klen--;

				if (!strncmp(&row->render[i], keywords[j], klen)
					&& is_separator(row->render[i + klen])) {
					memset(&row->hl[i], is_keyword2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
					i += klen;
					break;
				}
			}
			if (keywords[j] != NULL) {
				prev_sep = 0;
				continue; 
			}
		}

		prev_sep = is_separator(c);

		/* JK */
		if (isspace(c) && i > 0 && prev_char == '.' && prev_hl == HL_NUMBER)
			row->hl[i - 1] = HL_NORMAL; /* Denormalize sentence ending colon. */
		prev_char = c; 
			
		i++;
	}

	changed = (row->hl_open_comment != in_comment); 
	row->hl_open_comment = in_comment; 
	if (changed && row->idx + 1 < E.numrows) {
		editor_update_syntax(&E.row[row->idx + 1]);
	}
}

int
editor_syntax_to_colour(int hl) {
	switch(hl) {
		case HL_COMMENT: 
		case HL_MLCOMMENT: return 36; 
		case HL_KEYWORD1: return 33;
		case HL_KEYWORD2: return 32; 
		case HL_NUMBER: return 31; 
		case HL_STRING: return 35; 
		case HL_MATCH: return 34; 
		default: return 37; 
	}
}

void 
editor_set_syntax(struct editor_syntax *syntax) {
	int filerow; 

	E.syntax = syntax; 
	E.tab_stop = E.syntax->tab_stop;
	E.is_soft_indent = ! (E.syntax->flags & HARD_TABS); 
	E.is_auto_indent = E.syntax->is_auto_indent;

	for (filerow = 0; filerow < E.numrows; filerow++) {
		editor_update_syntax(&E.row[filerow]); 
	}
}

int
editor_select_syntax_highlight(char *mode) {
	unsigned int j; 
	int mode_found = 0; 
	char *p = NULL ;
	E.syntax = NULL;

	if (E.filename == NULL && mode == NULL)
		return 0;  

	for (j = 0; j < HLDB_ENTRIES; j++) {
		struct editor_syntax *s = &HLDB[j];
		unsigned int i = 0;

		/* Set explicitely based on cmd line option or M-x set-command-mode (& another prompt for the mode). */
		if (mode != NULL) {
			if (s->filetype) {
				if (! strcasecmp(mode, s->filetype)) {
					editor_set_syntax(s);

					mode_found = 1; 
					editor_set_status_message("Mode set to '%s'", s->filetype);
					return 0; 
				}
			}
		} else { /* mode == NULL, set it based on the filematch. */

			while (s->filematch[i]) {
				p = strstr(E.filename, s->filematch[i]); 
				if (p != NULL) {
					int patlen = strlen(s->filematch[i]); 
					if (s->filematch[i][0] != '.' || p[patlen] == '\0') {
						editor_set_syntax(s);
						return 0; 
					}
				}
				i++;
			} 
		}
	}

	if (mode != NULL && ! mode_found) {
		editor_set_status_message("Unknown mode '%s'", mode);
		return -1; 
	}

	return 0; 
}

/*** row operations ***/

int
editor_row_cx_to_rx(erow *row, int cx) {
	int rx = 0;
	int j; 

	for (j = 0; j < cx; j++) {
		if (row->chars[j] == '\t') 
			rx += (E.tab_stop - 1) - (rx % E.tab_stop);
		rx++; 
	}
	return rx; 
}

int
editor_row_rx_to_cx(erow *row, int rx) {
	int cur_rx = 0;
	int cx; 

	for (cx = 0; cx < row->size; cx++) {
		if (row->chars[cx] == '\t')
			cur_rx = (E.tab_stop - 1) - (cur_rx % E.tab_stop);

		cur_rx++; 

		if (cur_rx > rx)
			return cx; 

	}

	return cx; 
}

void
editor_update_row(erow *row) {
	int j; 
	int idx = 0;
	int tabs = 0; 

	for (j = 0; j < row->size; j++) { // There may always be tabs.
		if (row->chars[j] == '\t') 
			tabs++;
	}

	free(row->render); 
	row->render = malloc(row->size + tabs * (E.tab_stop - 1) + 1);

	for (j = 0; j < row->size; j++) {
		if (row->chars[j] == '\t') {
			row->render[idx++] = ' ';
			while (idx % E.tab_stop != 0) 
				row->render[idx++] = ' ';
		} else {
			row->render[idx++] = row->chars[j];
		}
	}

	row->render[idx] = '\0';
	row->rsize = idx; 

	editor_update_syntax(row);
}


void
editor_insert_row(int at, char *s, size_t len) {
	int j; 
	if (at < 0 || at > E.numrows > 0)
		return; 

	E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
	memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));
	for (j = at + 1; j <= E.numrows; j++)
		E.row[j].idx++; 

	E.row[at].idx = at; 
  	
  	E.row[at].size = len;
  	E.row[at].chars = malloc(len + 1);
  	memcpy(E.row[at].chars, s, len);
  	E.row[at].chars[len] = '\0';

  	E.row[at].rsize = 0;
  	E.row[at].render = NULL; 
  	E.row[at].hl = NULL;
  	E.row[at].hl_open_comment = 0; 

  	editor_update_row(&E.row[at]); 
  	
  	E.numrows++;
  	E.dirty++; 
}

void
editor_free_row(erow *row) {
	free(row->render);
	free(row->chars);
	free(row->hl);
}

void
editor_del_row(int at) {
	int j;
	if (at < 0 || at > E.numrows)
		return;

	editor_free_row(&E.row[at]);
	memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
	for (j = at; j < E.numrows - 1; j++)
		E.row[j].idx--;

	E.numrows--;
	E.dirty++;
}


int
editor_row_insert_char(erow *row, int at, char c) {
	int insert_len = 0;
	int i = 0; 
	//int old_at = at; 
	int no_of_spaces = 0;

	if (at < 0 || at > row->size) 
		at = row->size; 

	if (c == '\t' && E.is_soft_indent) {
		/* 
			Calculate the number of spaces until the next tab stop. 
			Add E.tab_stop number of spaces if we are at the stop.
		*/
		no_of_spaces = E.tab_stop - (at % E.tab_stop);
		if (no_of_spaces == 0)
			no_of_spaces = E.tab_stop;

		insert_len = no_of_spaces;
		c = ' '; /* Tabs to spaces; swords to plows. */
	} else {
		/* Not a tab char or hard tabs set. */
		insert_len = 1; 
	}

	row->chars = realloc(row->chars, row->size + insert_len + 1); /* plus 1 is \0 */ 
	memmove(&row->chars[at + insert_len], &row->chars[at], row->size - at + 1); 

	for (i = 0; i < insert_len; i++) {
		row->size++;
		row->chars[at++] = c;
	} 

	editor_update_row(row); 
	
	E.dirty++; 

/*
	editor_set_status_message("cy.cx=%d.%d t=%d, soft=%s: '%c', %d -> %d (at mod ts = %d), insert_len = %d", 
			E.cy, at, E.tab_stop, (E.is_soft_indent ? "on" : "off"), c, old_at, at, 
			(old_at % E.tab_stop == 0 ? E.tab_stop : old_at % E.tab_stop), insert_len);
*/
	return insert_len;
}

void 
editor_row_append_string(erow *row, char *s, size_t len) {
	row->chars = realloc(row->chars, row->size + len + 1);
	memcpy(&row->chars[row->size], s, len);
	row->size += len; 
	row->chars[row->size] = '\0';
	editor_update_row(row);
	E.dirty++; 
}

/**
* at = E.cx - 1
*/
int
editor_row_del_char(erow *row, int at) {
	int del_len = 1; /* Default del len. */
	int i = 0; 
	int enough_spaces_to_the_left = 1; 

	if (at < 0 || at >= row->size)
		return 0;

	if (E.is_soft_indent) {
		if ((at+1) % E.tab_stop == 0) {
			/* There has to be at least E.tab_stop spaces to the left of 'at'.
				Note: start counting from TAB_STOP below & upwards. */
			for (i = at + 1 - E.tab_stop; enough_spaces_to_the_left && i >= 0 && i < at; i++) {
				if (row->chars[i] != ' ') {
					enough_spaces_to_the_left = 0; 
				}
			}

			if (enough_spaces_to_the_left)
				del_len = E.tab_stop;
		} else 
			enough_spaces_to_the_left = 0; 
	} 

//	memmove(&row->chars[at - del_len + 1], &row->chars[at + 1], row->size - at); 
	memmove(&row->chars[at + 1 - del_len], &row->chars[at + 1], row->size - at + 1); 
	row->size -= del_len;
	editor_update_row(row);
	E.dirty++; 


	if (E.debug)
		editor_set_status_message("cy.cx=%d.%d t=%d, soft=%s: at+q+ET=%d, at/ts=%d, at=%d -> %d, en=%d, del_len=%d", 
			E.cy, at, E.tab_stop, (E.is_soft_indent ? "on" : "off"), 
			at+1-E.tab_stop,
			((at+1) % E.tab_stop), at, at-del_len+1, 
			enough_spaces_to_the_left, del_len);

	return del_len; 
}

/*** editor operations ***/
void
editor_insert_char(int c) {
	if (E.cy == E.numrows) {
		editor_insert_row(E.numrows, "", 0); 
	}

	/* If soft_indent, we may insert more than one character. */
	E.cx += editor_row_insert_char(&E.row[E.cy], E.cx, c);  
}

void
editor_insert_newline() {
	int i = 0;
	int no_of_chars_to_indent = 0;
	int iter = 1; 
	char *buf;

	if (E.cx == 0) {
		editor_insert_row(E.cy, "", 0); 
		no_of_chars_to_indent = 0; 
	} else {
		// E.cx != 0 (> 0) so like 
		// def foo:<INSERT NEWLINE ie PRESS ENTER>
		erow *row = &E.row[E.cy];

		if (E.is_auto_indent) {
			iter = 1; 
			// Cutoff point is cursor == E.cx
			for (i = 0; iter && i < E.cx; i++) {
				if ((row->chars[i] == ' ' && E.is_soft_indent)	
					|| (row->chars[i] == '\t' && !E.is_soft_indent)) {
					no_of_chars_to_indent++;
				} else {
					iter = 0;
				}
			}

			//if (no_of_chars_to_indent > 0) {

			if (E.is_soft_indent
				&& (no_of_chars_to_indent % E.tab_stop == 0)
				&& strcasecmp(E.syntax->filetype, "Python") == 0) { /* Little extra for Python mode. */
				iter = 1; 
				for (i = E.cx-1; iter && i >= 0; i--) {
					if (row->chars[i] == ':') {
						no_of_chars_to_indent += E.tab_stop;
						iter = 0;
					} else if (!isspace(row->chars[i])) {
						iter = 0; /* non-SPC terminates. */
					}
				}	
			} else if (!E.is_soft_indent
			 		&& !strcasecmp(E.syntax->filetype, "Makefile")) {
				iter = 1; 
				for (i = 0; iter && i < E.cx; i++) {
					if (row->chars[i] == ':') { // target: dep
						no_of_chars_to_indent++;
						iter = 0;
					} 
				}
			}		

			/* # of new spaces + the end of row. */
			buf = malloc(no_of_chars_to_indent + row->size - E.cx + 1);
			if (no_of_chars_to_indent > 0) {
				memset(buf, E.is_soft_indent ? ' ' : '\t', no_of_chars_to_indent);
			}
			memcpy(&buf[no_of_chars_to_indent], &row->chars[E.cx], row->size - E.cx);
			buf[no_of_chars_to_indent + row->size - E.cx] = '\0';
			//} 
		} // is_auto_indent

		// Last section: common to both auto-indent & not. 		
		if (no_of_chars_to_indent) {
			editor_insert_row(E.cy + 1, buf, strlen(buf));
			free(buf);
		} else {
			editor_insert_row(E.cy + 1, &row->chars[E.cx], row->size - E.cx); 
		}

		// Update the split upper row.

		row = &E.row[E.cy]; /* Reassign, because editor_insert_row() calls realloc(). */
		row->size = E.cx; 
		row->chars[row->size] = '\0'; 

		editor_update_row(row); 
	}
	
	E.cy++; 
	E.cx = no_of_chars_to_indent; // was: = 0 
}

void
editor_del_char() {
	erow *row;

	if (E.cy == E.numrows) {
		if (E.cy > 0)
			editor_move_cursor(ARROW_LEFT);
		return; 
	}

	if (E.cx == 0 && E.cy == 0) 
		return;

	row = &E.row[E.cy];
	if (E.cx > 0) {
		E.cx -= editor_row_del_char(row, E.cx - 1);
	} else {
		E.cx = E.row[E.cy - 1].size; 
		editor_row_append_string(&E.row[E.cy - 1], row->chars, row->size); 
		editor_del_row(E.cy);
		E.cy--; 
	}
}

/*** file I/O ***/

char *
editor_rows_to_string(int *buflen) {
	int totlen = 0; 
	int j; 
	char *buf; 
	char *p; 

	for (j = 0; j < E.numrows; j++)
		totlen += E.row[j].size + 1; 

	*buflen = totlen; 

	buf = malloc(totlen); 
	p = buf; 

	for (j = 0; j < E.numrows; j++) {
		memcpy(p, E.row[j].chars, E.row[j].size);
		p += E.row[j].size; 
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

void
editor_open(char *filename) {
 	char *line = NULL;
  	size_t linecap = 0;
  	ssize_t linelen;
  	struct stat stat_buffer; 

  	free(E.filename); 
  	free(E.absolute_filename); 
  	free(E.basename);

  	E.filename = strdup(filename); 

  	editor_select_syntax_highlight(NULL); 
 
  	// TODO new file (not necessarily need be writeable)
  	if (stat(filename, &stat_buffer) == -1) {
  		if (errno == ENOENT) {
  			E.is_new_file = 1; 
  			E.dirty = 0; 
  			return; 
  		} else {
  			die("stat");
  		}
  	}

 	FILE *fp = fopen(filename, "r");
  	if (!fp) {
  		die("fopen");
 	}

	E.absolute_filename = realpath(filename, NULL); 
	E.basename = editor_basename(filename);

  	while ((linelen = getline(&line, &linecap, fp)) != -1) {
    	if (linelen > 0 && (line[linelen - 1] == '\n' 
    					|| line[linelen - 1] == '\r'))
      		linelen--;

      	editor_insert_row(E.numrows, line, linelen);
	}
	free(line);
	fclose(fp);
	E.dirty = 0; 
}

void
editor_save() {
	int len; 
	char *buf; 
	int fd; 

	if (E.filename == NULL) {
		E.filename = editor_prompt("Save as: %s", NULL);
		if (E.filename == NULL) {
			editor_set_status_message("Save aborted");
			return; 
		}
	}

	E.absolute_filename = realpath(E.filename, NULL); 
	E.basename = editor_basename(E.filename);

	buf = editor_rows_to_string(&len);
	fd = open(E.filename, O_RDWR | O_CREAT, 0644);
	if (fd != -1) {
		if (ftruncate(fd, len) != -1) {
			if (write(fd, buf, len) == len) {
        		close(fd);
        		free(buf);
        		E.dirty = 0;
        		E.is_new_file = 0;  

        		editor_set_status_message("%d bytes written to %s", len, 
        			E.absolute_filename ? E.absolute_filename : E.filename);
				return;
			}
			editor_select_syntax_highlight(NULL); 
		}
		close(fd);
	}
	free(buf);
	editor_set_status_message("Can't save: I/O error: %s", strerror(errno));
}




/*** M-x command ***/

/**
  Return:
  0 = no argument gotten
  1 = one argument gotten.
  -1 = argument conversion error. 
 */ 
int
editor_get_command_argument(struct command_str *c, int *ret_int, char **ret_string) {
	char *raw_str = NULL;
	char *original_raw_string = NULL; 
	int raw_int = 0;
	int rc; 

	if (c == NULL)
		return 0; 

	/* FIXME now assume existence of %s in c->prompt. */
	raw_str = editor_prompt(c->prompt, NULL);
	if (raw_str == NULL)
		return 0; /* Aborted. */

	original_raw_string = strdup(raw_str); 

	if (c->command_arg_type == COMMAND_ARG_TYPE_STRING) {
		*ret_string = raw_str; 
		return 1; // One argument 

	} else if (c->command_arg_type == COMMAND_ARG_TYPE_INT) {
		*ret_string = original_raw_string; /* For the error status message */

		// convert
		if (strlen(raw_str) > 8) {
			raw_str[8] = '\0';
		}

		rc = sscanf(raw_str, "%d", &raw_int); /* strtoimax(raw_str, NULL, 10); */

		if (rc == 0) { 
			return -1; 
		}

		*ret_int = raw_int;
		return 1; 
	}

	return 0; 
}


void
editor_command() {
	char *command = NULL;
	//char *argument = NULL; 
	unsigned int i = 0;
	int int_arg = 0;
	char *char_arg = NULL;
	int found = 0; 

	command = editor_prompt("Command: %s", NULL);
	if (command == NULL) {
		editor_set_status_message("M-x command aborted.");
		return; 
	}

	for (i = 0; i < COMMAND_ENTRIES; i++) {
		struct command_str *c = &COMMANDS[i];

		if (!strncmp(c->command_str, command, strlen(c->command_str))) {
			found = 1; 
			if (c->command_arg_type == COMMAND_ARG_TYPE_INT 
				|| c->command_arg_type == COMMAND_ARG_TYPE_STRING) {
				/* should do union. */

				// rc=1 is good: it's the number of successfully parsed arguments.
				int rc = editor_get_command_argument(c, &int_arg, &char_arg);
				if (rc == 0) { // Aborted
					editor_set_status_message("%s aborted.", c->command_str);
					free(char_arg);
					free(command);
					return; 
				} else if (rc == -1) {
					editor_set_status_message(c->error_status, char_arg);
					free(char_arg);
					free(command);
					return;
				}
			}

			// all arg types.
			switch (c->command_key) {
			case COMMAND_SET_MODE:
				if (editor_select_syntax_highlight(char_arg) == 0) { 
					editor_set_status_message(c->success, char_arg);
				} else {
					editor_set_status_message(c->error_status, char_arg);
				}
				break;
			case COMMAND_SET_TAB_STOP:
				if (int_arg >= 2) { 
					E.tab_stop = int_arg; 
					editor_set_status_message(c->success, int_arg);
				} else {
					editor_set_status_message(c->error_status, char_arg);
				}
				break;
			case COMMAND_SET_AUTO_INDENT: {
				int auto_indent_set = 0;
				if (!strcasecmp(char_arg, "on") 
					|| !strcasecmp(char_arg, "t") || !strcasecmp(char_arg, "true")) {
					E.is_auto_indent = 1;
					auto_indent_set = 1;
				} else if (!strcasecmp(char_arg, "off") 
					|| !!strcasecmp(char_arg, "f") || strcasecmp(char_arg, "false")) {
					E.is_auto_indent = 0;
					auto_indent_set = 1;
				}
				if (auto_indent_set)
					editor_set_status_message(c->success);
				else
					editor_set_status_message(c->error_status, char_arg);
				break;
			}
			case COMMAND_SET_HARD_TABS:
				E.is_soft_indent = 0;
				editor_set_status_message(c->success);
				break;
			case COMMAND_SET_SOFT_TABS:
				E.is_soft_indent = 1;
				editor_set_status_message(c->success);
				break;
			case COMMAND_SAVE_AS:
			case COMMAND_OPEN_FILE:
				editor_set_status_message("Not implemented yet.");
				break;				
			default:
				editor_set_status_message("Got command: '%s'", c->command_str);
				break;
			} 

			free(char_arg);
			free(command);
			return;
		} /* if !strncasecmp */ 
	} /* for */

	if (! found) {
		editor_set_status_message("Unknown command: '%s'");
	}

	free(command);
	return;
}

/*** find ***/

void
editor_find_callback(char *query, int key) {
	static int last_match = -1; 
	static int direction = 1; 
	static int saved_hl_line; 
	static char *saved_hl; 

	if (saved_hl) {
		memcpy(E.row[saved_hl_line].hl, saved_hl, E.row[saved_hl_line].rsize);
		free(saved_hl);
		saved_hl = NULL; 
	}

	int current; 
	erow *row; 
	char *match; 
	int i; 

	if (key == '\r' || key == '\x1b') {
		last_match = -1; 
		direction = 1; 
		return; 
	} else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
		direction = 1; 
	} else if (key == ARROW_DOWN || key == ARROW_UP) {
		direction = -1; 
	} else {
		last_match = -1;
		direction = 1; 
	}

	if (last_match == -1)
		direction = 1; 
	current = last_match; 

	for (i = 0; i < E.numrows; i++) {
		
		current += direction; 
		if (current == -1)
			current = E.numrows - 1; 
		else if (current == E.numrows)
			current = 0; 

		row = &E.row[current];
		match = strstr(row->render, query); 
		if (match) {
			last_match = current; 
			E.cy = current; 
			E.cx = editor_row_rx_to_cx(row, match - row->render); 
			E.rowoff = E.numrows; 

			saved_hl_line = current; 
			saved_hl = malloc(row->rsize);
			memcpy(saved_hl, row->hl, row->rsize);
			memset(&row->hl[match - row->render], HL_MATCH, strlen(query));

			break; 
		}
	}

}

void
editor_find() {
	int saved_cx = E.cx; 
	int saved_cy = E.cy; 
	int saved_coloff = E.coloff; 
	int saved_rowoff = E.rowoff;

	char *query = editor_prompt("Search: %s (Use ESC/Arrows/Enter)", editor_find_callback); 
	if (query) {
		free(query);
	} else {
		E.cx = saved_cx;
		E.cy = saved_cy; 
		E.coloff = saved_coloff; 
		E.rowoff = saved_rowoff; 
	}
}


/*** append buffer ***/

struct abuf {
	char *b;
	int len; 
};

#define ABUF_INIT { NULL, 0 }

void
ab_append(struct abuf *ab, const char *s, int len) {
	char *new = realloc(ab->b, ab->len + len);
	
	if (new == NULL)
		return;

	memcpy(&new[ab->len], s, len); /* ! */
	ab->b = new;
	ab->len += len; 
}

void
ab_free(struct abuf *ab) {
	free(ab->b);
}

/*** clipboard ***/

void
clipboard_clear() {
	free(C.row);
	C.row = NULL; 
	C.numrows = 0;
	C.is_full = 0; 
}

void
clipboard_add_line_to_clipboard() {
	if (E.cy < 0 || E.cy >= E.numrows)
		return; 

	if (C.is_full) {
		clipboard_clear();
	}

	erow *row = &E.row[E.cy];	

	// Append to the end.
	C.row = realloc(C.row, sizeof(clipboard_row) * (C.numrows + 1));
	
	C.row[C.numrows].row = malloc(row->size);
	memcpy(C.row[C.numrows].row, row->chars, row->size);
	C.row[C.numrows].size = row->size;
	C.row[C.numrows].orig_x = E.cx;
	C.row[C.numrows].orig_y = E.cy;
	C.row[C.numrows].is_eol = 1; 
	C.numrows++;		

	editor_del_row(E.cy);
}

void
clipboard_yank_lines() { 
	int j = 0; 
	for (j = 0; j < C.numrows; j++) {
		editor_insert_row(E.cy++, C.row[j].row, C.row[j].size);		
	}

	editor_set_status_message("Yank lines!");
}


/*** output ***/

/**
	The first if statement checks if the cursor is above 
	the visible window, and if so, scrolls up to where the cursor is. 
	The second if statement checks if the cursor is past the bottom 
	of the visible window, and contains slightly more complicated 
	arithmetic because E.rowoff refers to what’s at the top of 
	the screen, and we have to get E.screenrows involved to talk 
	about what’s at the bottom of the screen.
*/
void
editor_scroll() {
	E.rx = 0; 

	if (E.cy < E.numrows) 
		E.rx = editor_row_cx_to_rx(&E.row[E.cy], E.cx);

	if (E.cy < E.rowoff)
		E.rowoff = E.cy;

	if (E.cy >= E.rowoff + E.screenrows)
		E.rowoff = E.cy - E.screenrows + 1; 
	
	if (E.rx < E.coloff) 
    	E.coloff = E.rx;
  	
  	if (E.rx >= E.coloff + E.screencols) 
    	E.coloff = E.rx - E.screencols + 1;
}

void
editor_draw_rows(struct abuf *ab) {
	int y;
	int filerow; 

	for (y = 0; y < E.screenrows; y++) {
		filerow = y + E.rowoff; 

		if (filerow >= E.numrows) {
			if (!E.is_banner_shown && E.numrows == 0 && y == E.screenrows / 3) {
				int padding = 0;
	      		char welcome[80];
    	  		int welcomelen = snprintf(welcome, sizeof(welcome),
        			"Kilo editor -- version %s", KILO_VERSION);
      			if (welcomelen > E.screencols) 
      				welcomelen = E.screencols;
      		
	      		padding = (E.screencols - welcomelen) / 2;
	      		if (padding) {
	        		ab_append(ab, "~", 1);
	        		padding--;
	      		}
	      		while (padding--) 
	      			ab_append(ab, " ", 1);

	      		ab_append(ab, welcome, welcomelen);

	      		// Banner.

	      	} else { // / 3
				ab_append(ab, "~", 1);
			}
		} else {
			char *c; 
			unsigned char *hl; 
			int j; 
			int current_colour = -1; 
			int len = E.row[filerow].rsize - E.coloff;
			if (len < 0)
				len = 0; 

      		if (len > E.screencols) 
      			len = E.screencols;

      		c  = &E.row[filerow].render[E.coloff];
      		
      		hl = &E.row[filerow].hl[E.coloff];

      		for (j = 0; j < len; j++) {
      			if (iscntrl(c[j])) {
      				char sym = (c[j] <= 26) ? '@' + c[j] : '?';
      				ab_append(ab, "\x1b[7m", 4);
      				ab_append(ab, &sym, 1);
      				ab_append(ab, "\x1b[m", 3);
      				if (current_colour != -1) {
      					char buf[16];
      					int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_colour); 
      					ab_append(ab, buf, clen);
      				}

      			} else if (hl[j] == HL_NORMAL) {
      				if (current_colour != -1) {
      					ab_append(ab, "\x1b[39m", 5); /* Text colours 30-37 (0=blak, 1=ref,..., 7=white. 9=reset*/
      					current_colour = -1; 
      				}
      				ab_append(ab, &c[j], 1);
      				
      			} else {
      				int colour = editor_syntax_to_colour(hl[j]);
      				if (colour != current_colour) {
      					char buf[16];
      					int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", colour);
      					ab_append(ab , buf, clen);
      					current_colour = colour; 
      				}

      				ab_append(ab, &c[j], 1);
      			}
      		}

      		ab_append(ab, "\x1b[39m", 5); /* Final reset. */
		}

		ab_append(ab, "\x1b[K", 3); /* K = erase line */
     	ab_append(ab, "\r\n", 2);
    }
}

#define ESC_PREFIX "\x1b["
#define ESC_PREFIX_LEN 2
#define APPEND_ESC_PREFIX(ab) (ab_append(ab, ESC_PREFIX, ESC_PREFIX_LEN))

enum esc_codes {
	ESC_BOLD = 1, 
	ESC_DIM = 2, 
	ESC_UNDERLINED = 4,
	ESC_BLINK = 5, 
	ESC_REVERSE = 7,
	ESC_HIDDEN = 8, 
	ESC_RESET_ALL = 0,
	ESC_RESET_BOLD = 21,
	ESC_RESET_DIM = 22,
	ESC_RESET_UNDERLINED = 24,
	ESC_RESET_BLINK = 25,
	ESC_RESET_REVERSE = 27,
	ESC_RESET_HIDDEN = 28
};

void
esc_invert(struct abuf *ab) {
	APPEND_ESC_PREFIX(ab);
	ab_append(ab, "7m", 2);
}

void
esc_bold(struct abuf *ab) {
	APPEND_ESC_PREFIX(ab);
	ab_append(ab, "1m", 2);
}

void
esc_reset_all(struct abuf *ab) {
	APPEND_ESC_PREFIX(ab);
	ab_append(ab, "0m", 2);
}


void
editor_draw_status_bar(struct abuf *ab) {
	int len = 0;
	int rlen = 0;
	char status[80], rstatus[80];

	//ab_append(ab, "\x1b[7m", 4); 
	esc_invert(ab);
	len = snprintf(status, sizeof(status), "-- %.16s %s - %d lines %s", 
		E.basename ? E.basename : "[No name]", E.is_new_file ? "(New file)" : "", E.numrows, 
		E.dirty ? "(modified)" : ""); 
	rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d", 
		E.syntax != NULL ? E.syntax->filetype : "no ft", E.cy + 1, E.numrows);

	if (len > E.screencols)
		len = E.screencols; 

	ab_append(ab, status, len); 

	while (len < E.screencols) {
		if (E.screencols - len == rlen) {
			ab_append(ab, rstatus, rlen);
			break; 
		} else {
			ab_append(ab, " ", 1);
			len++;
		}
	}

	esc_reset_all(ab);
	//ab_append(ab, "\x1b[m", 3); 
	ab_append(ab, "\r\n", 2); 
}

void
editor_draw_message_bar(struct abuf *ab) {
	int msglen; 
	ab_append(ab, "\x1b[K", 3); 
	msglen = strlen(E.statusmsg); 
	if (msglen > E.screencols)
		msglen = E.screencols; 
	if (msglen && time(NULL) - E.statusmsg_time < 5) 
		ab_append(ab, E.statusmsg, msglen);

}

/**
	The first byte is \x1b, which is the escape character, 
	or 27 in decimal. (Try and remember \x1b, we will be using it a lot.) 
	The other three bytes are [2J.

	We are writing an escape sequence to the terminal. 
	Escape sequences always start with an escape character (27) 
	followed by a [ character. 

	Escape sequences instruct the terminal to do various text 
	formatting tasks, such as coloring text, moving the cursor around, 
	and clearing parts of the screen.

	We are using the J command (Erase In Display) to clear the screen. 
	Escape sequence commands take arguments, which come before the command. 
	In this case the argument is 2, which says to clear the entire screen. 
	<esc>[1J would clear the screen up to where the cursor is, 
	and <esc>[0J would clear the screen from the cursor up to the end of the screen. 
	Also, 0 is the default argument for J, so just <esc>[J by itself would also clear 
	the screen from the cursor to the end.
*/

void 
editor_refresh_screen() {
	char buf[32];
	struct abuf ab = ABUF_INIT;

	editor_scroll();

  	ab_append(&ab, "\x1b[?25l", 6); /* cursor off (l = reset mode) */ 
	ab_append(&ab, "\x1b[H", 3);

	editor_draw_rows(&ab);
	editor_draw_status_bar(&ab);
	editor_draw_message_bar(&ab);

  	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", 
  		(E.cy - E.rowoff) + 1, (E.rx + E.coloff) + 1);
  	ab_append(&ab, buf, strlen(buf));
 	ab_append(&ab, "\x1b[?25h", 6); /* cursor on (h = set mode) */

	write(STDOUT_FILENO, ab.b, ab.len);
	ab_free(&ab);
}

void
editor_set_status_message(const char *fmt, ...) {
	va_list ap; 
	va_start(ap, fmt);
	vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
	va_end(ap);
	E.statusmsg_time = time(NULL);
}

/*** input ***/

char *
editor_prompt(char *prompt, void (*callback) (char *, int)) {
	size_t bufsize = 128; 
	char *buf = malloc(bufsize); 
	size_t buflen = 0; 
	int c; 

	buf[0] = '\0';

	while (1) {
		editor_set_status_message(prompt, buf); 
		editor_refresh_screen();

		c = editor_read_key();
		if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
			if (buflen != 0) 
				buf[--buflen] = '\0';
		} else if (c == '\x1b') {
			editor_set_status_message("");
			
			if (callback)
				callback(buf, c); 

			free(buf);
			return NULL; 
		} else if (c == '\r') {
			if (buflen != 0) {
				editor_set_status_message("");

				if (callback)
					callback(buf, c); 

				return buf; 
			}
		} else if (!iscntrl(c) && c < 128) {
			if (buflen == bufsize - 1) {
				bufsize *= 2; 
				buf = realloc(buf, bufsize); 
			}
			buf[buflen++] = c; 
			buf[buflen] = '\0';
		}

		if (callback)
			callback(buf, c); 
	}
}

void 
editor_move_cursor(int key) {
	int rowlen;
	erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

  	switch (key) {
    case ARROW_LEFT:
    	if (E.cx != 0) {
      		E.cx--;
    	} else if (E.cy > 0) {
        	E.cy--;
       		E.cx = E.row[E.cy].size;
      	}
      	break;
    case ARROW_RIGHT:
    	//if (E.cx != E.screencols - 1)
    	if (row && E.cx < row->size) {
      		E.cx++;
    	} else if (row && E.cx == row->size) {
        	E.cy++;
        	E.cx = 0;
      	}
      	break;
    case ARROW_UP:
    	if (E.cy != 0)
      		E.cy--;
      	break;
    case ARROW_DOWN:
    	if (E.cy < E.numrows)
      		E.cy++;
      	break;
  	}

    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  	rowlen = row ? row->size : 0;
  	if (E.cx > rowlen) {
    	E.cx = rowlen;
  	}


/*  	if (E.debug)
  		editor_set_status_message("x.y=%d.%d / %d.%d auto=%d soft=%d", 
  			E.cy, E.cx, E.numrows, rowlen, E.is_auto_indent, E.is_soft_indent);
  			*/
}

int 
editor_normalize_key(int c) {
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

	return c; 
}

void 
editor_process_keypress() {
	static int quit_times = KILO_QUIT_TIMES; 
	static int previous_key = -1; 

	int c = editor_normalize_key(editor_read_key());

	/* Clipboard full after the first non-KILL_LINE_KEY. */
	if (previous_key == KILL_LINE_KEY && c != KILL_LINE_KEY)
		C.is_full = 1; 

	previous_key = c; 

	E.is_banner_shown = 1; // After the first keypress, yes. 

	switch (c) {
		case '\r':
			editor_insert_newline(); 
			break;
		case QUIT_KEY:
			if (E.dirty && quit_times > 0) {
				editor_set_status_message("WARNING!!! File has unsaved changes. "
					"Press Ctrl-Q %d more times to quit.", quit_times);
				quit_times--;
				return; 
			}

			/* Clear the screen at the end. */
			write(STDOUT_FILENO, "\x1b[2J", 4);
      		write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
		case SAVE_KEY:
			editor_save();
			break; 
		case HOME_KEY:
			E.cx = 0;
			break;
		case END_KEY:
			if (E.cy < E.numrows)
				E.cx = E.row[E.cy].size; 
			break;
		case FIND_KEY:
			editor_find();
			break; 
		case BACKSPACE:
		case CTRL_KEY('h'):
		case DEL_KEY:
			if (c == DEL_KEY) 
				editor_move_cursor(ARROW_RIGHT);
			editor_del_char();
			break; 
		case PAGE_DOWN:
		case PAGE_UP: { 
			int times = 0;
			if (c == PAGE_UP) {
				E.cy = E.rowoff;
				times = E.screenrows; 
			} else if (c == PAGE_DOWN) {
				E.cy = E.rowoff + E.screenrows - 1;

				if (E.cy <= E.numrows) {
					times = E.screenrows;
				} else {
					E.cy = E.numrows; 
					times = E.numrows - E.rowoff; 
				}
			}
			while (times--)
				editor_move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
			break;
		}
		case ARROW_UP:
		case ARROW_LEFT:
		case ARROW_RIGHT:
		case ARROW_DOWN:
			editor_move_cursor(c);
			break;
		case CTRL_KEY('l'):
    	case '\x1b':
      		break;
      	case KILL_LINE_KEY:
      		clipboard_add_line_to_clipboard();
      		break;
      	case YANK_KEY:
      		clipboard_yank_lines(); 
      		break;

      	case CLEAR_MODIFICATION_FLAG_COMMAND:
      		if (E.dirty) {
      			editor_set_status_message("Modification flag cleared.");
      			E.dirty = 0;
      		}
      		break;

      	case COMMAND_KEY:
      		editor_command();
      		break; 

		default:
			editor_insert_char(c);
			break; 
	}

	quit_times = KILO_QUIT_TIMES; 
}

/*** init ***/

void
init_editor() {
	/* editor config */
	E.cx = E.cy = E.rx = E.numrows = E.rowoff = E.coloff = E.dirty = 0;
	E.row = NULL; 
	E.filename = NULL; 
	E.absolute_filename = NULL; 
	E.basename = NULL; 
	E.statusmsg[0] = '\0';
	E.statusmsg_time = 0; 
	E.syntax = NULL; 
	E.is_new_file = 0;
	E.is_banner_shown = 0; 
	E.tab_stop = DEFAULT_KILO_TAB_STOP;

	/* TODO */
	E.is_soft_indent = 0;
	E.is_auto_indent = 0;

	E.debug = 0; 

	/* clipboard */
	C.row = NULL; 
	C.numrows = 0; 
	C.is_full = 0; // !Ctrl-K after Ctrl-K => clipboard contents filled. New Ctrl-K & is_full => clipbord_clear().

	if (get_window_size(&E.screenrows, &E.screencols) == -1)
		die("get_window_size");

	E.screenrows -= 2; /* Room for the status bar & status messages. */
}

int 
main(int argc, char **argv) {
	enable_raw_mode();
	init_editor();

	if (argc >= 2) {
		if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0) {
			E.debug = 1; 
			if (argc >= 3)
				editor_open(argv[2]);
		} else {
			editor_open(argv[1]);
		}
	}

	editor_set_status_message("Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find | Ctrl-K copy line | Ctrl-Y paste");

	while (1) {
		editor_refresh_screen();
		editor_process_keypress();
	}

	return 0;
}
