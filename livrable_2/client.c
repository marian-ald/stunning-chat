#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>

#include "helpers.h"

#define MAX 80 

#define IP "127.0.0.1"
#define PORT 8080
#define SA struct sockaddr 
 

/* function
 * Reads a message from keyboard into the buffer and sends to
 * the channel defined by sockfd
*/
void type_and_send(char* buffer, int dimens, int sockfd) {
	int pos = 0;
	int return_val;

	printf("You> ");
	while ((buffer[pos++] = getchar()) != '\n');
	return_val = send(sockfd, buffer, dimens, 0); 
	CHECK(return_val < 0, "Error receiving message from server");
}


int CHECK_fin_message(char* message) {
	if (strncmp(message, "fin", 3) == 0)
	{
		printf("Client has stopped\n");
		return 1;
	}
	return 0;
}

int main(int argc, char* argv[]) 
{

	char buff_send[MAX];
	char buff_recv[MAX];

	int sockfd; 
	int return_val;
	int stop_chat = 0;

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

	return_val = connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
	CHECK(return_val != 0, "Cannot connect to the server\n");

	// Client is waiting for hello message from server
	bzero(buff_recv, MAX);
	return_val = recv(sockfd, buff_recv, MAX, 0); 
	CHECK(return_val < 0, "Error receiving the message from server");

    printf("From server: %s\n", buff_recv);

    while (!stop_chat) {
		// Client is waiting for a message from server
		bzero(buff_recv, MAX);
		return_val = recv(sockfd, buff_recv, MAX, 0); 
		CHECK(return_val < 0, "Error receiving the message from server");

	    printf("From server> %s\n", buff_recv);

	    // If "fin" message is received, stop the chat
		stop_chat = CHECK_fin_message(buff_recv);
	    if (!stop_chat) {

			// Type a message to send to the server
			type_and_send(buff_send, MAX, sockfd);
			stop_chat = CHECK_fin_message(buff_send);
		}
	}

	close(sockfd); 
} 
