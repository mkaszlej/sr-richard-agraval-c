#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#include "communication.h"
#include "defines.h"
#include "main.h"

extern int nodeCount;
extern sem_t waiting_mutex;		
extern sem_t error_mutex;		
extern sem_t counter_waiting_mutex;		
extern int waiting;
extern int queue_counter;
extern int global_error_flag;

int critial_section()
{
	if( !await_critical_section() )
	{
		fprintf(stderr,"[%d] *** ERROR WHILE WAITING FOR CRITICAL SECTION\n", get_clock() );
		global_error_flag = 0;
		return -1;
	}
	enter_critical_section();
	leave_critical_section();
	
	return 0;
}

int await_critical_section()
{
	int i,access=0;
	printf("[%d] *** AWAITING CRITIAL SECTION: *** \n", get_clock());
	while(!access)
	{
		access = 1;
		for(i = 0 ; i < nodeCount ; i++ )
			if( is_node_ok(i) == 0 ) access = 0;
	
		if(global_error_flag) return 0;
	
		sleep(0.1);
	}
	return 1;
}

void * enter_critical_section()
{
	int i;

	/* incerement clock! TODO dlaczego? */
	increment_clock();
	
	printf("*******************************\n[%d] CRITICAL_SECTION", get_clock() );
	for(i=0; i<MAX_CRITICAL_TIME-1; i++){
		sleep(1);
		printf("-");
		fflush(stdout);
	}
	printf("\n*******************************\n");

	return NULL;
}

void * leave_critical_section()
{
	int i;
	/* incerement clock! */
	increment_clock();
	/* reset flag */
	set_stop_waiting();
	/* reset all nodes */
	for(i=0; i<nodeCount; i++)
	{
		//reset all nodes
		reset_node_ok(i);
	}
	printf("[%d] *** LEFT CRITICAL_SECTION *** \n", get_clock() );

	return NULL;
}

void set_start_waiting()
{
	int ret=0;
	while(ret == 0)
	{
		sem_wait (&waiting_mutex);
			if(waiting == 1)
			{
				printf("[%d] *** CURRENTLY WAITING! WQ: %d\n", get_clock(), queue_counter );
				ret = 1;
			}
			else if( get_waiting_queue_counter() == 0 ){
				 printf("[%d] *** START CRITIAL SECTION REQUEST. WQ CLEAN: %d\n", get_clock(), queue_counter );
				 waiting = 1;
				 ret = 1;
			}
			else
				printf("[%d] *** CANT ISSUE REQUEST, THREADS IN WQ: %d\n", get_clock(), queue_counter );

		sem_post (&waiting_mutex);
		sleep(0.3);
	}
}

void set_stop_waiting()
{
	sem_wait (&waiting_mutex);
		waiting = 0;
		printf("[%d] *** FINALIZED CRITICAL SECTION REQUEST. WAITING COUNT: %d\n", get_clock(), queue_counter );
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

void increment_waiting_queue()
{
	sem_wait (&counter_waiting_mutex);
		queue_counter++;
		printf("[%d] *** ADDING TO WAITING QUEUE. COUNT: %d\n", get_clock(), queue_counter );
	sem_post (&counter_waiting_mutex);	
}

void decrement_waiting_queue()
{
	sem_wait (&counter_waiting_mutex);
		queue_counter--;
		printf("[%d] *** REMOVING WAITING QUEUE. COUNT: %d\n", get_clock(), queue_counter );
	sem_post (&counter_waiting_mutex);	
}

int get_waiting_queue_counter()
{
	int ret;
	sem_wait (&counter_waiting_mutex);
		ret = queue_counter;
	sem_post (&counter_waiting_mutex);	
	return ret;
}	

void * raise_error(int type, int param)
{

	sem_wait (&error_mutex);
	
	fprintf(stderr, "[%d] *** HANDLING ERROR %d-%d: \n", get_clock(), type, param);
	
	fprintf(stderr, "[%d] *** INCREMENTING CLOCK \n", get_clock() );
	
	global_error_flag = 1;
	
	if( type == 1 ) remove_node(param);
	
	sem_post (&error_mutex);
	
	set_stop_waiting();
	
	return NULL;

}


