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


char cmds[CMD_NB][MAX] = {
	"\\cmd - list all commands",
	"\\list - show a list with all channels",
	"\\join - join to one of listed channels",
	"\\exit - exit from the channel"};



pthread_t threads[2];


int CHECK_fin_message(char* message)
{
	if (strncmp(message, "fin", 3) == 0)
	{
		printf("Client has stopped\n");
		return 1;
	}
	return 0;
}

void print_commands() {
	printf("Available commands\n");
	for (int i = 0; i < CMD_NB; ++i)
	{
		printf("%s\n", cmds[i]);
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
	char channel[MAX];
	// int in_channel = 0;

	while (1) {

		fgets(buffer, MAX, stdin);
		printf("\n");

		if (strcmp(buffer, "cmd\n") == 0)
		{
			print_commands();
			continue;
		}
		if (strcmp(buffer, "list\n") == 0)
		{
			buffer[strlen(buffer) - 1] = '\0';
		}
		else if (strcmp(buffer, "join\n") == 0)
		{
			printf("Introduce channel's name:");
			fgets(channel, MAX, stdin);
			channel[strlen(channel) - 1] = '\0';
			sprintf(buffer, "join-%s", channel);
		}
		else if (strcmp(buffer, "exit\n") == 0)
		{
			// if (!in_channel)
			// {
			// 	printf("Not in a channel\n");
			// 	continue;
			// }
			// else
			// {
				buffer[strlen(buffer) - 1] = '\0';
			// }
		}
		else
		{
			continue;
		}

		return_val = send(*fd_server, buffer, MAX, 0);
		CHECK(return_val < 0, "Client fails sending message to server");

		// if (CHECK_fin_message(buffer)) {
		// 	pthread_cancel(threads[1]);
		// 	break;
		// }
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

	while (1) {
		return_val = recv(*fd_server, buffer, MAX, 0); 
		CHECK(return_val < 0, "Client fails receiving message from server");
		printf("Server: ");
		puts(buffer);

		if (CHECK_fin_message(buffer)) {
			pthread_cancel(threads[0]);
			break;
		}
	}
	return 0;
}


int main(int argc, char* argv[])
{
	int sockfd; 
	int return_val;
	struct sockaddr_in servaddr; 

	if (argc < 3)
	{
		printf("Args: parameter for server's ip&port is missing\n");	
		exit(0);
	}


	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd != -1) {
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

	return_val = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	CHECK(return_val != 0, "Cannot connect to the server\n");

	// // Client is waiting for hello message from server
	// bzero(buff_recv, MAX);
	// return_val = recv(sockfd, buff_recv, MAX, 0); 
	// CHECK(return_val < 0, "Error receiving the message from server");

	// printf("Server: %s\n", buff_recv);

	print_commands();

	/*
	Create 2 threads:
		- the first thread reads from keyboard and sends to server->client 2
		- the second thread receives from client 2 and sends to server->client 1
	*/
	return_val = pthread_create(&threads[0], 0, send_msg, (void *)(&sockfd));
	CHECK(return_val != 0, "Failed to create thread");

	return_val = pthread_create(&threads[1], 0, receive_msg, (void *)(&sockfd));
	CHECK(return_val != 0, "Failed to create thread");

	for (int i = 0; i < 2; ++i)
	{
		return_val = pthread_join(threads[i], 0);
		CHECK(return_val != 0, "Failed to join thread");
	}

	return_val = close(sockfd);
	CHECK(return_val != 0, "Fail closing socket");
} 
