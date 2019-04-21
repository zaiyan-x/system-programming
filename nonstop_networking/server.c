/**
 * Nonstop Networking
 * CS 241 - Spring 2019
 */

#include "common.h"
#include "vector.h"
#include "dictionary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

/* Server Macros */
#define MAX_CLIENTS 32
#define MAX_EVENTS 32
#define MAX_HEADER_SIZE 1024
#define MAX_R_W_SIZE 1024
#define MAX_FILENAME_SIZE 256
#define MAX_VERB_SIZE 8

/* Client State Macros */
#define READ_HEADER 0
#define READ_SIZE 1
#define READ_PUT 2
#define WRITE_LIST 3
#define WRITE_REPLY_OK 4
#define WRITE_REPLY_ERROR 5
#define WRITE_GET 6
#define WRITE_SIZE 7

/* Status Macros */
#define ACTION_PAUSED 1
#define ACTION_COMPLETE 0

/* Global Server Variables */
static char* SERVER_DIR;
static vector * FILE_VECTOR;
static int SOCKET_FD;
static int EPOLL_FD;
static dictionary * CLIENT_DIC;

/* Server Function Declaration */
//Server Helpers
void read_header(int client_fd, client* current_client);
void read_size(int client_fd, client* current_client);
void read_put(int client_fd, client* current_client);
void write_list(int client_fd, client* current_client);
void write_reply_ok(int client_fd, client* current_client);
void write_reply_error(int client_fd, client* current_client);
void write_get(int client_fd, client* current_client);
void write_size(int client_fd, client* current_client);
void setup_delete(int client_fd, client* current_client);
void setup_file(int client_fd, client* current_client);
void setup_get(int client_fd, client* current_client);
void setup_put(int client_fd, client* current_client);
void delete_file(char * filename);

//Server Infrastructures
void server_listen_to_client();
void setup_server(char * port);
void dispatch_client(int client_fd);
void dispatch_action(int client_fd, client* current_client);
void* client_copy_constructor(void* elem);
void client_destructor(void* elem);
void shutdown_server();
void log_error(int client_fd, client* current_client, const char* error_message);
void log_ok(int client_fd, client* current_client);
void reset_epoll_mode_to_write(int client_fd);


/* Client Information Struct */
typedef struct client_ {
	int state;
	verb client_verb;
	char filename[MAX_FILENAME_SIZE];
	int offset;
	const char * error_message;
	char header[MAX_HEADER_SIZE];
} client;

void delete_file(char * filename, int i) {
	char path[MAX_HEADER_SIZE];
	memset(path, 0, MAX_HEADER_SIZE);
	sprintf(path, "%s/%s", SERVER_DIR, filename);
	if (unlink(path) != 0) {
		perror("SERVER: unlink() failed!");
	}
	vector_erase(FILE_DIR, i);
	return;
}

void setup_delete(int client_fd, client* current_client) {
	int i = 0;
	int found = 0;
	char * filename = NULL;
	VECTOR_FOR_EACH(FILE_VECTOR, node, {
		filename = (char*) node;
		if (strcmp(filename, current_client->filename) == 0) {
			found = 1;
			break;
		}
		i++;
	});
	
	if (found == 0) {
		log_error(current_client, err_no_such_file);
		write_reply_error(client_fd, current_client);
	} else {
		delete_file(current_client->filename, i);
		log_ok(client_fd, current_client);
	}
}

void setup_file(int client_fd, client* current_client);
void setup_get(int client_fd, client* current_client);
void setup_put(int client_fd, client* current_client);

void reset_epoll_mode_to_write(int client_fd) {
	struct epoll_event ev;
	ev.events - EPOLLOUT;
	ev.data.fd = client_fd;
	epoll_ctl(EPOLL_FD, EPOLL_CTL_MOD, client_fd, &ev);
}

void log_ok(int client_fd, client* current_client) {
	current_client->state = WRITE_REPLY_OK;
	current_client->offset = 0;
	reset_epoll_mode_to_write(client_fd);
}

void log_error(int client_fd, client* current_client, const char* error_message) {
	current_client->error_message = error_message;
	current_client->state = WRITE_REPLY_ERROR;
	current_client->offset = 0;
	reset_epoll_mode_to_write(client_fd);
}

/* Main Part */
void read_header(int client_fd, client* current_client) {
	char * buffer = current_client->header + current_client->offset;
	int count = MAX_HEADER_SIZE - current_client->offset;
	int status = ACTION_PAUSED;
	int total_byte_read = server_read_line_from_socket(client_fd, buffer, count, status);
	
	if (total_byte_read == -1) { //Something bad happened
		log_error(current_client, err_bad_request);
	}
	
	if (status == ACTION_PAUSED) {
		current_client->offset += total_byte_read;
	} else if (status == ACTION_COMPLETE) {
		memset(current_client->filename, 0, MAX_FILENAME_SIZE);
		char verb_string[MAX_VERB_SIZE];
		memset(verb_string, 0, MAX_VERB_SIZE);
		
		int retval = sscanf(current_client->header, "%s %s", verb_string, current_client->filename);
		
		if (retval == 0) { //Bad format
			log_error(client_fd, current_client, err_bad_request);
		}
		if (strcmp(verb_string, "LIST") == 0) {
			current_client->client_verb = LIST;
			setup_list(client_fd, current_client);
		} else if (strcmp(verb_string, "GET") == 0) {
			current_client->client_verb = GET;
			if (retval != 2) {
				log_error(client_fd, current_client, err_bad_request);
			}
			setup_get(client_fd, current_client);
		} else if (strcmp(verb_string, "DELETE") == 0) {
			current_client->client_verb = DELETE;
			if (retval != 2) {
				log_error(client_fd, current_client, err_bad_request);
			}
			setup_delete(client_fd, current_client);
		} else if (strcmp(verb_string, "PUT") == 0) {
			current_client->client_verb = PUT;
			if (retval != 2) {
				log_error(client_fd, current_client, err_bad_request);
			}
			setup_put(client_fd, current_client);
		} else { //Something really bad happened
			log_error(client_fd, current_client, err_bad_request);
		}
	}	
}
void shutdown_server() {

}

void* client_copy_constructor(void* elem) {
	client * copy = malloc(sizeof(client));
	memcpy(copy, elem, sizeof(client));
	return copy;
}

void* client_destructor(void* elem) {
	free(elem);
}

void dispatch_client(int client_fd) {
	if (!dictionary_contains(CLIENT_DIC, &client_fd)) {
		perror("SERVER: dictionary does not contain client_fd!");
		exit(1);
	}

	client* current_client = （client*) dictionary_get(CLIENT_DIC, &client_fd);
	if (current_client->state == READ_HEADER) {
		read_header(client_fd, current_client);
	} else if (current_client->state == READ_SIZE) {
		read_size(client_fd, current_client);
	} else if (current_client->state == READ_PUT) {
		read_put(client_fd, current_client);
	} else if (current_client->state == WRITE_LIST) {
		write_list(client_fd, current_client);
	} else if (current_client->state == WRITE_REPLY_OK) {
		write_reply_ok(client_fd, current_client);
	} else if (current_client->state == WRITE_REPLY_ERROR) {
		write_reply_error(client_fd, current_client);
	} else if (current_client->state == WRITE_REPLY_ERROR_MESSAGE) {
		write_reply_error_message(client_fd, current_client);
	} else if (current_client->state == WRITE_GET) {
		write_get(client_fd, current_client);
	} else if (current_client->state == WRITE_SIZE) {
		write_size(client_fd, current_client);
	} else {
		perror("SERVER: unknown state happened!");
	}
}


void setup_server(char * port) {
	//Setup signal handler
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = shutdown_server;
	if (sigaction(SIGINT, &act, NULL) < 0) {
		perror("SERVER: sigaction() failed!");
		exit(1);
	}
	
	//Setup temporary directory
	char template[] = "XXXXXX";
	SERVER_DIR = mkdtemp(template);
	print_temp_directory(SERVER_DIR);

	//Setup file vector
	FILE_VECTOR = string_vector_create();

	//Setup socket
	SOCKET_FD = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

	//Server bind
	struct addrinfo hints;
	struct addrinfo * result;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, port, &hints, &result)) {
		perror("SERVER: getaddrinfo() failed!");
		exit(1);
	}

	int optval = 1;
	setsockopt(SOCKET_FD, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

	if (bind(SOCKET_FD, result->ai_addr, result->ai_addrlen) != 0) {
		perror("SERVER: bind() failed!");
		exit(1);
	}
	
	//Clean up result
	freeaddrinfo(result);

	//Activate the unconnected socket
	if (listen(SOCKET_FD, MAX_CLIENTS)) {
		perror("SERVER: listen() failed!");
		exit(1);
	}

	//Initiate epoll fd
	EPOLL_FD = epoll_create1(0);
	if (EPOLL_FD == -1) {
		perror("SERVER: epoll_create1() failed!");
		exit(1);
	}

	//Initiate client dictionary data structure
	CLIENT_DIC = dictionary_create(int_hash_function,
				       int_compare,
				       int_copy_constructor,
				       int_destructor,
				       client_copy_constructor,
				       client_destructor);

	return;
}

void server_listen_to_client() {
	//Setup epoll data
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];

	ev.events = EPOLLIN;
	ev.data.fd = SOCKET_FD;

	//Add server socket to epoll
	if (epoll_ctl(EPOLL_FD, EPOLL_CTL_ADD, SOCKET_FD, &ev) == -1) {
		perror("SERVER: epoll_ctl() failed!");
		exit(1);
	}

	//Start server
	while (1) {
		//Wait for client
		int num_of_client = epoll_wait(EPOLL_FD, events, MAX_EVENTS, -1);
		if (num_of_client == -1) {
			perror("SERVER: epoll_wait() failed!");
			exit(1);
		}
		
		//Loop through all clients
		int i;
		int client_fd;
		int fd_flag;
		client new_client;
		for (i = 0; i < num_of_client; i++) {
			//If we got our listen socket back, we have a new connection
			if (events[i].data.fd == SOCKET_FD) {
				//Accept
			 	client_fd = accept(SOCKET_FD, NULL, NULL);
				if (client_fd == -1) {
					perror("SERVER: accept() failed!");
					exit(1);
				}

				//Set the client_fd to NON_BLOCKING
				fd_flag = fcntl(client_fd, F_GETFL, 0);
				fcntl(client_fd, F_SETFL, fd_flag | O_NONBLOCK);

				//Append new client to epoll
				ev.events = EPOLLIN;
				ev.data.fd = client_fd;
				if (epoll_ctl(EPOLL_FD, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
					perror("SERVER: epoll_ctl() failed!");
					exit(1);
				}

				//Add new key-value pair to dictionary
				memset(&new_client, 0, sizeof(client));
				new_client.state = READ_HEADER;
				new_client.buffer = 0;
				dictionary_set(CLIENT_DIC, &client_fd, &new_client);
		} else {
			dispatch_client(events[i].data.fd);
		}
}

int main(int argc, char **argv) {
    //Check server usage
	if (argc != 2) {
		print_server_usage();
		exit(1);
	}
	//Setup file vector
	FILE_VECTOR = string_vector_create();

	//Setup server
	setup_server(argv[1]);

	//Run server
	server_listen_to_client();
}
