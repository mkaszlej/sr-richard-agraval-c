#include <fcntl.h>
//#include <inttypes.h>
//#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/socket.h>
//#include <sys/types.h>
#include <unistd.h>

#include "communication.h"
#include "defines.h"
#include "jsmn/jsmn.h"


int token_string( char* js, jsmntok_t t, char *s)
{
	return ( strncmp(js+(t).start, s, (t).end - (t).start) == 0 && strlen(s) == (t).end - (t).start );
}



int parser_read(int sock, char * buffer)
{
	int n,s;
	int continue_reading=0;
	int parser_return_code;

	char bigBuffer[255];
	bzero(bigBuffer,255);

	char* bigBufferPtr = bigBuffer+1; //potem cofamy wskaznik żeby ominąć \0

	jsmntok_t token[10];
	jsmn_parser p;
	jsmn_init(&p);

	//for select function:
	int filedesc = open( "dev/ttyS0", O_RDWR ); //first free fd
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 5000;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock, &set);

	do{

		s = select(sock+1, &set, NULL, NULL, &timeout);//read(sock,buffer,255);	//odczytaj z bufora
		if(s<0){return -1;}

		printf("\n#timeout#\n");
		fflush(stdout);

		n=0;//profilaktycznie...

		//czy jest cos do odczytania
		if (FD_ISSET(sock, &set)){
			printf("!");

			n=read(sock,buffer,255);	//odczytaj z bufora
			if (n < 0){return -1;}
			if(n > 0)
			{
				strncat (bigBufferPtr-1, buffer, n);   // kopiuje n znakow, dostawia na koncu \0
				continue_reading = jsmn_parse(&p, bigBuffer, token, 10);
			}
		}

		if( continue_reading==JSMN_ERROR_PART || n==0 ) continue_reading=1;

		sleep(1);
	}
	while(continue_reading==1);

	return strcpy(buffer,bigBuffer);

}

int do_parse_json(char * buffer,int* clock)
{
	int i;
	int type_flag = -1;
	int clock_flag = -1;
	
	int parser_return_code;
	jsmntok_t token[10];
	jsmn_parser p;
	
	for(i=0; i<10; i++) token[i].type=-1;

	jsmn_init(&p);
	parser_return_code = jsmn_parse(&p, buffer, token, 10);
	if(parser_return_code != JSMN_SUCCESS) return ERROR;
	
	//SZUKAJ TYPU I ZEGARA
	for(i=0; i<10; i++)
	{
		if(token[i].type == -1) break;
		if( token_string(buffer, token[i], "type") || token_string(buffer, token[i], "Type") || token_string(buffer, token[i], "TYPE") ) type_flag = i+1;
		if( token_string(buffer, token[i], "clock") || token_string(buffer, token[i], "Clock") || token_string(buffer, token[i], "CLOCK") ) clock_flag = i+1;
	}

	fprintf(stderr,"### type_flag: %d, clock_flag: %d\n", type_flag, clock_flag);

	//NIE ZNALEZIONO! BLAD
	if(type_flag == -1 || clock_flag == -1 ) return ERROR;
	//ZLE TYPY! BLAD
	if(token[type_flag].type != JSMN_STRING && token[clock_flag].type != JSMN_PRIMITIVE) return ERROR;

	int rec_clock = strtol( buffer+token[clock_flag].start, NULL, 0 );

	update_clock(rec_clock);
	(*clock) = rec_clock;
	
	if( token_string(buffer, token[type_flag], "order") ) return ORDER;
	else if( token_string(buffer, token[type_flag], "ok") ) return OK;
	else return ERROR;
	
}
