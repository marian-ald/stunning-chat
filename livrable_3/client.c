#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "helpers.h"

#define SA struct sockaddr 


pthread_t threads[2];
// pthread_t threads[6];
int file_is_sending = 0;

void *send_file(void* param_send);
void recv_file(int fd);
void *send_msg(void* fd);
void *receive_msg(void* fd);


void join_main_threads() {
	for (int i = 0; i < 2; ++i)
	{
		int return_val = pthread_join(threads[i], 0);
		CHECK(return_val != 0, "Failed to join thread");
	}
}

/*
 * Reads a message from keyboard into the buffer and sends to
 * the channel defined by sockfd
*/
void *send_msg(void* fd)
{
	int *fd_server = (int *)fd;
	int return_val;
	char buffer[MAX];
	char file_name[MAX];
	char buff_ser[MAX];
	char mm[10] = "mamam\0";
	printf("%s|len=%d\n", mm, strlen(mm));

	while (1)
	{

		fgets(buffer, MAX, stdin);
		printf("\n");

		if (strncmp(buffer, "file", 4) == 0 && strlen(buffer) == 5)
		{
			// todo pornesc thread nou send
			// list_dir();
			// fgets(buffer, MAX, stdin);
			// buffer[strlen(buffer) - 1] = '\0';

			// send_file(fd_server);
			if (!file_is_sending)
			{




				list_dir();
				fgets(file_name, MAX, stdin);
				file_name[strlen(file_name) - 1] = '\0';
				FILE* fp = file_exists(file_name);
				if (fp == NULL)
				{
					continue;
				}
				else
				{
					sprintf(buffer, "%d-file\n", CTRL_FILE);
					send_chunk(buffer, *fd_server);
				}
				file_is_sending = 1;
				param_send_t p_send;
				p_send.fd = *fd_server;
				p_send.file_name = file_name;
				p_send.other = fp;


				return_val = pthread_create(&threads[2], 0, send_msg, (void *)fd_server);
				return_val = pthread_create(&threads[3], 0, send_file, (void *)(&p_send));

				// printf("!!!!! am creat 2 threaduri noi\n" );
				return_val = pthread_join(threads[2], 0);
				CHECK(return_val != 0, "Failed to join thread");
				return_val = pthread_join(threads[3], 0);
				CHECK(return_val != 0, "Failed to join thread");
				// printf("!!!!! am facut join la cele 2 threaduri noi\n" );

				file_is_sending = 0;
			}
			else
			{
				printf("Another file is sending now. Please wait\n");
			}
			continue;
		}

		serial_msg(buffer, buff_ser, TXT_MSG);
		printf("Client trimite la S:%s| lung=%d\n", buff_ser, strlen(buff_ser));

		return_val = send(*fd_server, buff_ser, MAX, 0);
		CHECK(return_val < 0, "Client fails sending message to server");

		// printf("before C l175\n");
		if (is_fin_msg(buff_ser))
		{
			pthread_cancel(threads[1]);
			return 0;
		}
		// printf("after C l181\n");

	}
	
	return 0;
}

/* Function which received a message from server and print it
*/
void *receive_msg(void* fd)
{
	int *fd_server = (int *)fd;
	int return_val;
	char buffer[MAX];

	while (1)
	{
		return_val = recv(*fd_server, buffer, MAX, 0);
		// get_content(buffer, buffer);
		CHECK(return_val < 0, "Client fails receiving message from server");

		if (is_fin_msg(buffer))
		{
			printf("fin\n");
			pthread_cancel(threads[0]);
			break;
		}
		if (is_file_msg(buffer))
		{
			// todo pornesc thread nou rcv
			printf("_______Receive file_______\n");
			recv_file(*fd_server);
		}
		else
		{
			get_content(buffer, buffer);
			printf("Server: ");
			puts(buffer);
		}
	}
	return 0;
}

void *send_file(void* param_send)
{
	param_send_t p_send = *(param_send_t*)param_send;
	FILE* fp = (FILE*)p_send.other	;
	int f_size;
	int fd = p_send.fd;
	char buffer[MAX];
	char buff_ser[MAX];

	// char* path_file;


	/* 	List all the files from the source directory and choose a file
		to send. Replace last character of the filename '\n' with '\0'
	*/
	// list_dir();
	// fgets(file_name, MAX, stdin);
	// file_name[strlen(file_name) - 1] = '\0';

	// path_file = (char*)malloc(sizeof(SEND_DIR) + strlen(p_send.file_name));
	// sprintf(path_file, "%s%s", SEND_DIR, p_send.file_name);

	// /* Test if the file exists in the directory */
	// fp = fopen(path_file, "r");
	// if (fp == NULL) {
	// 	// printf("len |%s| nume %d\n", file_name, strlen(file_name));
	// 	printf("______ File %s does not exist ______\n", p_send.file_name);
	// 	return NULL;
	// }
	f_size = fsize(fp);


	/* Send the filename to the server */
	serial_msg(p_send.file_name, buffer, CTRL_FILE);
	send_chunk(buffer, fd);



	/* Send the number of chunks of the file to the server */
	sprintf(buffer, "%d", f_size);
	serial_msg(buffer, buff_ser, CTRL_FILE);

	// itoa(nb_chunks, buffer, 10);
	send_chunk(buff_ser, fd);


	// todo in loc de print fac send
	// printf("nume fisier: %s\n", p_send.file_name);
	// printf("size file = %d\n", f_size);

	// printf("nb pachete = %f\n", (float)f_size / MAX);
	while (fgets(buffer, MAX, fp) != NULL) {
		// printf("%s\n", buffer);
		/* Send each chunk of the file to the server */
		serial_msg(buffer, buff_ser, CTRL_FILE);
		send_chunk(buff_ser, fd);
		// sleep(5);
		// printf("trimit:%s\n", buffer);
	}
	printf("______ File successfully sent ______\n");
	// free(path_file);
	fclose(fp);
	pthread_cancel(threads[2]);
	return NULL;
}

void recv_file(int fd)
{
	FILE* fp = NULL;
	int f_size;
	char *file_name;
	char buffer[MAX];

	recv_chunk(buffer, fd);

	printf("buffer before:%s\n", buffer);

	get_content(buffer, buffer);
	printf("buffer after:%s\n", buffer);

	file_name = (char *)malloc(sizeof(RCV_DIR) + strlen(buffer));
	sprintf(file_name, "%s%s", RCV_DIR, buffer);
	// printf("file_name is %s\n", );
	// printf("path fisier:  %s\n", file_name);

	/* Open destination file */
	fp = fopen(file_name, "w");
	CHECK(fp == NULL, "Failed to open destination file");
	// printf("fp = %d\n", fp);

	recv_chunk(buffer, fd);
	get_content(buffer, buffer);

	printf("dim fisier=|%s|\n", buffer);
	f_size = atoi(buffer);

	printf("f_size = %d\n", f_size);


	while (f_size > 0) {
		// printf("%s\n", buffer);
		int nb_B_recv = recv_chunk(buffer, fd);
		get_content(buffer, buffer);

		fputs(buffer, fp);
		f_size -= nb_B_recv;
		// printf("f_size = %d ||||| buffer = %s\n", f_size, buffer);

	}
	printf("______ File successfully received ______\n");

	fclose(fp);
}


int main(int argc, char* argv[])
{

	char buff_recv[MAX];
	int sockfd; 
	int return_val;
	struct sockaddr_in servaddr; 

	if (argc < 3)
	{
		printf("Args: parameter for server's ip&port is missing\n");	
		exit(0);
	}


	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd != -1)
	{
		printf("Success creating client socket\n");
	} else {
		printf("Failed creating client socket\n");
	}

	bzero(&servaddr, sizeof(servaddr)); 

	servaddr.sin_family = AF_INET; 
	// argv[1] is the server's ip
	servaddr.sin_addr.s_addr = inet_addr(argv[1]); 
	// argv[2] is the server's port on which it is listening
	servaddr.sin_port = htons(atoi(argv[2])); 

	return_val = connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
	CHECK(return_val != 0, "Cannot connect to the server\n");

	// Client is waiting for hello message from server
	bzero(buff_recv, MAX);
	return_val = recv(sockfd, buff_recv, MAX, 0); 
	CHECK(return_val < 0, "Error receiving the message from server");

	printf("Server: %s\n", buff_recv);

	/* Create 2 threads:
		- one to receive messages
		- one to send messages
	*/
	// create_main_threads(&sockfd);
	return_val = pthread_create(&threads[0], 0, send_msg, (void *)&sockfd);
	CHECK(return_val != 0, "Failed to create thread");

	return_val = pthread_create(&threads[1], 0, receive_msg, (void *)&sockfd);
	CHECK(return_val != 0, "Failed to create thread");

	// for (int i = 0; i < 2; ++i)
	// {
	// 	return_val = pthread_join(threads[i], 0);
	// 	CHECK(return_val != 0, "Failed to join thread");
	// }
	join_main_threads();

	return_val = close(sockfd);
	CHECK(return_val != 0, "Fail closing socket");
} 
