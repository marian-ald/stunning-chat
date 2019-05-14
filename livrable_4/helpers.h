#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

// Maximum length of the message
#define MAX 80 
#define CMD_NB 7
#define CHAN_NB 4

//MAX num of clients in a channel
#define MAX_CLI 3


/* useful macro for handling error codes */
#define CHECK(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while (0)

typedef struct client_info
{
	pthread_t thread;
	int fd;
	int thread_nb;
} client_info_t;

/*
typedef struct clients_array
{
	client_info_t* array;
	int size;
	int pos;
	int start_pos;
} client_array_t;
*/

// typedef struct crt_client
// {
// 	client_array_t *c_array;
// 	int position;
// } crt_client_t;

typedef struct body_msg_t
{
	char type[10];
	char body[MAX];
	char name[MAX];
	char descr[MAX];
} body_msg_t;

typedef struct channel_t
{
	int id;
	int nb_clients;
	sem_t mutex;
	char* c_name;
	char* c_descr;
} channel_t;

void parse_msg(char* raw_msg, body_msg_t* msg);

// client_array_t* init_array(client_array_t* client_array);

// void add_client(client_array_t* c_array, int fd);

// void finish_clients(client_array_t* c_array);

// void start_client(client_array_t* clients_array, int client_id);

// void deinit_array(client_array_t* client_array);
