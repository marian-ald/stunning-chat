#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include<pthread.h> 

// Maximum length of the message
#define MAX 80
#define CMD_NB 7

/* Sseful macro for handling error codes */
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
	pthread_mutex_t mutex;
	char c_name[MAX];
	char c_descr[MAX];
} channel_t;

void parse_msg(char* raw_msg, body_msg_t* msg);
