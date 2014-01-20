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
extern sem_t waiting_mutex;

extern int waiting;

void * critial_section()
{
	await_critical_section();
	enter_critical_section();
	leave_critical_section();
}

void * await_critical_section()
{
	int i,access=0;
	printf("[%d] *** AWAITING CRITIAL SECTION: *** \n", get_clock());
	while(!access)
	{
		access = 1;
		for(i = 0 ; i < nodeCount ; i++ )
			if( node[i].ok != 1 ) access = 0;
		sleep(0.3);
	}
}

void * enter_critical_section()
{
	printf("******************************\n[%d]CRITICAL_SECTION\n******************************\n", get_clock() );
	sleep(10);
}

void * leave_critical_section()
{
	int i;
	/* reset flag */
	set_waiting(0);
	/* reset all nodes */
	for(i; i<nodeCount; i++)
		node[i].ok = 0;
	printf("[%d] *** LEFT CRITICAL_SECTION *** \n", get_clock() );
}

void set_waiting(int value)
{
	sem_wait (&waiting_mutex);
		waiting = value;
	sem_post (&waiting_mutex);
}

int get_waiting()
{
	int ret;
	sem_wait (&waiting_mutex);
		ret = waiting;
	sem_post (&waiting_mutex);	
	return ret;
}
