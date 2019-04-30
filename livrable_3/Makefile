# Makefile

CFLAGS = -Wall -g -pthread

# The port on which the server listens to
PORT = 8080

# The number of clients for a server
NB_CLIENTS = 3

# Server's IP
IP_SERVER = 127.0.0.1

all: server client

# Compile server.c
server: server.c helpers.c

# Compile server_v2.c
server_v2: server_v2.c helpers.c

# Compile client.c
client: client.c helpers.c

.PHONY: clean run_server run_client

# Run the server
run_server:
	./server ${PORT}

# Run the server
run_server_v2:
	./server_v2 ${PORT} ${NB_CLIENTS}

# Run the client 	
run_client:
	./client ${IP_SERVER} ${PORT}

clean:
	rm -f server client server_v2
