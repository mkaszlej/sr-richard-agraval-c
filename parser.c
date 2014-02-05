#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "communication.h"
#include "jsmn/jsmn.h"



int token_string( char* js, jsmntok_t t, char *s)
{
	return ( strncmp(js+(t).start, s, (t).end - (t).start) == 0 && strlen(s) == (t).end - (t).start );
}

extern const int TYPE_ORDER;
extern const int TYPE_OK;
extern const int PARSE_ERROR;

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
	if(parser_return_code != JSMN_SUCCESS) return PARSE_ERROR;
	
	//SZUKAJ TYPU I ZEGARA
	for(i=0; i<10; i++)
	{
			if( token_string(buffer, token[i], "type") ) type_flag = i+1;
			if( token_string(buffer, token[i], "clock") ) clock_flag = i+1;
	}

	//NIE ZNALEZIONO! BLAD
	if(type_flag == -1 || clock_flag == -1 ) return PARSE_ERROR;
	//ZLE TYPY
	if(token[type_flag].type != JSMN_STRING && token[clock_flag].type != JSMN_PRIMITIVE) return PARSE_ERROR;

	int rec_clock = strtol( buffer+token[clock_flag].start, NULL, 0 );

	update_clock(rec_clock);
	
	if( token_string(buffer, token[type_flag], "order") ) return TYPE_ORDER;
	else if( token_string(buffer, token[type_flag], "ok") ) return TYPE_OK;
	else return PARSE_ERROR;
	
}
