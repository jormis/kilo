#ifndef INIT_H
#define INIT_H

#include "const.h"
#include "data.h"
#include "terminal.h"
#include "syntax.h"
#include "clipboard.h"
#include "buffer.h"

//void init_buffer(int init_command_key);
void init_clipboard();
void init_editor();
void init_config(struct editor_config *cfg);

#endif

