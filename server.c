#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "communication.h"

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
	sprintf(response, "{type:\"ok\",clock:%d}", get_clock() );

	/* sending message increments clock */
	increment_clock();
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
	
		
	
	// Token will point to end of json.
	token = strtok(buffer, "}");
	token++;
	json = (char *)malloc( n );
	strcpy(json,token);

    printf("[%d]RM[%d] json: {%s} \n", get_clock(), sock ,json);
	fflush(stdout);

	token = strtok(json,",:");
	token = strtok(NULL,",:");

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
		fprintf( stderr, "[%d]RM _STARTING SERVER SOCKET_ ERROR opening socket\n", get_clock() );
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
		fprintf( stderr, "[%d]RM _STARTING SERVER SOCKET_ ERROR on binding\n", get_clock() );
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
	
		printf("[%d]RM[%d] Connection started port: %d \n", get_clock(), newsockfd, global_port);

        if (newsockfd < 0)
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
			
		//printf("[%d]RM[%d] rd->ip:%d port:%d;rd->port: %d\n",  get_clock(), newsockfd, rd->ip, rd->port, ntohs(cli_addr.sin_port) );
		
		/* create a second thread which executes inc_x(&x) */
		if(pthread_create(& process_thread, NULL, receiveMessage, rd)) {
			fprintf(stderr, "[%d]RM[%d] Error creating thread\n", get_clock(), newsockfd );
			exit(1);
		}
		//pthread_detach(process_thread);

    } /* end of while */

    return NULL;

}
