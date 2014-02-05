CC=gcc
CFLAGS= -w -g
LDFLAGS=

all: jsmn richard-agravala 
	
jsmn:
	pushd jsmn; make clean; make ; popd
	
richard-agravala: main.h communication.h globals.h
	clear; $(CC) $(CFLAGS) main.c server.c logic_clock.c config.c send.c node.c critical.c parser.c processMessage.c ./jsmn/libjsmn.a -lpthread -o test

clean:
	rm -rf *.o server richard-agravala
