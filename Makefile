CC=gcc
CFLAGS= -w -g
LDFLAGS=

all: richard-agravala 
	
	
richard-agravala: main.h communication.h message.h  
	clear; $(CC) $(CFLAGS) main.c server.c client.c clock.c config.c -lpthread -o test 

clean:
	rm -rf *.o server richard-agravala
