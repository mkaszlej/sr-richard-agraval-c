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

void * sendBroadcast(void * json_ptr)
{
	int i;
	
    //message to send
    char *json = (char*)json_ptr;

	if(json == NULL)
	{
		fprintf(stderr,"SEND BROADCAST: received null message\n");
		return;
	}
	else
		printf("SEND BROADCAST: message: %s \n", json);

    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

	//copy message to buffer
    char buffer[128];
    bzero(buffer,128);
    strcpy(buffer,json);
    
    //Send a message to each node
    for(i=0; i<nodeCount; i++)
	{
		/* Create a socket point */
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) 
		{
			fprintf(stderr, "SEND BROADCAST[%d]: ERROR opening socket\n", i);
//			return;
			continue;
		}

		server = gethostbyname(node[i].ip);
		if (server == NULL) {
			fprintf(stderr,"SEND BROADCAST[%d] - ERROR, no such host: %s\n", i, node[i].ip );
//			return;
			continue;
		}

		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		serv_addr.sin_port = htons(node[i].port);

		/* Now connect to the server */
		if (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0) 
		{
			fprintf(stderr, "SEND BROADCAST[%d] - ERROR connecting\n", i);
//			return;
			continue;
		}	

		/* Send message to the server */
		n = write(sockfd,buffer,strlen(buffer));
		if (n < 0) 
		{
			fprintf(stderr,"SEND BROADCAST[%d] - ERROR writing to socket\n", i);
			//return;
			continue;
		}
		else
			printf("SEND BROADCAST[%d] - send to %s|%s|%d \n",i, node[i].name, node[i].ip, node[i].port);
    
		/* TODO: CHECK IF WE NEED RESPONSE...
		 * Now read server response 
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		if (n < 0) 
		{
			fprintf("CLIENT[%d] - ERROR reading from socket\n", number);
			return;
		}
		printf("%s\n",buffer);*/
		//close(sockfd);
    }
    
    return;
}
