#include <stdio.h>
#include <unistd.h>
#include "client.h"

int main(int argc, char* argv[])
{
	char *host = NULL;
	int port = 5001;
	int i, option;

	/* Odczyt wejscia */
	while ((option = getopt (argc, argv, "h:p:")) != -1)
 	{
		switch(option)
		{
			case 'h':
				host = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case '?':
				if (optopt == "h" || optopt == "p" )
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

	if( host == NULL ) 
	{
		fprintf(stderr, "UWAGA: Konieczne jest podanie hosta (-h)\n");
		exit(1);
	}
	
	printf("HOST: %s\nPORT: %d\n", host, port);

	client(port,host);

	return 0;
}

