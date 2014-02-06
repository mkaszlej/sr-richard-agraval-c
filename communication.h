#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

//Metoda realizuje nasluch na sokecie
void *listenMessages(void *not_used);

//Odebranie wiadomosci
void *receiveMessage(void *socket_fd_ptr);

//Odebranie wiadomosci
void *receiveMessage2(void *socket_fd_ptr);

//Send broadcast
void *sendBroadcast(void *json_ptr);

//Send message
void *send_message(void *message_ptr);

void *send_response(int sock);
void *send_response2(long ip);

void *add_to_waiting_queue(int sock, long ip);
void *add_to_waiting_queue2(int sock, long ip);
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
	int ok;
} send_thread_data;

//Data to receiving thread
typedef struct{
	int sockfd;
	int port;
	long ip;
	char * ip_name;
} receive_thread_data;

void testJson(char *);
int do_parse_json(char *, int*);
#endif
