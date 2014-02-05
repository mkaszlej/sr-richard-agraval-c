#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "communication.h"
#include "jsmn.h"

extern int global_port;
extern int waiting_clock;
extern long local_address;

void *add_to_waiting_queue(int sock, long ip)
{
	int node_id = find_node( ip );
	if(node_id == -1)
	{
		fprintf(stderr, "[%d]RM wq[%d]: ERROR no such node in config ip:%s\n", get_clock(), sock, ip );
		close(sock);
		return;
	}
	
	increment_waiting_queue();
	 
	while( get_waiting() == 1 )
	{
		sleep(0.1);
	}
	
	decrement_waiting_queue();
	
	printf("[%d]RM[%d] RELEASING FROM WAITING QUEUE -> IP: %d\n", get_clock(), sock, ip );

	send_response(sock);
	
	return;
}

void *send_response(int sock)
{
	int n;
	/* Prepare response */
	char response[256];
	sprintf(response, "{\"type\":\"ok\",\"clock\":%d}", get_clock() );

	/* sending message increments clock */
	//increment_clock();
	
	/* send ok */
	n = write(sock, response, 30);
	if (n < 0) 
	{
		fprintf(stderr, "[%d]RM[%d]: ERROR writing to socket", get_clock(), sock);
		exit(1);
	}	
	else printf("[%d]RM[%d] sent response: %s\n", get_clock(), sock, response);

	close(sock);
}

void testJson(char * buffer)
{

	int r;
	jsmn_parser p;
	jsmntok_t tok[10];
	const char *js;

	printf("TESTING JSON: %s", buffer);

	js = "\"strVar\" : \"hello world\"";
	jsmn_init(&p);
	r = jsmn_parse(&p, js, tok, 10);
	check(r == JSMN_SUCCESS && tok[0].type == JSMN_STRING 
			&& tok[1].type == JSMN_STRING);
	check(TOKEN_STRING(js, tok[0], "strVar"));
	check(TOKEN_STRING(js, tok[1], "hello world"));

	js = "\"strVar\" : \"escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\\"";
	jsmn_init(&p);
	r = jsmn_parse(&p, js, tok, 10);
	check(r == JSMN_SUCCESS && tok[0].type == JSMN_STRING 
			&& tok[1].type == JSMN_STRING);
	check(TOKEN_STRING(js, tok[0], "strVar"));
	check(TOKEN_STRING(js, tok[1], "escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\"));

	js = "\"strVar\" : \"\"";
	jsmn_init(&p);
	r = jsmn_parse(&p, js, tok, 10);
	check(r == JSMN_SUCCESS && tok[0].type == JSMN_STRING 
			&& tok[1].type == JSMN_STRING);
	check(TOKEN_STRING(js, tok[0], "strVar"));
	check(TOKEN_STRING(js, tok[1], ""));

	return 0;

}


void *receiveMessage(void *fd_void_ptr)
{
	receive_thread_data * ra = (receive_thread_data *)fd_void_ptr;

    int sock = ra->sockfd; //copy socket file descriptor to local var
	int port = ra->port;   //sender port
	long ip = ra->ip;		//senter ip treated with inet_addr()

    int n, type, clock;
    char buffer[256];
    char *token,*json;

	struct sockaddr *address = malloc(sizeof(struct sockaddr));


	printf("[%d]RM[%d] Receiving Messagee from %d:%d\n", get_clock(), sock, ip, port );

	bzero(buffer,256);		//zeruj buffer

	n = read(sock,buffer,255);	//odczytaj z bufora
	
	if (n < 0)
	{
		fprintf(stderr, "[%d]RM[%d]: ERROR reading from socket - exiting\n", get_clock(), sock);
		return NULL;
	}
	else if (n == 0)
	{
		fprintf(stderr, "[%d]RM[%d]: ERROR nothing to read from socket\n", get_clock(), sock);
		return NULL;
	}

	printf("BUFFER:%s|\n",buffer);
	
	testJson(buffer);
	
	
	//TODO check buffor size!
		
	// Token will point to end of json.
	token = strtok(buffer, "}");
	token++;
	json = (char *)malloc( n );
	strcpy(json,token);

    printf("[%d]RM[%d] json: {%s} \n", get_clock(), sock ,json);
	fflush(stdout);

	token = strtok(json,",:");
	token = strtok(NULL,",:");

	//############# TUTAJ SEGMENT

	if( strcmp(token, "ok") == 0 || strcmp(token, "\"ok\"") == 0 ) type = 1;
	else type = 0;

	//printf("type: %d\n", type);

	token = strtok(NULL, ",:");
	token = strtok(NULL, ",:");
	
	clock = atoi(token);
	//printf("clock: %d\n", clock);

	/*Zwieksz zegar*/
	update_clock(clock);
	
	/*Message is parsed we can free json*/
	free(json);
	
	/* We`ve received ok message */
	if( type == 1 )
	{
		/* This should never happen */
		if(get_waiting() == 1)
		{
			fprintf(stderr, "[%d]RM[%d]: ERROR received ok while not waiting\n", get_clock(), sock);
			close(sock);
			return;			
		}
		
		/* Find IP of sender */
		int node_id = find_node( ip );
		if( node_id == -1 )
		{
			fprintf(stderr, "[%d]RM[%d]: ERROR no such node in config ip:%d\n", get_clock(), sock, ip );
			close(sock);
		}
		else
		{
			printf("[%d]RM[%d]: RECEIVED OK FROM NODE: %d IP: %d\n", get_clock(), sock, node_id, ip );
			set_node_ok(node_id);
			close(sock);
		}
	}
	/* We-ve received order message */
	else
	{
		/* We are not waiting for critical section - we can agree */
		if(get_waiting() == 0)
		{
			printf("[%d]RM[%d] NOT WAITING -> SENDING OK to IP: %d\n", get_clock(), sock, ip );
			send_response(sock);
		}
		else/* waiting == 1 */
		{
			if( clock < waiting_clock )
			{
				printf("[%d]RM[%d] WAITING BUT HE WAS FIRST -> SENDING OK to IP: %d\n", get_clock(), sock, ip );
				send_response(sock);
			}
			else if(clock == waiting_clock)
			{
			
				if( ip <= local_address )
				{
					printf("[%d]RM[%d] WAITING, SAME TIME! %d <= %d <- MY IP LOWER -> SENDING OK to IP: %d\n", get_clock(), sock, ip, local_address, ip );
					send_response(sock);
				}
				else
				{
					printf("[%d]RM[%d] WAITING, SAME TIME! %d > %d <- MY IP HIGHIER -> WAITING QUEUE to IP: %d\n", get_clock(), sock, ip, local_address, ip );
					add_to_waiting_queue(sock, ip);
				}
			}
			else
			{
				printf("[%d]RM[%d] WAITING AND I WAS FIRST! -> WAITING QUEUE to IP: %d\n", get_clock(), sock, ip );
				add_to_waiting_queue(sock, ip);
			}
		}
	}

    printf("[%d]RM[%d] FINISHED ITS DUTY\n", get_clock(), sock);

    return NULL;
}
