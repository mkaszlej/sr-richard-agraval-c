CC=gcc
CFLAGS= -Wall
LDFLAGS=

all: richard-agravala server
		
richard-agravala: main.c client.c client.h
	clear; $(CC) $(CFLAGS) main.c client.c -o richard-agravala 

server: server.c
	$(CC) $(CFLAGS) server.c -o server
	
clean:
	rm -rf *.o server richard-agravala
