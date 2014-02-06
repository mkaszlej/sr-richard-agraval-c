
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "communication.h"
#include "defines.h"
#include "main.h"

extern nodeAddress node[];
extern int		waiting_clock;
extern long 	local_address;


void *receive_message(void *fd_void_ptr)
{
	receive_thread_data * ra = (receive_thread_data *)fd_void_ptr;

    int sock = ra->sockfd; 	//copy socket file descriptor to local var
	int port = ra->port;   	//sender port
	long ip = ra->ip;
    int n,clock;

    int id = find_node(ip);
    if(id != -1) node[id].last_message =  (unsigned)time(NULL);

    char buffer[256];
	bzero(buffer,256);		//zeruj buffer

	//CZYTAJ DO BUFORA
	if( parser_read(sock, buffer) == -1 ) {
		fprintf(stderr, "[%d]RM[%d]: ERROR reading from socket - exiting\n", get_clock(), sock);
		return NULL;
	}

	fprintf(stdout, "[%d]RM[%d]: odczytalem: %s \n", get_clock(), sock, buffer);

	//PARSUJ
	int parser_response = do_parse_json(buffer,&clock) ;
	
	switch (parser_response) {
		case OK:

			/* This should never happen */
			if(get_waiting() == 0)
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
				send_response(ip);
				close(sock);
			}
			/* waiting == 1 */
			else{

				if( clock < waiting_clock ){
					printf("[%d]RM[%d] WAITING BUT HE WAS FIRST -> SENDING OK to IP: %d\n", get_clock(), sock, ip );
					send_response(ip);
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
						send_response(ip);
						close(sock);
					}
				}
				else{
					printf("[%d]RM[%d] WAITING AND I WAS FIRST! -> WAITING QUEUE to IP: %d\n", get_clock(), sock, ip );
					add_to_waiting_queue(sock, ip);
					send_response(ip);
					close(sock);
				}
			}

			break;

		case ERROR:
		    printf("[%d]RM[%d] ERROR PARSING JSON - ignoring\n", get_clock(), sock);
			break;
	}

	if(ra!=NULL) free(ra);
    return NULL;
}
