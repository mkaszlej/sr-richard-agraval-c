CC=gcc
CFLAGS= -Wall
LDFLAGS=

all: richard-agravala server
		
richard-agravala: main.c socket.c socket.h
	$(CC) $(CFLAGS) main.c socket.c -o richard-agravala

server: server.c
	$(CC) $(CFLAGS) server.c -o server
	
clean:
	rm -rf *.o server richard-agravala
