CC=gcc
CFLAGS= -w -g
LDFLAGS=

all: richard-agravala 
	
	
richard-agravala: main.h communication.h globals.h
	clear; $(CC) $(CFLAGS) main.c server.c clock.c config.c send.c node.c critical.c -lpthread -o test 

clean:
	rm -rf *.o server richard-agravala
