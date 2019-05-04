#include <stdio.h>
#include <stdlib.h>

#define NB_CLIENTS 2
#define PORT_FILE 8888
#define NB_WAIT_SEC 0

// Maximum length of the message
#define MAX 80 
#define RCV_DIR "recv/"
#define SEND_DIR "send/"


#define TXT_MSG 1	// fin, msg text
#define TXT_FIS 2
#define CTRL_IP 3 	// IP
#define CTRL_FILE 4
#define CTRL_P_IP 5 // Port+IP


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

typedef struct cli_info {
	int port;
	char IP[20];
	int type;
} cli_info_t;

typedef struct arg_thread_t {
	int* fd;
	int thread_nb;
	cli_info_t cli_info;
} arg_thread_t;

typedef struct param_send {
	int fd;
	char file_name[MAX];
	int thread_nb;
	void *other;
} param_send_t;


typedef struct msg {
	char* msg1;
	char* msg2;
	int type;
} msg_t;


FILE* file_exists(char* file_name);

int choose_files(int nb_files, param_send_t* f_details);

int get_files_nb(char* buff);

void send_chunk(char* buffer, int fd);

int recv_chunk(char* buffer, int fd);

int fsize(FILE *fp);

void get_content(char* source, char* dest);

void serial_msg(char* msg, char* buffer, int type);

void list_dir();

int file_exist (char *file_name);

int is_file_msg(char* message);

int is_fin_msg(char* message);

int is_ctrl(char* message, int ctrl);

void serial_msg(char* msg, char* buffer, int type);

void parse_ip_port(char* buff, cli_info_t* cli);

int choose_files_nb();
