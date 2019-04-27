#include <stdio.h>
#include <stdlib.h>

// Maximum length of the message
#define MAX 80 
#define RCV_DIR "recv/"
#define SEND_DIR "send/"

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

typedef struct param_send {
	int fd;
	char *file_name;
} param_send_t;


void create_main_threads();

void join_main_threads();

int fsize(FILE *fp);

void *receive_msg(void* fd);

void *send_msg(void* fd);

void *send_file(void *fd_p);

void recv_file(int fd);

void list_dir();

int file_exist (char *file_name);