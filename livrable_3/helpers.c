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
	printf("Client trimite: %s\n", buffer);
	CHECK(return_val < 0, "Client fails sending message to server");
}

int recv_chunk(char* buffer, int fd)
{
	bzero(buffer, MAX); 
	int return_val = recv(fd, buffer, MAX, 0);
	printf("Client primeste:%s\n", buffer);
	CHECK(return_val < 0, "Client fails recv message from the server");

	return strlen(buffer);
}


int is_fin_msg(char* message)
{
	char deserial[MAX];
	strcpy(deserial, message);
	char* type = strtok(deserial, "-");

	// printf("========> %s\n", type);
	if (strncmp(type, "1", 1) == 0)
	{
		// printf("helpers.c l53||| type=%s\n", type);
	 	type = strtok(NULL, "-");
	 	// printf("after strtok type=|%s|___len=%d\n", type, strlen(type));
		if (strcmp(type, "fin\n") == 0)
		{	
			printf("Bye. Client has closed.\n");
			return 1;
		}
	}
	return 0;
}

/* Check if the message is an ip ctrl msg
	and store just the ip in the same string,
	without the flag
*/

int is_ctrl(char* message, int ctrl) {
	char deserial[MAX];
	strcpy(deserial, message);

	char* type = strtok(deserial, "-");
	if (atoi(type) == ctrl) {
		printf("Am primit ip de la server\n");
		type = strtok(NULL, "-");
		strcpy(message, type);
		return 1;
	}	
	return 0;
}

int is_file_msg(char* message)
{
	char deserial[MAX];
	strcpy(deserial, message);
	char* type = strtok(deserial, "-");

	// printf("========> %s\n", type);
	if (strncmp(type, "4", 1) == 0)
	{
		// printf("helpers.c l53||| type=%s\n", type);
	 	type = strtok(NULL, "-");
	 	// printf("after strtok type=|%s|___len=%d\n", type, strlen(type));
		if (strcmp(type, "file\n") == 0)
		{	
			printf("File recv.\n");
			return 1;
		}
	}
	return 0;


	// //strncmp(message, "file", 4) == 0 ? return 1 : return 0
	// if (strncmp(message, "file", 4) == 0)
	// {
	// 	// printf("Trebuie sa trimit un fisier\n");
	// 	return 1;
	// }
	// return 0;

}

void serial_msg(char* msg, char* buffer, int type)
{
	// memcpy(buffer, &cli_info->port, sizeof(int));
	// memcpy(buffer + sizeof(int), cli_info->IP, strlen(cli_info->IP) + 1);
	sprintf(buffer, "%d-%s", type, msg);
}

void get_content(char* source, char* dest)
{
	char deserial[MAX];
	strcpy(deserial, source);
	// printf("aiciiiiiiiiiiiiii\n");
	char* type = strtok(deserial, "-");

	// printf("========>before strtok %s\n", type);
	// printf("helpers.c l53||| type=%s\n", type);
 	type = strtok(NULL, "-");
	// printf("========>after strtok %s\n", type);
 	
 	strcpy(dest, type);
}

void parse_ip_port(char* buff, cli_info_t* cli_info)
{
	char deserial[MAX];
	strcpy(deserial, buff);
	// printf("aiciiiiiiiiiiiiii\n");
	char* type = strtok(deserial, "-");

	// printf("========>before strtok %s\n", type);
	printf("Parsez IP+PORT \n");
 	type = strtok(NULL, "-");
 	strcpy(cli_info->IP, type);
 	printf("IP = %s\n", cli_info->IP);
	// printf("========>after strtok %s\n", type);
 	
  	type = strtok(NULL, "-");
 	printf("PORT=%d\n", atoi(type));
 	cli_info->port = atoi(type);
}

int get_files_nb(char* buff)
{
	char deserial[MAX];
	strcpy(deserial, buff);
	char* type = strtok(deserial, "-");

 	type = strtok(NULL, "-");

 	printf("Received nb_files = %d\n", atoi(type));
 	return atoi(type);	
}

int choose_files_nb()
{
	char buffer[MAX];
	int nb_files = 0;

	printf("Introduce the NUMBER of files: to cancel, type: 'C' ______\n\n----->");
	while (1) {
		fgets(buffer, MAX, stdin);

		if (strcmp(buffer, "C\n") == 0) {
			printf("Transfer canceled\n");
			return 0;
		}
		nb_files = atoi(buffer);

		if (nb_files) {
			return nb_files;
		}
		else
		{
			printf("Invalid number of files, type again\n");
		}
	}
	return 0;
}

FILE* file_exists(char* file_name)
{
	char* path_file = (char*)malloc(sizeof(SEND_DIR) + strlen(file_name));
	CHECK(path_file == NULL, "Fail to alloc memory");

	sprintf(path_file, "%s%s", SEND_DIR, file_name);

	/* Test if the file exists in the directory */
	FILE* fp = fopen(path_file, "r");
	if (fp == NULL) {
		// printf("len |%s| nume %d\n", file_name, strlen(file_name));
		printf("______ File '%s' does not exist ______\n", file_name);
		return NULL;
	}
	free(path_file);
	return fp;
}

int choose_files(int nb_files, param_send_t* f_details)
{
	char buffer[MAX];
	FILE *fp;
	int i = 0;

	while (i < nb_files) {
		fgets(buffer, MAX, stdin);
		if (strcmp(buffer, "C\n") == 0) {
			printf("Transfer canceled\n");
			return 0;
		}
		buffer[strlen(buffer) - 1] = '\0';
		fp = file_exists(buffer);
		if (fp == NULL) {
			continue;
		}
		f_details[i].other = fp;
		strcpy(f_details[i].file_name, buffer);

		++i;
	}
	return 1;
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
		printf("______ Choose a file to send from: ______\n");
		while ((ep = readdir (dp)))
		{
			if(strcmp(ep->d_name,".")!=0 && strcmp(ep->d_name,"..")!=0) {
				printf("%s\n",ep->d_name);
			}
		}    
		(void) closedir (dp);
		printf("______ ");
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
// void create_main_threads(int* sockfd) {
// 	int return_val = pthread_create(&threads[0], 0, send_msg, (void *)sockfd);
// 	CHECK(return_val != 0, "Failed to create thread");

// 	return_val = pthread_create(&threads[1], 0, receive_msg, (void *)sockfd);
// 	CHECK(return_val != 0, "Failed to create thread");
// }

// void join_main_threads() {
// 	for (int i = 0; i < 2; ++i)
// 	{
// 		int return_val = pthread_join(threads[i], 0);
// 		CHECK(return_val != 0, "Failed to join thread");
// 	}
// }