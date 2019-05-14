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
		strcpy(channels[i].c_name, "None");
		strcpy(channels[i].c_descr, "None");
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
		if (channel_id >= nb_channels || channels[channel_id].id == -1)
		{	
			sprintf(buf, "%d is not a valid channel id\n", channel_id);
		}
		else if (client->channel_id != -1)
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
			sprintf(buf, "Welcome to channel %d!\n", channel_id);
			client->channel_id = channel_id;
			channels[channel_id].nb_clients++;
		}
	}
}


void rm_cli_from_channel(l_node* client, char* buf)
{
	printf("Inainte de stergere channel_id = %d\n", client->channel_id);
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
		printf("channels[i].id = %d\n", channels[i].id);
		if (channels[i].id > -1)
		{

		// if (channels[i].c_name == NULL)
		// {
		// 	sprintf(send_buf, "id:%d  name:\"\"  nb_cli:%d/%d  descr:\"\"\n",
		// 		channels[i].id, channels[i].nb_clients, nb_clients);
		// }
		// else
		// {
			sprintf(send_buf, "id:%d  name:%s  nb_cli=%d/%d  descr: %s", channels[i].id,
			channels[i].c_name, channels[i].nb_clients, nb_clients, channels[i].c_descr);
	
			return_val = send(fd_client, send_buf, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");
		}
	}
}

int valid_channel(int id, char *buf)
{
	if (id > nb_channels || channels[id].id == -1)
	{
		sprintf(buf, "This channel does not exist.\n");
		return 0;
	}
	return 1;
}

void remove_channel(l_node* client, int id)
{
	char send_buf[MAX];
	int return_val;
			printf("5\n");

	if (valid_channel(id, send_buf))
	{
			printf("6\n");

		if (channels[id].nb_clients > 0)
		{
			printf("7\n");

			sprintf(send_buf, "This channel is not empty, wait for the users to exit.\n");
		}
		else
		{
			printf("8\n");

			channels[id].id = -1;
			printf("val=%d\n", channels[id].id);
			sprintf(send_buf, "Channel %d has been removed.\n", id);
			printf("9\n");

		}
	}
	return_val = send(client->fd, send_buf, MAX, 0);
	CHECK(return_val < 0, "Server fails sending message to client");
}

void edit_channel(l_node* client, int id, body_msg_t* msg)
{
	char send_buf[MAX];
	int return_val;

	if (valid_channel(id, send_buf))
	{
		strcpy(channels[id].c_name, msg->name);
		strcpy(channels[id].c_descr, msg->descr);
		sprintf(send_buf, "\nChannel %d has been succesfully edited.\n", id);
	}
	// else printf("not validddddddddddddddddd\n");

	return_val = send(client->fd, send_buf, MAX, 0);
	CHECK(return_val < 0, "Server fails sending message to client");
}

int add_channel(l_node* client, body_msg_t* msg)
{
	char send_buf[MAX];
	int return_val;

	for (int i = 0; i < nb_channels; ++i)
	{
		if (channels[i].id == -1)
		{
			channels[i].id = i;
			strcpy(channels[i].c_name, msg->name);
			strcpy(channels[i].c_descr, msg->descr);
			return i;
		}
	}
	channels = (channel_t*)realloc(channels, 2 * nb_channels * sizeof(channel_t));
	channels[nb_channels].id = nb_channels;
	return_val = nb_channels;
	strcpy(channels[nb_channels].c_name, msg->name);
	strcpy(channels[nb_channels].c_descr, msg->descr);

	for (int i = nb_channels + 1; i < 2 * nb_channels; ++i)
	{
		channels[i].id = -1;
	}
	nb_channels *= 2;
	return return_val;
}



void* recv_send(void* cli)
{

	// crt_client_t* crt_client = (crt_client_t*)c_array_v;
	// client_array_t* c_array = crt_client->c_array;
	// l_node* client = (l_node*)cli;
	int key = *(int*)cli;
	char recv_buf[MAX];
	char send_buf[MAX];
	int return_val;
	printf("key is %d\n", key);
	l_node* client = find(clients, key);
	
	if (client==NULL)
	{
		printf("null\n");
	}

	int fd_client = client->fd;
	body_msg_t msg;
	int channel_id = -1;
	// int edit_channel_id = -1;
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
			if (channel_id == -1)
			{
				sprintf(send_buf, "You are not connected to a channel.\n");
				return_val = send(fd_client, send_buf, MAX, 0);
				CHECK(return_val < 0, "Server fails sending message to client");
				continue;
			}
			sem_wait(&channels[channel_id].mutex);

			rm_cli_from_channel(client, send_buf);
			sem_post(&channels[channel_id].mutex);
			channel_id = -1;
			return_val = send(fd_client, send_buf, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");
			continue;
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
		if (strncmp(recv_buf, "rm_c", 4) == 0)
		{
			parse_msg(recv_buf, &msg);
			int rm_channel_id = atoi(msg.body);
			printf("1\n");
			printf("Client rm channel %d\n", rm_channel_id);
			printf("2\n");
			sem_wait(&channels[rm_channel_id].mutex);
			printf("3\n");
			remove_channel(client, rm_channel_id);

			sem_post(&channels[rm_channel_id].mutex);
		}
		else if (strncmp(recv_buf, "edit_c", 6) == 0)
		{
			parse_msg(recv_buf, &msg);
			int edit_channel_id = atoi(msg.body);
			printf("nume = %s\n", msg.name);
			printf("descr = %s\n", msg.descr);
			printf("Client edit channel %d\n", edit_channel_id);
			sem_wait(&channels[edit_channel_id].mutex);
			edit_channel(client, edit_channel_id, &msg);

			sem_post(&channels[edit_channel_id].mutex);
		}
		else if (strncmp(recv_buf, "add_c", 5) == 0)
		{
			parse_msg(recv_buf, &msg);
			printf("nume = %s\n", msg.name);
			printf("descr = %s\n", msg.descr);

			// sem_wait(&channels[edit_channel_id].mutex);
			return_val = add_channel(client, &msg);
			sprintf(send_buf, "Successfully created channel %d.\n", return_val);
			return_val = send(fd_client, send_buf, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");
			continue;
			// sem_post(&channels[edit_channel_id].mutex);
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
		sprintf(buffer, "\nHello, client! You are connected to the server\n");
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

