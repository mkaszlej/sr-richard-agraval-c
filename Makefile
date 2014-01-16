CC=gcc
CFLAGS=  -g
LDFLAGS=

all: richard-agravala 
	
	
richard-agravala: main.h communication.h message.h 
	clear; $(CC) $(CFLAGS) main.c server.c client.c -lpthread -o test 

clean:
	rm -rf *.o server richard-agravala
