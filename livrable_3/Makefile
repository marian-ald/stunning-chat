# Makefile

CFLAGS = -Wall -g -pthread

# The port on which the server listens to
PORT = 8080

# Server's IP
IP_SERVER = 127.0.0.1

all: server client_v_1_2

# Compile server.c
server: server.c helpers.c

# Compile client_v_1_2.c
client_v_1_2: client_v_1_2.c helpers.c

.PHONY: clean run_server run_client_v_1_2

# Run the server
run_server:
	./server ${PORT}

# Run the client_v_1_2 	
run_client_v_1_2:
	./client_v_1_2 ${IP_SERVER} ${PORT}

clean:
	rm -f server client_v_1_2
