CC=cc
kilo:	kilo.c
	$(CC) kilo.c -o kilo -O2 -Wall -Wextra -pedantic -std=c99
