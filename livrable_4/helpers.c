#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h> 
#include <string.h>

#include "helpers.h"


void parse_msg(char* raw_msg, body_msg_t* msg) {
	char deserial[MAX];
	strcpy(deserial, raw_msg);
	char* type = strtok(deserial, "-");

	strcpy(msg->type, type);
	type = strtok(NULL, "-");
	strcpy(msg->body, type);

	if (!strcmp(msg->type, "edit_c") || !strcmp(msg->type, "add_c"))
	{
		type = strtok(NULL, "-");
		strcpy(msg->name, type);

		type = strtok(NULL, "-");
		strcpy(msg->descr, type);
 	}
}
