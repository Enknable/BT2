CC=gcc

CFLAGS= -std=c99 -Wall -pedantic


all: bt

bt: bt.o BTLIB.o
	$(CC) bt.o BTLIB.o -o bt
	
bt.o: bt.c
	$(CC) bt.c -c
	
BTLIB.o: BTLIB.c
	$(CC) BTLIB.c -c