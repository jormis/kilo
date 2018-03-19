#ifndef KILO_H
#define KILO_H

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
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "const.h"
#include "data.h"
#include "row.h"
#include "highlight.h"
#include "filetypes.h" /* language keywords */
#include "syntax.h"
#include "undo.h"
#include "terminal.h"
#include "key.h"
#include "command.h"
#include "clipboard.h"
#include "init.h"
#include "help.h"
#include "version.h"
#include "file.h"

/*** defines ***/
/*
	2018-03-19
	Latest:
        - 0.4.3 R mode
        - 0.4.2 Groovy mode
        - 0.4.1 Some cleaning
        - 0.4   Split kilo.c into multiple files
        - 0.3.9.10 "Vagrantfile" triggers Ruby-mode
        - 0.3.9.9 Dockerfile mode name changed to Docker & added ".dockerignore" to Docker mode.
        - 0.3.9.8 golang mode
        - 0.3.9.7 goto-line also refreshes screen
        - 0.3.9.6 screen resize
        - 0.3.9.5 nginx mode
        - 0.3.9.4 SQL mode
        - 0.3.9.3 Dockerfile mode
        - 0.3.9.2 refactor indent calculation a bit.
        - 0.3.9.1 Bazel autoindent && Erlang autoindent bug fix. TODO: generalize autoindent
        - 0.3.9 Bazel mode
        - 0.3.8 refresh (Ctrl-L)
        - 0.3.7 goto-beginning & goto-end (Esc-A, Esc-E)
        - Undo for next & previous buffer commands. (2017-05-30)
        - Ctrl-O open file Ctrl-N new buffer 
        - M-x mark (but no kill/copy region yet); opening a  new file don't cause segfault.
        - M-x open-file
        - Fixed open files bug where no files were opened in some cases.
        - Slightly better editor_del_char() but undo for del does not work.
        - delete-buffer
        - create-buffer, next-buffer, previous-buffer works (need open-file, every E->dirty check), test delete-buffer.
        - M-x goto-line
	- When aborted, the find command stops at the position.
       	- Fixed cursor left movement bug when past screen length.
        - --debug 4 debugs cursor & screen position 
        - Elm mode (incomplete but we'll get there)
	- Ruby mode
	- M-x undo works (but not 100%) on Ctrl-K/Ctrl-Y
	- Help (-h, --help)
	- Basically, limit input to ASCII only in command_insert_character().

        TODO set mode upon saving a new buffer unless the mode has been set already.
        TODO *Change* internal data structure to rope or gap ...
        TODO BUG: go mode uses spaces not tabs (also: indentation). 
        TODO BUG: backspace at the end of a line that's longer than screencols.
        TODO BUG: cursor up or down when at or near the end of line: faulty pos
        TODO BUG: soft/hard tab mix (like in this very file) messes pos calc
        TODO BUG: make integer arg parsing more robust (goto-line NaN segfaults)
        TODO SQL mode can be context insensitive
        TODO indentation revamp; more config with modes
	TODO (0.5) Emacs style C-K or C-SPC & C/M-W
	TODO (0.6) *Help* mode (BUFFER_TYPE_READONLY)
        TODO M-x search-and-replace (version 0: string; version 1: regexps)
	TODO *Command* or *Shell* buffer (think of REPL) 
	TODO (0.7) M-x compile (based on Mode & cwd contents): like Emacs (output)
	     [Compiling based on HL mode & working directory: make, mvn build, ant, lein]
	TODO ~/.kilorc/.kilo.conf (tab-width) (M-x set-tab-width)
	TODO M-x TAB command completion
	TODO M-x command buffer & context-sensitive parameter buffer.
        TODO (0.8) change internal data structure into rope or gap or similar 
	TODO (0.8.5) Store the last command argument context-sensitively
	TODO Proper command line options (-lgetopt)
	TODO (0.9) Unicode support (-lncurses)
	TODO (1.0) Forth interpreter from libforth ... (also: M-x forth-repl)
        TODO (1.1) Configuration files (with forth)
        TODO (1.1.5) Filetypes as configuration
        TODO (1.2) M-x hammurabi and other games. (maybe BUFFER_TYPE_INTERACTIVE)
*/

#endif

