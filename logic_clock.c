#include <semaphore.h>
#include "communication.h"
#include "main.h"

int logic_clock = 0;

extern sem_t mutex;

int get_clock(){
	int ret;
	sem_wait (&mutex);
	ret = logic_clock;
	sem_post (&mutex);	
	return ret;
}

int increment_clock()
{
	int ret;
	sem_wait (&mutex);
	logic_clock++;
	ret = logic_clock;
	sem_post (&mutex);
	return ret;
}

int update_clock(int new)
{
	int ret;
	sem_wait (&mutex);
	if(new > logic_clock)
	logic_clock = new;
	logic_clock++;
	ret = logic_clock;
	sem_post (&mutex);
	return ret;
}
