/**
 * Nonstop Networking
 * CS 241 - Spring 2019
 */
#include "common.h"
#include "format.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
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
#define MAX_HEADER_SIZE 1024
#define MAX_R_W_SIZE 1024

//Function Declaration
char **parse_args(int argc, char **argv);
verb check_args(char **args);
int client_connect_to_server(const char* host, const char* port);
void client_send_request_header(int socket_fd, verb request_verb, char** args);
void client_send_request_main(int socket_fd, verb request_verb, char** args);
void client_send_request(int socket_fd, verb request_verb, char** args);
void client_receive_response_error(int socket_fd, char** args);
void client_receive_response_main(int socket_fd, verb request_verb, char** args);
void client_receive_response(int socket_fd, verb request_verb, char** args);
void clean_up(int socket_fd, char** args);

void clean_up(int socket_fd, char** args) {
	close(socket_fd);
	free(args);
}

void client_receive_response_main(int socket_fd, verb request_verb, char** args) {
	if (request_verb == DELETE || request_verb == PUT) {
		print_success();
		return;
	}
	// First, get package size
	size_t response_size = 0;
	ssize_t byte_read = client_read_all_from_socket(socket_fd, (char*) &response_size, sizeof(size_t));

	if (byte_read <= 0) { //something wrong with the response
		print_invalid_response();
		clean_up(socket_fd, args);
		exit(1);
	}

	//Prepare utilities
	size_t total_byte_read = 0;
	size_t total_byte_to_read = response_size;
	size_t current_byte_to_read = 0;
	ssize_t current_byte_read = 0;
	char line[MAX_R_W_SIZE + 1];
	memset(line, 0, MAX_R_W_SIZE + 1);

	if (request_verb == LIST) {
		while (total_byte_read < response_size) {
			memset(line, 0, MAX_R_W_SIZE + 1);
			current_byte_to_read = (total_byte_to_read < MAX_R_W_SIZE) ? total_byte_to_read : MAX_R_W_SIZE;
			current_byte_read = client_read_all_from_socket(socket_fd, line, current_byte_to_read);
			if (handle_return_value(current_byte_read, current_byte_to_read, total_byte_to_read) == 1) {
				clean_up(socket_fd, args);
				exit(1);
			}
			total_byte_read += current_byte_read;
			total_byte_to_read -= current_byte_read;
			printf("%s", line);
		}
		printf("\n");
		if (client_read_all_from_socket(socket_fd, line, MAX_R_W_SIZE) != 0) {
			print_received_too_much_data();
		}
	} else if (request_verb == GET) {
		FILE * local_file = fopen(args[LOCAL_FILE_INDEX], "w");
		
		if (local_file == NULL) {
			perror("client failed to open local file!");
			clean_up(socket_fd, args);
			exit(1);
		}

		while (total_byte_read < response_size) {
			memset(line, 0, MAX_R_W_SIZE + 1);
			current_byte_to_read = (total_byte_to_read < MAX_R_W_SIZE) ? total_byte_to_read : MAX_R_W_SIZE;
			current_byte_read = client_read_all_from_socket(socket_fd, line, current_byte_to_read);
			if (handle_return_value(current_byte_read, current_byte_to_read, total_byte_to_read) == 1) {
				clean_up(socket_fd, args);
				exit(1);
			}
			total_byte_read += current_byte_read;
			total_byte_to_read -= current_byte_read;
			fwrite(line, 1, current_byte_read, local_file);
		}
		if (client_read_all_from_socket(socket_fd, line, MAX_R_W_SIZE) != 0) {
			print_received_too_much_data();
		}
		fclose(local_file);
	}
}

void client_receive_response_error(int socket_fd, char** args) {
	//Set up utilities
	char line[MAX_HEADER_SIZE];
	memset(line, 0, MAX_HEADER_SIZE);

	//Proceed to read error message
	ssize_t byte_read = client_read_line_from_socket(socket_fd, line, MAX_HEADER_SIZE);
	if (byte_read <= 0) { //something is wrong with the response header
		print_invalid_response();
		clean_up(socket_fd, args);
		exit(1);
	}
	print_error_message(line);
}

void client_receive_response(int socket_fd, verb request_verb, char** args) {
	//Set up utilities
	char line[MAX_HEADER_SIZE];
	memset(line, 0, MAX_HEADER_SIZE);

	//Read server response
	ssize_t byte_read = client_read_line_from_socket(socket_fd, line, MAX_HEADER_SIZE);
	if (byte_read <= 0) { //something is wrong with the response header
		print_invalid_response();
		clean_up(socket_fd, args);
		exit(1);
	}

	if (strcmp(line, "OK") == 0) { //OK
		client_receive_response_main(socket_fd, request_verb, args);
	} else if (strcmp(line, "ERROR") == 0) { //ERROR
		client_receive_response_error(socket_fd, args);
	} else { //wrong response
		print_invalid_response();
		clean_up(socket_fd, args);
		exit(1);
	}
}

void client_send_request(int socket_fd, verb request_verb, char** args) {
	client_send_request_header(socket_fd, request_verb, args);
	client_send_request_main(socket_fd, request_verb, args);
	return;
}

void client_send_request_header(int socket_fd, verb request_verb, char** args) {
	//Construct request first
	size_t line_length = 0;
	char line[MAX_HEADER_SIZE];
	memset(line, 0, MAX_HEADER_SIZE);
	if (request_verb == LIST) {
		//1 for \n | 1 for null byte
		line_length = strlen(args[VERB_TYPE_INDEX]) + 2;
		sprintf(line, "%s\n", args[VERB_TYPE_INDEX]);
	} else {
		//1 for space | 1 for \n | 1 for null byte
		line_length = strlen(args[VERB_TYPE_INDEX]) + strlen(args[REMOTE_FILE_INDEX]) + 3;
		sprintf(line, "%s %s\n", args[VERB_TYPE_INDEX], args[REMOTE_FILE_INDEX]);
	}
	ssize_t byte_written = client_write_all_to_socket(socket_fd, line, line_length);
	if (byte_written < (ssize_t) line_length) {
		print_invalid_response();
		clean_up(socket_fd, args);
		exit(1);
	}
}

void client_send_request_main(int socket_fd, verb request_verb, char** args) {
	//Construct request second
	if (request_verb == PUT) {
		//Try to open file
		FILE * local_file = fopen(args[LOCAL_FILE_INDEX], "r");
		if (local_file == NULL) {
			perror(err_no_such_file);
			exit(1);
		}
		
		//Proceed to send the request of file_size
		fseek(local_file, 0, SEEK_END);
		size_t file_size = ftell(local_file);
		fseek(local_file, 0, SEEK_SET);

		//Send file_size to server
		ssize_t byte_written = 0;
		byte_written = client_write_all_to_socket(socket_fd, (char*) &file_size, sizeof(size_t));
		if (byte_written <= 0) {
			print_connection_closed(); //MODIFIED
			clean_up(socket_fd, args);
			exit(1);
		}

		//Proceed to send binary data to server
		size_t total_byte_written = 0;
		size_t total_byte_to_write = file_size;
		size_t current_byte_to_write = 0;
		ssize_t current_byte_written = 0;
		char line[MAX_R_W_SIZE];
		memset(line, 0, MAX_R_W_SIZE);

		while (total_byte_to_write > 0) {
			memset(line, 0, MAX_R_W_SIZE);
			current_byte_to_write = (total_byte_to_write < MAX_R_W_SIZE) ? total_byte_to_write : MAX_R_W_SIZE;
			fread(line, 1, current_byte_to_write, local_file);
			current_byte_written = client_write_all_to_socket(socket_fd, line, current_byte_to_write);
			if (handle_return_value(current_byte_written, current_byte_to_write, total_byte_to_write) == 1) {
				clean_up(socket_fd, args);
				exit(1);
			}
			total_byte_written += current_byte_written;
			total_byte_to_write -= current_byte_written;
		}
		fclose(local_file);
		return;
	} else { //GET, LIST, DELETE do not have the second line
		return;
	}
}



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
		perror("failed to create client socket!");
		return -1;
	}
	
	//Acquire addrinfo of the host and port
	struct addrinfo hints;
	struct addrinfo * result;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(host, port, &hints, &result)) {
		perror("getaddrinfo() failed!");
		freeaddrinfo(result);
		return -1;
	}
	if (connect(socket_fd, result->ai_addr, result->ai_addrlen) == -1) {
		perror("connect() failed!");
		freeaddrinfo(result);
		return -1;
	}

	//Clean up
	freeaddrinfo(result);

	//Connection is good, return
	return socket_fd;
	
}


int main(int argc, char **argv) {
	//Parse information
	char** args = parse_args(argc, argv);
	verb request_verb = check_args(args);

	//Get socket
	int socket_fd = client_connect_to_server(args[HOST_INDEX], args[PORT_INDEX]);
	if (socket_fd == -1) {
		free(args);
		exit(1);
	}
	
	//Ready to send command to server
	client_send_request(socket_fd, request_verb, args);

	//Request sent, shutdown the write end of the socket
	if (shutdown(socket_fd, SHUT_WR) != 0) {
		perror("client failed to shutdown socket!");
	}

	//Listen to server's response
	client_receive_response(socket_fd, request_verb, args);

	//Clean up
	clean_up(socket_fd, args);
	return 0;
}


