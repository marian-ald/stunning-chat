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
#define SA struct sockaddr 


int main(int argc, char* argv[]) 
{
	char buffer[MAX];
	int sockfd;
	socklen_t len_cli;
	int i, return_val;
	struct sockaddr_in servaddr, cli;
	int nb_clients = 2;
	client_array_t *client_array;

	if (argc < 2)
	{
		printf("Args: server's port(and Nb_clients - optional)\n");	
		exit(0);
	}
	if (argc == 3) {
		nb_clients = atoi(argv[2]);
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

	len_cli = sizeof(len_cli);

	/* 	Allocate memory for  the structure which contains the information
		about clients
	*/
	client_array = (client_array_t*)malloc(sizeof(client_array_t));
	CHECK(client_array == NULL, "Fail allocating memory");

	while (1)
	{
		client_array = init_array(client_array);
		printf("\nWaiting for %d clients to connect\n", nb_clients);

		/* Wait for nb_clients to connect to the server */
		for (i = 0; i < nb_clients; ++i)
		{
			/* Send a hello message to clients after their connection */
			int fd_client = accept(sockfd, (SA*)&cli, &len_cli);
			sprintf(buffer, "Hello, client! Please wait for the other client to connect\n"); 	
			int return_val = send(fd_client, buffer, strlen(buffer), 0); 
			CHECK(return_val < 0, "Server fails sending hello message to client");

			printf("Accepted client %d from %d\n", i + 1, nb_clients);

			/* Add information for a client in the array */
			add_client(client_array, fd_client);

		}
		/* Send start_chat to each client */
		for (int i = 0; i < nb_clients; ++i)
		{
			/* Create a thread for each client */
			start_client(client_array, i);
		}

		/* Close the thread and the socket for each client connected to the server */
		finish_clients(client_array);

		/* Free allocated memory for the array of clients */
		deinit_array(client_array);
	}

	// Close the server's socket for accepting clients
	return_val = close(sockfd); 
	CHECK(return_val != 0, "Fail closing socket");
} 

