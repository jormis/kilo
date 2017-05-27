/*

kilo.c -- lightweight editor

Based on the kilo project (https://github.com/antirez/kilo):

Copyright (c) 2016, Salvatore Sanfilippo <antirez at gmail dot com>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
	2017-05-27
	Latest:
        - M-x open-file
        - fixed open files bug where no files were opened in some cases.
        - slightly better editor_del_char() but undo for del does not work.
        - delete-buffer
        - create-buffer, next-buffer, previous-buffer works (need open-file, every E->dirty check), test delete-buffer.
        - M-x goto-line
	- When aborted, the find command stops at the position.
       	- fixed cursor left movement bug when past screen length.
        - --debug 4 debugs cursor & screen position 
        - Elm mode (incomplete but we'll get there)
	- Ruby mode
	- undo works (but not 100%) on Ctrl-K/Ctrl-Y
	- help (-h, --help)
	- Basically, limit input to ASCII only in command_insert_character().

        TODO BUG: backspace at the end of a line that's longer than screencols.
        TODO M-x goto-beginning, goto-end (of file) [optional]
	TODO (0.4) Emacs style C-K or C-SPC & C/M-W
	TODO (0.5) Split kilo.c into multiple source files. 
	TODO (0.6) *Help* mode (BUFFER_TYPE_READONLY)
        TODO search-and-replace
	TODO *Command* or *Shell* buffer (think of REPL) 
	TODO (0.7) M-x compile (based on Mode & cwd contents): like Emacs (output)
	     [Compiling based on HL mode & working directory: make, mvn build, ant, lein]
	TODO ~/.kilorc/.kilo.conf (tab-width) (M-x set-tab-width)
	TODO M-x TAB command completion
	TODO M-x command buffer & context-sensitive parameter buffer.
	TODO (0.8) Store the last command argument context-sensitively
	TODO Proper command line options (-lgetopt)
	TODO (0.9) Unicode support (-ncurses)
	TODO (1.0) Forth interpreter from libforth ... (also: M-x forth-repl)
*/

#define KILO_VERSION "kilo -- a simple editor version 0.3.4"
#define DEFAULT_KILO_TAB_STOP 8
#define KILO_QUIT_TIMES 3
#define STATUS_MESSAGE_ABORTED "Aborted."

#define DEBUG_UNDOS (1<<0)
#define DEBUG_COMMANDS (1<<1)
#define DEBUG_CURSOR (1<<2) 

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
	QUIT_KEY, // 1010
	SAVE_KEY,
	FIND_KEY,
	CLEAR_MODIFICATION_FLAG_KEY, /* M-c */ // TODO -> *_KEY

	/* These are more like commands.*/
	MARK_KEY, /* Ctrl-Space */
	KILL_LINE_KEY, /* Ctrl-K (nano mode) */
	DEL_TO_THE_EOL_KEY, /* Ctrl-K (emacs mode)*/
	DEL_FROM_MARK_KEY, /* Ctrl-W */
	COPY_TO_CLIPBOARD_KEY, /* M-w */
	YANK_KEY, /* Ctrl-Y */
	COMMAND_KEY, /* M-x */
	COMMAND_UNDO_KEY, /* C-u TODO BETTER KEY NEEDED */
	COMMAND_INSERT_NEWLINE, /* 1022 Best undo for deleting newline (backspace in the beginning of row ...*/
        ABORT_KEY, /* NOT IMPLEMENTED */
        GOTO_LINE_KEY, /* Ctrl-G */
        NEXT_BUFFER_KEY, /* Esc-N? */
        PREVIOUS_BUFFER_KEY /* Esc-P */
};

/*
`erow.hl' is an array of unsigned char values, meaning integers in the range of 0 to 255. 
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

/* */
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

/* Global terminal information. Separated from editor_config which is tied to buffer. */
struct term_config {
        int screenrows; 
        int screencols;
        struct termios orig_termios;
};

struct term_config TERMINAL; 

/* Buffer */
struct editor_config {
	int cx, cy; 
	int rx; 
	int rowoff;
	int coloff; 
	//int screenrows; TODO reintroduce
	//int screencols; TODO reintroduce
	int numrows;
	erow *row; 
	int dirty; 
	char *filename; 
	char *absolute_filename; 
	char *basename; 
	char statusmsg[80];
	time_t statusmsg_time; 
	struct editor_syntax *syntax; 
	//struct termios orig_termios;
	int is_new_file; 
	int is_banner_shown; /* If shown once do not show again. */
	int is_soft_indent; 
	int is_auto_indent; 
	int tab_stop;  
	int debug; 
};

/* Points to current_buffer->E. */
struct editor_config *E;

/* row (E.row in phase 1; from E.cx' to E.cx'' in phase 2)
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

char *Makefile_HL_extensions[] = { "Makefile", "makefile", ".mk", ".mmk", NULL };
char *Makefile_HL_keywords[] = { NULL };

char *Erlang_HL_extensions[] = { ".erl", NULL };
char *Erlang_HL_keywords[] = {
        "after", "and", "andalso", "band", "begin", "bnot", "bor", 
        "bsl", "bsr", "bxor", "case", "catch", "cond", "div", "end",
        "fun", "if", "let", "not", "of", "or", "orelse", "receive",
        "rem", "try", "when", "xor", 
        "->", // can do? 
        "-module", "-export", "-import",
        NULL
};

char *JS_HL_extensions[] = { ".js", NULL };
char *JS_HL_keywords[] = {
        "abstract", "arguments", "await", 
        "boolean|", "break", "byte|",
        "case", "catch ", "char|", "class", "const", "continue",
        "debugger", "default", "delete", "do", "double|", 
        "else", "enum", "eval", "export", "extends", 
        "false", "final", "finally", "float|", "for", "function", 
        "goto",
        "if", "implements", "import", "in", "instanceof", "int|", "interface", 
        "let", "long|",
        "native", "new", "null", 
        "package", "private", "protected", "public",
        "return", 
        "short|", "static", "super", "switch", "synchronized", 
        "this", "throw", "throws", "transient", "true", "try", "typeof", 
        "var", "void|", "volatile",
        "while", "with", 
        "yield", 

        /* And even more: JavaScript Object, Properties, and Methods. */
        "Array|", "Date|", "hasOwnProperty", 
        "Infinity|", "isFinite", "isNaN", "isPrototypeOf",
        "length", "Math", "NaN|", "name", "Number|", "Object|",
        "prototype", "String|", "toString", "undefined", "valueOf",
        
        /* Java reserved words. */ 
        
        "getClass", "java", "JavaArray|", "javaClass", "JavaClass", 
        "JavaObject|", "JavaPackage",
        
        /* Other reserved words. HTML and Window objects and properties. */

        "alert", "all", "anchor", "anchors", "area", "assign", 
        "blur", "button", 
        "checkbox", "clearInterval", "clearTimeout", "clientInformation",
        "close", "closed", "confirm", "constructor", "crypto",
        "decodeURI", "decodeURIComponent", "defaultStatus", "document",
        "element", "elements", "embed", "embeds", "encodeURI", 
        "encodeURIComponent", "escape", "event", 
        "fileUpload", "focus", "form", "forms", "frame", "frames", "frameRate",
        "hidden", "history", 
        "image", "images", "innerHeight", "innerWidth", 
        "layer", "layers", "link", "location", 
        "mimeTypes", 
        "navigate", "navigator", 
        "offscreenBuffering", "open", "opener", "option", "outerHeight", 
        "outerWidth", 
        "packages", "pageXOffset", "pageYOffset", "parent", "parseFloat", 
        "parseInt", "password", "pkcs11", "plugin", "prompt", "propertyIsEnum",
        "radio", "reset", 
        "screenX", "screenY", "scroll", "secure", "select", "self", 
        "setInterval", "setTimeout", "status", "submit", 
        "taint", "text", "textarea", "top", "unescape", "untaint", 
        "window",

        /* HTML Event Handlers. */
        
        "onblur", "onclick", "onerror", "onfocus", "onkeydown", "onkeyup", 
        "onmouseover", "onmouseup", "onmousedown", "onload", "onsubmit",
                
        NULL
};

char *Shell_HL_extensions[] = { ".sh", NULL };
char *Shell_HL_keywords[] = {
	// compgen -k
	"if", "then", "else", "elif", "fi",
	"case", "esac", "for", "select", "while", "until",
	"do", "done", "in", "function", "time", "[[", "]]",
	NULL
};

char *Perl_HL_extensions[] = { ".pl", ".perl", NULL };
char *Perl_HL_keywords[] = {
	/* Perl functions. */
	"-A", "-B", "-b", "-C", "-c", "-d", "-e", "-f", "-g", "-k", "-l", "-M", "-O", "-o", 
	"-p", "-r" "-R", "-S", "-s", "-T", "-t", "-u", "-w", "-W", "-X", "-x", "-z",

	"abs", "accept", "alarm", "atan2", "AUTOLOAD", 
	"BEGIN", "bind", "binmode", "bless", "break", 
	"caller", "chdir", "CHECK", "chmod", "chomp", "chop", "chown", "chr", "chroot",
	"close", "closedir", "connect", "cos", "crypt", 
	"dbmclose", "defined", "delete", "DESTROY", "die", "dump", 
	"each", "END", "endgrent", "endhostent", "endnetent", "endprotoent", "endpwent",
	"endservent", "eof", "eval", "exec", "exists", "exit", 
	"fcntl", "fileno", "flock", "fork", "format", "formline", 
	"getc", "getgrent", "getgrgid", "getgrnam", "gethostbyaddr", "gethostbyname",
	"gethostent", "getlogin", "getnetbyaddr", "getnetbyname", "getnetent", 
	"getpeername", "getpgrp", "getppid", "getpriority", "getprotobyname", 
	"getprotobynumber", "getprotoent", "getpwent", "getpwnam", "getpwuid", 
	"getservbyport", "getservent", "getsockname", "getsockopt", "glob", "gmtime", 
	"goto", "grep", 
	"hex",
	"index",
	"INIT", "int", "ioctl", 
	"join",
	"keys", "kill",
	"last", "lc", "lcfirst", "length", "link", "listen", "local", "localtime", 
	"log", "lstat",  
	"map", "mkdir", "msgctl", "msgget", "msgrcv", "msgsnd", "my",
	"next", "not",
	"oct", "open", "opendir", "ord", "our", 
	"pack", "pipe", "pop", "pos", "print", "printf", "prototype", "push",
	"quotemeta", 
	"rand", "read", "readdir", "readline", "readlink", "readpipe", "recv",
	"redo", "ref", "rename", "require", "reset", "return", "reverse", "rewinddir", 
	"rindex", "rmdir", 
	"say", "scalar", "seek", "seekdir", "select", "semctl", "semget", "semop", 
	"send", "setgrent", "sethostent", "setnetent", "setpgrp", "setpriority", 
	"setprotoent", "setpwent", "setservent", "setsockopt", "shift", 
	"shmctl", "shmget", "shmread", "shmwrite", "shutdown", "sin", "sleep",
	"socket", "socketpair", "sort", "splice", "split", "sprintf", "sqrt", "srand",
	"stat", "state", "study", "substr", "symlink", "syscall", "sysopen", "sysread",
	"sysseek", "system", "syswrite", 
	"tell", "telldir", "tie", "tied", "time", "times", "truncate", 
	"uc", "ucfirst", "umask", "undef", "UNITCHECK", "unlink", "unpack", "unshift",
	"untie", "use", "utime",
	"values", "vec", "wait", "waitpid", "wantarray", "warn", "write",

	/* Perl syntax */

	"__DATA__", "__END__", "__FILE__", "__LINE__", "__PACKAGE__", 
	"and", "cmp", "continue", "CORE", "do", "else", "elsif", "eq", "exp", 
	"for", "foreach", "ge", "gt", "if", "le", "lock", "lt", 
	"m", "ne", "no", "or", "package", "q", "qq", "qr", "qw", "qx", 
	"s", "sub", "tr", "unless", "until", "while", "xor", "y",

	NULL
};

char *Ruby_HL_extensions[] = { ".rb", NULL };
char *Ruby_HL_keywords[] = {
        "__ENCODING__", "__LINE__", "__FILE__", "BEGIN", "END", 
        "alias", "and", "begin", "break", 
        "case", "class", 
        "def", "defined?", "do", 
        "else", "elsif", "end", "ensure", 
        "false", "for",
        "if", "in", 
        "module", 
        "next", "nil", "not",
        "or", 
        "redo", "rescue", "retry", "return", 
        "self", "super", 
        "then", "true", 
        "undef", "unless", "until", 
        "when", "while",
        "yield", 
        
   
    NULL
}; 

/* https://guide.elm-lang.org */
char *Elm_HL_extensions[] = { ".elm", NULL };
char *Elm_HL_keywords[] = {
        "if", "then", "else", "case", "of", "let", "in", "type", 
        /* Maybe not 'where' */
        "module", "where", "import", "exposing", "as", "port",
        "infix", "infixl", "infixr", 
        
        NULL
};  

char *PHP_HL_extensions[] = { ".php", NULL };
char *PHP_HL_keywords[] = {
        "__halt_compiler", 
        "abstract", "and", "array", "as", 
        "break", 
        "callable", "case", "catch", "class", "clone", "const", "continue",
        "declare", "default", "die", "do", 
        "echo", "else", "elseif", "empty", "enddeclare", "endfor", 
        "endforeach", "endif", "endswitch", "endwhile", "eval", "exit", 
        "extends", 
        "final", "finally", "for", "foreach", "function", 
        "global", "goto", 
        "if", "implements", "include", "include_once", "instanceof",
        "insteadof", "interface", "isset", 
        "list", 
        "namespace", "new", 
        "or",
        "print", "private", "protected", "public", 
        "require", "require_once", "return", 
        "static", "switch", 
        "throw", "trait", "try", 
        "unset", "use", 
        "var", 
        "while", 
        "xor",
        "yield",
        
        "__CLASS__", "__DIR__", "__FILE__", "__FUNCTION__", "__LINE__", 
        "__METHOD__", "__NAMESPACE__", "__TRAIT__",
        
        NULL
};
 
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
	{
		"Python",
		Python_HL_extensions,
		Python_HL_keywords,
		"#",
		"'''", "'''",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
		4,
		1
	},
        {
                "Erlang",
                Erlang_HL_extensions, 
                Erlang_HL_keywords,
                "%",
                "", "",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1 // TODO make bitfield?
        },
        {
                "JavaScript",
                JS_HL_extensions,
                JS_HL_keywords,
                "//",
                "/*", "*/",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1
        },
        {
        	"Shell",
        	Shell_HL_extensions,
        	Shell_HL_keywords,
        	"#",
        	"", "",
        	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
        	8,
        	1
        },
        {
        	"Perl", 
        	Perl_HL_extensions,
        	Perl_HL_keywords,
        	"#", 
        	"", "", /* ^= ^= comments missing */
        	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
        	4,
        	1
        },
        {
                "Ruby",
                Ruby_HL_extensions, 
                Ruby_HL_keywords,
                "#",
                "", "",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1
        },
        {
                "PHP",
                PHP_HL_extensions, 
                PHP_HL_keywords,
                "//", // TODO also '#'
                "/*", "*/",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1
        },
        {
                "Elm",
                Elm_HL_extensions,
                Elm_HL_keywords,
                "--",
                "", "",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1                
        }
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

/**
* Everything is a command. 
*/
enum command_key {
	COMMAND_NO_CMD = 0,
	COMMAND_SET_MODE = 1,
	COMMAND_SET_TAB_STOP,
	COMMAND_SET_SOFT_TABS,
	COMMAND_SET_HARD_TABS,
	COMMAND_SET_AUTO_INDENT,
	COMMAND_SAVE_BUFFER, 
	COMMAND_SAVE_BUFFER_AS,  
	COMMAND_OPEN_FILE, // TODO
	COMMAND_MOVE_CURSOR_UP,
	COMMAND_MOVE_CURSOR_DOWN, // cmd_key = 10
	COMMAND_MOVE_CURSOR_LEFT,
	COMMAND_MOVE_CURSOR_RIGHT,
	COMMAND_MOVE_TO_START_OF_LINE,
	COMMAND_MOVE_TO_END_OF_LINE,
	COMMAND_KILL_LINE,
	COMMAND_YANK_CLIPBOARD,
	COMMAND_UNDO,
	COMMAND_INSERT_CHAR, /* for undo */
	COMMAND_DELETE_CHAR, /* for undo */
	COMMAND_DELETE_INDENT_AND_NEWLINE, /* for undo only */
        COMMAND_GOTO_LINE, /* TODO */
        COMMAND_CREATE_BUFFER,
        COMMAND_SWITCH_BUFFER,
        COMMAND_DELETE_BUFFER,
        COMMAND_NEXT_BUFFER, /* Esc-N */
        COMMAND_PREVIOUS_BUFFER /* Esc-P */
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
                COMMAND_CREATE_BUFFER,
                "create-buffer",
                COMMAND_ARG_TYPE_NONE,
                NULL,
                "Buffer created.",
                NULL
        },
        {
                COMMAND_SWITCH_BUFFER,
                "switch-buffer",
                COMMAND_ARG_TYPE_STRING,
                "Buffer: %s",
                "Buffer created.",
                "Failed to switch to buffer: '%s'"
        },
        {
                /* DELETE CURRENT BUFFER */
                COMMAND_DELETE_BUFFER,
                "delete-buffer",
                COMMAND_ARG_TYPE_NONE,
                NULL,
                "The current buffer deleted.",
                "Unsaved changes. Save buffer or clear modification bit."
        },
        {
                COMMAND_NEXT_BUFFER,
                "next-buffer",
                COMMAND_ARG_TYPE_NONE,
                NULL,
                "Switched to next buffer.",
                NULL
        },
        {
                COMMAND_PREVIOUS_BUFFER,
                "previous-buffer",
                COMMAND_ARG_TYPE_NONE,
                NULL,
                "Switched to previous buffer.",
                NULL
        },      
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
		"Auto indent is %s",
		"Invalid auto indent mode: '%s'"
	},
	{
		COMMAND_SAVE_BUFFER,
		"save-buffer",
		COMMAND_ARG_TYPE_STRING, /* Prompt for new file only. */
		"Save as: %s",
		"%d bytes written successfully to %s", // Special handling: %d and %s
		"Can't save, I/O error: %s" // %s = error
	},
	{
                // Open file for reading, save current buffer to it.
		COMMAND_SAVE_BUFFER_AS,
		"save-buffer-as",
		COMMAND_ARG_TYPE_STRING,
		"Save buffer as: %s",
		"%d bytes written successfully to %s", // Special handling: %d and %s
		"Can't save, I/O error: %s" // %s = error
	},
        {       
                COMMAND_OPEN_FILE,
                "open-file",
                COMMAND_ARG_TYPE_STRING,
                "Open file: %s",
                "%s opened.",
                "Cannot open file: %s"
        },       
	{
		COMMAND_MOVE_CURSOR_UP,
		"move-cursor-up",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_MOVE_CURSOR_DOWN,
		"move-cursor-down",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_MOVE_CURSOR_LEFT,
		"move-cursor-left",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_MOVE_CURSOR_RIGHT,
		"move-cursor-right",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_KILL_LINE,
		"kill-line",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_YANK_CLIPBOARD,
		"yank",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_UNDO,
		"undo",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"",
		NULL
	},
	{
		COMMAND_INSERT_CHAR,
		"insert-char",
		COMMAND_ARG_TYPE_STRING,
		"Character: %s",
		"Inserted '%c'",
		"Failed to insert a char:"
	},
	{
		COMMAND_DELETE_CHAR,
		"delete-char",
		COMMAND_ARG_TYPE_NONE,
		NULL,
		"Deleted",
		"No characters to delete!"
	},
        { /* TODO */
                COMMAND_GOTO_LINE,
                "goto-line",
                COMMAND_ARG_TYPE_INT,
                "Goto line: %s",
                "Jumped",
                "Failed to goto line: "
        }       
};

#define COMMAND_ENTRIES (sizeof(COMMANDS) / sizeof(COMMANDS[0]))

/*** undo ***/

struct undo_str {
	int command_key; // orginal command
	int undo_command_key; // undo command (if need to be run)
	int cx; 
	int cy;
	int orig_value; // set-tabs, auto-indent
	struct clipboard *clipboard; // kill, yank
	struct undo_str *next; // Because of stack.
};

//struct undo_str *undo_stack;  

/** buffers **/

/* 	Multiple buffers. Editor config and undo stack are buffer specific; 
   	clipboard and editor syntax aren't. 

	Buffer types: file (like now), readonly (*Help), command (M-x compile, M-x repl?)

   	TODO: macros to handle buffer->E....blaa. ?
	TODO commands: open-file (C-x C-f), create-buffer, switch-buffer, 
	close-buffer(-and-save?), kill-buffer (?).
 */

/*** prototypes ***/

void editor_set_status_message(const char *fmt, ...);
void editor_refresh_screen();
char *editor_prompt(char *prompt, void (*callback) (char *, int));
void editor_move_cursor(int key);
struct command_str *command_get_by_key(int command_key);
void command_move_cursor(int command_key);
void undo_push_simple(int command_key, int undo_command_key);
void undo_push_one_int_arg(int command_key, int undo_command_key, int orig_value);
void undo_clipboard_kill_lines(struct clipboard *copy); 
struct clipboard *clone_clipboard();
struct undo_str *init_undo_stack();
void die(const char *s);
void init_config(struct editor_config *cfg); 

/** buffer */
enum buffer_type {
        BUFFER_TYPE_FILE = 0,
        BUFFER_TYPE_READONLY, 
        BUFFER_TYPE_COMMAND
};

struct buffer_str {
	int type; /* file, command, read (*Help*, for example) */
	struct editor_config E;
	struct undo_str *undo_stack;
        struct buffer_str *prev; 
	struct buffer_str *next; 
}; 


#define EDITOR_CONFIG &current_buffer->E

struct buffer_str *buffer; 
struct buffer_str *current_buffer;

/* create_buffer() - Becomes the current buffer. Cannot be undone. */
struct buffer_str *
create_buffer(int type, int verbose) {
        struct editor_config *old_E = E; 
        struct command_str *c = command_get_by_key(COMMAND_CREATE_BUFFER);
	struct buffer_str *p = malloc(sizeof(struct buffer_str));
	if (p == NULL)
		die("new buffer");

        p->type = type;
        p->undo_stack = init_undo_stack();
        p->prev = NULL; 
        p->next = NULL; 
        init_config(&p->E);

        if (current_buffer != NULL) {
                while (current_buffer->next != NULL)
                        current_buffer = current_buffer->next; 
                                
                current_buffer->next = p;
                p->prev = current_buffer;  
                current_buffer = p; 
                if (verbose)
                        editor_set_status_message(c->success);
                
        } else {
                // This is a call to init_buffer(); 
                current_buffer = p; 
        }
        
        E = &current_buffer->E; 
        if (old_E != NULL)
                E->debug = old_E->debug;
        return current_buffer;        
}

void
command_next_buffer() {
        if (current_buffer->next != NULL) {
                struct command_str *c = command_get_by_key(COMMAND_NEXT_BUFFER);
                current_buffer = current_buffer->next;
                E = &current_buffer->E;
                undo_push_simple(COMMAND_NEXT_BUFFER, COMMAND_PREVIOUS_BUFFER);
                if (c != NULL)
                        editor_set_status_message(c->success);
        }
}

void
command_previous_buffer() {
        if (current_buffer->prev != NULL) {
                struct command_str *c = command_get_by_key(COMMAND_PREVIOUS_BUFFER);
                current_buffer = current_buffer->prev;
                E = &current_buffer->E;
                undo_push_simple(COMMAND_PREVIOUS_BUFFER, COMMAND_NEXT_BUFFER);
                if (c != NULL)
                        editor_set_status_message(c->success);
        }
}

/* Cannot be undone.*/
void
delete_current_buffer() {
        struct buffer_str *new_current = NULL;
        struct undo_str *u = NULL;
        struct undo_str *n = NULL; 
        struct command_str *c = command_get_by_key(COMMAND_DELETE_BUFFER);
        
        if (current_buffer == NULL)
                return;
                
        if (current_buffer->prev == NULL && current_buffer->next == NULL) {
                editor_set_status_message("Only buffer -- cannot be deleted.");
                return; // Cannot delete the only buffer.    
        }

        // Not so fast: are there any unsaved changes? 
        // Ok, this is a quick and dirty solution for this buffer only.
        if (E->dirty) {
                editor_set_status_message(c->error_status);
                return; 
        }

        // Free undo_stack.
        u = current_buffer->undo_stack;
        while (u != NULL) {
                if (u->clipboard != NULL) { /* clipboard */
                        if (u->clipboard->row != NULL) {
                                int i = 0;
                                for (i = 0; i < u->clipboard->numrows; i++) {
                                        struct clipboard_row *r = &u->clipboard->row[i];
                                        if (r->row != NULL)
                                                free(r->row);
                                }
                                
                                free(u->clipboard->row); // clipboard_row * (a realloc'd chunk)
                        }
                        free(u->clipboard);
                } 
                
                n = u->next;
                free(u);
                u = n;
        }

        if (current_buffer->prev != NULL) {
                current_buffer->prev->next = current_buffer->next;
                new_current = current_buffer->prev; /* Prev has priority. */
        } 
        
        if (current_buffer->next != NULL) {
                current_buffer->next->prev = current_buffer->prev;
                if (new_current == NULL) /* If no prev the set current to next. */
                        new_current = current_buffer->next;
        }

        if (new_current == NULL)
                die("new current");
                
        free(current_buffer);        
        current_buffer = new_current;
        E = &current_buffer->E; 
        editor_set_status_message(c->success);
}                

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
editor_read_key() {
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

	if (E->syntax == NULL)
		return; 

	in_comment = (row->idx > 0 && E->row[row->idx - 1].hl_open_comment); 
	keywords = E->syntax->keywords; 

	scs = E->syntax->singleline_comment_start;
	mcs = E->syntax->multiline_comment_start;
	mce = E->syntax->multiline_comment_end;

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

		if (E->syntax->flags & HL_HIGHLIGHT_STRINGS) {
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

		if (E->syntax->flags & HL_HIGHLIGHT_NUMBERS) {
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

		if (isspace(c) && i > 0 && prev_char == '.' && prev_hl == HL_NUMBER)
			row->hl[i - 1] = HL_NORMAL; /* Denormalize sentence ending colon. */
		prev_char = c; 
			
		i++;
	}

	changed = (row->hl_open_comment != in_comment); 
	row->hl_open_comment = in_comment; 
	if (changed && row->idx + 1 < E->numrows) {
		editor_update_syntax(&E->row[row->idx + 1]);
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

	E->syntax = syntax; 
	E->tab_stop = E->syntax->tab_stop; // TODO refactor E->tab_stop away
	E->is_soft_indent = ! (E->syntax->flags & HARD_TABS); 
	E->is_auto_indent = E->syntax->is_auto_indent;

	for (filerow = 0; filerow < E->numrows; filerow++) {
		editor_update_syntax(&E->row[filerow]); 
	}
}

int
editor_select_syntax_highlight(char *mode) {
	unsigned int j; 
	int mode_found = 0; 
	char *p = NULL ;
	E->syntax = NULL;

	if (E->filename == NULL && mode == NULL)
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
				p = strstr(E->filename, s->filematch[i]); 
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
			rx += (E->tab_stop - 1) - (rx % E->tab_stop);
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
			cur_rx = (E->tab_stop - 1) - (cur_rx % E->tab_stop);

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
	row->render = malloc(row->size + tabs * (E->tab_stop - 1) + 1);

	for (j = 0; j < row->size; j++) {
		if (row->chars[j] == '\t') {
			row->render[idx++] = ' ';
			while (idx % E->tab_stop != 0) 
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
	if (at < 0 || at > E->numrows > 0)
		return; 

	E->row = realloc(E->row, sizeof(erow) * (E->numrows + 1));
	memmove(&E->row[at + 1], &E->row[at], sizeof(erow) * (E->numrows - at));
	for (j = at + 1; j <= E->numrows; j++)
		E->row[j].idx++; 

	E->row[at].idx = at; 
  	E->row[at].size = len;
  	E->row[at].chars = malloc(len + 1);
  	memcpy(E->row[at].chars, s, len);
  	E->row[at].chars[len] = '\0';
  	E->row[at].rsize = 0;
  	E->row[at].render = NULL; 
  	E->row[at].hl = NULL;
  	E->row[at].hl_open_comment = 0; 

  	editor_update_row(&E->row[at]); 
  	
  	E->numrows++;
  	E->dirty++; 
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
	if (at < 0 || at > E->numrows)
		return;

	editor_free_row(&E->row[at]);
	memmove(&E->row[at], &E->row[at + 1], sizeof(erow) * (E->numrows - at - 1));
	for (j = at; j < E->numrows - 1; j++)
		E->row[j].idx--;

	E->numrows--;
	E->dirty++;
}

int
editor_row_insert_char(erow *row, int at, char c) {
	int insert_len = 0;
	int i = 0; 
	int no_of_spaces = 0;

	if (at < 0 || at > row->size) 
		at = row->size; 

	if (c == '\t' && E->is_soft_indent) {
		/* 
		 * Calculate the number of spaces until the next tab stop. 
		 * Add E.tab_stop number of spaces if we are at the stop.
		 */
		no_of_spaces = E->tab_stop - (at % E->tab_stop);
		if (no_of_spaces == 0)
			no_of_spaces = E->tab_stop;

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
	E->dirty++; 
	return insert_len;
}

void 
editor_row_append_string(erow *row, char *s, size_t len) {
	row->chars = realloc(row->chars, row->size + len + 1);
	memcpy(&row->chars[row->size], s, len);
	row->size += len; 
	row->chars[row->size] = '\0';
	editor_update_row(row);
	E->dirty++; 
}

/**
* at = E.cx - 1
*/
int
editor_row_del_char(erow *row, int at) {
	int len = 1; /* Default del len. */
	int i = 0; 
	int enough_spaces_to_the_left = 1; 

	if (at < 0 || at >= row->size)
		return 0;

	if (E->is_auto_indent) {
		if ((at+1) % E->tab_stop == 0) {
			/* There has to be at least E.tab_stop spaces to the left of 'at'.
				Note: start counting from TAB_STOP below & upwards. */
			for (i = at + 1 - E->tab_stop; enough_spaces_to_the_left && i >= 0 && i < at; i++) {
				if ((E->is_soft_indent && row->chars[i] != ' ')
					|| (!E->is_soft_indent && row->chars[i] != '\t')) {
					enough_spaces_to_the_left = 0; 
				}
			}

			if (enough_spaces_to_the_left)
				len = E->tab_stop;
		} else 
			enough_spaces_to_the_left = 0; 
	} 

	memmove(&row->chars[at + 1 - len], &row->chars[at + 1], row->size - at + 1); 
	row->size -= len;
	editor_update_row(row);
	E->dirty++; 

	return len; 
}

/*** editor operations ***/
void
editor_insert_char(int c) {
	if (E->cy == E->numrows) {
		editor_insert_row(E->numrows, "", 0); 
	}

	/* If soft_indent, we may insert more than one character. */
	E->cx += editor_row_insert_char(&E->row[E->cy], E->cx, c);  
}

/**
 * If auto_indent is on then we calculate the number of indents.
 * Here you can add language/mode specific indents. 
 */
int
calculate_indent(erow *row) {
	int iter = 0;
	int no_of_chars_to_indent = 0;
	int i = 0; 

	if (E->is_auto_indent) {
		iter = 1; 
		// Cutoff point is cursor == E.cx
		for (i = 0; iter && i < E->cx; i++) {
			if ((row->chars[i] == ' ' && E->is_soft_indent)	
				|| (row->chars[i] == '\t' && !E->is_soft_indent)) {
				no_of_chars_to_indent++;
			} else {
				iter = 0;
			}
		}

		if (E->is_soft_indent
			&& (no_of_chars_to_indent % E->tab_stop == 0)) {

			if (!strcasecmp(E->syntax->filetype, "Python")) { /* Little extra for Python mode. */
				iter = 1; 
				for (i = E->cx-1; iter && i >= 0; i--) {
					if (row->chars[i] == ':' || row->chars[i] == '\\') { // : or '\\'' 
						no_of_chars_to_indent += E->tab_stop;
						iter = 0;
					} else if (!isspace(row->chars[i])) {
						iter = 0; /* non-SPC terminates. */
					}
				}
			} else if (!strcasecmp(E->syntax->filetype, "Erlang")) {
				iter = 1; 
				for (i = E->cx-1; iter && i >= 1; i--) { // NB: i >= 1
					if (row->chars[i-1] == '-' || row->chars[i] == '>') { // ->
						no_of_chars_to_indent += E->tab_stop;
						iter = 0;
					} else if (!isspace(row->chars[i])) {
						iter = 0; /* non-SPC terminates. */
					}
				}				
			} else if (!strcasecmp(E->syntax->filetype, "Elm")) {
                                iter = 1;
                                for (i = E->cx-1; iter && i >= 0; i--) {
                                        if (row->chars[i] == '=') {
                                                no_of_chars_to_indent += E->tab_stop;
                                                iter = 0;
                                        } else if (!isspace(row->chars[i])) {
                                                iter = 0; 
                                        }
                                }                                
                        }
		} else if (!E->is_soft_indent
		 		&& !strcasecmp(E->syntax->filetype, "Makefile")) {
			iter = 1; 
			for (i = 0; iter && i < E->cx; i++) {
				if (row->chars[i] == ':') { // target: dep
					no_of_chars_to_indent++;
					iter = 0;
				} 
			}
		}
	}

	return no_of_chars_to_indent; 
}

int
editor_insert_newline() {
	int no_of_chars_to_indent = 0;
	char *buf;
	erow *row; 

	if (E->cx == 0) {
		editor_insert_row(E->cy, "", 0); 
		no_of_chars_to_indent = 0; 
	} else {
		row = &E->row[E->cy];

		no_of_chars_to_indent = calculate_indent(row);

		if (no_of_chars_to_indent > 0) {
			/* # of new spaces + the end of row. */
			buf = malloc(no_of_chars_to_indent + row->size - E->cx + 1);
			if (no_of_chars_to_indent > 0) {
				memset(buf, E->is_soft_indent ? ' ' : '\t', no_of_chars_to_indent);
			}
			memcpy(&buf[no_of_chars_to_indent], &row->chars[E->cx], row->size - E->cx);
			buf[no_of_chars_to_indent + row->size - E->cx] = '\0';
			editor_insert_row(E->cy + 1, buf, strlen(buf));
			free(buf);
		} else {
			editor_insert_row(E->cy + 1, &row->chars[E->cx], row->size - E->cx); 
		}

		// Update the split upper row.
		row = &E->row[E->cy]; /* Reassign, because editor_insert_row() calls realloc(). */
		row->size = E->cx; 
		row->chars[row->size] = '\0'; 

		editor_update_row(row); 
	}
	
	E->cy++; 
	E->cx = no_of_chars_to_indent; // was: = 0 

	return no_of_chars_to_indent; 
}

/**
 * The functionality ought to be in command_del_char() because undo*()s are also here.
 * TODO FIXME XXX
 * @param undo (1=called from undo(); 0=normal operation) 
 */
void
editor_del_char(int undo) {
	erow *row;

	if (E->cy == E->numrows) {
		if (E->cy > 0) {
			if (undo)
				editor_move_cursor(ARROW_LEFT);
			else
				command_move_cursor(COMMAND_MOVE_CURSOR_LEFT);
		}
		return; 
	}

	if (E->cx == 0 && E->cy == 0) 
		return;

	row = &E->row[E->cy];
	if (E->cx > 0) {
		int orig_cx = E->cx; 
		int char_to_be_deleted = row->chars[E->cx];
		int len = editor_row_del_char(row, E->cx - 1);

		if (len > 0) {
			int current_cx = E->cx;
			E->cx = orig_cx;

			while (current_cx < E->cx) {
				E->cx = current_cx; 
				if (!undo) {
					if (current_cx == orig_cx) { // ??
						undo_push_one_int_arg(COMMAND_DELETE_CHAR, COMMAND_INSERT_CHAR, 
							char_to_be_deleted);
					} else {
						undo_push_one_int_arg(COMMAND_DELETE_CHAR, COMMAND_INSERT_CHAR, 
							E->is_soft_indent ? (int) ' ' : (int) '\t');
					}
				}
				current_cx++;
			}
		}

		E->cx = orig_cx - len;

                if (E->coloff > 0) {
                        E->coloff -= len;
                        if (E->coloff < 0)
                                E->coloff = 0;
                }

	} else { 
		if (!undo)
			undo_push_one_int_arg(COMMAND_DELETE_CHAR, COMMAND_INSERT_CHAR, '\r');

		E->cx = E->row[E->cy - 1].size; 
		editor_row_append_string(&E->row[E->cy - 1], row->chars, row->size); 
		editor_del_row(E->cy);
		E->cy--; 
	}
}

/*** file I/O ***/

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

/**
 M-x open-file; also used when starting the editor to open files
 specified in the command line.
*/
void
command_open_file(char *filename) {
 	char *line = NULL;
  	size_t linecap = 0;
  	ssize_t linelen;
        struct stat stat_buffer; 
        FILE *fp = NULL;
        
        if (E->dirty > 0  
                || E->numrows > 0 || E->cx > 0 || E->cy > 0 
                ||  E->filename != NULL) {
                /* This buffer is in use. Create a new one & use it. */
                (void) create_buffer(BUFFER_TYPE_FILE, 0); 
        }

        E->filename = strdup(filename); 
        editor_select_syntax_highlight(NULL);

	E->absolute_filename = realpath(filename, NULL); 
	E->basename = editor_basename(filename);

  	if (stat(E->absolute_filename, &stat_buffer) == -1) {
  		if (errno == ENOENT) {
  			E->is_new_file = 1; 
  			E->dirty = 0; 
  			return; 
  		} else {
  			die("stat");
  		}
  	}

 	fp = fopen(E->absolute_filename, "r");
  	if (!fp) {
  		die("fopen");
 	}

  	while ((linelen = getline(&line, &linecap, fp)) != -1) {
                if (linelen > 0 && (line[linelen - 1] == '\n' 
                        || line[linelen - 1] == '\r'))
      		        linelen--;

                editor_insert_row(E->numrows, line, linelen);
	}

	free(line);
	fclose(fp);
	E->dirty = 0; 
}


/**
 *  rc = 0 OK
 * rc = -1 error
 * rc = 1 aborted
 */
void
editor_save(int command_key) {
	int len; 
	char *buf; 
	int fd; 
	char *tmp; 

	struct command_str *c = command_get_by_key(command_key);
	if (c == NULL) {
		editor_set_status_message("Unknown command! Cannot save!?!");
		return;
	}

	if (command_key == COMMAND_SAVE_BUFFER_AS || E->filename == NULL) {
		tmp = editor_prompt(c->prompt, NULL); 
		if (tmp != NULL) {
			free(E->filename);
			free(E->absolute_filename);
			free(E->basename);

			E->filename = tmp; 
			E->absolute_filename = realpath(E->filename, NULL); 
			E->basename = editor_basename(E->filename);

		} else {
			editor_set_status_message(STATUS_MESSAGE_ABORTED); // TODO ABORT message in conf.
			return; 
		}
	}

	buf = editor_rows_to_string(&len);
	fd = open(E->filename, O_RDWR | O_CREAT, 0644);
	if (fd != -1) {
		if (ftruncate(fd, len) != -1) {
			if (write(fd, buf, len) == len) {
        		      close(fd);
        		      free(buf);
        		      E->dirty = 0;
        		      E->is_new_file = 0;  

        		      editor_set_status_message(c->success, // TODO Special case: both %d and %s
        			len, E->absolute_filename ? E->absolute_filename : E->filename);
				return;
			}
			editor_select_syntax_highlight(NULL); 
		}
		close(fd);
	}
	free(buf);

	editor_set_status_message(c->error_status, strerror(errno));
	return; 
}


/*** undo ***/
struct undo_str *
init_undo_stack() {
	struct undo_str *undo_stack = malloc(sizeof(struct undo_str));
	if (undo_stack == NULL)
		die("undo stack");

	undo_stack->undo_command_key = COMMAND_NO_CMD;
	undo_stack->command_key = COMMAND_NO_CMD; // The terminus, new stack entries on top of this.
	undo_stack->cx = 0;
	undo_stack->cy = 0; 
	undo_stack->orig_value = 0;
	undo_stack->clipboard = NULL; 
	undo_stack->next = NULL; 

        return undo_stack; 
}

void
undo_debug_stack() {
	struct undo_str *p = current_buffer->undo_stack;
	char *debug = NULL; 
	int i = 1; 
	int currlen = 0;
	int from = 0;

	if (!(E->debug & DEBUG_UNDOS))
		return;

	while (p != NULL) {
		char *entry = malloc(80);
		int len = sprintf(entry, "|%d={%d,%d,%d}", i++, p->command_key, p->undo_command_key, p->orig_value);
		
		debug = realloc(debug, currlen + len + 1);
		memcpy(&debug[currlen], entry, len);
		debug[currlen + len] = '\0';
		currlen += len; 
		free(entry);	

		p = p->next; 
	} 

	if (currlen > TERMINAL.screencols)
		from = currlen - TERMINAL.screencols; 
	else
		from = 0; 

	editor_set_status_message(&debug[from]);
}

struct undo_str *
alloc_and_init_undo(int command_key) {
	struct undo_str *undo = malloc(sizeof(struct undo_str));
	if (undo == NULL)
		die("alloc_and_init_undo");

	undo->cx = E->cx;
	undo->cy = E->cy; 
	undo->command_key = command_key; 
        undo->clipboard = NULL; 
	undo->next = current_buffer->undo_stack;
	current_buffer->undo_stack = undo; 

	return undo; 
}

/* no args for commands. */
void
undo_push_simple(int command_key, int undo_command_key) {
//	if (current_buffer->undo_stack == NULL)
//		current_buffer->undo_stack = init_undo_stack(); 

	struct undo_str *undo = alloc_and_init_undo(command_key);
	undo->undo_command_key = undo_command_key; 
	undo->orig_value = -1; 
	undo_debug_stack();
}

void
undo_push_one_int_arg(int command_key, int undo_command_key, int orig_value) {
//	if (current_buffer->undo_stack == NULL)
//		current_buffer->undo_stack = init_undo_stack(); 

	struct undo_str *undo = alloc_and_init_undo(command_key);
	undo->undo_command_key = undo_command_key; 
	undo->orig_value = orig_value; 
	undo_debug_stack(); 
}

void
undo_push_clipboard() {
//	if (current_buffer->undo_stack == NULL)
//		current_buffer->undo_stack = init_undo_stack(); 

	struct clipboard *copy = clone_clipboard();
	struct undo_str *undo = alloc_and_init_undo(COMMAND_KILL_LINE);
	undo->undo_command_key = COMMAND_YANK_CLIPBOARD; 
	undo->clipboard = copy; 
	undo->orig_value = -999; 
	undo_debug_stack(); 
}

void
undo() {
	if (current_buffer->undo_stack == NULL) {
		current_buffer->undo_stack = init_undo_stack();
		return;
	}

	if (current_buffer->undo_stack->command_key == COMMAND_NO_CMD)
		return; 

	struct undo_str *top = current_buffer->undo_stack; 	
	current_buffer->undo_stack = top->next; 

	undo_debug_stack();

	switch(top->undo_command_key) {
	case COMMAND_MOVE_CURSOR_UP:
		editor_move_cursor(ARROW_UP);
		break;
	case COMMAND_MOVE_CURSOR_DOWN:
		editor_move_cursor(ARROW_DOWN);
		break;
	case COMMAND_MOVE_CURSOR_LEFT:
		editor_move_cursor(ARROW_LEFT);
		break;
	case COMMAND_MOVE_CURSOR_RIGHT:
		editor_move_cursor(ARROW_RIGHT);
		break;
	case COMMAND_SET_TAB_STOP:
		E->tab_stop = top->orig_value;
		break;
	case COMMAND_SET_HARD_TABS:
		E->is_soft_indent = 0;
		break;
	case COMMAND_SET_SOFT_TABS:
		E->is_soft_indent = 1; 
		break;
	case COMMAND_SET_AUTO_INDENT:
		E->is_auto_indent = top->orig_value;
		break;
	case COMMAND_DELETE_CHAR:
		editor_del_char(1);
		break;
	case COMMAND_INSERT_CHAR:
		if (top->orig_value == '\r')
			editor_insert_newline(); 
		else
			editor_insert_char(top->orig_value);
		break; 
	case COMMAND_DELETE_INDENT_AND_NEWLINE: {
		// Undoes command_insert_newline().
		int del = top->orig_value; 
		while (del-- > 0)
			editor_del_char(1);
		break; 
		}
	case COMMAND_YANK_CLIPBOARD:
		undo_clipboard_kill_lines(top->clipboard);
		free(top->clipboard->row);
		free(top->clipboard);
                break;
        case COMMAND_GOTO_LINE: 
                E->cy = top->orig_value; 
                break; 
	default:
		break; 
	}

	top->next = NULL; 
	free(top); 
}

/*** M-x command ***/

void
command_debug(int command_key) {
	if (!(E->debug & DEBUG_COMMANDS))
		return;

	struct command_str *c = command_get_by_key(command_key);
	if (c != NULL)
		editor_set_status_message(c->success);
}

/**
 * Return command_str by command_key 
 */
struct command_str *
command_get_by_key(int command_key) {
	unsigned int i;
	for (i = 0; i < COMMAND_ENTRIES; i++) {
		if (COMMANDS[i].command_key == command_key) 
			return &COMMANDS[i];
	}
	return NULL;
}

void
command_insert_char(int character) {
	if (character <= 31 && character != 9) // TODO 9 = TABKEY
		return;

	undo_push_simple(COMMAND_INSERT_CHAR, COMMAND_DELETE_CHAR);
	editor_insert_char(character);
}

void 
command_delete_char() {
	editor_del_char(0);
	undo_debug_stack(); 
	command_debug(COMMAND_DELETE_CHAR);
}

void 
command_insert_newline() {
	int indent_len = editor_insert_newline();
	if (E->is_auto_indent 
		&& (indent_len % E->tab_stop == 0)) { // 1 for newline
		indent_len = indent_len / E->tab_stop; // no of tabs, soft or hard.
	}

	// Delete indented+1 chars at once.
	undo_push_one_int_arg(COMMAND_INSERT_CHAR, COMMAND_DELETE_INDENT_AND_NEWLINE, 
		indent_len + 1); // newline here

	if (E->debug & DEBUG_COMMANDS)
		editor_set_status_message("Inserted newline");
}
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
		*ret_string = original_raw_string; 
		free(raw_str);
		return 1; // One argument 
	} else if (c->command_arg_type == COMMAND_ARG_TYPE_INT) {
		*ret_string = original_raw_string; /* For the error status message */

		// convert
		if (strlen(raw_str) > 8) {
			raw_str[8] = '\0';
		}

		rc = sscanf(raw_str, "%d", &raw_int); /* strtoimax(raw_str, NULL, 10); */

		free(raw_str);

		if (rc == 0) { 
			return -1; 
		}

		*ret_int = raw_int;
		return 1; 
	}

	free(raw_str);
	return 0; 
}

void
command_move_cursor(int command_key) {
	switch(command_key) {
	case COMMAND_MOVE_CURSOR_UP:
		/* Start macro */
		undo_push_simple(COMMAND_MOVE_CURSOR_UP, COMMAND_MOVE_CURSOR_DOWN);
		editor_move_cursor(ARROW_UP);
		/* End macro */
		break;
	case COMMAND_MOVE_CURSOR_DOWN:
		undo_push_simple(COMMAND_MOVE_CURSOR_DOWN, COMMAND_MOVE_CURSOR_UP);
		editor_move_cursor(ARROW_DOWN);
		break;
	case COMMAND_MOVE_CURSOR_LEFT:
		undo_push_simple(COMMAND_MOVE_CURSOR_LEFT, COMMAND_MOVE_CURSOR_RIGHT);
		editor_move_cursor(ARROW_LEFT);
		break;
	case COMMAND_MOVE_CURSOR_RIGHT:
		undo_push_simple(COMMAND_MOVE_CURSOR_RIGHT, COMMAND_MOVE_CURSOR_LEFT);
		editor_move_cursor(ARROW_RIGHT);
		break;
	default:
		break;		
	}
}

void
command_goto_line() {
        int int_arg = 0;
        char *char_arg = NULL;
        int current_cy = E->cy;
        
	struct command_str *c = command_get_by_key(COMMAND_GOTO_LINE);
	if (c == NULL)
                return;
                        
        int rc = editor_get_command_argument(c, &int_arg, &char_arg);
        if (rc == 0) {
                editor_set_status_message(STATUS_MESSAGE_ABORTED);
                free(char_arg);
                return;
        } else if (rc == -1) {
                editor_set_status_message(c->error_status, char_arg);
                free(char_arg); 
        }
        
        if (int_arg >= 0 && int_arg < E->numrows) 
                E->cy = int_arg;
                
        free(char_arg); 
        undo_push_one_int_arg(COMMAND_GOTO_LINE, COMMAND_GOTO_LINE, current_cy);
}
	
void
exec_command() {
	char *command = NULL; 
	unsigned int i = 0;
	int int_arg = 0;
	char *char_arg = NULL;
	int found = 0; 

	char *tmp = editor_prompt("Command: %s", NULL);
	if (tmp == NULL) {
		editor_set_status_message(STATUS_MESSAGE_ABORTED);
		return; 
	}

	command = strdup(tmp);
	free(tmp);

	for (i = 0; i < COMMAND_ENTRIES; i++) {
		struct command_str *c = &COMMANDS[i];

		if (!strncmp(c->command_str, command, strlen(command))) {
			found = 1; 
			if ((c->command_arg_type == COMMAND_ARG_TYPE_INT 
				|| c->command_arg_type == COMMAND_ARG_TYPE_STRING)
				/* FIXME remove exception... */
				&& (c->command_key != COMMAND_SAVE_BUFFER 
					&& c->command_key != COMMAND_SAVE_BUFFER_AS)) {
				/* should do union. */

				// rc=1 is good: it's the number of successfully parsed arguments.
				int rc = editor_get_command_argument(c, &int_arg, &char_arg);
				if (rc == 0) { // Aborted
					editor_set_status_message(STATUS_MESSAGE_ABORTED);
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
					undo_push_one_int_arg(COMMAND_SET_TAB_STOP, COMMAND_SET_TAB_STOP, E->tab_stop);
					E->tab_stop = int_arg; 
					editor_set_status_message(c->success, int_arg);
				} else {
					editor_set_status_message(c->error_status, char_arg);
				}
				break;
			case COMMAND_SET_AUTO_INDENT: {
				int auto_indent_set = 0;
				if (!strcasecmp(char_arg, "on") 
					|| !strcasecmp(char_arg, "t") || !strcasecmp(char_arg, "true")) {
					E->is_auto_indent = 1;
					auto_indent_set = 1;
				} else if (!strcasecmp(char_arg, "off") 
					|| !!strcasecmp(char_arg, "f") || strcasecmp(char_arg, "false")) {
					E->is_auto_indent = 0;
					auto_indent_set = 1;
				}
				if (auto_indent_set)
					editor_set_status_message(c->success, E->is_auto_indent ? "on" : "off"); 
				else
					editor_set_status_message(c->error_status, char_arg);
				break;
			}
			case COMMAND_SET_HARD_TABS:
				E->is_soft_indent = 0;
				editor_set_status_message(c->success);
				break;
			case COMMAND_SET_SOFT_TABS:
				E->is_soft_indent = 1;
				editor_set_status_message(c->success);
				break;
			case COMMAND_SAVE_BUFFER:
			case COMMAND_SAVE_BUFFER_AS:
				editor_save(c->command_key);
				break;
			case COMMAND_MOVE_CURSOR_UP:
			case COMMAND_MOVE_CURSOR_DOWN:
			case COMMAND_MOVE_CURSOR_LEFT:
			case COMMAND_MOVE_CURSOR_RIGHT:
				command_move_cursor(c->command_key); // away (ref. helper f.)
				break;
			case COMMAND_UNDO:
				undo();
				break; 
			case COMMAND_INSERT_CHAR:
				if (strlen(char_arg) > 0) {
					int character = (int) char_arg[0];
					command_insert_char(character);
				} else {
					editor_set_status_message(c->error_status);
				}
				break; 
			case COMMAND_DELETE_CHAR:
				command_delete_char();
				break;	
			case COMMAND_INSERT_NEWLINE:
				command_insert_newline();
				break;
                        case COMMAND_GOTO_LINE:
                                if (int_arg >= 0 && int_arg < E->numrows) {
                                        undo_push_one_int_arg(COMMAND_GOTO_LINE, COMMAND_GOTO_LINE, E->cy);
                                        E->cy = int_arg;
                                }
                                break;
                        case COMMAND_CREATE_BUFFER:
                                /* Note: for *Help* and *Compile* we set the type differently
                                   and the buffer creation is not done here. */
                                (void) create_buffer(BUFFER_TYPE_FILE, 1);
                                break;
                        case COMMAND_DELETE_BUFFER:
                                delete_current_buffer();
                                break;
                        case COMMAND_NEXT_BUFFER:
                                command_next_buffer();
                                break;
                        case COMMAND_PREVIOUS_BUFFER:
                                command_previous_buffer();
                                break;
                        case COMMAND_OPEN_FILE:
                                /* Note: is current buffer is empty, open into it. 
                                Otherwise, create new buffer and open the file it.*/
                                command_open_file(char_arg);
                                break;
			default:
				editor_set_status_message("Got command: '%s'", c->command_str);
				break;
			} 

                        if (char_arg != NULL)
			     free(char_arg);
			free(command);
			return;
		} /* if !strncasecmp */ 
	} /* for */

	if (!found) {
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
		memcpy(E->row[saved_hl_line].hl, saved_hl, E->row[saved_hl_line].rsize);
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

	for (i = 0; i < E->numrows; i++) {		
		current += direction; 
		if (current == -1)
			current = E->numrows - 1; 
		else if (current == E->numrows)
			current = 0; 

		row = &E->row[current];
		match = strstr(row->render, query); 
		if (match) {
			last_match = current; 
			E->cy = current; 
			E->cx = editor_row_rx_to_cx(row, match - row->render); 
			E->rowoff = E->numrows; 

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
	/*
	int saved_cx = E.cx; 
	int saved_cy = E.cy; 
	int saved_coloff = E.coloff; 
	int saved_rowoff = E.rowoff; */
	char *query = editor_prompt("Search: %s (Use ESC/Arrows/Enter)", editor_find_callback); 
	if (query) {
		free(query);
	} else {
				// Leave us at the last find position.
		/* else {
			E.cx = saved_cx;
			E.cy = saved_cy; 
			E.coloff = saved_coloff; 
			E.rowoff = saved_rowoff;
		} */
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
        int i; 
	if (C.row != NULL) { /* C.row is an array of clipboard_rows containing char *row. */
                for (i = 0; i < C.numrows; i++) {
                        if (C.row[i].row != NULL) {
                                free(C.row[i].row);
                        }
                }         
		free(C.row); 
	}

        C.row = NULL; 
	C.numrows = 0;
	C.is_full = 0; 
}

void
clipboard_add_line_to_clipboard() {
	if (E->cy < 0 || E->cy >= E->numrows)
		return; 

	if (C.is_full) {
		clipboard_clear();
	}

	erow *row = &E->row[E->cy];	

	// Append to the end.
	C.row = realloc(C.row, sizeof(clipboard_row) * (C.numrows + 1));
	C.row[C.numrows].row = malloc(row->size);
	memcpy(C.row[C.numrows].row, row->chars, row->size);
	C.row[C.numrows].size = row->size;
	C.row[C.numrows].orig_x = E->cx;
	C.row[C.numrows].orig_y = E->cy;
	C.row[C.numrows].is_eol = 1; 
	C.numrows++;		

	editor_del_row(E->cy);
}

void
clipboard_yank_lines() { 
	int j = 0; 
	for (j = 0; j < C.numrows; j++) {
		editor_insert_row(E->cy++, C.row[j].row, C.row[j].size);		
	}

	editor_set_status_message("Yank lines!");
}

void
undo_clipboard_kill_lines(struct clipboard *copy) { 
	int j = 0; 

	if (copy == NULL) {
		editor_set_status_message("Unable to undo kill lines.");
		return;
	}
	
	for (j = 0; j < copy->numrows; j++) {
		editor_insert_row(E->cy++, copy->row[j].row, copy->row[j].size);		
	}

	E->cx = copy->row[copy->numrows-1].orig_x;
	E->cy = copy->row[copy->numrows-1].orig_y; 

	editor_set_status_message("Undo kill lines!");
}

struct clipboard *
clone_clipboard() {
	int i = 0;
	struct clipboard *copy = malloc(sizeof(struct clipboard));
	if (copy == NULL) {
		editor_set_status_message("Failed to create clipboard to the undo stack.");
		return NULL; 
	}

	copy->is_full = 1; 
	copy->numrows = C.numrows;
	copy->row = malloc(C.numrows * sizeof(struct clipboard_row));
	if (copy->row == NULL) {
		editor_set_status_message("Failed to create clipboard rows to the undo stack.");
		return NULL; 
	}

	for (i = 0; i < C.numrows; i++) {
		clipboard_row *from = &C.row[i]; 
		clipboard_row *to = &copy->row[i];
		to->row = malloc(from->size);
		memcpy(to->row, from->row, from->size);
		to->size = from->size; 
		to->orig_x = from->orig_x;
		to->orig_y = from->orig_y;
		to->is_eol = from->is_eol;
	}

	return copy; 
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
	E->rx = 0; 

	if (E->cy < E->numrows) 
		E->rx = editor_row_cx_to_rx(&E->row[E->cy], E->cx);

	if (E->cy < E->rowoff)
		E->rowoff = E->cy;

	if (E->cy >= E->rowoff + TERMINAL.screenrows)
		E->rowoff = E->cy - TERMINAL.screenrows + 1; 
	
	if (E->rx < E->coloff) 
                E->coloff = E->rx;
  	
  	if (E->rx >= E->coloff + TERMINAL.screencols) 
                E->coloff = E->rx - TERMINAL.screencols + 1;
}

void
editor_draw_rows(struct abuf *ab) {
	int y;
	int filerow; 

	for (y = 0; y < TERMINAL.screenrows; y++) {
		filerow = y + E->rowoff; 

		if (filerow >= E->numrows) {
			if (!E->is_banner_shown && E->numrows == 0 && y == TERMINAL.screenrows / 3) {
				int padding = 0;
	      	        	char welcome[80];
                                int welcomelen = snprintf(welcome, sizeof(welcome),
        			     "%s", KILO_VERSION);
      			        if (welcomelen > TERMINAL.screencols) 
      				      welcomelen = TERMINAL.screencols;
      		
	      		        padding = (TERMINAL.screencols - welcomelen) / 2;
                                if (padding) {
                                        ab_append(ab, "~", 1);
	        		        padding--;
	      		        }

	      		        while (padding--) 
	         			ab_append(ab, " ", 1);

	      		        ab_append(ab, welcome, welcomelen);
	      	        } else { // / 3
			     ab_append(ab, "~", 1);
		        }
		} else {
			char *c; 
			unsigned char *hl; 
			int j; 
			int current_colour = -1; 
			int len = E->row[filerow].rsize - E->coloff;
			if (len < 0)
				len = 0; 

      		        if (len > TERMINAL.screencols) 
      			       len = TERMINAL.screencols;

      		        c  = &E->row[filerow].render[E->coloff];      		
      		        hl = &E->row[filerow].hl[E->coloff];

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
                                                ab_append(ab, buf, clen);
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
		E->basename ? E->basename : "[No name]", E->is_new_file ? "(New file)" : "", E->numrows, 
		E->dirty ? "(modified)" : ""); 
	rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d", 
		E->syntax != NULL ? E->syntax->filetype : "no ft", E->cy + 1, E->numrows);

	if (len > TERMINAL.screencols)
		len = TERMINAL.screencols; 

	ab_append(ab, status, len); 

	while (len < TERMINAL.screencols) {
		if (TERMINAL.screencols - len == rlen) {
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
debug_cursor() {
	char cursor[80];
	snprintf(cursor, sizeof(cursor), "cx=%d rx=%d cy=%d len=%d coloff=%d screencols=%d", 
		E->cx, E->rx, E->cy, E->row[E->cy].size, E->coloff, TERMINAL.screencols);
	editor_set_status_message(cursor); 
}

void
editor_draw_message_bar(struct abuf *ab) {
	int msglen; 
	ab_append(ab, "\x1b[K", 3); 

        if (E->debug & DEBUG_CURSOR) {
        	debug_cursor();
        }

	msglen = strlen(E->statusmsg); 
	if (msglen > TERMINAL.screencols)
		msglen = TERMINAL.screencols; 
	if (msglen && time(NULL) - E->statusmsg_time < 5) 
		ab_append(ab, E->statusmsg, msglen);

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
  		(E->cy - E->rowoff) + 1, (E->rx + E->coloff) + 1);
  	ab_append(&ab, buf, strlen(buf));
 	ab_append(&ab, "\x1b[?25h", 6); /* cursor on (h = set mode) */

	write(STDOUT_FILENO, ab.b, ab.len);
	ab_free(&ab);
}

void
editor_set_status_message(const char *fmt, ...) {
	va_list ap; 
	va_start(ap, fmt);
	vsnprintf(E->statusmsg, sizeof(E->statusmsg), fmt, ap);
	va_end(ap);
	E->statusmsg_time = time(NULL);
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
	else if (c == CTRL_KEY('u'))
		c = COMMAND_UNDO_KEY;
	else if (c == CTRL_KEY('g'))
		c = GOTO_LINE_KEY;
	return c; 
}

void 
editor_process_keypress() {
	static int quit_times = KILO_QUIT_TIMES; 
	static int previous_key = -1; 

	int c = editor_normalize_key(editor_read_key());

	/* Clipboard full after the first non-KILL_LINE_KEY. */
	if (previous_key == KILL_LINE_KEY && c != KILL_LINE_KEY) {
		C.is_full = 1; 
		undo_push_clipboard();
	}

	previous_key = c; 

	E->is_banner_shown = 1; // After the first keypress, yes. 

	switch (c) {
	case '\r':
		command_insert_newline(); 
		break;
	case QUIT_KEY:
		if (E->dirty && quit_times > 0) {
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
		editor_save(COMMAND_SAVE_BUFFER);
		break; 
	case HOME_KEY:
		E->cx = 0;
		break;
	case END_KEY:
		if (E->cy < E->numrows)
			E->cx = E->row[E->cy].size; 
		break;
	case FIND_KEY:
		editor_find();
		break; 
	case BACKSPACE:
	case CTRL_KEY('h'):
	case DEL_KEY:
		if (c == DEL_KEY) 
			command_move_cursor(COMMAND_MOVE_CURSOR_RIGHT);
		command_delete_char();
		break; 
	case PAGE_DOWN:
	case PAGE_UP: { 
		int times = 0;
		if (c == PAGE_UP) {
			E->cy = E->rowoff;
			times = TERMINAL.screenrows; 
		} else if (c == PAGE_DOWN) {
			E->cy = E->rowoff + TERMINAL.screenrows - 1;

			if (E->cy <= E->numrows) {
				times = TERMINAL.screenrows;
			} else {
				E->cy = E->numrows; 
				times = E->numrows - E->rowoff; 
			}
		}
		while (times--)
			command_move_cursor(c == PAGE_UP 
			? COMMAND_MOVE_CURSOR_UP : COMMAND_MOVE_CURSOR_DOWN);
		break;
		}
	case ARROW_UP:
		command_move_cursor(COMMAND_MOVE_CURSOR_UP);
		break;
	case ARROW_LEFT:
		command_move_cursor(COMMAND_MOVE_CURSOR_LEFT);
		break;
	case ARROW_RIGHT:
		command_move_cursor(COMMAND_MOVE_CURSOR_RIGHT);
		break;
        case ARROW_DOWN:
		command_move_cursor(COMMAND_MOVE_CURSOR_DOWN);
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
      	case CLEAR_MODIFICATION_FLAG_KEY:
      		if (E->dirty) {
      			editor_set_status_message("Modification flag cleared.");
      			E->dirty = 0;
      		}
      		break;
      	case COMMAND_KEY:
      		exec_command();
      		break; 
      	case COMMAND_UNDO_KEY:
      		undo();
      		break;
        case GOTO_LINE_KEY:
                command_goto_line();
                break;
        case NEXT_BUFFER_KEY:
                command_next_buffer();
                break;
        case PREVIOUS_BUFFER_KEY:
                command_previous_buffer(); 
                break;
	default:
		command_insert_char(c);
		break; 
	}

	quit_times = KILO_QUIT_TIMES; 
}


/*** init ***/
void
init_clipboard() {
	clipboard_clear();
}

void
init_config(struct editor_config *cfg) {
	/* editor config */
	cfg->cx = 0;
	cfg->cy = 0;
	cfg->rx = 0;
	cfg->numrows = 0;
	cfg->rowoff = 0;
	cfg->coloff = 0;
	cfg->dirty = 0;
	cfg->row = NULL; 
	cfg->filename = NULL; 
	cfg->absolute_filename = NULL; 
	cfg->basename = NULL; 
	cfg->statusmsg[0] = '\0';
	cfg->statusmsg_time = 0; 
	cfg->syntax = NULL; 
	cfg->is_new_file = 0;
	cfg->is_banner_shown = 0; 
	cfg->tab_stop = DEFAULT_KILO_TAB_STOP;
	cfg->is_soft_indent = 0;
	cfg->is_auto_indent = 0;
	cfg->debug = 0; 	
}

void
init_buffer() {
	buffer = create_buffer(BUFFER_TYPE_FILE, 0);
        //current_buffer = buffer; 
        //E = &current_buffer->E;  
        //init_config(E);  
}

void
init_editor() {
        init_buffer(); // Side effect: sets E.
	init_clipboard(); // C

        /* XXX TODO Need global terminal settings for new buffer config initialization. */
	if (get_window_size(&TERMINAL.screenrows, &TERMINAL.screencols) == -1)
		die("get_window_size");

        /* TODO BACK TO current_buffer->E at some point as E will be tied to a buffer, TERMINAL is global */
	TERMINAL.screenrows -= 2; /* Room for the status bar & status messages. */
}


#define KILO_HELP "\r\n\r\nkilo is a simple text editor that understands ascii.\r\n" \
 	"Basic Commands:\r\n" \
	"\tCtrl-Q quit\r\n" \
	"\tCtrl-F find\r\n" \
	"\tCtrl-S save\r\n" \
	"\tCtrl-K kill/copy full line to clipboard\r\n" \
	"\tCtrl-Y yank clipboard\r\n" \
	"\tCtrl-U undo\r\n" \
        "\tCtrl-G goto line\r\n" \
        "\tEsc-N next buffer\r\n" \
        "\tEsc-P previous buffer\r\n" \
	"\r\n" \
	"Movement:\r\n" \
	"\tArrow keys\r\n" \
	"\tPage Down/Ctrl-V Page Down\r\n" \
	"\tPage Up/Esc-V    Page Up\r\n" \
	"\tHome/Ctrl-A      Beginning of line\r\n" \
	"\tEnd/Ctrl-E       End of line\r\n" \
	"\r\n" \
	"Esc-C clears the modification flag.\r\n" \
	"Esc-X <command>:\r\n" \
	"\tset-tab-stop, set-auto-indent, set-hard-tabs, set-soft-tabs,\r\n" \
	"\tsave-buffer-as, undo (Ctrl-U), set-mode, goto-line (Ctrl-G)\r\n" \
        "\tcreate-buffer, next-buffer (Esc-N), previous-buffer (Esc-P),\r\n" \
        " delete-buffer\r\n" \
	"\r\n" \
	"The supported higlighted file modes are:\r\n" \
	"C, Elm, Erlang, Java, JavaScript, Makefile, Perl, Python, Ruby, Shell & Text.\r\n" \
        "\r\n" \
        "Usage: kilo [--help|--version|--debug level] [file] [file] ...\r\n" \
        "\tDebug levels: 1 = undo stack; 4 = cursor x & y coordinates.\r\n"  

void 
display_help() {
	printf(KILO_VERSION);
	printf(KILO_HELP);
}

void 
open_argument_files(int index, int argc, char **argv) {
        /* We have already called init_buffer() once before argument parsing. */
        command_open_file(argv[index]);
        
        while (++index < argc) {
                (void) create_buffer(BUFFER_TYPE_FILE, 0);
                command_open_file(argv[index]);
        }
}

void
parse_options(int argc, char **argv) {
	if (argc >= 3) {
		if (!strcmp(argv[1], "-d") || !strcmp(argv[1], "--debug")) {
			E->debug = atoi(argv[2]);

                        /* TODO multiple files: create buffers & open. */
			if (argc >= 4) {
                                open_argument_files(3, argc, argv);                        
                        }
		}  else {
                        open_argument_files(1, argc, argv);
                }
	} else if (argc >= 2) {
		if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
			printf("%s\r\n", KILO_VERSION);
			exit(0);
		} else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
			display_help();
			exit(0);
		} else {
                        open_argument_files(1, argc, argv);

		}
	}
}

int 
main(int argc, char **argv) {
	enable_raw_mode();
	init_editor();
	parse_options(argc, argv); // Also opens file.

	editor_set_status_message("Ctrl-S=save|C-Q=quit|C-F=find|C-K=copy|C-Y=paste|C-G=goto|C-U=undo");

	while (1) {
		editor_refresh_screen();
		editor_process_keypress();
	}

	return 0;
}
