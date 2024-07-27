CFLAGS= -Wall -Wpedantic -g -ggdb

make:
	gcc -o poly main.c $(CFLAGS)
