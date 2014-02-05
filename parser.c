#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "communication.h"
#include "jsmn/jsmn.h"
#include "defines.h"


int token_string( char* js, jsmntok_t t, char *s)
{
	return ( strncmp(js+(t).start, s, (t).end - (t).start) == 0 && strlen(s) == (t).end - (t).start );
}


int do_parse_json(char * buffer)
{
	int i;
	int type_flag = -1;
	int clock_flag = -1;
	
	int parser_return_code;
	jsmntok_t token[10];
	jsmn_parser p;
	
	jsmn_init(&p);
	parser_return_code = jsmn_parse(&p, buffer, token, 10);
	if(parser_return_code != JSMN_SUCCESS) return ERROR;
	
	//SZUKAJ TYPU I ZEGARA
	for(i=0; i<10; i++)
	{
			if( token_string(buffer, token[i], "type") ) type_flag = i+1;
			if( token_string(buffer, token[i], "clock") ) clock_flag = i+1;
	}

	fprintf(stderr,"### type_flag: %d, clock_flag: %d\n", type_flag, clock_flag);

	//NIE ZNALEZIONO! BLAD
	if(type_flag == -1 || clock_flag == -1 ) return ERROR;
	//ZLE TYPY
	if(token[type_flag].type != JSMN_STRING && token[clock_flag].type != JSMN_PRIMITIVE) return ERROR;

	int rec_clock = strtol( buffer+token[clock_flag].start, NULL, 0 );

	update_clock(rec_clock);
	
	if( token_string(buffer, token[type_flag], "order") ) return ORDER;
	else if( token_string(buffer, token[type_flag], "ok") ) return OK;
	else return ERROR;
	
}
