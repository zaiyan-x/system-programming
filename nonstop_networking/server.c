/**
 * Nonstop Networking
 * CS 241 - Spring 2019
 */

#include "common.h"
#include "vector.h"
#include "dictionary.h"
#include "format.h"

#include <netdb.h>
#include <unistd.h>
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
#define MAX_REPLY_SIZE 1024
#define MAX_FILENAME_SIZE 256
#define MAX_VERB_SIZE 8

/* Client State Macros */
#define READ_HEADER 0
#define READ_SIZE 1
#define READ_FILE 2
#define WRITE_FILE 3
#define WRITE_REPLY_OK 4
#define WRITE_REPLY_ERROR 5
#define WRITE_SIZE 6

/* Status Macros */
#define ACTION_PAUSED 1
#define ACTION_COMPLETE 0
#define CONNECTED 0
#define CONNECTION_LOST 1
/* Client Information Struct */
typedef struct client_ {
	int state;
	verb client_verb;
	char filename[MAX_FILENAME_SIZE];
	int offset;
	char reply[MAX_REPLY_SIZE];
	char header[MAX_HEADER_SIZE];
	FILE * file;
	size_t file_size;
} client;

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
void read_file(int client_fd, client* current_client);
void write_file(int client_fd, client* current_client);
void write_reply_ok(int client_fd, client* current_client);
void write_reply_error(int client_fd, client* current_client);
void write_size(int client_fd, client* current_client);
void setup_delete(int client_fd, client* current_client);
void setup_list(int client_fd, client* current_client);
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
FILE * open_file(char * filename, char * flag);
void shutdown_client(int client_fd);

/* Main Part */
FILE * open_file(char * filename, char * flag) {
	char path[MAX_HEADER_SIZE];
	memset(path, 0, MAX_HEADER_SIZE);
	sprintf(path, "%s/%s", SERVER_DIR, filename);
	return fopen(path, flag);
}

void delete_file(char * filename) {
	char path[MAX_HEADER_SIZE];
	memset(path, 0, MAX_HEADER_SIZE);
	sprintf(path, "%s/%s", SERVER_DIR, filename);
	if (unlink(path) != 0) {
		perror("SERVER: unlink() failed!");
	}
	return;
}

/*
 * Primary function for DELETE
 */
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
		log_error(client_fd, current_client, err_no_such_file);
		write_reply_error(client_fd, current_client);
	} else {
		delete_file(current_client->filename);
		vector_erase(FILE_VECTOR, i);
		log_ok(client_fd, current_client);
	}
}

/*
 * Primary function for LIST
 */
void setup_list(int client_fd, client* current_client) {
	size_t total_file_size = 0;
	size_t current_file_size = 0;
	FILE * list_temp_file = tmpfile();	
	current_client->file = list_temp_file;
	char * current_filename = NULL;
	
	VECTOR_FOR_EACH(FILE_VECTOR, node, {
		current_filename = (char*) node;
		current_file_size = strlen(current_filename);
		total_file_size += current_file_size + 1;
		fwrite(current_filename, 1, current_file_size, list_temp_file);
		fwrite("\n", 1, 1, list_temp_file);
	});
	current_client->file_size = total_file_size - 1;
	log_ok(client_fd, current_client);
}
			
/*
 * Primary function for GET
 */
void setup_get(int client_fd, client* current_client) {
	int found = 0;
	char * filename = NULL;
	VECTOR_FOR_EACH(FILE_VECTOR, node, {
		filename = (char*) node;
		if (strcmp(filename, current_client->filename) == 0) {
			found = 1;
			break;
		}
	});
	if (found == 0) {
		log_error(client_fd, current_client, err_no_such_file);
	} else {
		current_client->file = open_file(current_client->filename, "r");
		if (!current_client->file) {
			log_error(client_fd, current_client, err_no_such_file);
			return;
		}
		fseek(current_client->file, 0, SEEK_END);
		current_client->file_size = ftell(current_client->file);
		fseek(current_client->file, 0, SEEK_SET);
		
		log_ok(client_fd, current_client);
	}	
}
			
/*
 * Primary function for PUT
 */
void setup_put(int client_fd, client* current_client) {
	current_client->state = READ_SIZE;
	current_client->offset = 0;
	current_client->file = open_file(current_client->filename, "w");
	if (current_client->file == NULL) {
		perror("SERVER: fopen() failed!");
		shutdown_client(client_fd);
	}
}

void write_reply_ok(int client_fd, client* current_client) {
	char * buffer = current_client->reply + current_client->offset;
	size_t count = strlen(buffer);
	int status = CONNECTED;
	ssize_t total_byte_written = server_write_all_to_socket(client_fd, buffer, count, &status);
	if (total_byte_written == -1) { //Something bad happened
		shutdown_client(client_fd);
		return;
	}
	if ((size_t)total_byte_written == count) {
		if (current_client->client_verb == PUT || current_client->client_verb == DELETE) {
			shutdown_client(client_fd);
			return;
		} else if (current_client->client_verb == LIST || current_client->client_verb == GET) {
			current_client->state = WRITE_SIZE;
			current_client->offset = 0;
			return;
		} else {
			perror("SERVER: abnormal VERB found in struct!");
			return;
		}
	}
	if (status == CONNECTION_LOST) {
		shutdown_client(client_fd);
		return;
	}
	current_client->offset += total_byte_written;
}

void write_reply_error(int client_fd, client* current_client) {
	char * buffer = current_client->reply + current_client->offset;
	size_t count = strlen(buffer);
	int status = CONNECTED;
	ssize_t total_byte_written = server_write_all_to_socket(client_fd, buffer, count, &status);
	if (total_byte_written == -1 ) { //Something bad happened
		shutdown_client(client_fd);
		return;
	}
	if ((size_t)total_byte_written == count) {
		shutdown_client(client_fd);
		return;
	}
	if (status == CONNECTION_LOST) {
		shutdown_client(client_fd);
		return;
	}
	current_client->offset += total_byte_written;
}

void reset_epoll_mode_to_write(int client_fd) {
	struct epoll_event ev;
	ev.events = EPOLLOUT;
	ev.data.fd = client_fd;
	epoll_ctl(EPOLL_FD, EPOLL_CTL_MOD, client_fd, &ev);
}

void log_ok(int client_fd, client* current_client) {
	char * buffer = current_client->reply;
	memset(buffer, 0, MAX_REPLY_SIZE);
	sprintf(buffer, "OK\n");
	current_client->state = WRITE_REPLY_OK;
	current_client->offset = 0;
	reset_epoll_mode_to_write(client_fd);
}

void log_error(int client_fd, client* current_client, const char* error_message) {
	char * buffer = current_client->reply;
	memset(buffer, 0, MAX_REPLY_SIZE);
	sprintf(buffer, "ERROR\n%s\n", error_message);
	current_client->state = WRITE_REPLY_ERROR;
	current_client->offset = 0;
	reset_epoll_mode_to_write(client_fd);
}
			
void read_size(int client_fd, client* current_client) {
	int status = CONNECTED;
	char * buffer = (char*) (&(current_client->file_size)) + current_client->offset;
	size_t count = sizeof(size_t) - current_client->offset;
	ssize_t total_byte_read = server_read_all_from_socket(client_fd, buffer, count, &status);
	
	if (total_byte_read < 0) { //something wrong with the response
		log_error(client_fd, current_client, err_bad_request);
		return;
	}
	
	if ((size_t)total_byte_read == count) {
		current_client->state = READ_PUT;
		current_client->file = open_file(current_client->filename, "w");
		if (current_client->file == NULL) {
			perror("SERVER: fopen() failed!");
			exit(1);
		}
		return;
	}
	current_client->offset += total_byte_read;
}

void write_size(int client_fd, client* current_client) {
	int status = CONNECTED;
	char * buffer = (char*) (&(current_client->file_size)) + current_client->offset;
	size_t count = sizeof(size_t) - current_client->offset;
	ssize_t total_byte_written = server_write_all_to_socket(client_fd, buffer, count, &status);
	
	if (total_byte_written < 0) {
		shutdown_client(client_fd);
		return;
	}
	if ((size_t)total_byte_written == count) {
		current_client->state = WRITE_FILE;
		current_client->offset = 0;
		return;
	}
	current_client->offset += total_byte_written;
}

void write_file(int client_fd, client* current_client) {
	FILE * file = current_client->file;
	size_t total_byte_written = ftell(file);
	size_t total_byte_to_write = current_client->file_size - total_byte_written;
	size_t current_byte_to_write = 0;
	ssize_t current_byte_written = 0;
	char line[MAX_R_W_SIZE];
	int status = CONNECTED;
	
	while (total_byte_to_write > 0) {
		memset(line, 0, MAX_R_W_SIZE);
		current_byte_to_write = (total_byte_to_write < MAX_R_W_SIZE) ? total_byte_to_write : MAX_R_W_SIZE;
		fread(line, 1, current_byte_to_write, file);
		current_byte_written = server_write_all_to_socket(client_fd, line, current_byte_to_write, &status);
		if (current_byte_written < 0 || status == CONNECTION_LOST) {
			fclose(file);
			shutdown_client(client_fd);
			return;
		}
		if ((size_t)current_byte_written < current_byte_to_write) { //rewind
			fseek(file, current_byte_written - current_byte_to_write, SEEK_CUR);
			break;
		}
		total_byte_written += current_byte_written;
		total_byte_to_write -= current_byte_written;
	}
}

void read_file(int client_fd, client* current_client) {
	FILE * file = current_client->file;
	size_t total_byte_read = ftell(file);
	size_t total_byte_to_read = current_client->file_size - total_byte_read;
	size_t current_byte_to_read = 0;
	ssize_t current_byte_read = 0;
	char line[MAX_R_W_SIZE];
	int status = CONNECTED;
	
	while (total_byte_to_read > 0) {
		memset(line, 0, MAX_R_W_SIZE);
		current_byte_to_read = (total_byte_to_read < MAX_R_W_SIZE) ? total_byte_to_read : MAX_R_W_SIZE;
		current_byte_read = server_read_all_from_socket(client_fd, line, current_byte_to_read, &status);
		if (current_byte_read < 0 || status == CONNECTION_LOST) {
			fclose(file);
			delete_file(file);
			log_error(client_fd, current_client, err_bad_file_size);
			return;
		}

		fwrite(line, 1, total_byte_read, file);
		
		if (current_byte_read < current_byte_to_read) {
			total_byte_read += current_byte_read;
			total_byte_to_read -= current_byte_read;
			break;
		}
		
		total_byte_read += current_byte_read;
		total_byte_to_read -= current_byte_read;
	}
	
	if (total_byte_to_read <= 0) { //finished
		fclose(file);
		//Test if overflow
		current_byte_read = server_read_all_from_socket(client_fd, line, MAX_R_W_SIZE, &status);
		if (current_byte_read != 0) {
			delete_file(file);
			log_error(client_fd, current_client, err_bad_file_size);
			return;
		}
		
		//PUT is successful
		vector_push_back(FILE_VECTOR, current_client->filename);
		log_ok(client_fd, current_client);
	}
}
		
			
void read_header(int client_fd, client* current_client) {
	char * buffer = current_client->header + current_client->offset;
	int count = MAX_HEADER_SIZE - current_client->offset;
	int status = ACTION_PAUSED;
	ssize_t total_byte_read = server_read_line_from_socket(client_fd, buffer, count, &status);
	
	if (total_byte_read == -1) { //Something bad happened
		log_error(client_fd, current_client, err_bad_request);
		return;
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
			return;
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
	rmdir(SERVER_DIR);
	vector_destroy(FILE_VECTOR);
	dictionary_destroy(CLIENT_DIC);
	close(EPOLL_FD);
}

void shutdown_client(int client_fd) {
	//Remove socket from EPOLL_FD
	epoll_ctl(EPOLL_FD, EPOLL_CTL_DEL, client_fd, NULL);

	//Close client socket
	shutdown(client_fd, SHUT_RDWR);
	close(client_fd);
	
	//Remove client*
	dictionary_remove(CLIENT_DIC, &client_fd);
}

void* client_copy_constructor(void* elem) {
	client * copy = malloc(sizeof(client));
	memcpy(copy, elem, sizeof(client));
	return copy;
}

void client_destructor(void* elem) {
	free(elem);
	return;
}

void dispatch_client(int client_fd) {
	if (!dictionary_contains(CLIENT_DIC, &client_fd)) {
		perror("SERVER: dictionary does not contain client_fd!");
		exit(1);
	}

	client* current_client = (client*) dictionary_get(CLIENT_DIC, &client_fd);
	if (current_client->state == READ_HEADER) {
		read_header(client_fd, current_client);
	} else if (current_client->state == READ_SIZE) {
		read_size(client_fd, current_client);
	} else if (current_client->state == READ_FILE) {
		read_file(client_fd, current_client);
	} else if (current_client->state == WRITE_FILE) {
		write_file(client_fd, current_client);
	} else if (current_client->state == WRITE_REPLY_OK) {
		write_reply_ok(client_fd, current_client);
	} else if (current_client->state == WRITE_REPLY_ERROR) {
		write_reply_error(client_fd, current_client);
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
				new_client.offset = 0;
				dictionary_set(CLIENT_DIC, &client_fd, &new_client);
			} else {
				dispatch_client(events[i].data.fd);
			}
		}
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
