#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "main.h"
#include "communication.h"

extern nodeAddress node[];
extern int nodeCount;
extern int global_port;

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
		fprintf(stderr,"[%d][%d] SEND MESSAGE: received null message\n", get_clock(), data->local_clock );
		return;
	}
	else
		printf("[%d][%d] SEND MESSAGE: message: %s \n", get_clock(), data->local_clock, json);

	/* copy message to buffer */
    char buffer[30];
    bzero(buffer,30);
    strcpy(buffer,json);
    
    /* send a message to node */

	/* create a socket point */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	{
		fprintf(stderr, "[%d][%d] SEND MESSAGE: ERROR opening socket for ip: %s:%d\n", get_clock(), data->local_clock, data->ip, data->port);
		return NULL;
	}

	/* get host name*/
	server = gethostbyname(data->ip);
	if (server == NULL) {
		fprintf(stderr, "[%d][%d] SEND MESSAGE: ERROR no such host - ip: %s:%d\n", get_clock(), data->local_clock, data->ip, data->port);
		return NULL;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(data->port);

	/* now connect to the server */
	if (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0) 
	{
		fprintf(stderr, "[%d][%d] SEND MESSAGE: ERROR connecting for ip: %s:%d\n", get_clock(), data->local_clock, data->ip, data->port);
		return NULL;
	}	

	/* Send message to the server */
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0) 
	{
		fprintf(stderr,"[%d][%d] SEND MESSAGE: ERROR writing to socket for ip: %s:%d\n", get_clock(), data->local_clock, data->ip, data->port);
		return NULL;
	}
	else
		printf("[%d][%d] SEND MESSAGE: message %s send to ip: %s:%d\n", get_clock(), data->local_clock, data->json, data->ip, data->port);
    
	/* TODO: CHECK IF WE NEED RESPONSE... */
	await_response(sock_fd, data);

    return NULL;
}

void await_response(int sock_fd, send_thread_data * data)
{

	/* allocate buffer */
    char buffer[256];
	bzero(buffer,256);
	
	/* Now read server response */
	n = read(sockfd,buffer,255);
	if (n < 0) 
	{
		fprintf(stderr,"[%d][%d] SEND MESSAGE AWAIT RESPONSE: ERROR reading from for ip: %s:%d\n", get_clock(), data->local_clock, data->ip, data->port);
		return NULL;
	}

}
