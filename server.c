#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

int mystrcmp(char const* s1, char const* s2, int n){
  int i;
  for(i=0; i < n; ++i){
    unsigned char c1 = (unsigned char)(s1[i]);
    unsigned char c2 = (unsigned char)(s2[i]);
    if(tolower(c1) != tolower(c2))
      return 0;
    if(c1 == '\0' || c2 == '\0')
      break;
  }
  return 1;
}

void doprocessing (int sock)
{
    int n;
    char buffer[256];

    do 
    {
	bzero(buffer,256);
        n = read(sock,buffer,255);
        if (n < 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }
        printf("Here is the message: %s \n",buffer);
	fflush(stdout);
        n = write(sock,"I got your message",18);

        if (n < 0) 
        {
            perror("ERROR writing to socket");
            exit(1);
	}	
    }
    while( mystrcmp( "exit" , buffer , 4 ) != 1 );

}

int readInput( int argc , char *argv[] )
{
        int portNo = -1;
        int i, option;

        /* Odczyt wejscia */
        while ((option = getopt (argc, argv, "p:")) != -1)
        {
                switch(option)
                {
                        case 'p':
                                portNo = atoi(optarg);
                                break;
                        case '?':
                                if (optopt == "p"  )
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


	return portNo;
}

int main( int argc, char *argv[] )
{

    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int  n;

    portno = readInput( argc, argv );
    if (portno < 0) portno = 5001;
    printf("\nODCZYTAŁEM PORT: %d\n", portno);

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    //portno = 5001;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
 
    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
         perror("ERROR on binding");
         exit(1);
    }
    /* Now start listening for the clients, here 
     * process will go in sleep mode and will wait 
     * for the incoming connection
     */
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while (1) 
    {
        newsockfd = accept(sockfd, 
                (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
        {
            perror("ERROR on accept");
            exit(1);
        }
        /* Create child process */
        int pid = fork();
        if (pid < 0)
        {
            perror("ERROR on fork");
	    exit(1);
        }
        if (pid == 0)  
        {
            /* This is the client process */
            close(sockfd);
            doprocessing(newsockfd);
            exit(0);
        }
        else
        {
            close(newsockfd);
        }
    } /* end of while */
}
