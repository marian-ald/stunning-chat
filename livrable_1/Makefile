# Makefile

CFLAGS = -Wall -g

# The port on which the server listens to
PORT = 8080

# Server's IP
IP_SERVER = 127.0.0.1

all: server client 

# Compile server.c
server: server.c

# Compile client.c
client: client.c

.PHONY: clean run_server run_client

# Run the server
run_server:
	./server ${PORT}

# Run the client 	
run_client:
	./client ${IP_SERVER} ${PORT}

clean:
	rm -f server client
