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

/* Vector with all the channels and their information */
channel_t *channels;

/* List with the clients connected to the server */
l_node* clients = NULL;

/* Mutex to cinchronize the access to the list of clients */
pthread_mutex_t mutex_clients;

/* Mutex to cinchronize the access to the entire vector of
channels */
pthread_mutex_t mutex_channels;

int nb_channels, nb_clients;

/* Allocate memory for 'n' channels and initialize them */
void init_channels(int n)
{
	int return_val;
	channels = (channel_t*)malloc(n * sizeof(channel_t));
	CHECK(channels == NULL, "Fail to alloc memory");

	for (int i = 0; i < n; ++i)
	{
		channels[i].id = i;
		channels[i].nb_clients = 0;
		return_val = pthread_mutex_init(&channels[i].mutex, NULL);
		CHECK(return_val != 0, "Fail to init mutex");
		strcpy(channels[i].c_name, "None");
		strcpy(channels[i].c_descr, "None");
	}
}

/* A client connects to a channel. Increment the number of clients in
	the channel and add the channel's id in the client's struct
*/
void add_cli_to_channel(int channel_id, l_node* client, char* buf)
{
	if (client == NULL)
	{
		printf("Client nod found\n");
	}
	else
	{
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

/* 	A client disconnects from a channel.
	Decrement the number of clients in the channel.
*/
void rm_cli_from_channel(l_node* client, char* buf)
{
	sprintf(buf, "You disconnected from channel %d.\n", client->channel_id);
	channels[client->channel_id].nb_clients--;
	client->channel_id = -1;
}

/* A client close its session with the server */
void exit_cli_from_channel(l_node* client)
{
	int return_val = close(client->fd);
	CHECK(return_val != 0, "Fail closing socket");

	channels[client->channel_id].nb_clients--;
	clients = rm_node(clients, client->key);
}

/* 	Broadcast a message to all clients connected to the same channel
	as the sender of the message */
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

/* Print the channels and their information */
void list_channels(int fd_client)
{
	char send_buf[MAX];
	int return_val;

	for (int i = 0; i < nb_channels; ++i)
	{
		/* Ih the current channel is not closed */
		if (channels[i].id > -1)
		{
			sprintf(send_buf, "id:%d  name:%s  nb_cli=%d/%d  descr: %s", channels[i].id,
			channels[i].c_name, channels[i].nb_clients, nb_clients, channels[i].c_descr);
	
			return_val = send(fd_client, send_buf, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");
		}
	}
}

/* Check if the channel's id is valid */
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

	if (valid_channel(id, send_buf))
	{
		if (channels[id].nb_clients > 0)
		{
			sprintf(send_buf, "This channel is not empty, wait for the users to exit.\n");
		}
		else
		{
			channels[id].id = -1;
			sprintf(send_buf, "Channel %d has been removed.\n", id);
		}
	}
	return_val = send(client->fd, send_buf, MAX, 0);
	CHECK(return_val < 0, "Server fails sending message to client");
}


/* Update the channel's information by modifying its name and
	description */
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
	return_val = send(client->fd, send_buf, MAX, 0);
	CHECK(return_val < 0, "Server fails sending message to client");
}

/* Add a new channel in the vector */
int add_channel(l_node* client, body_msg_t* msg)
{
	int return_val;

	/* Navigate through the vector and check if there's a free
		space, otherwise allocate new memory */
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

	/* Allocate memory, by doubling the current vector's dimension */
	channels = (channel_t*)realloc(channels, 2 * nb_channels * sizeof(channel_t));
	CHECK(channels == NULL, "Fail to allocate memory");

	/* Add the information for the new channel: name, description,
		number of clients */
	channels[nb_channels].id = nb_channels;
	channels[nb_channels].nb_clients = 0;
	return_val = nb_channels;
	strcpy(channels[nb_channels].c_name, msg->name);
	strcpy(channels[nb_channels].c_descr, msg->descr);

	/* Mark the invalid new channels*/
	for (int i = nb_channels + 1; i < 2 * nb_channels; ++i)
	{
		channels[i].id = -1;
	}
	nb_channels *= 2;
	return return_val;
}


/* 	Function which runs for each client connected to the server on
different threads */
void* recv_send(void* cli)
{
	int key = *(int*)cli;
	char recv_buf[MAX];
	char send_buf[MAX];
	int return_val;
	l_node* client = find(clients, key);
	

	int fd_client = client->fd;
	body_msg_t msg;
	int channel_id = -1;
	while(1)
	{
		/* Receive a message from a client */
		return_val = recv(fd_client, recv_buf, MAX, 0);
		CHECK(return_val < 0, "Server fails receiving message from client");

		if (strcmp(recv_buf, "\\list") == 0)
		{
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
			/* Wait the access to the resource */
			pthread_mutex_lock(&mutex_clients);
			pthread_mutex_lock(&channels[channel_id].mutex);

			add_cli_to_channel(channel_id, client, send_buf);
			/* Free the access to the resource */
			pthread_mutex_unlock(&mutex_clients);
			pthread_mutex_unlock(&channels[channel_id].mutex);

			return_val = send(fd_client, send_buf, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");

			continue;
		}
		if (strncmp(recv_buf, "\\exit", 4) == 0)
		{
			if (channel_id == -1)
			{
				sprintf(send_buf, "You are not connected to a channel.\n");
				return_val = send(fd_client, send_buf, MAX, 0);
				CHECK(return_val < 0, "Server fails sending message to client");
				continue;
			}
			pthread_mutex_lock(&channels[channel_id].mutex);
			rm_cli_from_channel(client, send_buf);
			pthread_mutex_unlock(&channels[channel_id].mutex);
			channel_id = -1;

			return_val = send(fd_client, send_buf, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");

			continue;
		}
		if (strncmp(recv_buf, "\\fin", 4) == 0)
		{
			/* If the client is not connected  to a channel, just
			remove client's information, else update also channel's
			information */
			if (channel_id == -1)
			{
				clients = rm_node(clients, client->key);
			}
			else
			{
				pthread_mutex_lock(&mutex_clients);
				pthread_mutex_lock(&channels[channel_id].mutex);

				exit_cli_from_channel(client);

				pthread_mutex_unlock(&mutex_clients);
				pthread_mutex_unlock(&channels[channel_id].mutex);
			}
			return 0;
		}
		if (strncmp(recv_buf, "rm_c", 4) == 0)
		{
			parse_msg(recv_buf, &msg);
			int rm_channel_id = atoi(msg.body);

			pthread_mutex_lock(&channels[rm_channel_id].mutex);
			remove_channel(client, rm_channel_id);
			pthread_mutex_unlock(&channels[rm_channel_id].mutex);
			continue;
		}
		else if (strncmp(recv_buf, "edit_c", 6) == 0)
		{
			parse_msg(recv_buf, &msg);
			int edit_channel_id = atoi(msg.body);

			pthread_mutex_lock(&channels[edit_channel_id].mutex);
			edit_channel(client, edit_channel_id, &msg);
			pthread_mutex_unlock(&channels[edit_channel_id].mutex);
		}
		else if (strncmp(recv_buf, "add_c", 5) == 0)
		{
			parse_msg(recv_buf, &msg);

			pthread_mutex_lock(&mutex_channels);
			return_val = add_channel(client, &msg);
			pthread_mutex_unlock(&mutex_channels);

			sprintf(send_buf, "Successfully created channel %d.\n", return_val);
			return_val = send(fd_client, send_buf, MAX, 0);
			CHECK(return_val < 0, "Server fails sending message to client");
			continue;
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
	clients = NULL;

	if (argc < 4)
	{
		printf("Args: port, nb chaines, nb_clients/chaine\n");	
		exit(0);
	}
	nb_channels = atoi(argv[2]);
	nb_clients = atoi(argv[3]);

	init_channels(nb_channels);

	/* Initialize mutex */
	return_val = pthread_mutex_init(&mutex_clients, NULL);
	CHECK(return_val != 0, "Fail to init mutex");

	return_val = pthread_mutex_init(&mutex_channels, NULL);
	CHECK(return_val != 0, "Fail to init mutex");

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 

	/* Add server's connecting information */
	bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(atoi(argv[1])); 

	/* Associate a port to the server's socket */
	bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} else {
		printf("Server listening..\n"); 
	}

	len_cli = sizeof(len_cli);

	while (1)
	{
		/* Send a hello message to clients after their connection */
		int fd_client = accept(sockfd, (struct sockaddr*)&cli, &len_cli);

		/* For each client connected to the server add a node in the clients list */
		pthread_mutex_lock(&mutex_clients);
		clients = add_first(clients, 0, fd_client);
		clients->key = i;
		clients->channel_id = -1;
		pthread_mutex_unlock(&mutex_clients);
		
		/* Send a 'hello' message to the client */
		sprintf(buffer, "\nHello, client! You are connected to the server\n");
		return_val = send(fd_client, buffer, strlen(buffer), 0); 	
		CHECK(return_val < 0, "Server fails sending hello message to client");

		return_val = pthread_create(&(clients->thread), 0, recv_send, &clients->key);
		CHECK(return_val != 0, "Fail to create thread");
		++i;
	}
	/* Free allocated resources */
	free(channels);
    pthread_mutex_destroy(&mutex_channels); 
    pthread_mutex_destroy(&mutex_clients); 

	// Close the server's socket for accepting clients
	return_val = close(sockfd); 
	CHECK(return_val != 0, "Fail closing socket");
} 

