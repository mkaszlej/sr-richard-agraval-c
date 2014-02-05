#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <netinet/in.h>
#include "main.h"
#include "communication.h"
#include "globals.h"

int main(int argc, char* argv[])
{
	int i, option;
	int force_close = -1;

	FILE *configFile;

	/* Inicjalizuj mutex */
	sem_init(&mutex, 0, 1);
	sem_init(&node_mutex, 0, 1);
	sem_init(&waiting_mutex, 0, 1);
	sem_init(&counter_waiting_mutex, 0, 1);
	sem_init(&error_mutex, 0, 1);

	/* Ustaw flagi */
	set_stop_waiting();

	/* Odczyt wejscia */
	while ((option = getopt (argc, argv, "l:c:p:f:")) != -1)
 	{
		switch(option)
		{
			case 'c':
				configPath = optarg; //Przypisz sciezke do pliku konfiguracyjnego
				break;
			case 'p':
				global_port = atoi(optarg); //Przypisz port nasluchu
				break;
			case 'f':
				force_close = atoi(optarg); //Przypisz port nasluchu
				break;
			case 'l':
				local_address = inet_addr(optarg);
				break;				
			case '?':
				if (optopt == "c" || optopt == "p" || optopt == "f" || optopt == "l" )
					fprintf(stderr, "UWAGA: Opcja %c wymaga argumentu\n", optopt);
				else if (isprint(optopt))
					fprintf(stderr, "UWAGA: Nieznana opcja %c\n", optopt);
				else 
					fprintf(stderr, "UWAGA: Nieznana opcja `\\x%x'i\n", optopt);
				return 1;
			default:
				break;
		}
	}
	for (i = optind; i < argc; i++)
        	printf ("UWAGA: Argument, bez opcji %s\n", argv[i]);

	if( configPath == NULL ) 
	{
		printf("Domyslny plik konfiguracyjny przjety: ./config \n");
		configPath = configDefault;
	}
	
	if(global_port == -1)
	{
		global_port = 5001;
		printf("Domyslny port %d\n", global_port);
	}
	else printf("Ustawiono port %d\n", global_port);
	
	if(force_close >= 0)
	{
		printf("Forcing fd: %d to close\n", force_close);
		close(force_close);
	}
	
	if(local_address<0)
	{
		fprintf(stderr, "NO LOCAL IP ADDRESS SPECIFIED\n");
		exit(1);
	}
	
	//START CONFIG FILE MONITORING THREAD
	pthread_t config_monitor_thread;
	if(pthread_create( & config_monitor_thread, NULL, configLoop, configPath))
	{
		fprintf(stderr, "Error starting client thread\n");
    }

	//START COMMUNICATION LISTENING THREAD
	pthread_t server_thread;
	if(pthread_create( & server_thread, NULL, listenMessages, NULL)){
		fprintf(stderr, "Error starting server thread\n");
		exit(1);
	}

	//START MAIN PROGRAM LOOP
	mainLoop();

	fclose(configFile);
	return 0;
}

void mainLoop()
{
		while(1)
		{
				getchar();
				broadcast();
		}
}

void *broadcast()
{
	//TODO: we can save memory by allocating nodeActive here:
	pthread_t send_thread[nodeCount];
	send_thread_data * sd;
	int local_clock, i, rc;
	char * json;
	
	printf("\n[%d] USER ISSUED CRITICAL SECTION REQUEST\n", get_clock());
	
	/* Reset all node ok */
	for(i=0; i<nodeCount; i++)
	{
		send_thread[i] = NULL;
        reset_node_ok(i);
	}
	
	/* Ustaw flagę oczekiwania */
	set_start_waiting();
	
	/* Pobierz czas zgłoszenia */
	waiting_clock = get_clock();
	
	/* Utwórz wysyłanego jsona */
	json = malloc(30*sizeof(char));
	sprintf(json,"{\"type\":\"order\",\"clock\":%d}\0", waiting_clock );

	for(i=0 ; i<nodeCount ; i++)
	{
		//OMMIT SENDING TO DISABLED NODES
		if( node[i].active == 0 ) continue;
		
		/* Alokuj strukturę send_thread_data - thread ją potem zwalnia */
		sd = (send_thread_data*)malloc(sizeof(send_thread_data));
		sd->node_id = i;
		sd->json = json;
		sd->ip = node[i].ip_name;
		sd->port = node[i].port;
		sd->ok = 0;
		
		if(pthread_create( & send_thread[i],  NULL, send_message, sd))
		{ 
			fprintf(stderr, "[%d] Error starting %d client thread\n", get_clock(), i);
			free(sd);
		}
	}

	/* Wait for threads to join */
	for(i=0; i<nodeCount ; i++)
	{
		if(send_thread[i] != NULL)
		{
			void * status;
			rc = pthread_join( send_thread[i], &status);
		}
	}
	
	if( critial_section() == -1 )
	{
		free(json);
		broadcast();
	}
	else
		free(json);

}



