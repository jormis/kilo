CC=cc
OBJS = buffer.o clipboard.o command.o file.o filetypes.o find.o help.o \
	init.o key.o kilo.o output.o row.o syntax.o terminal.o undo.o version.o
CFLAG = -Wall -g
INCLUDE =
LIBS =

kilo:${OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LIBS}
	
clean:
	-rm -f *.o core *.core
			
