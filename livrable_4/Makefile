# Makefile

CFLAGS = -Wall -g -pthread

# The port on which the server listens to
PORT = 8080

# Server's IP
IP_SERVER = 127.0.0.1

NB_CHANNELS = 2
NB_CLI = 2
all: client server

# Compile server.c
server: server.c helpers.c linked_list.c

# Compile client.c
client: client.c

.PHONY: clean run_server run_client

# Run the server
run_server:
	./server ${PORT} ${NB_CHANNELS} ${NB_CLI}

# Run the client 	
run_client:
	./client ${IP_SERVER} ${PORT}

clean:
	rm -f client server
