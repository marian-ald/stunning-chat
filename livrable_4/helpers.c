#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h> 
#include <string.h>

#include "helpers.h"

pthread_mutex_t lock;

/* The function receives a message from a client and the send
	a brodcast of that message to all other s */
// void* recv_send(void* c_array_v)
// {

// 	// crt_client_t* crt_client = (crt_client_t*)c_array_v;
// 	// client_array_t* c_array = crt_client->c_array;

// 	char buffer[MAX];
// 	int return_val;
// 	int fd_client;
// 	// int id = crt_client->position;
// 	// int fd_client = c_array->array[id].fd;

// 	// sprintf(buffer, "Start chat\n"); 	
// 	// return_val = send(c_array->array[id].fd, buffer, strlen(buffer), 0); 
// 	// CHECK(return_val < 0, "Server fails sending start_chat message to client");


// 	while(1)
// 	{
// 		/* Receive a message from a client */
// 		return_val = recv(fd_client, buffer, MAX, 0);
// 		CHECK(return_val < 0, "Server fails receiving message from client");

// 		printf("Recv from client: \n");
// 		// /* Send the message to the other clients */
// 		// for (int i = 0; i < c_array->pos; ++i)
// 		// {
// 		// 	if (c_array->array[i].thread_nb != id) {
// 		// 		return_val = pthread_mutex_lock(&lock);
// 		// 		CHECK(return_val < 0, "Fail locking the mutex");

// 		// 		return_val = send(c_array->array[i].fd, buffer, MAX, 0);
// 		// 		CHECK(return_val < 0, "Server fails sending message to client");

// 		// 		return_val = pthread_mutex_unlock(&lock);  
// 		// 		CHECK(return_val < 0, "Fail unlocking the mutex");
// 		// 	}
// 		// }

// 		// if (strncmp(buffer, "fin", 3) == 0)
// 		// {
// 		// 	/* Stop the threads after receiving 'fin' message */
// 		// 	for (int i = 0; i < c_array->pos; ++i)
// 		// 	{
// 		// 		if (c_array->array[i].thread_nb != id)
// 		// 		{
//   //           		pthread_cancel(c_array->array[i].thread);
//   //           		CHECK(return_val < 0, "Fail canceling thread");
//   //           	}
//   //           }
// 		// 	break;
// 		// }
// 	}
// 	return 0;
// }

void parse_msg(char* raw_msg, body_msg_t* msg) {
	char deserial[MAX];
	strcpy(deserial, raw_msg);
	char* type = strtok(deserial, "-");

 	strcpy(msg->type, type);

 	type = strtok(NULL, "-");
 	strcpy(msg->body, type); 	
}


// /* Initialize the struct for the array of clients */
// client_array_t* init_array(client_array_t* client_array)
// {
// 	client_array->array = (client_info_t *)malloc(sizeof(client_info_t));
// 	client_array->size = 1;
// 	client_array->pos = 0;

// 	int return_val = pthread_mutex_init(&lock, NULL);
// 	CHECK(return_val < 0, "Error to init the mutex");

// 	return client_array;
// }


// void deinit_array(client_array_t* client_array)
// {
// 	free(client_array->array);
// 	int return_val = pthread_mutex_destroy(&lock);
// 	CHECK(return_val < 0, "Error to destroy the mutex");
// }

// /* Add the information for each client in the array */
// void add_client(client_array_t* c_array, int fd)
// {
// 	/* If the array is full, double its dimension */
// 	if (c_array->size == c_array->pos) {
// 		c_array->array = realloc(c_array->array, 2 * c_array->size * sizeof(client_info_t));
// 		c_array->size *= 2;
// 	}

// 	/* Allocate space for the pthread_t object in the client_info */
// 	c_array->array[c_array->pos].fd = fd;
// 	c_array->array[c_array->pos].thread_nb = c_array->pos;
// 	c_array->pos++;
// }

// // /* Create a thread for each client connected to the server */
// // void start_client(client_array_t* c_array, int client_id)
// // {
// // 	crt_client_t* crt_client;
// // 	int return_val;

// // 	crt_client = (crt_client_t *)malloc(sizeof(crt_client_t));
// // 	CHECK(crt_client == NULL, "Fail allocating memory");


// // 	crt_client->c_array = c_array;
// // 	crt_client->position = client_id;
// // 	c_array->start_pos = client_id;

// // 	return_val = pthread_create(&c_array->array[client_id].thread, 0, recv_send, NULL);
// // 	CHECK(return_val != 0, "Fail to create thread");
// // }

// void finish_clients(client_array_t* c_array)
// {
// 	int return_val;
// 	/* Join the thread and close the socket for each client */
// 	for (int i = 0; i < c_array->pos; ++i)
// 	{
// 		return_val = pthread_join(c_array->array[i].thread, 0);
// 		CHECK(return_val != 0, "Fail joining client");

// 		return_val = close(c_array->array[i].fd);
// 		CHECK(return_val != 0, "Fail to close socket");		
// 	}
// }