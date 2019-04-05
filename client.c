#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 

#define MAX 80 
//#define PORT 44947 
#define IP "127.0.0.1"
#define PORT 8080
#define SA struct sockaddr 

void func(int sockfd) 
{ 
    char buff[MAX]; 
    int n; 
    while (1) { 
        bzero(buff, sizeof(buff)); 
        printf("Send the string: "); 
        n = 0; 
        while ((buff[n++] = getchar()) != '\n');
 
        send(sockfd, buff, sizeof(buff), 0); 
 
        bzero(buff, sizeof(buff)); 
 
        recv(sockfd, buff, sizeof(buff), 0); 
        printf("From Server : %s\n", buff); 

        if ((strncmp(buff, "exit", 4)) == 0) { 
            printf("Client Exit...\n"); 
            break; 
        } 
    } 
} 
  
int main() 
{

	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd != -1) {
		printf("Success creating client socket\n");
	} else {
		printf("Failed creating client socket\n");
	}

	bzero(&servaddr, sizeof(servaddr)); 

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	//servaddr.sin_addr.s_addr = inet_addr(IP); 

	servaddr.sin_port = htons(PORT); 

	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("Cannot connect to the server\n");
	}

	func(sockfd); 

	close(sockfd); 
} 
