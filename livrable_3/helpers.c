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
	CHECK(return_val < 0, "Client fails sending message to server");
}

int recv_chunk(char* buffer, int fd)
{
	bzero(buffer, MAX); 
	int return_val = recv(fd, buffer, MAX, 0);
	CHECK(return_val < 0, "Client fails recv message from the server");

	return strlen(buffer);
}

void send_file(int fd, char* file_name)
{
	FILE* fp = NULL;
	int f_size;

	// char *file_name = "send/test_fis";
	char buffer[MAX];
	char* path_file;

	path_file = (char*)malloc(sizeof(SEND_DIR) + strlen(file_name));
	sprintf(path_file, "%s%s", SEND_DIR, file_name);

	fp = fopen(path_file, "r");
	if (fp == NULL) {
		// printf("len |%s| nume %d\n", file_name, strlen(file_name));
		printf(">>>>> File %s does not exist <<<<<\n", file_name);
		return;
	}
	f_size = fsize(fp);


	/* Send the filename to the server */
	send_chunk(file_name, fd);



	/* Send the number of chunks of the file to the server */
	sprintf(buffer, "%d", f_size);
	// itoa(nb_chunks, buffer, 10);
	send_chunk(buffer, fd);


	// todo in loc de print fac send
	printf("nume fisier: %s\n", file_name);
	printf("size file = %d\n", f_size);

	// printf("nb pachete = %f\n", (float)f_size / MAX);
	while (fgets(buffer, MAX, fp) != NULL) {
		// printf("%s\n", buffer);
		/* Send each chunk of the file to the server */
		send_chunk(buffer, fd);
		// printf("trimit:%s\n", buffer);
	}
	printf(">>>>> File successfully sent <<<<<\n");
	free(path_file);
	fclose(fp);
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
	fclose(fp);
}

int file_exist (char *file_name)
{
	struct stat buffer;
	char* path_file = (char *)malloc(sizeof(RCV_DIR) + strlen(file_name));
	sprintf(path_file, "%s%s", RCV_DIR, file_name);
	// printf("testez daca exista fis: %s\n", path_file);
	return (stat (path_file, &buffer) == 0);
}

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
	}
	else
	{
		perror ("Error opening the directory");
	}
}