#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

//Metoda realizuje nasluch na sokecie
void *listenMessages(void *not_used);

//Odebranie wiadomosci
void *receiveMessage(void *socket_fd_ptr);

//Send broadcast
void *sendBroadcast(void *json_ptr);

//Send message
void *sendMessage(void *message_ptr);

//-------TIMER--------

//Get logic timer value
int getCurrentTime();

//Update logic timer with received message clock
int updateTime(int clock);

//Increment time by one
int incrementTime();
#endif
