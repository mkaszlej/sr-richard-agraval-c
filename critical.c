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
			if( is_node_ok(i) == 0 ) access = 0;
	
		sleep(0.1);
	}
}

void * enter_critical_section()
{
	int i;
	printf("*******************************\n[%d] CRITICAL_SECTION", get_clock() );
	for(i=0; i<10; i++){
		sleep(1);
		printf("-");
		fflush(stdout);
	}
	printf("\n*******************************\n");
}

void * leave_critical_section()
{
	int i;
	/* incerement clock! */
	increment_clock();
	/* reset flag */
	set_waiting(0);
	/* reset all nodes */
	for(i; i<nodeCount; i++)
	{
		//reset all nodes
		reset_node_ok(i);
	}
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
