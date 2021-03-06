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

/* 	List with all available commands and their description
for the user */
char cmds[CMD_NB][MAX] = {
	"\\cmd - list all commands",
	"\\list - show a list with all channels",
	"\\join - join to one of listed channels",
	"\\exit - exit from the channel",
	"\\add_c - add a new channel",
	"\\edit_c - edit a channel",
	"\\rm_c - remove channel"
	};


pthread_t threads[2];
int is_connected = 0;

int check_fin_message(char* message)
{
	if (strncmp(message, "\\fin", 4) == 0)
	{
		printf("Client has been disconnected\n");
		return 1;
	}
	return 0;
}

/* Displaly the user's available commands */
void print_commands() {
	printf("Available commands\n");
	for (int i = 0; i < CMD_NB; ++i)
	{
		printf("%s\n", cmds[i]);
	}
}

/* Read from stdion the name and the description for a channel */
void read_details(char* buffer)
{
	char aux[MAX];

	printf("Introduce channel's name: ");
	fgets(aux, MAX, stdin);
	aux[strlen(aux) - 1] = '-';
	strcat(buffer, aux);
	printf("Introduce channel's description: ");
	fgets(aux, MAX, stdin);
	aux[strlen(aux) - 1] = '\0';
	strcat(buffer, aux);
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
	int ok_exit = 0;

	while (1) {
		/* Read a command from stdin, parse it, and send it to the
		server */
		fgets(buffer, MAX, stdin);
		printf("\n");

		if (strcmp(buffer, "\\cmd\n") == 0)
		{
			print_commands();
			continue;
		}
		if (strcmp(buffer, "\\list\n") == 0)
		{
			buffer[strlen(buffer) - 1] = '\0';
		}
		else if (strcmp(buffer, "\\join\n") == 0)
		{
			printf("Introduce channel's id: ");
			fgets(channel, MAX, stdin);
			channel[strlen(channel) - 1] = '\0';
			sprintf(buffer, "join-%s", channel);
		}
		else if (strcmp(buffer, "\\exit\n") == 0)
		{
			buffer[strlen(buffer) - 1] = '\0';
			is_connected = 0;
		}
		else if (check_fin_message(buffer))
		{
			pthread_cancel(threads[1]);
			ok_exit = 1;
		}
		else if (strcmp(buffer, "\\rm_c\n") == 0)
		{
			printf("Introduce channel's id: ");
			fgets(channel, MAX, stdin);
			channel[strlen(channel) - 1] = '\0';
			sprintf(buffer, "rm_c-%s", channel);
		}
		else if (strcmp(buffer, "\\edit_c\n") == 0)
		{
			printf("Introduce channel's id: ");
			fgets(channel, MAX, stdin);
			channel[strlen(channel) - 1] = '-';
			sprintf(buffer, "edit_c-%s", channel);
			read_details(buffer);
		}
		else if (strcmp(buffer, "\\add_c\n") == 0)
		{
			sprintf(buffer, "add_c-%d-", 0);
			read_details(buffer);
		}
		else
		{
			if (is_connected)
			{
				return_val = send(*fd_server, buffer, MAX, 0);
				CHECK(return_val < 0, "Client fails sending message to server");
			}
			continue;
		}

		return_val = send(*fd_server, buffer, MAX, 0);
		CHECK(return_val < 0, "Client fails sending message to server");

		if (ok_exit) {
			pthread_cancel(threads[1]);
			break;
		}
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
		puts(buffer);

		if (strncmp(buffer, "Welcome", 7) == 0)
		{
			is_connected = 1;
			continue;
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
		int return_val = pthread_join(threads[i], 0);
		CHECK(return_val != 0, "Failed to join thread");
	}

	return_val = close(sockfd);
	CHECK(return_val != 0, "Fail closing socket");
} 
