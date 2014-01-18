#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "communication.h"
#include "message.h" 

extern int global_port;

void *receiveMessage(void *fd_void_ptr)
{
    int sock = (*(int *)fd_void_ptr);	//copy socket file descriptor to local var

    printf("SERVER THREAD[%d]: Receiving Message\n", sock);

    int n;
    char buffer[256];
    char rest[256];
    char *token,*json;

    Message * m; //pointer to message

//	TODO: jak z utrzymywaniem połączenia?
//    do 
//    {
	bzero(buffer,256);		//zeruj buffer
	bzero(rest,256);
	n = read(sock,buffer,255);	//odczytaj z bufora
	if (n < 0)
	{
		fprintf(stderr, "THREAD[%d]: ERROR reading from socket - exiting\n", sock);
		return NULL;
	}
	else if (n == 0)
	{
		fprintf(stderr, "THREAD[%d]: ERROR nothing to read from socket\n", sock);
		return NULL;
	}
	printf("BUFFER:%s|\n",buffer);
	
	// Token will point to end of json.
	token = strtok(buffer, "}");
	token++;
	m = (Message *)malloc(sizeof(Message));
	json = (char *)malloc( n );
	strcpy(json,token);
	m->json = json;

	token = strtok(NULL,"}");
	if(token != NULL) strcpy(rest,token);

    printf("[%d]SERVER[%d] json: {%s} \nRest is: %s\n", get_clock(), sock ,m->json, rest);

	fflush(stdout);

	token = strtok(m->json,",:");
	token = strtok(NULL,",:");

	if( strcmp(token, "ok") == 0 ) m->type = 1;
	else m->type = 0;

	printf("type: %d\n", m->type);

	token = strtok(NULL, ",:");
	token = strtok(NULL, ",:");
	
	m->clock = atoi(token);
	printf("clock: %d\n", m->clock);

	/*Zwieksz zegar*/
	update_clock(m->clock);
	

	//temporary
	free(json);
	free(m);

        n = write(sock,"Acknowledged",18);
        if (n < 0) 
        {
            fprintf(stderr, "THREAD[%d]: ERROR writing to socket", sock);
            exit(1);
	}	


//    }
//    while( mystrcmp( "exit" , buffer , 4 ) != 1 );

    //close(sock);
//	shutdown(sock,0);
    printf("[%d] SERVER[%d] FINISHED ITS DUTY\n", get_clock(), sock);

    return NULL;

}

void *listenMessages(void *x_void_ptr)
{

    int sockfd, newsockfd, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int  n;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    //portno = 5001;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(global_port);	//port defined in main.h
 
    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
         perror("ERROR on binding");
         exit(1);
    }
    
    /* Now start listening for the clients, here 
     * process will go in sleep mode and will wait 
     * for the incoming connection
     */
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while (1) 
    {

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	
		printf("[SERVERSOCKET:%d] Nawiązano połączenie \n", global_port);

        if (newsockfd < 0)
        {
            perror("ERROR on accept");
            exit(1);
        }


		/* this variable is our reference to the second thread */
		pthread_t process_thread;
		
		int socket_fd = newsockfd;
		
		/* create a second thread which executes inc_x(&x) */
		if(pthread_create(& process_thread, NULL, receiveMessage, &socket_fd)) {
			fprintf(stderr, "Error creating thread\n");
			exit(1);
		}
		//pthread_detach(process_thread);

    } /* end of while */

    return NULL;

}
