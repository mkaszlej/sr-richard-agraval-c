CC=gcc
CFLAGS= -w -g
LDFLAGS=

all: richard-agravala 
		
richard-agravala: main.h communication.h globals.h
	cd ./jsmn; make clean; make ; cd ..
	clear; $(CC) $(CFLAGS) main.c listen_messages.c logic_clock.c config.c send_message.c node.c critical.c parser.c receive_message.c communication_utils.c ./jsmn/libjsmn.a -lpthread -o test

clean:
	rm -rf *.o server richard-agravala
