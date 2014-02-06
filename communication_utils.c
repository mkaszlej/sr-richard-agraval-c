/*
 * communication_utils.c
 *
 *  Created on: 6 lut 2014
 *      Author: mkaszlej
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "communication.h"
#include "main.h"

extern nodeAddress node[];

void *add_to_waiting_queue(int sock, long ip)
{

	increment_waiting_queue();

	while( get_waiting() == 1 )
	{
		sleep(0.1);
	}

	decrement_waiting_queue();

	printf("[%d]RM[%d] RELEASING FROM WAITING QUEUE -> IP: %d\n", get_clock(), sock, ip );

	return NULL;
}

void *send_response(long ip)
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
