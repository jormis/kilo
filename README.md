# kilo

kilo is a simple text editor that understands ascii.

It is based on the kilo project (https://github.com/antirez/kilo):

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
	Esc-C    clear the modification flag
	Esc-X	 <command> (see below)

# Movement:
	Arrow keys
	Page Down/Ctrl-V Page Down
	Page Up/Esc-V    Page Up
	Home/Ctrl-A      Beginning of line
	End/Ctrl-E       End of line
	Esc-A            Beginning of file
	Esc-E            End of file

# Commands:
    Esc-X <command>, where <command> is one of the following:

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

