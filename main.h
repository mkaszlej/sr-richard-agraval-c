#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdio.h>
#include <semaphore.h>

//Maksymalna liczba wezlow
#define MAX_NODES 10

//struktura zawierajaca adres ip i port wezla
typedef struct {
	char name[10];
	char ip_name[16];
	long ip;
	int port;
	char ok;
	char active;
	int last_message;
} nodeAddress;

//Glowna petla programu
void mainLoop();

//zarzadzanie wezlami
void add_node( char *name, char * ip, int port ); //dodaj nowy wezel
int find_node( long ip ); //zwraca indeks
void remove_node( int node ); //usun wezel

void reset_node_ok( int );
void set_node_ok( int );
int is_node_ok( int );

//Wątek z pętlą monitorującą konfig
void *configLoop( void *configPath_ptr );
//Zrwaca > 0 jesli zmiany w pliku konfiguracyjnym
int configModificationTime();
//parsuj wejscie do programu
int parseInput(int argc , char ** argv);
//parsuj config
int parseConfig( FILE * );
int parseConfigLine(char *);

//critical section
int critial_section();
int await_critical_section();
void * enter_critical_section();
void * leave_critical_section();
void * raise_error(int type, int param);

//waiting flag
void set_waiting(int value);
int get_waiting();

//wyslij broadcast
void *broadcast();

#endif
