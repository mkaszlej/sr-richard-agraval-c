#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

//Metoda realizuje nasluch na sokecie
void *listen_messages(void *not_used);

//Wysylanie wiadomosci
void *send_message(void *message_ptr);

//Odebranie wiadomosci
void *receive_message(void *socket_fd_ptr);

//W communication_utils
void *send_response(long ip);
void *add_to_waiting_queue(int sock, long ip);

//------- Timer --------

//Get logic timer value
int get_clock();

//Update logic timer with received message clock
int update_clock(int new);

//Increment time by one
int increment_clock();


//------ Parser ----------
int parser_read(int, char*);
int do_parse_json(char *, int*);

//------- Data -----------

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
} receive_thread_data;


#endif
