#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h> 
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "helpers.h"

pthread_t threads[6];
int file_is_sending = 0;


int fsize(FILE *fp){
    int prev, size;

	prev = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, prev, SEEK_SET);
    return size;
}

void send_chunk(char* buffer, int fd)
{
	int return_val = send(fd, buffer, MAX, 0);
	CHECK(return_val < 0, "Client fails sending message to server");
}

int recv_chunk(char* buffer, int fd)
{
	bzero(buffer, MAX); 
	int return_val = recv(fd, buffer, MAX, 0);
	CHECK(return_val < 0, "Client fails recv message from the server");

	return strlen(buffer);
}


int CHECK_fin_message(char* message)
{
	if (strncmp(message, "fin", 3) == 0)
	{
		printf("Bye. Client has closed.\n");
		return 1;
	}
	return 0;
}

int is_file_msg(char* message)
{
	//strncmp(message, "file", 4) == 0 ? return 1 : return 0
	if (strncmp(message, "file", 4) == 0)
	{
		// printf("Trebuie sa trimit un fisier\n");
		return 1;
	}
	return 0;

}


/*
 * Reads a message from keyboard into the buffer and sends to
 * the channel defined by sockfd
*/
void *send_msg(void* fd)
{
	int *fd_server = (int *)fd;
	int return_val;
	char *buffer = (char*)malloc(MAX * sizeof(char));
	char *file_name = (char*)malloc(MAX * sizeof(char));

	while (1)
	{

		fgets(buffer, MAX, stdin);
		printf("\n");
		return_val = send(*fd_server, buffer, MAX, 0);
		CHECK(return_val < 0, "Client fails sending message to server");

		if (CHECK_fin_message(buffer))
		{
			pthread_cancel(threads[1]);
			break;
		}
		if (is_file_msg(buffer))
		{
			// todo pornesc thread nou send
			// list_dir();
			// fgets(buffer, MAX, stdin);
			// buffer[strlen(buffer) - 1] = '\0';

			// send_file(fd_server);
			if (!file_is_sending)
			{
				file_is_sending = 1;
				list_dir();
				fgets(file_name, MAX, stdin);
				file_name[strlen(file_name) - 1] = '\0';

				param_send_t p_send;
				p_send.fd = *fd_server;
				p_send.file_name = file_name;

				return_val = pthread_create(&threads[2], 0, send_msg, (void *)fd_server);
				return_val = pthread_create(&threads[3], 0, send_file, (void *)(&p_send));

				// printf("!!!!! am creat 2 threaduri noi\n" );
				return_val = pthread_join(threads[2], 0);
				CHECK(return_val != 0, "Failed to join thread");
				return_val = pthread_join(threads[3], 0);
				CHECK(return_val != 0, "Failed to join thread");
				// printf("!!!!! am facut join la cele 2 threaduri noi\n" );

				file_is_sending = 0;
			}
			else
			{
				printf("Another file is sending now. Please wait\n");
			}
		}
	}
	free(buffer);
	free(file_name);

	return 0;
}

/* Function which received a message from server and print it
*/
void *receive_msg(void* fd)
{
	int *fd_server = (int *)fd;
	int return_val;
	char buffer[MAX];

	while (1)
	{
		return_val = recv(*fd_server, buffer, MAX, 0); 
		CHECK(return_val < 0, "Client fails receiving message from server");
		printf("Server: ");
		puts(buffer);

		if (CHECK_fin_message(buffer))
		{
			pthread_cancel(threads[0]);
			break;
		}
		if (is_file_msg(buffer))
		{
			// todo pornesc thread nou rcv
			recv_file(*fd_server);
		}
	}
	return 0;
}
void *send_file(void* param_send)
{
	param_send_t p_send = *(param_send_t*)param_send;
	FILE* fp = NULL;
	int f_size;
	int fd = p_send.fd;
	char buffer[MAX];
	char* path_file;


	/* 	List all the files from the source directory and choose a file
		to send. Replace last character of the filename '\n' with '\0'
	*/
	// list_dir();
	// fgets(file_name, MAX, stdin);
	// file_name[strlen(file_name) - 1] = '\0';

	path_file = (char*)malloc(sizeof(SEND_DIR) + strlen(p_send.file_name));
	sprintf(path_file, "%s%s", SEND_DIR, p_send.file_name);

	/* Test if the file exists in the directory */
	fp = fopen(path_file, "r");
	if (fp == NULL) {
		// printf("len |%s| nume %d\n", file_name, strlen(file_name));
		printf(">>>>> File %s does not exist <<<<<\n", p_send.file_name);
		return NULL;
	}
	f_size = fsize(fp);


	/* Send the filename to the server */
	send_chunk(p_send.file_name, fd);



	/* Send the number of chunks of the file to the server */
	sprintf(buffer, "%d", f_size);
	// itoa(nb_chunks, buffer, 10);
	send_chunk(buffer, fd);


	// todo in loc de print fac send
	// printf("nume fisier: %s\n", p_send.file_name);
	// printf("size file = %d\n", f_size);

	// printf("nb pachete = %f\n", (float)f_size / MAX);
	while (fgets(buffer, MAX, fp) != NULL) {
		// printf("%s\n", buffer);
		/* Send each chunk of the file to the server */
		send_chunk(buffer, fd);
		// sleep(5);
		// printf("trimit:%s\n", buffer);
	}
	printf(">>>>> File successfully sent <<<<<\n");
	free(path_file);
	fclose(fp);
	pthread_cancel(threads[2]);
	return NULL;
}

void recv_file(int fd)
{
	FILE* fp = NULL;
	int f_size;
	char *file_name;
	char buffer[MAX];

	recv_chunk(buffer, fd);
	file_name = (char *)malloc(sizeof(RCV_DIR) + strlen(buffer));
	sprintf(file_name, "%s%s", RCV_DIR, buffer);
	// printf("file_name is %s\n", );
	// printf("path fisier:  %s\n", file_name);

	/* Open destination file */
	fp = fopen(file_name, "w");
	CHECK(fp == NULL, "Failed to open destination file");
	// printf("fp = %d\n", fp);

	recv_chunk(buffer, fd);

	f_size = atoi(buffer);
	// printf("f_size = %d\n", f_size);


	while (f_size > 0) {
		// printf("%s\n", buffer);
		int nb_B_recv = recv_chunk(buffer, fd);
		fputs(buffer, fp);
		f_size -= nb_B_recv;
		// printf("f_size = %d ||||| buffer = %s\n", f_size, buffer);

	}
	printf(">>>>> File successfully received <<<<<\n");

	fclose(fp);
}

// int file_exist (char *file_name)
// {
// 	struct stat buffer;
// 	char* path_file = (char *)malloc(sizeof(RCV_DIR) + strlen(file_name));
// 	sprintf(path_file, "%s%s", RCV_DIR, file_name);
// 	// printf("testez daca exista fis: %s\n", path_file);
// 	return (stat (path_file, &buffer) == 0);
// }

void list_dir()
{
	DIR *dp;
	struct dirent *ep;     
	dp = opendir (SEND_DIR);
	if (dp != NULL)
	{
		printf(">>>>> Choose a file to send from: <<<<<\n");
		while ((ep = readdir (dp)))
		{
			if(strcmp(ep->d_name,".")!=0 && strcmp(ep->d_name,"..")!=0) {
				printf("%s\n",ep->d_name);
			}
		}    
		(void) closedir (dp);
		printf(">>>>> ");
	}
	else
	{
		perror ("Error opening the directory");
	}
}


/*
Create 2 threads:
	- the first thread reads from keyboard and sends to server->client 2
	- the second thread receives from client 2 and sends to server->client 1
*/
void create_main_threads(int* sockfd) {
	int return_val = pthread_create(&threads[0], 0, send_msg, (void *)sockfd);
	CHECK(return_val != 0, "Failed to create thread");

	return_val = pthread_create(&threads[1], 0, receive_msg, (void *)sockfd);
	CHECK(return_val != 0, "Failed to create thread");
}

void join_main_threads() {
	for (int i = 0; i < 2; ++i)
	{
		int return_val = pthread_join(threads[i], 0);
		CHECK(return_val != 0, "Failed to join thread");
	}
}