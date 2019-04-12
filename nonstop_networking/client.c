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
#define 

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
	verb cmd = check_args(args);

	//Get socket
	int socket_fd = client_connect_to_server(args[HOSTNAME_IDX])
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
