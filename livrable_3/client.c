#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h> 
#include <errno.h>

#include "helpers.h"

#define SA struct sockaddr 


pthread_t* send_threads;
pthread_t* recv_threads;

/* 	Semaphore used to synchronize send/recv msg threads.
	The 'send_msg' thread can not send messages until it receives the ip from server
*/
sem_t sem_IP;

/* Current client IP and listening port for peer to peer transmision */
cli_info_t cli_info;

/* Flag to mark that a set of files are currently trensfering */
int file_is_sending = 0;


void* send_file(void* param_send);
void* recv_file(void* thread_nb);
void* send_msg(void* fd);
void* receive_msg(void* fd);


/*
	Reads a message from keyboard into the buffer and sends to the channel defined
by sockfd.
	When 'file' is typed, the content of the send directory is listed and the user
has to introduce the number of files he wants to send, then the name of each file.
*/
void *send_msg(void* fd)
{
	int *fd_server = (int *)fd;
	int return_val, i;
	char buffer[MAX];
	char buff_ser[MAX];
	int sockfd;
	param_send_t* files_details;

	struct sockaddr_in serv_addr;

	while (1)
	{
		fgets(buffer, MAX, stdin);
		printf("\n");
		/* Check if the user wants to send some files */
		if (strncmp(buffer, "file", 4) == 0 && strlen(buffer) == 5)
		{
			/* If there is not already a transfer in progress, go further. A transfer is also
			when more files are currently sending*/
			if (!file_is_sending)
			{
				list_dir();

				int files_nb = choose_files_nb();

				if (files_nb < 1) {
					continue;
				}
				/* Introduce the file names and check if files exist */
				files_details = (param_send_t *)calloc(files_nb, sizeof(param_send_t));
				CHECK(files_details == NULL, "Fail to alloc memory");

				return_val = choose_files(files_nb, files_details);

				if (return_val == 0)
				{
					continue;
				}

				/* Send the 'file' message to the other client to be prepared
					for receiving
				*/
				sprintf(buffer, "%d-file\n", CTRL_FILE);
				send_chunk(buffer, *fd_server);

				file_is_sending = 1;
				sem_wait(&sem_IP); 

				/* Open the server's socket on which it accepts connections from clients */
				sockfd = socket(AF_INET, SOCK_STREAM, 0);
				CHECK(sockfd < 0, "Fail to create sockfd");

				/* Set server's details: IPV4, address, port */
				serv_addr.sin_family = AF_INET; 
				serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
				serv_addr.sin_port = htons(PORT_FILE); 
				bzero(&(serv_addr.sin_zero),8);

				/* 	Associate a port to the server's socket. If the port is already
				assigned, choose another one, by incrementing the current port */
				int port = PORT_FILE;
				do {
					return_val = bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr));
					CHECK(return_val < 0 && errno != EADDRINUSE, "Failed to bind");
					if (return_val == -1)
					{
						port += 2;
						serv_addr.sin_port = htons(port);
					}
				} while(return_val == -1);

				return_val = listen(sockfd, 5);
				CHECK(return_val != 0, "Listen fails");

		    	cli_info.port = ntohs(serv_addr.sin_port);
 
				/* Send the IP & port to the server->other client for peer-to-peer connection */
				sprintf(buffer, "%d-%s-%d", CTRL_P_IP, cli_info.IP, cli_info.port);
				send_chunk(buffer, *fd_server);

				/* Send the number of files to be waited by the other client */ 
				sprintf(buffer, "%d-%d", CTRL_FILE, files_nb);
				send_chunk(buffer, *fd_server);

				/* 	Allocate space for another files_nb + 1 threads
					- files_nb threads, 1 thread for one file to transfer
					- 1 thread to send messages(because the current one is blocked
						by the sys-call join_thread)
				*/
				send_threads = (pthread_t *)realloc(send_threads, (files_nb + 3) * sizeof(pthread_t));

				return_val = pthread_create(&send_threads[2], 0, send_msg, (void *)fd_server);
				for (i = 0; i < files_nb; ++i)
				{
					files_details[i].fd = sockfd;
					files_details[i].thread_nb = i;
					return_val = pthread_create(&send_threads[i + 3], 0, send_file, (void *)(files_details) + i * sizeof(param_send_t));				
					CHECK(return_val != 0, "Failed to create thread");
					printf("_____Created SEND thread nb=%d from %d_____\n", i+1, files_nb);
				}
				for (i = files_nb + 2; i > 1; --i)
				{
					return_val = pthread_join(send_threads[i], 0);
					CHECK(return_val != 0, "Failed to join thread");					
				}
				free(files_details);

				return_val = close(sockfd);
				CHECK(return_val < 0, "Error closing socket");

				send_threads = (pthread_t *)realloc(send_threads, 2 * sizeof(pthread_t));

				file_is_sending = 0;
			}
			else
			{
				printf("Another file is sending now. Please wait\n");
			}
			continue;
		}

		serial_msg(buffer, buff_ser, TXT_MSG);

		return_val = send(*fd_server, buff_ser, MAX, 0);
		CHECK(return_val < 0, "Client fails sending message to server");

		if (is_fin_msg(buff_ser))
		{
			pthread_cancel(send_threads[1]);
			return 0;
		}
	}
	return 0;
}

/* Function which receives a message from server and print it
*/
void *receive_msg(void* fd)
{
	int *fd_server = (int *)fd;
	int return_val, i;
	char buffer[MAX];
	int files_nb = 0;
	int *thread_nb;

	while (1)
	{
		recv_chunk(buffer, *fd_server);

		if (is_fin_msg(buffer))
		{
			printf("fin\n");
			pthread_cancel(send_threads[0]);
			break;
		}
		if (is_file_msg(buffer))
		{
			recv_chunk(buffer, *fd_server);
			parse_ip_port(buffer, &cli_info);

			printf("_____Client connects to the other client: IP=%s, PORT=%d_____\n", cli_info.IP, cli_info.port);

			/* Receive the number of files */
			recv_chunk(buffer, *fd_server);
			files_nb = get_files_nb(buffer);

			recv_threads = (pthread_t *)realloc(recv_threads, (files_nb + 1) * sizeof(pthread_t));

			return_val = pthread_create(&recv_threads[0], 0, receive_msg, fd);
			CHECK(return_val != 0, "Failed to create thread");

			thread_nb = (int*)calloc(files_nb, sizeof(int));
			for (i = 0; i < files_nb; ++i)
			{
				thread_nb[i] = i;
				return_val = pthread_create(&recv_threads[i + 1], 0, recv_file,
					(void*)(thread_nb) + i * sizeof(int));
				CHECK(return_val != 0, "Failed to create thread");
				printf("_____Created RECV thread %d from %d_____\n", i + 1, files_nb);
			}

			for (i = files_nb; i >= 0; --i)
			{
				return_val = pthread_join(recv_threads[i], 0);
				CHECK(return_val != 0, "Failed to join thread");					
			}
			continue;
		}
		else if (is_ctrl(buffer, CTRL_IP))
		{
			sem_post(&sem_IP);
			strcpy(cli_info.IP, buffer);
			continue;
		}
		/* Display the message received from the server */
		get_content(buffer, buffer);
		printf("Server: ");
		puts(buffer);
	}
	return 0;
}

void *send_file(void* param_send)
{
	param_send_t p_send = *(param_send_t*)param_send;
	FILE* fp = (FILE*)p_send.other;
	int f_size;
	char buffer[MAX];
	char buff_ser[MAX];
	int fd, return_val;
	struct sockaddr_in cli;
	socklen_t len_cli;

	fd = accept(p_send.fd, (struct sockaddr*)&cli, &len_cli);
	CHECK(fd < 0, "Fail accepting client");

	f_size = fsize(fp);

	/* Send the filename to the server */
	serial_msg(p_send.file_name, buffer, CTRL_FILE);
	send_chunk(buffer, fd);

	/* Send the number of chunks of the file to the server */
	sprintf(buffer, "%d", f_size);
	serial_msg(buffer, buff_ser, CTRL_FILE);
	send_chunk(buff_ser, fd);

	while (fgets(buffer, MAX, fp) != NULL)
	{
		/* Send each chunk of the file to the server */
		serial_msg(buffer, buff_ser, CTRL_FILE);
		sleep(NB_WAIT_SEC);
		send_chunk(buff_ser, fd);
	}
	printf("_____File successfully sent_____\n\n");

	return_val = fclose(fp);
	CHECK(return_val != 0, "Fail to close file stream");

	return_val = close(fd);
	CHECK(return_val < 0, "Error closing socket");

	if (p_send.thread_nb == 0) {
		pthread_cancel(send_threads[2]);
	}

	return NULL;
}

void* recv_file(void* thread_nb)
{
	FILE* fp = NULL;
	int thread_id = *(int*)thread_nb;
	int f_size, return_val;
	int sockfd;
	char *file_name;
	char buffer[MAX];
	struct sockaddr_in servaddr; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK(sockfd <= 0,"Fail creating client socket\n");

	bzero(&servaddr, sizeof(servaddr)); 

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(cli_info.IP);
	servaddr.sin_port = htons(cli_info.port); 

	return_val = connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
	CHECK(return_val != 0, "Cannot connect to the server\n");

	/* Receive the file name */
	recv_chunk(buffer, sockfd);
	get_content(buffer, buffer);

	/* Create the full path to recv directory */
	file_name = (char *)malloc(sizeof(RCV_DIR) + strlen(buffer));
	sprintf(file_name, "%s%s", RCV_DIR, buffer);
	
	/* Open destination file */
	fp = fopen(file_name, "w");
	CHECK(fp == NULL, "Failed to open destination file");

	/* Receive the file dimension */
	recv_chunk(buffer, sockfd);
	get_content(buffer, buffer);
	f_size = atoi(buffer);

	/* Receive the content while the size already received is > 0,
		and decrement the number of bytes at each step
	*/ 
	while (f_size > 0) {
		int nb_B_recv = recv_chunk(buffer, sockfd);
		get_content(buffer, buffer);
		sleep(NB_WAIT_SEC);
		f_size -= nb_B_recv;
		fputs(buffer, fp);
	}
	
	printf("_____File successfully received_____\n");
	return_val = fclose(fp);
	CHECK(return_val != 0, "Fail to close file stream");

	return_val = close(sockfd);
	CHECK(return_val < 0, "Error closing socket");

	if (thread_id == 0) {
		pthread_cancel(recv_threads[0]);
		CHECK(return_val != 0, "Fail to cancel thread");
	}
	return 0;
}

int main(int argc, char* argv[])
{
	char buff_recv[MAX];
	int sockfd; 
	int return_val;
	struct sockaddr_in servaddr; 
	send_threads = (pthread_t *)calloc(2, sizeof(pthread_t));

	if (argc < 3)
	{
		printf("Args: parameter for server's ip&port is missing\n");	
		exit(0);
	}

	sem_init(&sem_IP, 0, 0);

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
	recv_chunk(buff_recv, sockfd);

	printf("Server: %s\n", buff_recv);

	/* Create 2 threads:
		- one to receive messages
		- one to send messages
	*/
	// create_main_threads(&sockfd);
	return_val = pthread_create(&send_threads[0], 0, send_msg, (void *)&sockfd);
	CHECK(return_val != 0, "Failed to create thread");

	return_val = pthread_create(&send_threads[1], 0, receive_msg, (void *)&sockfd);
	CHECK(return_val != 0, "Failed to create thread");

	for (int i = 0; i < 2; ++i)
	{
		int return_val = pthread_join(send_threads[i], 0);
		CHECK(return_val != 0, "Failed to join thread");
	}

	sem_destroy(&sem_IP); 

	return_val = close(sockfd);
	CHECK(return_val != 0, "Fail closing socket");
	free(send_threads);
} 
