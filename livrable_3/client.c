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
	create_main_threads(&sockfd);
	// for (int i = 0; i < 2; ++i)
	// {
	// 	return_val = pthread_join(threads[i], 0);
	// 	CHECK(return_val != 0, "Failed to join thread");
	// }
	join_main_threads();

	return_val = close(sockfd);
	CHECK(return_val != 0, "Fail closing socket");
} 
