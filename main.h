#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdio.h>


//Maksymalna liczba wezlow
#define MAX_NODES 10

//struktura zawierajaca adres ip i port wezla
typedef struct {
	char name[10];
	char ip[16];
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
int findNode( char *name, char *ip, int port ); //zwraca indeks

//wyslij broadcast
void *broadcast();

//parsuj wejscie do programu
int parseInput(int argc , char ** argv);

//parsuj config
int parseConfig( FILE * );
int parseConfigLine(char *);

//logowanie
log(char *);

//void * client( void * ptr );

/* Error handling */
void terminate();

#endif
