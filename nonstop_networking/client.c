/**
 * Nonstop Networking
 * CS 241 - Spring 2019
 */
#include "common.h"
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//MACRO Declaration
#define HOST_INDEX 0
#define PORT_INDEX 1
#define VERB_TYPE_INDEX 2
#define REMOTE_FILE_INDEX 3
#define LOCAL_FILE_INDEX 4

//Function Declaration
char **parse_args(int argc, char **argv);
verb check_args(char **args);
int client_connect_to_server(const char* host, const char* port);



/*
 * Sets up a connection to a chatroom server and returns
 * the file desciptor associated with the connection.
 * 
 * host - Server to connect to.
 * port - Port to connect to server on.
 * 
 * Returns integer of valid file descriptor, or exit(1) on failure.
 * NOTE: This code is taken from charming_chatroom lab (collab with wcwang2)
*/
int client_connect_to_server(const char * host, const char * port) {
	//Establish client socket first
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1) { //failed to create socket
		print_error_message("failed to create client socket!");
		exit(1);
	}
	
	//Acquire addrinfo of the host and port
	struct addrinfo hints;
	struct addrinfo * result;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(host, port, &hints, &result)) {
		print_error_message("getaddrinfo() failed!");
		exit(1);
	}
	if (connect(socket_fd, result->ai_addr, result->ai_addrlen) == -1) {
		print_error_message("connect() failed!");
		exit(1);
	}

	//Clean up
	freeaddrinfo(result);

	//Connection is good, return
	return socket_fd;
	
}


int main(int argc, char **argv) {
	//Parse information
	char** args = parse_args(argc, argv);
	verb  = check_args(args);

	//Get socket
	int socket_fd = client_connect_to_server(args[HOST_INDEX], args[PORT_INDEX]);
	
	//Ready to send command to server
	client_send	
}


