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
#include "linked_list.h"

char channels[CHAN_NB][MAX] = {
	"Dota2 - \"Dota2 players\"",
	"LOL - \"LOL players\"",
	"animals - \"Nat Geo Wild rocks\"",
	"got - \"the best series\""};


void* recv_send(void* cli)
{

	// crt_client_t* crt_client = (crt_client_t*)c_array_v;
	// client_array_t* c_array = crt_client->c_array;
	l_node* client = (l_node*)cli;
	char recv_buf[MAX];
	char send_buf[MAX];
	int return_val;
	int fd_client = client->fd;
	body_msg_t msg;

	// int id = crt_client->position;
	// int fd_client = c_array->array[id].fd;

	// sprintf(buffer, "Start chat\n"); 	
	// return_val = send(c_array->array[id].fd, buffer, strlen(buffer), 0); 
	// CHECK(return_val < 0, "Server fails sending start_chat message to client");


	while(1)
	{
		/* Receive a message from a client */
		return_val = recv(fd_client, recv_buf, MAX, 0);
		CHECK(return_val < 0, "Server fails receiving message from client");

		printf("Recv from client: %s\n", recv_buf);

		if (strcmp(recv_buf, "list") == 0)
		{
			sprintf(send_buf, "list-%d", CHAN_NB);

			return_val = send(fd_client, send_buf, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");

			for (int i = 0; i < CHAN_NB; ++i)
			{
				sprintf(send_buf, "%s", channels[i]);
				return_val = send(fd_client, send_buf, MAX, 0);
				CHECK(return_val < 0, "Server fails sending message to client");
			}
			continue;
		}
		if (strncmp(recv_buf, "join", 4) == 0)
		{
			parse_msg(recv_buf, &msg);

		}

		// /* Send the message to the other clients */
		// for (int i = 0; i < c_array->pos; ++i)
		// {
		// 	if (c_array->array[i].thread_nb != id) {
		// 		return_val = pthread_mutex_lock(&lock);
		// 		CHECK(return_val < 0, "Fail locking the mutex");

				// return_val = send(c_array->array[i].fd, buffer, MAX, 0);
				// CHECK(return_val < 0, "Server fails sending message to client");

		// 		return_val = pthread_mutex_unlock(&lock);  
		// 		CHECK(return_val < 0, "Fail unlocking the mutex");
		// 	}
		// }

		// if (strncmp(buffer, "fin", 3) == 0)
		// {
		// 	/* Stop the threads after receiving 'fin' message */
		// 	for (int i = 0; i < c_array->pos; ++i)
		// 	{
		// 		if (c_array->array[i].thread_nb != id)
		// 		{
  //           		pthread_cancel(c_array->array[i].thread);
  //           		CHECK(return_val < 0, "Fail canceling thread");
  //           	}
  //           }
		// 	break;
		// }
	}
	return 0;
}

int main(int argc, char* argv[]) 
{
	char buffer[MAX];
	int sockfd;
	socklen_t len_cli;
	int i, return_val;
	struct sockaddr_in servaddr, cli;
	int nb_clients = 2;
	client_array_t *client_array;
	list_t* clients = (list_t *)malloc(sizeof(list_t));

	int join_is_done = -1;

	if (argc < 2)
	{
		printf("Args: server's port\n");	
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
	bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

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
	// client_array = (client_array_t*)malloc(sizeof(client_array_t));
	// CHECK(client_array == NULL, "Fail allocating memory");

	while (1)
	{
		// client_array = init_array(client_array);
		// printf("\nWaiting for %d clients to connect\n", nb_clients);

		// /* Wait for nb_clients to connect to the server */
		// for (i = 0; i < nb_clients; ++i)
		// {
		// 	/* Send a hello message to clients after their connection */
		int fd_client = accept(sockfd, (struct sockaddr*)&cli, &len_cli);
printf("1\n");
		add_first(clients, 0, fd_client);
printf("2\n");
		sprintf(buffer, "Hello, client! You are connected to the server\n");
printf("3\n");
		int return_val = send(fd_client, buffer, strlen(buffer), 0); 
printf("4\n");		
		CHECK(return_val < 0, "Server fails sending hello message to client");
if (clients == NULL) {
	printf("goooool\n");
}
		return_val = pthread_create(&(clients->first->thread), 0, recv_send, (void *)clients->first);
printf("5\n");
		CHECK(return_val != 0, "Fail to create thread");

		// if (join_is_done) {
		// 	if (join_is_done == 1) {
		// 		// join() de tid
		// 	}
		// }
		// 	printf("Accepted client %d from %d\n", i + 1, nb_clients);

		// 	/* Add information for a client in the array */
		// 	add_client(client_array, fd_client);

		// }
		// /* Send start_chat to each client */
		// for (int i = 0; i < nb_clients; ++i)
		// {
		// 	/* Create a thread for each client */
		// 	start_client(client_array, i);
		// }

		// /* Close the thread and the socket for each client connected to the server */
		// finish_clients(client_array);

		// /* Free allocated memory for the array of clients */
		// deinit_array(client_array);


	}

	// Close the server's socket for accepting clients
	return_val = close(sockfd); 
	CHECK(return_val != 0, "Fail closing socket");
} 

