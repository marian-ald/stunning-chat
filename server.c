#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 
  

void loop(int sockfd) 
{ 
    char buff[MAX]; 
	char buff_send[MAX]; 
	    
	int n;


    while (1) { 
        bzero(buff, MAX); 
  
        recv(sockfd, buff, sizeof(buff), 0); 


		printf("From client:%s\t To client: ", buff); 
        bzero(buff_send, MAX); 
        //n = 0;
        //while ((buff[n++] = getchar()) != '\n'); 
	sprintf(buff_send, "Size of the string is: %d", strlen(buff) - 1); 
	//printf("Trimit catre client:  %s\n", buff_send); 

        send(sockfd, buff_send, sizeof(buff_send), 0); 
  
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    } 
} 

int main() 
{ 
	char buff_send_client1[MAX];
	char buff_recv_from_client1[MAX];

	pid_t pid;
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
  
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 

    bzero(&servaddr, sizeof(servaddr)); 
  
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    bind(sockfd, (SA*)&servaddr, sizeof(servaddr));

    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } else
        printf("Server listening..\n"); 
    len = sizeof(cli); 
  	
	//while (1) {
	
	// Client_1  asks for connection 
	connfd = accept(sockfd, (SA*)&cli, &len);

	if (connfd < 0) { 
	    printf("server acccepts failed...\n"); 
	    exit(0); 
	} else {
	    printf("server acccepts the 1st client...\n");	
		// The server sends an ACK to Client_1
		
  	}
	//	pid = fork();

	//}	

    loop(connfd); 
    close(sockfd); 
} 

