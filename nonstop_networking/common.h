/**
 * Nonstop Networking
 * CS 241 - Spring 2019
 */
#pragma once
#include <stddef.h>
#include <sys/types.h>

#define LOG(...)                      \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
    } while (0);

typedef enum { GET, PUT, DELETE, LIST, V_UNKNOWN } verb;

/* client.c */
ssize_t client_write_all_to_socket(int socket, const char * buffer, size_t count); 
ssize_t client_read_line_from_socket(int socket, char * buffer, size_t count);
ssize_t client_read_all_from_socket(int socket, char * buffer, size_t count);
size_t handle_return_value(ssize_t byte_executed, size_t byte_to_execute, size_t total_byte_to_execute);

/* server.c */
ssize_t server_read_line_from_socket(int socket, char * buffer, size_t count, int* status);
ssize_t server_write_all_to_socket(int socket, char * buffer, size_t count);
ssize_t server_read_all_from_socket(int socket, char *buffer, size_t count, int* status);

//Helper functions
char** parse_args(int argc, char** argv);
verb check_args(char** args);
