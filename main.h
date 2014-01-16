#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

//Maksymalna liczba wezlow
#define MAX_NODES 10

//struktura zawierajaca adres ip i port wezla
typedef struct {
	char name[10];
	int port;
	char ip[16];
	char is_thread_running;
} nodeAddress;

//Tablica z adresami wezlow
//nodeAddress node[MAX_NODES];

//int nodeCount = 0 ;	//Indeks na pierwszy pusty element w tablicy node

//int global_port = -1;	//Gloablny port nasluchiwania

//static const char configDefault[] = "./config"; //Domyslny plik konfiguracyjny 
//char *configPath = NULL; //Sciezka do pliku konfiguracyjnego
//int  config_last_modified;
int configModificationTime(); //Zrwaca > 0 jesli zmiany w pliku konfiguracyjnym


//Wątek z pętlą monitorującą konfig
void *configLoop( void *configPath_ptr );

//Glowna petla programu
void mainLoop();

//zarzadzanie wezlami
int addNode( char *name, char * ip, int port ); //dodaj nowy wezel
int removeNode( int node ); //usun wezel
int findNode( char *name, char *ip, int port ); //zwraca indeks

//wyslij broadcast
void *broadcast(const char *);

//parsuj wejscie do programu
int parseInput(int argc , char ** argv);

//parsuj config
int parseConfig(FILE*);
int parseConfigLine(char *);

//logowanie
log(char *);

void * client( void * ptr );

#endif
