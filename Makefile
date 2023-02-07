all: httpServer
	gcc httpServer.c -o httpServer -lpthread
