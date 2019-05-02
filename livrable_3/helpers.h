#include <stdio.h>
#include <stdlib.h>

// Maximum length of the message
#define MAX 80 
#define RCV_DIR "recv/"
#define SEND_DIR "send/"

// fin, msg text
#define TXT_MSG 1
#define TXT_FIS 2
#define CTRL_PIP 3 //port / ip
#define CTRL_FILE 4


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
	void *other;
} param_send_t;


typedef struct msg {
	char* msg1;
	char* msg2;
	int type;
} msg_t;

typedef struct cli_info {
	int port;
	char IP[20];
	int type;
} cli_info_t;

/*
	type = 1: Control message port/ip
*/
// void create_main_threads();

// void join_main_threads();

FILE* file_exists(char* file_name);

void send_chunk(char* buffer, int fd);

int recv_chunk(char* buffer, int fd);

int fsize(FILE *fp);

void get_content(char* source, char* dest);

void serial_msg(char* msg, char* buffer, int type);

// void *receive_msg(void* fd);

// void *send_msg(void* fd);

// void *send_file(void *fd_p);

// void recv_file(int fd);

void list_dir();

int file_exist (char *file_name);

int is_file_msg(char* message);

int is_fin_msg(char* message);

void serial_msg(char* msg, char* buffer, int type);