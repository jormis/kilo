# kilo

kilo is a simple text editor that understands ascii.

# Basic Commands:
	Ctrl-Q   quit
	Ctrl-F   find
	Ctrl-S   save
	Ctrl-K   kill/copy full line to clipboard
	Ctrl-Y   yank clipboard
	Ctrl-SPC set mark
	Ctrl-W   kill region from mark to clipboard
	Esc-W    copy region from mark to clipboard
	Ctrl-U   undo last command
	Ctrl-G   goto line
	Ctrl-O   open file
	Ctrl-N   new buffer
	Esc-N    next buffer
	Esc-P    previous buffer
	Ctrl-L   refresh screen (center to cursor row)

# Movement:
	Arrow keys
	Page Down/Ctrl-V Page Down
	Page Up/Esc-V    Page Up
	Home/Ctrl-A      Beginning of line
	End/Ctrl-E       End of line
	Esc-A            Beginning of file
	Esc-E            End of file

Esc-C clears the modification flag.

# Commands
Esc-X <command>:
	set-tab-stop, set-auto-indent, set-hard-tabs, set-soft-tabs,
	save-buffer-as, open-file, undo, set-mode, goto-line
	create-buffer, next-buffer, previous-buffer, delete-buffer
	mark, copy-region, kill-region, insert-char, delete-char
	goto-beginning, goto-end, refresh

The supported higlighted file modes are (M-x set-mode <mode>):
Bazel, C, Docker, Elm, Erlang, Go, Java, JavaScript, Makefile, nginx,
Perl, Python, Ruby, Shell, SQL & Text.

Usage: kilo [--help|--version|--debug level] [file] [file] ...
	Debug levels: 1 = undo stack; 4 = cursor x & y coordinates.

