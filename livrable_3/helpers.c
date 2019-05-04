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

/* Return the size of a file */
int fsize(FILE *fp){
    int prev, size;

	prev = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, prev, SEEK_SET);
    return size;
}

/* Send a message and check return value*/
void send_chunk(char* buffer, int fd)
{
	int return_val = send(fd, buffer, MAX, 0);
	CHECK(return_val < 0, "Client fails sending message to server");
}

/* Receive a message and check return value*/
int recv_chunk(char* buffer, int fd)
{
	bzero(buffer, MAX); 
	int return_val = recv(fd, buffer, MAX, 0);
	CHECK(return_val < 0, "Client fails recv message from the server");

	return strlen(buffer);
}

/* Check if a message is 'fin' */
int is_fin_msg(char* message)
{
	char deserial[MAX];
	strcpy(deserial, message);
	char* type = strtok(deserial, "-");

	if (strncmp(type, "1", 1) == 0)
	{
	 	type = strtok(NULL, "-");
		if (strcmp(type, "fin\n") == 0)
		{	
			printf("Bye. Client has closed.\n");
			return 1;
		}
	}
	return 0;
}

/* Check if the message is an ip ctrl msg and store just the ip
in the same string,	without the flag
*/
int is_ctrl(char* message, int ctrl) {
	char deserial[MAX];
	strcpy(deserial, message);

	char* type = strtok(deserial, "-");
	if (atoi(type) == ctrl) {
		type = strtok(NULL, "-");
		strcpy(message, type);
		return 1;
	}	
	return 0;
}

/* Check if a message is 'file' */
int is_file_msg(char* message)
{
	char deserial[MAX];
	strcpy(deserial, message);
	char* type = strtok(deserial, "-");

	if (strncmp(type, "4", 1) == 0)
	{
	 	type = strtok(NULL, "-");
		if (strcmp(type, "file\n") == 0)
		{	
			printf("_____File recv_____\n");
			return 1;
		}
	}
	return 0;
}

void serial_msg(char* msg, char* buffer, int type)
{
	sprintf(buffer, "%d-%s", type, msg);
}

/* Parse and return the content of a message */
void get_content(char* source, char* dest)
{
	char deserial[MAX];
	strcpy(deserial, source);
	char* type = strtok(deserial, "-");

 	type = strtok(NULL, "-"); 	
 	strcpy(dest, type);
}

/* Parse the message to extract the IP and the PORT */
void parse_ip_port(char* buff, cli_info_t* cli_info)
{
	char deserial[MAX];
	strcpy(deserial, buff);
	char* type = strtok(deserial, "-");

 	type = strtok(NULL, "-");
 	strcpy(cli_info->IP, type);
 	
  	type = strtok(NULL, "-");
 	cli_info->port = atoi(type);
}

int get_files_nb(char* buff)
{
	char deserial[MAX];
	strcpy(deserial, buff);
	char* type = strtok(deserial, "-");

 	type = strtok(NULL, "-");

 	printf("_____Received nb_files = %d_____\n", atoi(type));
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

/* Check if a file exists. If yes, open it an return the file
pointer, else return NULL */
FILE* file_exists(char* file_name)
{
	char* path_file = (char*)malloc(sizeof(SEND_DIR) + strlen(file_name));
	CHECK(path_file == NULL, "Fail to alloc memory");

	sprintf(path_file, "%s%s", SEND_DIR, file_name);

	/* Test if the file exists in the directory */
	FILE* fp = fopen(path_file, "r");
	if (fp == NULL) {
		printf("______ File '%s' does not exist ______\n", file_name);
		return NULL;
	}
	free(path_file);
	return fp;
}

/* Read from stdin the files to be sent */
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

/* List the files from the sending directory */
void list_dir()
{
	DIR *dp;
	struct dirent *ep;     
	dp = opendir (SEND_DIR);
	if (dp != NULL)
	{
		printf("______ Send directory: ______\n");
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
