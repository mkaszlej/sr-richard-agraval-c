#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

#include "/usr/include/semaphore.h"
#include "main.h"

//Tablica z adresami wezlow:
nodeAddress node[MAX_NODES];

//Globalnie ustawiony port:
int global_port = -1;

//Indeks na pierwszy pusty element w tablicy node:
int nodeCount = 0 ;     

//Ilosc aktywnych wezlow
int nodeActive = 0 ;     

//Domyslny plik konfiguracyjny:
static const char configDefault[] = "./config";

//Sciezka do pliku konfiguracyjnego
char *configPath = NULL; 

//Czas ostatniej modyfikacji pliku config
int  config_last_modified;

//mutex
sem_t mutex;
sem_t node_mutex;
sem_t waiting_mutex;
sem_t counter_waiting_mutex;
sem_t error_mutex;

//global waiting flag
int waiting;

int queue_counter = 0;

int global_error_flag = 0;

//global waiting start time
int waiting_clock;

long local_address = -1;


#endif
