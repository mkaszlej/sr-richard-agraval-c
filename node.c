#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "main.h"

extern nodeAddress node[];
extern int nodeCount;
extern int nodeActive;
extern sem_t node_mutex;


/*int findNode( char * name, char * ip , int port )
{
	int i;

	for( i=0 ; i< nodeCount ; i++ )
	{
		if( node[i].ip != NULL )
			if( strcmp(ip, node[i].ip) == 0 ) 
				if( node[i].port == port ) return i;
	}
	return -1;
}*/


int find_node( long ip )
{
		int i;
		for(i=0; i<nodeCount; i++)
			if(node[i].ip == ip) return i;
		return -1;
}

void add_node( char *name, char *ip, int port)
{
	printf("DODAJE NODE: %s IP: %s:%d\n", name, ip, port);
	strcpy( node[nodeCount].name , name);
	strcpy( node[nodeCount].ip_name , ip );
	node[nodeCount].ip = (long)inet_addr(ip);
	node[nodeCount].port = port;
	nodeCount++;
	nodeActive++;
}

void remove_node( int node_id )
{
	return;
}

int is_node_ok( int node_id )
{
	int ret=0;
	sem_wait (&node_mutex);
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
		node[node_id].ok = 0;
	sem_post (&node_mutex);
}
