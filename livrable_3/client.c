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

#include "helpers.h"

#define SA struct sockaddr 


pthread_t* threads;
pthread_t* recv_threads;

sem_t sem_IP;

// Current client IP and listening port for peer to peer transmision
cli_info_t cli_info;

// pthread_t threads[6];
int file_is_sending = 0;


void* send_file(void* param_send);
void* recv_file();
void* send_msg(void* fd);
void* receive_msg(void* fd);


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
	int return_val, i;
	char buffer[MAX];
	char file_name[MAX];
	char buff_ser[MAX];
	char **files_names;
	int sockfd;
	param_send_t* files_details;

	struct sockaddr_in serv_addr;

	while (1)
	{
		fgets(buffer, MAX, stdin);
		printf("\n");
		if (strncmp(buffer, "file", 4) == 0 && strlen(buffer) == 5)
		{
			// todo pornesc thread nou send
			if (!file_is_sending)
			{

				list_dir();

				// printf("______Introduce the NUMBER of files______\n-----> ");
				// fgets(buffer, MAX, stdin);
				// int files_nb = atoi(buffer);
				// if (!files_nb) {
				// 	printf("Invalid number of files\n");
				// }
				int files_nb = choose_files_nb();

				// files_names = calloc(nb, sizeof(char *));
				// for (i = 0; i < files_nb; ++i) {
				// 	files_names[i] = calloc(MAX, sizeof(char));
				// }

				if (files_nb < 1) {
					continue;
				}
				/* Introduce the file names and test if files exist */
				files_details = (param_send_t *)calloc(files_nb, sizeof(param_send_t));

				return_val = choose_files(files_nb, files_details);
				if (return_val == 0)
				{
					continue;
				}

				// fgets(file_name, MAX, stdin);
				// file_name[strlen(file_name) - 1] = '\0';
				// FILE* fp = file_exists(file_name);


				// if (fp == NULL)
				// {
				// 	continue;
				// }
				// else
				// {
				/* Send the 'file' message to the other client to be prepared
					for receiving
				*/
				sprintf(buffer, "%d-file\n", CTRL_FILE);
				send_chunk(buffer, *fd_server);

				/* Send the number of files to be waited by the other client */ 
				sprintf(buffer, "%d-%d", CTRL_FILE, files_nb);
				send_chunk(buffer, *fd_server);
					
				// }
				file_is_sending = 1;
				// param_send_t p_send;
				// p_send.fd = *fd_server;
				// // p_send.file_name = file_name;
				// strcpy(p_send.file_name, file_name);
				// p_send.other = fp;

				printf("Semafor intrare\n");
				sem_wait(&sem_IP); 
				printf("Semafor iesire\n");

				// Open the server's socket on which it accepts connections
				// from clients  
				sockfd = socket(AF_INET, SOCK_STREAM, 0); 
				// bzero(&serv_addr, sizeof(serv_addr)); 

				// Set server's details: IPV4, address, port
				serv_addr.sin_family = AF_INET; 
				serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
				serv_addr.sin_port = htons(PORT_FILE); 
				bzero(&(serv_addr.sin_zero),8);

				// Associate a port to the server's socket
				return_val = bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr));
				CHECK(return_val < 0, "Failed to bind");

				return_val = listen(sockfd, 5);
				CHECK(return_val != 0, "Listen fails");
				printf("Client listening for file transfer connections\n"); 

		    	// printf("\n Client listens on (%s , %d)", inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));
		    	printf("\n Client listens on (%s , %d)", serv_addr.sin_addr, ntohs(serv_addr.sin_port));
		    	cli_info.port = ntohs(serv_addr.sin_port);
 
				/* Send the IP & port to the server->other client for peer-to-peer connection */
				sprintf(buffer, "%d-%s-%d", CTRL_P_IP, cli_info.IP, cli_info.port);
				send_chunk(buffer, *fd_server);

				/* Alloca space for another files_nb + 1 threads
					- files_nb threads, 1 thread for one file to transfer
					- 1 thread to send messages(because the current one is blocked
						by the sys-call join_thread)
				*/
				threads = (pthread_t *)realloc(threads, (files_nb + 3) * sizeof(pthread_t));

				return_val = pthread_create(&threads[2], 0, send_msg, (void *)fd_server);
				for (i = 0; i < files_nb; ++i)
				{
					files_details[i + 3].fd = sockfd;
					return_val = pthread_create(&threads[i + 3], 0, send_file, (void *)files_details);				
					CHECK(return_val != 0, "Failed to create thread");
					printf("Created thread nb=%d from %d\n", i+1, files_nb);
				}
				for (i = files_nb + 2; i > 1; --i)
				{
					return_val = pthread_join(threads[i], 0);
					printf("Inchid thread %d\n", i);
					CHECK(return_val != 0, "Failed to join thread");					
				}

				// printf("!!!!! am creat 2 threaduri noi\n" );
				// return_val = pthread_join(threads[2], 0);
				// CHECK(return_val != 0, "Failed to join thread");
				// return_val = pthread_join(threads[3], 0);
				// CHECK(return_val != 0, "Failed to join thread");
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

		// return_val = send(*fd_server, buff_ser, MAX, 0);
		// CHECK(return_val < 0, "Client fails sending message to server");
		send_chunk(buff_ser, *fd_server);

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
	int return_val, i;
	char buffer[MAX];
	int files_nb = 0;

	while (1)
	{
		recv_chunk(buffer, *fd_server);

		if (is_fin_msg(buffer))
		{
			printf("fin\n");
			pthread_cancel(threads[0]);
			break;
		}
		if (is_file_msg(buffer))
		{
			// todo pornesc thread nou rcv
			recv_chunk(buffer, *fd_server);
			printf("Got from server IP = %s\n", buffer);
			parse_ip_port(buffer, &cli_info);

			/* Receive the number of files */
			files_nb = recv_chunk(buffer, *fd_server);
			recv_threads = (pthread_t *)realloc(recv_threads, (files_nb + 1) * sizeof(pthread_t));

			return_val = pthread_create(&recv_threads[0], 0, receive_msg, fd);
			for (i = 0; i < files_nb; ++i)
			{
				return_val = pthread_create(&recv_threads[i], 0, recv_file, 0);				
				CHECK(return_val != 0, "Failed to create thread");
				// printf("Created thread nb=%d from %d\n", i+1, files_nb);
			}

			for (i = files_nb; i >= 0; --i)
			{
				return_val = pthread_join(recv_threads[i], 0);
				printf("Inchid thread recv file %d\n", i);
				CHECK(return_val != 0, "Failed to join thread");					
			}
			// printf("Got from server IP = %s\n", buffer);

			// printf("_______Receive file_______\n");
			// recv_file(*fd_server);
			continue;
		}
		else if (is_ctrl(buffer, CTRL_IP))
		{
			printf("sleep 4s\n");
			sleep(4);
			sem_post(&sem_IP);
			printf("Got from server IP = %s\n", buffer);
			strcpy(cli_info.IP, buffer);
			continue;
		}
		// else if (is_ctrl(buffer, CTRL_P_IP))
		// {
		// 	// sem_post(&sem_IP);
		// 	printf("Got from server IP and port = %s\n", buffer);
		// 	printf("%s\n", buffer);
		// 	recv_chunk(buffer, *fd_server);
		// 	// strcpy(cli_info.IP, buffer);
		// 	continue;
		// }

		// if is PIP CTRL_P_IP
		get_content(buffer, buffer);
		printf("Server: ");
		puts(buffer);

	}
	return 0;
}

void *send_file(void* param_send)
{
	param_send_t p_send = *(param_send_t*)param_send;
	FILE* fp = (FILE*)p_send.other	;
	int f_size;
	// int fd = p_send.fd;
	char buffer[MAX];
	char buff_ser[MAX];
	int fd;
	struct sockaddr_in cli;
	socklen_t len_cli;


	fd = accept(p_send.fd, (struct sockaddr*)&cli, &len_cli);

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
// goto label;
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
	printf("______ File successfully sent ______\n\n");
	// free(path_file);
	fclose(fp);
	pthread_cancel(threads[2]);
// label:
	// printf("fd = %d\n", fd);

	return NULL;
}

void* recv_file()
{
	FILE* fp = NULL;
	int f_size, return_val;
	int sockfd;
	char *file_name;
	char buffer[MAX];
	struct sockaddr_in servaddr; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd != -1)
	{
		printf("Success creating client socket\n");
	} else {
		printf("Failed creating client socket\n");
	}

	bzero(&servaddr, sizeof(servaddr)); 

	printf("setez ip si port conexiune\n");
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(cli_info.IP);
	servaddr.sin_port = htons(cli_info.port); 


	return_val = connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
	CHECK(return_val != 0, "Cannot connect to the server\n");


	/* Receive the file name */
	recv_chunk(buffer, sockfd);

	// printf("buffer before:%s\n", buffer);
	get_content(buffer, buffer);
	// printf("buffer after:%s\n", buffer);

	file_name = (char *)malloc(sizeof(RCV_DIR) + strlen(buffer));
	sprintf(file_name, "%s%s", RCV_DIR, buffer);
	

	/* Open destination file */
	fp = fopen(file_name, "w");
	CHECK(fp == NULL, "Failed to open destination file");


	/* Receive the file dimension */
	recv_chunk(buffer, sockfd);
	get_content(buffer, buffer);

	printf("dim fisier=|%s|\n", buffer);
	f_size = atoi(buffer);

	printf("f_size = %d\n", f_size);


	while (f_size > 0) {
		// printf("%s\n", buffer);
		int nb_B_recv = recv_chunk(buffer, sockfd);
		get_content(buffer, buffer);

		fputs(buffer, fp);
		f_size -= nb_B_recv;
		// printf("f_size = %d ||||| buffer = %s\n", f_size, buffer);

	}
	//todo close sockfd
	printf("______ File successfully received ______\n");
	fclose(fp);
	return 0;
}

int main(int argc, char* argv[])
{
	char buff_recv[MAX];
	int sockfd; 
	int return_val;
	struct sockaddr_in servaddr; 
	threads = (pthread_t *)calloc(2, sizeof(pthread_t));

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

	sem_destroy(&sem_IP); 

	return_val = close(sockfd);
	CHECK(return_val != 0, "Fail closing socket");
} 
