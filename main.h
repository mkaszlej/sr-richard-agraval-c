#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdio.h>


//Maksymalna liczba wezlow
#define MAX_NODES 10

//struktura zawierajaca adres ip i port wezla
typedef struct {
	char name[10];
	char ip_name[16];
	long ip;
	int port;
	char ok;
} nodeAddress;

int configModificationTime(); //Zrwaca > 0 jesli zmiany w pliku konfiguracyjnym

//Wątek z pętlą monitorującą konfig
void *configLoop( void *configPath_ptr );

//Glowna petla programu
void mainLoop();

//zarzadzanie wezlami
int addNode( char *name, char * ip, int port ); //dodaj nowy wezel
//int removeNode( int node ); //usun wezel
int find_node( long ip ); //zwraca indeks

//wyslij broadcast
void *broadcast();

//parsuj wejscie do programu
int parseInput(int argc , char ** argv);

//parsuj config
int parseConfig( FILE * );
int parseConfigLine(char *);

void reset_node_ok( int );
void set_node_ok( int );
int is_node_ok( int );

//critical section
void * critial_section();
void * await_critical_section();
void * enter_critical_section();
void * leave_critical_section();

//waiting flag
void set_waiting(int value);
int get_waiting();

/* Error handling */
void terminate();

#endif
