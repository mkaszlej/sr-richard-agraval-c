#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "communication.h"

#define T2_MAX 2

extern nodeAddress node[];
extern int nodeCount;
extern int global_port;
extern int waiting_clock;

void * send_message(void * send_thread_data_ptr)
{
	int i;
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    	
	/* send meta_data */
	send_thread_data * data = (send_thread_data*)send_thread_data_ptr; 
	
    /* message to send */
    char *json = data->json;
	if(json == NULL)
	{
		fprintf(stderr,"[%d]SM[%d] received null message\n", get_clock(), waiting_clock );
		return;
	}
	else
		printf("[%d]SM[%d] message: %s \n", get_clock(), waiting_clock, json);

	/* copy message to buffer */
    char buffer[30];
    bzero(buffer,30);
    strcpy(buffer,json);
    
    /* send a message to node */

	/* create a socket point */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	{
		fprintf(stderr, "[%d]SM[%d] ERROR opening socket for ip: %s:%d\n", get_clock(), waiting_clock, data->ip, data->port);
		return NULL;
	}

	/* get host name*/
	server = gethostbyname(data->ip);
	if (server == NULL) {
		fprintf(stderr, "[%d]SM[%d] ERROR no such host - ip: %s:%d\n", get_clock(), waiting_clock, data->ip, data->port);
		return NULL;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(data->port);

	/* now connect to the server */
	if (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0) 
	{
		fprintf(stderr, "[%d]SM[%d] ERROR connecting for ip: %s:%d\n", get_clock(), waiting_clock, data->ip, data->port);
		return NULL;
	}	

	/* Send message to the server */
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0) 
	{
		fprintf(stderr,"[%d]SM[%d] ERROR writing to socket for ip: %s:%d\n", get_clock(), waiting_clock, data->ip, data->port);
		return NULL;
	}
	else
		printf("[%d]SM[%d] message %s send to ip: %s:%d\n", get_clock(), waiting_clock, data->json, data->ip, data->port);
    
	/* TODO: CHECK IF WE NEED RESPONSE... */
	await_response(sockfd, data);

	close(sockfd);
	/* we have to free malloced struct we received */
	if(data!=NULL) free(data);
    return NULL;
}

void await_response(int sockfd, send_thread_data * data)
{
	int type, clock_val, n;
	/* allocate buffer */
    char buffer[256];
    char * token;
	bzero(buffer,256);

	//START TIMEOOUT CLOCK
	clock_t t_start, t2;
	t_start = clock();

	/* do until we or someone else set node ok to 1 */
	while( !is_node_ok(data->node_id) )
	{
		
		/* Now read server response */
		n = read(sockfd,buffer,255);
	
		/* read call was blocking */
		/* now check timer ?? */
		t2 = clock()-t_start;
		if(T2_MAX < ((float)t2)/CLOCKS_PER_SEC)
			fprintf(stderr,"[%d]SM[%d]TOO LONG WAITING: %g FOR ANSWER FROM ID: %d, SHOULD REMOVE THIS NODE\n", get_clock(), waiting_clock, ((float)t2)/CLOCKS_PER_SEC, data->node_id);
	

		/* Error may indicate closed socket etc. - we ignore it */
		if (n < 0) 
		{
			fprintf(stderr,"[%d]SM[%d]AWAIT RESPONSE: ERROR reading from for ip: %s:%d\n", get_clock(), 	waiting_clock, data->ip, data->port);
			sleep(0.3);
			continue;
			//return NULL;
		}
		
		//nothing to read...
		if(n==0){sleep(0.3); continue;}

		// Token will point to end of json.
		token = strtok(buffer, "}");
				
		//If null continue to next iteration
		//if(token == NULL){ sleep(0.3); continue; }
		//We ommit opening {
		token++;

		//Allocate buffer for answer
		char *json = (char *)malloc( n+1 );
		strcpy(json,token);
		
		//get type
		token = strtok(json,",:");
		token = strtok(NULL,",:");
		if( strcmp(token, "ok") == 0 || strcmp(token, "\"ok\"") == 0 ) type = 1;
		else type = 0;
		if(type != 1){ 
			//we received something else - not ok!
			fprintf(stderr,"[%d]SM[%d]AWAIT RESPONSE: ERROR not OK received from ip: %s:%d\ntoken: %s type: %d\n", get_clock(), waiting_clock, data->ip, data->port, token, type);
			sleep(0.3);
			continue;
		}

		//get clock
		token = strtok(NULL, ",:");
		token = strtok(NULL, ",:");
		clock_val = atoi(token);
		
		//update clock with received value
		update_clock(clock_val);
		
		printf("[%d]SM[%d] SERVER RESPONDED [%d] type: %d, clock: %d\n", get_clock(), waiting_clock, sockfd, type, clock_val);
		
		//we get ok so set node to ok
		set_node_ok(data->node_id);
		
		//free json that is parsed and no longer need
		free(json);
		fflush(stdout);
		
		break;

	}
	
	return;
	
}
