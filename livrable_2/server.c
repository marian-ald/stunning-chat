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

// Useful defines
#define MAX 80 
#define SA struct sockaddr 

typedef struct arg_thread_t {
	int* fd;
	int thread_nb;
} arg_thread_t;


pthread_t threads[2];

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

		if (strncmp(buffer, "fin", 3) == 0) {
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
	struct sockaddr_in servaddr, cli; 
	arg_thread_t* arg_thread = NULL;
	int i;

	if (argc < 2)
	{
		printf("Args: parameter for server's port is missing\n");	
		exit(0);
	}
	// Open the server's socket on which it accepts connections
	// from clients  
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 

	bzero(&servaddr, sizeof(servaddr)); 

	// Set server's details: IPV4, address, port
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(atoi(argv[1])); 

	// Associate a port to the server's socket
	bind(sockfd, (SA*)&servaddr, sizeof(servaddr));

	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} else {
		printf("Server listening..\n"); 
	}

	// Allocate space for the 2 file descriptors
	fd_client = (int *)malloc(2 * sizeof(int));
	CHECK(fd_client == NULL, "Malloc failed");

	// Allocate space for the argument structure needed by pthread_create
	// arg_thread = (arg_thread_t *)malloc(2 * sizeof(arg_thread_t));
	arg_thread = malloc(2 * sizeof(arg_thread_t));
	CHECK(arg_thread == NULL, "Malloc failed");

	for (i = 0; i < 2; ++i) {
		arg_thread[i].fd = fd_client;
		arg_thread[i].thread_nb = i;
	}

	len_cli = sizeof(len_cli);
	while (1)
	{
		// The server waits the connection of 2 clients
		for (int i = 0; i < 2; ++i)
		{
			fd_client[i] = accept(sockfd, (SA*)&cli, &len_cli);

			if (fd_client[i] < 0)
			{ 
				printf("Server fails accepting client_%d\n", i + 1); 
				exit(0); 
			} else {
				printf("Server acccepts client_%d\n", i + 1);	
		  	}
		  	if (i == 0) {
				// Send hello message to client1
				sprintf(buffer, "Hello, client! Please wait for the other client to connect\n"); 
				return_val = send(fd_client[i], buffer, strlen(buffer), 0); 
				CHECK(return_val < 0, "Server fails sending hello message to client");
			} else {
				// Send start_chat message to client2
				sprintf(buffer, "Hello, client! Type a message\n"); 
				return_val = send(fd_client[i], buffer, strlen(buffer), 0); 
				CHECK(return_val < 0, "Server fails sending hello message to client");
			}
		}

		// Notify client1 that it can start chatting
		sprintf(buffer, "The other client is connected\n"); 
		return_val = send(fd_client[0], buffer, strlen(buffer), 0); 
		CHECK(return_val < 0, "Server fails sending hello message to client");

		/*
		Create 2 threads:
			- the first thread receives from client 1 and sends to client 2
			- the second thread receives from client 2 and sends to client 1
		*/
		for (int i = 0; i < 2; ++i)
		{
			return_val = pthread_create(&threads[i], 0, recv_send, (void *)(arg_thread) + i * sizeof(arg_thread_t));
			CHECK(return_val != 0, "Failed to create thread");
		}

		/* Join the 2 threads */
		for (int i = 0; i < 2; ++i)
		{
			return_val = pthread_join(threads[i], 0);
			CHECK(return_val != 0, "Failed to join thread");
		}

		/* Close the two sockets after ending the conversation */
		for (int i = 0; i < 2; ++i)
		{
			return_val = close(fd_client[i]);
			CHECK(return_val < 0, "Error closing socket for client");
		}

		printf("Clients 1&2 have stopped, waiting for others...\n\n");
	}

	// Free allocated memory
    free(fd_client);
    free(arg_thread);

	// Close the server's socket for accepting clients
	return_val = close(sockfd);
	CHECK(return_val < 0, "Error closing socket");
} 

