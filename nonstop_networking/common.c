/**
 * Nonstop Networking
 * CS 241 - Spring 2019
 */
#include "common.h"

ssize_t client_write_all_to_socket(int socket, const char* buffer, size_t count) {
	ssize_t byte_written = 0;
	ssize_t current_byte_written = 0;
	while (byte_written < (ssize_t) count
}
