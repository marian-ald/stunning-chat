#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "helpers.h"

pthread_t threads[2];


void serial_cli_info(cli_info_t *cli_info, char* buffer, int type) {
	sprintf(buffer, "%d%d\n%s", type, cli_info->port, cli_info->IP);
}


/* 	The function receives a mesage from a client and send it to the other
	The thread with thread_nb = 0:
		recv from socket 0
		send to socket 1
		cancels thread_id = 1
*/
void* recv_send(void *arg_thread)
{
	arg_thread_t* thread_info = (arg_thread_t *)arg_thread;
	int *fd_client_int = thread_info->fd;
	// The socket where the current thread writes to
	int write_to = thread_info->thread_nb;
	int return_val;
	char buffer[MAX];

	while (1) {
		return_val = recv(fd_client_int[write_to], buffer, MAX, 0);
		CHECK(return_val < 0, "Server fails receiving message from client");

		return_val = send(fd_client_int[1 - write_to], buffer, MAX, 0); 
		CHECK(return_val < 0, "Server fails sending message to client");

		if (is_file_msg(buffer))
		{
			/* Send the IP/port to the source client */
			serial_msg(thread_info->cli_info.IP, buffer, CTRL_IP);

			return_val = send(fd_client_int[write_to], buffer, MAX, 0); 
			CHECK(return_val < 0, "Server fails sending message to client");
			
			continue;
		}

		if (is_fin_msg(buffer))
		{
            pthread_cancel(threads[1 - write_to]);
			break;
		}
	}
	return 0;
}


int main(int argc, char* argv[]) 
{
	char buffer[MAX];

	int *fd_client;
	int sockfd;
	socklen_t len_cli;
	int return_val;
	struct sockaddr_in serv_addr, cli; 
	arg_thread_t* arg_thread = NULL;
	int i;	
	cli_info_t cli_info[2];

	if (argc < 2)
	{
		printf("Args: parameter for server's port is missing\n");	
		exit(0);
	}
	/* 	Open the server's socket on which it accepts connections
		from clients   */
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	CHECK(sockfd <= 0,"Fail creating client socket\n");

	bzero(&serv_addr, sizeof(serv_addr)); 

	/* Set server's details: IPV4, address, port */
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(atoi(argv[1])); 
	bzero(&(serv_addr.sin_zero),8); 

	/* Associate a port to the server's socket */
	return_val = bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr));
	CHECK(return_val < 0, "Failed to bind");

	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} else {
		printf("Server listening..\n"); 
	}

	/* Allocate space for the 2 file descriptors*/
	fd_client = (int *)malloc(2 * sizeof(int));
	CHECK(fd_client == NULL, "Malloc failed");

	/* Allocate space for the argument structure needed by pthread_create
	 arg_thread = (arg_thread_t *)malloc(2 * sizeof(arg_thread_t)); */
	arg_thread = malloc(NB_CLIENTS * sizeof(arg_thread_t));
	CHECK(arg_thread == NULL, "Malloc failed");

	for (i = 0; i < NB_CLIENTS; ++i) {
		arg_thread[i].fd = fd_client;
		arg_thread[i].thread_nb = i;
	}

	len_cli = sizeof(struct sockaddr_in);
	while (1)
	{
		/* The server waits the connection of 2 clients */
		for (int i = 0; i < NB_CLIENTS; ++i)
		{
			fd_client[i] = accept(sockfd, (struct sockaddr*)&cli, &len_cli);

	    	printf("\nServer got a connection from (%s , %d)\n", inet_ntoa(cli.sin_addr),ntohs(cli.sin_port));

	    	cli_info[i].port = ntohs(cli.sin_port);
			strcpy(cli_info[i].IP, inet_ntoa(cli.sin_addr));
			arg_thread[i].cli_info = cli_info[0];

			if (fd_client[i] < 0)
			{ 
				printf("Server fails accepting client_%d\n", i + 1); 
				exit(0); 
			} else {
				printf("Server acccepts client_%d\n", i + 1);	
		  	}
		  	if (i == 0) {
				/* Send hello message to client1 */
				sprintf(buffer, "%d-Hello, client! Please wait for the other client to connect\n", TXT_MSG); 
				return_val = send(fd_client[i], buffer, strlen(buffer), 0); 
				CHECK(return_val < 0, "Server fails sending hello message to client");
			}
		}

		for (int i = 0; i < NB_CLIENTS; ++i)
		{
			/* Notify clients that they can start chatting */
			sprintf(buffer, "%d-START_CHAT\n", TXT_MSG); 
			return_val = send(fd_client[i], buffer, strlen(buffer), 0); 
			CHECK(return_val < 0, "Server fails sending start_chat");
		}
		/*
		Create 2 threads:
			- the first thread receives from client 1 and sends to client 2
			- the second thread receives from client 2 and sends to client 1
		*/
		for (int i = 0; i < NB_CLIENTS; ++i)
		{
			return_val = pthread_create(&threads[i], 0, recv_send, (void *)(arg_thread) + i * sizeof(arg_thread_t));
			CHECK(return_val != 0, "Failed to create thread");
		}

		/* Join the 2 threads */
		for (int i = 0; i < NB_CLIENTS; ++i)
		{
			return_val = pthread_join(threads[i], 0);
			CHECK(return_val != 0, "Failed to join thread");
		}

		/* Close the two sockets after ending the conversation */
		for (int i = 0; i < NB_CLIENTS; ++i)
		{
			return_val = close(fd_client[i]);
			CHECK(return_val < 0, "Error closing socket for client");
		}

		printf("Clients 1&2 have stopped, waiting for others...\n\n");
	}

	/* Free allocated memory*/
    free(fd_client);
    free(arg_thread);

	/* Close the server's socket for accepting clients*/
	return_val = close(sockfd);
	CHECK(return_val < 0, "Error closing socket");
} 

