#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <unistd.h>
#include <string.h>

#include "communication.h"
#include "defines.h"

extern int global_port;


int doSelect(int sock)
{
	//for select function:
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 10000;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock, &set);

	return select(sock+1, &set, NULL, NULL, &timeout);

}

void *listen_messages(void *x_void_ptr)
{

    int sockfd, newsockfd, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int  n;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
		fprintf( stderr, "[%d]RM _STARTING SERVER SOCKET_ ERROR opening socket\n", get_clock() );
        exit(1);
    }
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(global_port);	//port defined in main.h
 
    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
		fprintf( stderr, "[%d]RM _STARTING SERVER SOCKET_ ERROR on binding\n", get_clock() );
         exit(1);
    }
    
    /* Now start listening for the clients, here 
     * process will go in sleep mode and will wait 
     * for the incoming connection
     */
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    int test=0;
    while (1) 
    {

    	test = doSelect(sockfd);
    	if( test < 0 )
    	{
    		fprintf( stderr, "[%d]RM[%d] BIG ERROR on accept\n", get_clock , newsockfd );
			exit(1);
    	}
    	if( test == 0 )
		{
    		sleep(1);
    		continue;
		}

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	
		printf("[%d]RM[%d] Connection started port: %d \n", get_clock(), newsockfd, global_port);

        if (newsockfd < 0 || test < 0)
        {
            fprintf( stderr, "[%d]RM[%d] ERROR on accept\n", get_clock , newsockfd );
            exit(1);
        }

		/* this variable is our reference to the second thread */
		pthread_t process_thread;

		/* create a receive data struct to sent to thread. Thread frees this data */
		receive_thread_data * rd = malloc(sizeof(receive_thread_data));
		rd->sockfd = newsockfd;
		rd->ip = cli_addr.sin_addr.s_addr;
		rd->port = ntohs(cli_addr.sin_port);
			
		/* execute in separate thread */
		if(pthread_create(& process_thread, NULL, receive_message, rd)) {
			fprintf(stderr, "[%d]RM[%d] Error creating thread\n", get_clock(), newsockfd );
			exit(1);
		}

    } /* end of while */

    return NULL;

}
