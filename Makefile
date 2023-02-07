CC=gcc
CFLAGS= -pthread

all: httpServer

httpServer: httpServer.c
	$(CC) $(CFLAGS) httpServer.c -o httpServer
