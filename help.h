#ifndef HELP_H
#define HELP_H

#include "const.h"

#define KILO_HELP "\r\n\r\nkilo is a simple text editor that understands ascii.\r\n" \
 	"Basic Commands:\r\n" \
	"\tCtrl-Q   quit\r\n" \
	"\tCtrl-F   find\r\n" \
	"\tCtrl-S   save\r\n" \
	"\tCtrl-K   kill/copy full line to clipboard\r\n" \
	"\tCtrl-Y   yank clipboard\r\n" \
        "\tCtrl-SPC set mark\r\n" \
        "\tCtrl-W   kill region from mark to clipboard\r\n" \
        "\tEsc-W    copy region from mark to clipboard\r\n" \
	"\tCtrl-U   undo last command\r\n" \
        "\tCtrl-G   goto line\r\n" \
        "\tCtrl-O   open file\r\n" \
        "\tCtrl-N   new buffer\r\n" \
        "\tEsc-N    next buffer\r\n" \
        "\tEsc-P    previous buffer\r\n" \
        "\tCtrl-L   refresh screen (center to cursor row)\r\n" \
        "\tEsc-C    clear the modification flag\r\n" \
        "\tEsc-X    <command> (see below)\r\n" \
	"\r\n" \
	"Movement:\r\n" \
	"\tArrow keys\r\n" \
	"\tPage Down/Ctrl-V Page Down\r\n" \
	"\tPage Up/Esc-V    Page Up\r\n" \
	"\tHome/Ctrl-A      Beginning of line\r\n" \
	"\tEnd/Ctrl-E       End of line\r\n" \
        "\tEsc-A            Beginning of file\r\n" \
        "\tEsc-E            End of file\r\n" \
	"\r\n" \
	"Esc-X <command> runs commands, <command> is on the following:\r\n" \
	"\tset-tab-stop, set-auto-indent, set-hard-tabs, set-soft-tabs,\r\n" \
	"\tsave-buffer-as, open-file, undo, set-mode, goto-line\r\n" \
        "\tcreate-buffer, next-buffer, previous-buffer, delete-buffer\r\n" \
        "\tmark, copy-region, kill-region, insert-char, delete-char\r\n" \
        "\tgoto-beginning, goto-end, refresh\r\n" \
	"\r\n" \
	"The supported higlighted file modes are (M-x set-mode <mode>):\r\n" \
	"Bazel, C, Docker, Elm, Erlang, Go, Java, JavaScript, Makefile, nginx,\r\n" \
        "Perl, Python, Ruby, Shell, SQL & Text.\r\n" \
        "\r\n" \
        "Usage: kilo [--help|--version|--debug level] [file] [file] ...\r\n" \
        "\tDebug levels: 1 = undo stack; 4 = cursor x & y coordinates.\r\n"  

void display_help();

#endif
