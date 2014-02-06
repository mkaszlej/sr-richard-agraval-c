#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include "main.h"
#include "communication.h"
#include <errno.h>


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
		raise_error(0, data->node_id);
		return;
	}
	else
		printf("[%d]SM[%d] message: %s \n", get_clock(), waiting_clock, json);

	/* copy message to buffer */
    char buffer[100];
    bzero(buffer,100);
    strcpy(buffer,json);
    
    /* send a message to node */

	/* create a socket point */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	{
		fprintf(stderr, "[%d]SM[%d] ERROR opening socket for ip: %s:%d\n", get_clock(), waiting_clock, data->ip, data->port);
		raise_error(1, data->node_id);
		return NULL;
	}

	/* get host name*/
	server = gethostbyname(data->ip);
	if (server == NULL) {
		fprintf(stderr, "[%d]SM[%d] ERROR no such host - ip: %s:%d\n", get_clock(), waiting_clock, data->ip, data->port);
		raise_error(1, data->node_id);
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
		raise_error(1, data->node_id);
		return NULL;
	}	

	/* Send message to the server */
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0) 
	{
		fprintf(stderr,"[%d]SM[%d] ERROR writing to socket for ip: %s:%d\n", get_clock(), waiting_clock, data->ip, data->port);
		raise_error(1, data->node_id);
		return NULL;
	}
	else
		printf("[%d]SM[%d] message %s send to ip: %s:%d\n", get_clock(), waiting_clock, data->json, data->ip, data->port);
    
	/* TODO: CHECK IF WE NEED RESPONSE... */
//	await_response(sockfd, data);

	close(sockfd);
	/* we have to free malloced struct we received */
	if(data!=NULL) free(data);
    return NULL;
}


