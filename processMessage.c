#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "communication.h"
#include "defines.h"
#include "main.h"

extern int global_port;
extern int waiting_clock;
extern long local_address;
extern nodeAddress node[];

void *add_to_waiting_queue2(int sock, long ip)
{
	int node_id = find_node( ip );
	if(node_id == -1)
	{
		fprintf(stderr, "[%d]RM wq[%d]: ERROR no such node in config ip:%s\n", get_clock(), sock, ip );
		close(sock);
		return NULL;
	}
	
	increment_waiting_queue();
	 
	while( get_waiting() == 1 )
	{
		sleep(0.1);
	}
	
	decrement_waiting_queue();
	
	printf("[%d]RM[%d] RELEASING FROM WAITING QUEUE -> IP: %d\n", get_clock(), sock, ip );

	send_response2(ip);
	close(sock);
	
	return NULL;
}

void *send_response2(long ip)
{
	int node_id = find_node(ip);
	if(node_id == -1){
		printf("[%d]RM NIE UDAŁO SIĘ ODNALEŹĆ NODE DLA IP: %d", get_clock(), ip);
		return NULL;
	}

	/* Alokuj strukturę send_thread_data - thread ją potem zwalnia */
	send_thread_data * sd;
	sd = (send_thread_data*)malloc(sizeof(send_thread_data));

	/* Prepare response */
	char response[256];
	sprintf(response, "{\"type\":\"ok\",\"clock\":%d}", get_clock() );

	sd->node_id = node_id;
	sd->json = response;
	sd->ip = node[node_id].ip_name;
	sd->port = node[node_id].port;
	sd->ok = 0;

	return send_message(sd);

}

void *receiveMessage2(void *fd_void_ptr)
{
	receive_thread_data * ra = (receive_thread_data *)fd_void_ptr;

    int sock = ra->sockfd; 	//copy socket file descriptor to local var
	int port = ra->port;   	//sender port
	long ip = ra->ip;
    int n,clock;

    char buffer[256];
	bzero(buffer,256);		//zeruj buffer


	n = read(sock,buffer,255);	//odczytaj z bufora

	//TODO obsluga bledow
	if (n < 0){
		fprintf(stderr, "[%d]RM[%d]: ERROR reading from socket - exiting\n", get_clock(), sock);
		return NULL;
	}
	else if (n == 0){
		fprintf(stderr, "[%d]RM[%d]: ERROR nothing to read from socket\n", get_clock(), sock);
		return NULL;
	}

	fprintf(stdout, "[%d]RM[%d]: odczytalem: %s \n", get_clock(), sock, buffer);

	//PARSUJ
	int parser_response = do_parse_json(buffer) ;
	
	switch (parser_response) {
		case OK:

			/* This should never happen */
			if(get_waiting() == 1)
			{
				fprintf(stderr, "[%d]RM[%d]: ERROR received ok while not waiting\n", get_clock(), sock);
				close(sock);
				return NULL;
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
			break;

		case ORDER:

			/* We are not waiting for critical section - we can agree */
			if(get_waiting() == 0){
				printf("[%d]RM[%d] NOT WAITING -> SENDING OK to IP: %d\n", get_clock(), sock, ip );
				send_response2(ip);
				close(sock);
			}
			/* waiting == 1 */
			else{

				if( clock < waiting_clock ){
					printf("[%d]RM[%d] WAITING BUT HE WAS FIRST -> SENDING OK to IP: %d\n", get_clock(), sock, ip );
					send_response2(ip);
					close(sock);
				}
				else if(clock == waiting_clock)
				{

					if( ip <= local_address ){
						printf("[%d]RM[%d] WAITING, SAME TIME! %d <= %d <- MY IP LOWER -> SENDING OK to IP: %d\n", get_clock(), sock, ip, local_address, ip );
						send_response(ip);
						close(sock);
					}
					else{
						printf("[%d]RM[%d] WAITING, SAME TIME! %d > %d <- MY IP HIGHIER -> WAITING QUEUE to IP: %d\n", get_clock(), sock, ip, local_address, ip );
						add_to_waiting_queue(sock, ip);
					}
				}
				else{
					printf("[%d]RM[%d] WAITING AND I WAS FIRST! -> WAITING QUEUE to IP: %d\n", get_clock(), sock, ip );
					add_to_waiting_queue(sock, ip);
				}
			}

			break;
		case ERROR:
		    printf("[%d]RM[%d] ERROR PARSING JSON\n", get_clock(), sock);
			break;
	}

    return NULL;
}
