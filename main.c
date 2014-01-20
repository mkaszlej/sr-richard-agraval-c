#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "main.h"
#include "communication.h"
#include "message.h"

//Tablica z adresami wezlow:
nodeAddress node[MAX_NODES];

//Globalnie ustawiony port:
int global_port = -1;

//Indeks na pierwszy pusty element w tablicy node:
int nodeCount = 0 ;     

//Domyslny plik konfiguracyjny:
static const char configDefault[] = "./config";

//Sciezka do pliku konfiguracyjnego
char *configPath = NULL; 

//Czas ostatniej modyfikacji pliku config
int  config_last_modified;

//mutex
sem_t mutex;
sem_t node_mutex;

//global waiting flag
int waiting;

//global waiting start time
int waiting_clock;

int main(int argc, char* argv[])
{
	int i, option;

	FILE *configFile;

	/* Inicjalizuj mutex */
	sem_init(&mutex, 0, 1);
	sem_init(&node_mutex, 0, 1);

	/* Ustaw flagi */
	waiting = 0;

	/* Odczyt wejscia */
	while ((option = getopt (argc, argv, "c:p:")) != -1)
 	{
		switch(option)
		{
			case 'c':
				configPath = optarg; //Przypisz sciezke do pliku konfiguracyjnego
				break;
			case 'p':
				global_port = atoi(optarg); //Przypisz port nasluchu
				break;
			case '?':
				if (optopt == "c" || optopt == "p" )
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
	pthread_t send_thread[nodeCount];
	send_thread_data * sd;
	int local_clock, i;
	char * json;
	
	/* Reset all node ok */
	for(i=0; i<nodeCount; i++) reset_node_ok(i);
	
	/* Ustaw flagę oczekiwania */
	waiting = 1;
	
	/* Pobierz czas zgłoszenia */
	waiting_clock = get_clock();
	
	/* Utwórz wysyłanego jsona */
	json = malloc(30*sizeof(char));
	sprintf(json,"{type:\"order\",clock:%d}\0", waiting_clock );

	for(i=0 ; i<nodeCount ; i++)
	{
		/* Alokuj strukturę send_thread_data - thread ją potem zwalnia */
		sd = (send_thread_data*)malloc(sizeof(send_thread_data));
		sd->node_id = i;
		sd->json = json;
		sd->ip = node[i].ip_name;
		sd->port = node[i].port;
		sd->ok = 0;
		
		if(pthread_create( & send_thread[i], NULL, send_message, sd))
		{
			fprintf(stderr, "[%d]Error starting %d client thread\n", get_clock(), i);
			free(sd);
			//terminate();
		}
	}

	//Wait for all threads to finish
	for(i=0; i<nodeCount; i++)
		pthread_join(send_thread[i], NULL);

	critial_section();

	free(json);
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

int find_node( long ip )
{
		int i;
		for(i=0; i<nodeCount; i++)
			if(node[i].ip == ip) return i;
		return -1;
}

void * critial_section()
{
	await_critical_section();
	enter_critical_section();
	leave_critical_section();
}

void * await_critical_section()
{
	int i,access=0;
	printf("[%d]AWAITING CRITIAL SECTION:\n******************************\n", get_clock());
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
	printf("[%d]CRITICAL_SECTION\n", get_clock() );
	sleep(20);
}

void * leave_critical_section()
{
	int i;
	/* reset flag */
	waiting=0;
	/* reset all nodes */
	for(i; i<nodeCount; i++)
		node[i].ok = 0;
	printf("[%d]LEFT CRITICAL_SECTION\n******************************\n", get_clock() );
}

