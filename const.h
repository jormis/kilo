#ifndef CONST_H
#define CONST_H

#define KILO_VERSION "kilo -- a simple editor version 0.4"
#define WELCOME_STATUS_BAR "Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find | kilo --help for more info"

#define DEFAULT_KILO_TAB_STOP 8
/* Default: soft tabs. */
#define HARD_TABS (1<<2)

#define KILO_QUIT_TIMES 3
#define STATUS_MESSAGE_ABORTED "Aborted."
#define DEFAULT_SEARCH_PROMPT "Search: %s (Use ESC/Arrows/Enter)"

#define DEBUG_UNDOS (1<<0)
#define DEBUG_COMMANDS (1<<1)
#define DEBUG_CURSOR (1<<2) 

#endif
