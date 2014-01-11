#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "server.h"

extern int global_port;

int mystrcmp(char const* s1, char const* s2, int n){
  int i;
  for(i=0; i < n; ++i){
    unsigned char c1 = (unsigned char)(s1[i]);
    unsigned char c2 = (unsigned char)(s2[i]);
    if(tolower(c1) != tolower(c2))
      return 0;
    if(c1 == '\0' || c2 == '\0')
      break;
  }
  return 1;
}

void *receiveMessage(void *fd_void_ptr)
{
    int sock = (*(int *)fd_void_ptr);	//get socket file descriptor

    printf("NEW THREAD[%d]: Receiving Message\n", sock);

    int n;
    char buffer[256];

    do 
    {
	bzero(buffer,256);		//zeruj buffer
        n = read(sock,buffer,255);	//odczytaj z bufora
        if (n < 0)
        {
            fprintf(stderr, "THREAD[%d]: ERROR reading from socket - exiting", sock);
            return NULL;
        }


        printf("THREAD[%d]: Here is the message: %s \n",sock ,buffer);
	fflush(stdout);


        n = write(sock,"Acknowledged",18);
        if (n < 0) 
        {
            fprintf(stderr, "THREAD[%d]: ERROR writing to socket", sock);
            exit(1);
	}	
    }
    while( mystrcmp( "exit" , buffer , 4 ) != 1 );

    close(sock);
    printf("THREAD[%d] FINISHED ITS DUTY\n", sock);

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

	/* create a second thread which executes inc_x(&x) */
	if(pthread_create(& process_thread, NULL, receiveMessage, &newsockfd)) {
		fprintf(stderr, "Error creating thread\n");
		exit(1);
	}

    } /* end of while */

    return NULL;

}
