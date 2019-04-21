/**
 * Nonstop Networking
 * CS 241 - Spring 2019
 */
#include "format.h"
#include "common.h"

#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

/* Status Macros */
#define ACTION_PAUSED 1
#define ACTION_COMPLETE 0

ssize_t server_read_all_from_socket(int socket, char * buffer, size_t count, int * status) {

}

ssize_t server_write_all_to_socket(int socket, char * buffer, size_t count) {
	ssize_t total_byte_written = 0;
	ssize_t current_byte_written = 0;
	while (total_byte_written < (ssize_t) count) {
		current_byte_written = write(socket, buffer + total_byte_written, count - total_byte_written);
		if (current_byte_written <= 0) {
			if (current_byte_written == -1 && errno == EINTR) { //retry
				continue;
			}
			if (current_byte_written == -1 && errno != EINTR) { //bad things happened
				return -1;
			}
			if (current_byte_written == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
				break;
			}
			if (current_byte_written == 0) {
				break;
			}
		}
		total_byte_written += current_byte_written;
	}
	return total_byte_written;
}

ssize_t server_read_line_from_socket(int socket, char * buffer, size_t count, int* status) {
	ssize_t total_byte_read = 0;
	ssize_t current_byte_read = 0;
	while (1) {
		current_byte_read = read(socket, buffer + total_byte_read, 1);
		if (current_byte_read <= 0) {
			if (current_byte_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
				break;
			}
			if (current_byte_read == -1 && errno == EINTR) { //retry
				continue;
			}
			if (current_byte_read == -1 && errno != EINTR) { //bad things happened
				return -1;
			}
			if (current_byte_read == 0) { //end prematurely
				return -1;
			}
		}
		if (buffer[total_byte_read] == '\n') {
			buffer[total_byte_read] = 0;
			total_byte_read += current_byte_read;
			*status = ACTION_COMPLETE;
			break;
		}
		total_byte_read += current_byte_read;
		if (total_byte_read == (ssize_t) count) {
			return -1;
		}
	}
	return total_byte_read;
}

size_t handle_return_value(ssize_t byte_executed, size_t byte_to_execute, size_t total_byte_to_execute) {
	if (byte_executed < 0) {
		print_connection_closed();
		return 1;
	}
	if ((size_t) byte_executed < byte_to_execute) {
		print_connection_closed();
		if (total_byte_to_execute - byte_executed > 0) {
			print_too_little_data();
		}
		return 1;
	}
	return 0;
}

ssize_t client_read_all_from_socket(int socket, char * buffer, size_t count) {
	ssize_t total_byte_read = 0;
	ssize_t current_byte_read = 0;
	while (total_byte_read < (ssize_t) count) {
		current_byte_read = read(socket, buffer + total_byte_read, count - total_byte_read);
		if (current_byte_read <= 0) {
			if (current_byte_read == -1 && errno == EINTR) {
				continue;
			}
			if (current_byte_read == -1 && errno != EINTR) {
				return -1;
			}
			if (current_byte_read == 0) {
				break;
			}
		}
		total_byte_read += current_byte_read;
	}
	return total_byte_read;
}

ssize_t client_read_line_from_socket(int socket, char * buffer, size_t count) {
	ssize_t total_byte_read = 0;
	ssize_t current_byte_read = 0;
	while (1) {
		current_byte_read = read(socket, buffer + total_byte_read, 1);
		if (current_byte_read <= 0) {
			if (current_byte_read == -1 && errno == EINTR) { //retry
				continue;
			}
			if (current_byte_read == -1 && errno != EINTR) { //bad things happened
				return -1;
			}
			if (current_byte_read == 0) { //end prematurely
				return -1;
			}
		}
		if (buffer[total_byte_read] == '\n') {
			buffer[total_byte_read] = 0;
			total_byte_read += current_byte_read;
			break;
		}
		total_byte_read += current_byte_read;
		if (total_byte_read == (ssize_t) count) {
			return -1;
		}
	}
	return total_byte_read;
}

ssize_t client_write_all_to_socket(int socket, const char* buffer, size_t count) {
	ssize_t total_byte_written = 0;
	ssize_t current_byte_written = 0;
	while (total_byte_written < (ssize_t) count) {
		current_byte_written = write(socket, buffer + total_byte_written, count - total_byte_written);
		if (current_byte_written <= 0) {
			if (current_byte_written == -1 && errno == EINTR) { //retry
				continue;
			}
			if (current_byte_written == -1 && errno != EINTR) { //bad things happened
				return -1;
			}
			if (current_byte_written == 0) {
				break;
			}
		}
		total_byte_written += current_byte_written;
	}
	return total_byte_written;
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
