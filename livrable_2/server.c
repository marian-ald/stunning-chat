#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "helpers.h"

#define MAX 80 

#define SA struct sockaddr 



int main(int argc, char* argv[]) 
{ 
    char buffer[MAX];

    int fd_client[2];
    int sockfd;
    socklen_t len_cli;
    int return_val;
    int crt_client;
    int stop_chat = 0;
    struct sockaddr_in servaddr, cli; 

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

    len_cli = sizeof(len_cli);
    while (1) {
    	// Client_1  asks for connection 
    	fd_client[0] = accept(sockfd, (SA*)&cli, &len_cli);

    	if (fd_client[0] < 0) { 
    	    printf("Server fails accepting client_1\n"); 
    	    exit(0); 
    	} else {
    	    printf("Server acccepts client_1\n");	
      	}

        // Send hello message to client1
        sprintf(buffer, "Hello, client1! Please wait for client2 to connect\n"); 
        return_val = send(fd_client[0], buffer, strlen(buffer), 0); 
        CHECK(return_val < 0, "Server fails sending hello message to client 1");

        //////////////////////////////////////////////////////////////////////

        // Client_2  asks for connection 
        fd_client[1] = accept(sockfd, (SA*)&cli, &len_cli);

        if (fd_client[1] < 0) { 
            printf("Server fails accepting client_2\n"); 
            exit(0); 
        } else {
            printf("Server acccepts client_2\n");   
        }

        // Send hello message to client2
        sprintf(buffer, "Hello, client2\n"); 
        return_val = send(fd_client[1], buffer, strlen(buffer), 0); 
        CHECK(return_val < 0, "Server fails sending hello message to client 2");

        /////////////////////////////////////////////////////////////////////

        // Notify client1 that he can start chattig
        sprintf(buffer, "START_CHAT\n"); 
        return_val = send(fd_client[0], buffer, strlen(buffer), 0); 
        CHECK(return_val < 0, "Server fails sending hello message to client 1");


        // Loop:
        // receive a message from client1
        // send it to client1
        // receive a message from client2
        // send it to client 1

        crt_client = 0;
        stop_chat = 0;

        while (!stop_chat) {
            return_val = recv(fd_client[crt_client], buffer, MAX, 0);
            CHECK(return_val < 0, "Server fails receiving message from client 1");

            return_val = send(fd_client[(crt_client + 1) % 2], buffer, strlen(buffer), 0); 
            CHECK(return_val < 0, "Server fails sending message to client 2");

            crt_client = (crt_client + 1) % 2;

            // If "fin" message is received, close the 2 sockets
            if (strncmp(buffer, "fin", 3) == 0) {
                return_val = close(fd_client[0]);
                CHECK(return_val < 0, "Error closing socket for client1");

                return_val = close(fd_client[1]);
                CHECK(return_val < 0, "Error closing socket for client2");

                stop_chat = 1;
            }

        }
        printf("Clients 1&2 have stopped, waiting for others...\n\n");
    }

    // Close the server's socket for accepting clients
    close(sockfd); 
} 

