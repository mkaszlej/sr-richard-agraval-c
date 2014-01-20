#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <arpa/inet.h>
#include "main.h"

extern int config_last_modified;
extern char* configPath;
extern int nodeCount;
extern nodeAddress node[];

void *configLoop( void * configPath_ptr )
{
	char * configPath = (char*)configPath_ptr;
	FILE *configFile;

	printf("Czytam konfiguracje z: %s\n" , configPath );
	
	if( (configFile=fopen(configPath, "r"))==NULL ){
		fprintf(stderr,"BLAD: Nie moge odczytac pliku konfiguracyjnego: %s\n", configPath);
		exit(1);
	}
	
	parseConfig(configFile); //Wczytaj konfiguracje
	config_last_modified = configModificationTime(); //Wez czas ostatniej modyfikacji
	while(1)
	{
		/*
 		* Sprawdzamy i ewentualnie parsujemy konfiguracje
 		*/ 	
		int modTime = configModificationTime();
		if( config_last_modified != modTime ) 
		{
			parseConfig(configFile);//Zmiany w pliku konfiguracyjnym
			config_last_modified = modTime;
		}
		
		sleep(2);
	//	broadcast("not_ok");
	}	
	return 0;
}

int configModificationTime()
{
  struct stat foo;
  time_t mtime;
 
  if (stat(configPath, &foo) < 0) {
    perror(configPath);
    fprintf(stderr, "BLAD: Nie moge czytac z pliku konfiguracyjnego\n");
    exit(1);
  }
  mtime = foo.st_mtime; /* seconds since the epoch */

  return (int)mtime;

}

int parseConfig(FILE * configFile)
{

	printf("zmiany w konfigu\n");

    	const int max_n= 50;
    	char buffer[max_n], *result;
    
	while(feof(configFile) == 0)
	{
        	result = fgets (buffer, max_n, configFile);   // czytamy ze standardowego wejścia
        	if (result != NULL) 
        	{
//            		printf ("ODCZYTALEM:%s", buffer);
			parseConfigLine(buffer);
        	}
        	else
            		printf ("\n blad odczytu\n");
    	}
	fseek(configFile, 0, 0);
   	return 0;

}

int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    if(ipAddress == NULL) return 0;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result;
}

int isValidPort(int port)
{
	if( port < 0 ) return 0;
	if( port > 65535 ) return 0;
	return 1;
}

int parseConfigLine(char *buffer)
{

	char *ip;
	char *portStr;
	char *name;
	int port=-1;

	name = strtok(buffer,"|");
	if(name != NULL)
	{
		ip = strtok(NULL,"|");
		if(ip != NULL)
		{
			portStr=strtok(NULL,"|");
 			if(portStr != NULL)
				port=atoi(portStr);
		}
	}

	if( isValidIpAddress(ip) && isValidPort(port) )
	{

		printf("ODCZYTALEM LINIE: %s IP= %s:%d\n", name, ip, port );
		//TODO: USUWANIE NODE!

		//jesli nie ma takiego, to dodaj
	//	if( findNode( name, ip, port ) == -1 )
			addNode( name, ip, port );	
	//	else printf("LINIA JUZ ISTNIEJE: %s IP %s:%d = %d\n", name, ip, port, findNode( name, ip, port));

	}
	else printf("UWAGA: Linia nie zawiera poprawnej konfiguracji %s IP %s:%d\n", name, ip, port );

	return 0;

}

int findNode( char * name, char * ip , int port )
{
	int i;

	for( i=0 ; i< nodeCount ; i++ )
	{
		
		if( node[i].ip != NULL )
			if( strcmp(ip, node[i].ip) == 0 ) 
				if( node[i].port == port ) return i;
	}
	return -1;
}

int addNode( char *name, char *ip, int port)
{
	printf("DODAJE NODE: %s IP: %s:%d\n", name, ip, port);
	strcpy( node[nodeCount].name , name);
	strcpy( node[nodeCount].ip_name , ip );
	node[nodeCount].ip = (long)inet_addr(ip);
	node[nodeCount].port = port;
	nodeCount++;
}
