/**
 * Charming Chatroom
 * CS 241 - Spring 2019
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
	ssize_t message_size = htonl(size);
	ssize_t byte_written = write_all_to_socket(socket, (char*) &message_size, MESSAGE_SIZE_DIGITS);
	if (byte_written == 0 || byte_written == -1) {
		return byte_written;
	}
    return message_size;
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
	ssize_t byte_to_read = count;
	ssize_t byte_read = 0;
	while (byte_to_read) {
		byte_read = read(socket, buffer + count - byte_to_read, byte_to_read);
		if (byte_read <= 0) {
			if (byte_read == -1 && errno == EINTR) {
				continue;
			}
		}
		byte_to_read -= byte_read;
	}
	return count;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    ssize_t byte_to_write = count;
	ssize_t byte_written = 0;
	while (byte_to_write) {
		byte_written = write(socket, buffer + count - byte_to_write, byte_to_write);
		if (byte_written <= 0) {
			if (byte_written == -1 && errno == EINTR) {
				continue;
			}
			perror(strerror(byte_written));
			return byte_written;
		}
		byte_to_write -= byte_written;
	}
	return count;
}
