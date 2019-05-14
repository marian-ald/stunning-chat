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

// char channels[CHAN_NB][MAX] = {
// 	"Dota2 - \"Dota2 players\"",
// 	"LOL - \"LOL players\"",
// 	"animals - \"Nat Geo Wild rocks\"",
// 	"got - \"the best series\""};

channel_t *channels;
l_node* clients = NULL;
int nb_channels, nb_clients;

void init_channels(int n)
{
	channels = (channel_t*)malloc(n * sizeof(channel_t));
	CHECK(channels == NULL, "Fail to alloc memory");

	for (int i = 0; i < n; ++i)
	{
		channels[i].id = i;
		channels[i].nb_clients = 0;
		sem_init(&channels[i].mutex, 0, 1);
		channels[i].c_name = NULL;
		channels[i].c_descr = NULL;
	}
}

int channels_dim(int n) {
	int dim = 0;
	char buffer[MAX];

	for (int i = 0; i < n; ++i)
	{
		sprintf(buffer, "%d", channels[i].id);
		printf("dim id%d = %d\n", i,  strlen(buffer));
		dim += strlen(buffer);
		if (channels[i].c_name != NULL)
		{
			dim += strlen(channels[i].c_name);
			dim += strlen(channels[i].c_descr);
		}
	}
	return dim;
}

// int find_pos()
// {

// }


void print_cli(l_node* client)
{
	printf("====================================\n");
	printf("key = %d\n", client->key);
	printf("channel_id = %d\n", client->channel_id);
	printf("fd = %d\n", client->fd);
}

void print_chan(int id) {
	printf("channel_id = %d\n", id);
	printf("nb_clients = %d\n", channels[id].nb_clients);
}


/* If client */
void add_cli_to_channel(int channel_id, l_node* client, char* buf)
{
	// l_node* client = find(clients, cli_id);
	if (client == NULL)
	{
		printf("Client nod found\n");
	}
	else
	{
		// print_cli(client);

		// printf("client has id = %d\n", client->key);
		if (client->channel_id != -1)
		{
			sprintf(buf, "You are in channel %d. Please exit first.\n",
				client->channel_id);
		}
		else if (channels[channel_id].nb_clients == nb_clients)
		{
			sprintf(buf, "Channel %d is already full\n", channel_id);
		}
		else
		{
			sprintf(buf, "Welcome to channel %d!\n", client->channel_id);
			client->channel_id = channel_id;
			channels[channel_id].nb_clients++;
		}
		// printf("after add\n");
		// print_cli(client);
	}
}


void rm_cli_from_channel(l_node* client, char* buf)
{
	if (client->channel_id == -1)
	{
		sprintf(buf, "You are not in any channel \n");
	}
	else
	{
		// print_cli(client);
		// printf("..............\n");
		// print_chan(client->channel_id);
		sprintf(buf, "You disconnected from channel %d.\n", client->channel_id);
		channels[client->channel_id].nb_clients--;
		client->channel_id = -1;


		// printf("----------------after rm\n");
		// print_cli(client);
		// printf("..............\n");
		// print_chan(client->channel_id);

	}

}

void send_msg_channel(l_node* client, char *msg)
{
	int return_val;
	l_node* aux = clients;

	while (aux != NULL)
	{
		if (aux->channel_id == client->channel_id &&
			aux->key != client->key)
		{
			return_val = send(aux->fd, msg, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");
		}
		aux = aux->next;
	}
}

void list_channels(int fd_client)
{
	char send_buf[MAX];
	int return_val;

	for (int i = 0; i < nb_channels; ++i)
	{
		if (channels[i].c_name == NULL)
		{
			sprintf(send_buf, "id:%d  name:\"\"  nb_cli:%d/%d  descr:\"\"\n",
				channels[i].id, channels[i].nb_clients, nb_clients);
		}
		else
		{
			sprintf(send_buf, "id:%d  name:%s  nb_cli=%d/%d  descr: %s\n", channels[i].id,
			channels[i].c_name, channels[i].nb_clients, nb_clients, channels[i].c_descr);
		}
	
		return_val = send(fd_client, send_buf, MAX, 0);
		CHECK(return_val < 0, "Server fails sending message to client");

	}
}

void remove_channel(int id)
{

}



void* recv_send(void* cli)
{

	// crt_client_t* crt_client = (crt_client_t*)c_array_v;
	// client_array_t* c_array = crt_client->c_array;
	// l_node* client = (l_node*)cli;
	int key = *(int*)cli;
	char recv_buf[MAX];
	char send_buf[MAX];
	int return_val, i;
	printf("key is %d\n", key);
	l_node* client = find(clients, key);
	
	if (client==NULL)
	{
		printf("null\n");
	}

	int fd_client = client->fd;
	body_msg_t msg;
	int channel_id = -1;
	int edit_channel_id = -1;
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

		if (strcmp(recv_buf, "\\list") == 0)
		{

			// printf("dim canale %d\n", channels_dim(nb_channels));
			// int c_dim = channels_dim(nb_channels) + 3 * nb_channels; // "1-name-descr-...." 3x'-' + dimens

			// char blank[1] = "";

			sprintf(send_buf, "Channels list:\n");
			return_val = send(fd_client, send_buf, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");
			list_channels(fd_client);

			continue;
		}
		if (strncmp(recv_buf, "join", 4) == 0)
		{
			parse_msg(recv_buf, &msg);
			channel_id = atoi(msg.body);
			printf("Client vrea in channel %d\n", channel_id);

			if (channel_id >= nb_channels)
			{	
				sprintf(send_buf, "%d is not a valid channel id\n", channel_id);
			}
			sem_wait(&channels[channel_id].mutex);
			add_cli_to_channel(channel_id, client, send_buf);
			sem_post(&channels[channel_id].mutex);

			return_val = send(fd_client, send_buf, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");
			continue;
		}
		if (strncmp(recv_buf, "\\exit", 4) == 0)
		{
			printf("Client vrea sa iasa \n");

			sem_wait(&channels[channel_id].mutex);

			rm_cli_from_channel(client, send_buf);

			sem_post(&channels[channel_id].mutex);

			return_val = send(fd_client, send_buf, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");
			return 0;
		}
		if (strncmp(recv_buf, "\\fin", 4) == 0)
		{
			printf("Client %d has stopped.\n", client->key);
			sem_wait(&channels[channel_id].mutex);
			rm_cli_from_channel(client, send_buf);
			sem_post(&channels[channel_id].mutex);
			return_val = close(fd_client);
			CHECK(return_val < 0, "Server fails sending message to client");
			return 0;
		}
		if (strncmp(recv_buf, "\\rm_c", 5) == 0)
		{
			parse_msg(recv_buf, &msg);
			int rm_channel_id = atoi(msg.body);
			printf("Client rm channel %d\n", rm_channel_id);

		}

		send_msg_channel(client, recv_buf);

	}
	return 0;
}


int main(int argc, char* argv[]) 
{
	char buffer[MAX];
	int sockfd;
	socklen_t len_cli;
	int i = 0, return_val;
	struct sockaddr_in servaddr, cli;

	//client_array_t *client_array;
	clients = NULL;

	if (argc < 4)
	{
		printf("Args: port, nb chaines, nb_clients/chaine\n");	
		exit(0);
	}
	nb_channels = atoi(argv[2]);
	nb_clients = atoi(argv[3]);
	printf("nb_channels = %d\n", nb_channels);


	init_channels(nb_channels);

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


	while (1)
	{

		/* Send a hello message to clients after their connection */
		int fd_client = accept(sockfd, (struct sockaddr*)&cli, &len_cli);

		clients = add_first(clients, 0, fd_client);
		clients->key = i;
		clients->channel_id = -1;
		sprintf(buffer, "Hello, client! You are connected to the server\n");
		printf("1\n");
		int return_val = send(fd_client, buffer, strlen(buffer), 0); 	

		printf("2\n");
		CHECK(return_val < 0, "Server fails sending hello message to client");
		
		if (clients == NULL) {
			printf("noooool\n");
		}

		return_val = pthread_create(&(clients->thread), 0, recv_send, &clients->key);
		// sleep(1);
		++i;
		printf("3\n");

		CHECK(return_val != 0, "Fail to create thread");




	}

	// Close the server's socket for accepting clients
	return_val = close(sockfd); 
	CHECK(return_val != 0, "Fail closing socket");
} 

