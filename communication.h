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
int get_clock();

//Update logic timer with received message clock
int update_clock(int new);

//Increment time by one
int increment_clock();

//Data to sending thread
typedef struct{
	int node_id;
	char * ip;
	int port;
	char * json;
	int local_clock;
} send_thread_data;
#endif
