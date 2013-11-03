CC=gcc

CFLAGS= -std=c99 -Wall -pedantic


all: bt

bt: bt.o
	$(CC) bt.o -o bt
	
bt.o: bt.c
	$(CC) bt.c -c