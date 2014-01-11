#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

//Metoda realizuje nasluch na sokecie
void *listenMessages(void *);

//Odebranie wiadomosci
void *receiveMessage(void *);

#endif
