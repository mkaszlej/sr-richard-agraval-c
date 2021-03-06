#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "main.h"
#include "communication.h"

extern nodeAddress node[];
extern int nodeCount;
extern int nodeActive;
extern sem_t node_mutex;

int find_node( long ip )
{
		int i;
		for(i=0; i<nodeCount; i++)
		{
			if(node[i].ip == ip) return i;
		}
		return -1;
}

void add_node( char *name, char *ip, int port)
{
	
	sem_wait (&node_mutex);
	
	printf("DODAJE NODE: %s IP: %s:%d\n", name, ip, port);
	strcpy( node[nodeCount].name , name);
	strcpy( node[nodeCount].ip_name , ip );
	node[nodeCount].ip = (long)inet_addr(ip);
	node[nodeCount].port = port;
	node[nodeCount].active = 1;
	nodeCount++;
	nodeActive++;
	
	sem_post (&node_mutex);
}

void remove_node( int node_id )
{
	
	sem_wait (&node_mutex);
	
	printf("[%d] *** DISABLING NODE[%d]: %s:%d\n", get_clock(), node_id, node[node_id].ip_name, node[node_id].port);
	//disable it
	node[node_id].active = 0;
	//keep it ok to break all current processes
	node[node_id].ok = 1;
	//decrement active node count 
	nodeActive--;

	sem_post (&node_mutex);	

}

int is_node_ok( int node_id )
{
	int ret=0;

	sem_wait (&node_mutex);	
	//for disabled nodes always return ok
	if( node[node_id].active == 0 ) ret = 1;
	if( node[node_id].ok == 1 )	ret=1;
	sem_post (&node_mutex);
	
	return ret;
}

void set_node_ok( int node_id )
{
	sem_wait (&node_mutex);	
		node[node_id].ok = 1;
	sem_post (&node_mutex);
}

void reset_node_ok( int node_id )
{
	sem_wait (&node_mutex);	
		//leave not active nodes with ok = 1
		if(node[node_id].active == 1)
			node[node_id].ok = 0;
	sem_post (&node_mutex);
}
