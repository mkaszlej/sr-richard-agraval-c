#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	char *ipNumber = NULL;
	int mode = 0;
	int i, option;

	/* Odczyt wejscia */
	while ((option = getopt (argc, argv, "i:m:")) != -1)
 	{
		switch(option)
		{
			case 'i':
				ipNumber = optarg;
				break;
			case 'm':
				mode = atoi(optarg);
				break;
			case '?':
				if (optopt == "i" || optopt == "m" )
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

	/* Chwilowo tylko wypisujemy co weszło */
	printf("IP: %s\nMODE: %d\n", ipNumber, mode);
	return 0;
}

