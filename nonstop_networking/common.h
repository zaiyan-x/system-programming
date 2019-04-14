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

ssize_t client_write_all_to_socket(int socket, const char * buffer, size_t count); 
ssize_t client_read_line_from_socket(int socket, char * buffer, size_t count);
ssize_t client_read_all_from_socket(int socket, char * buffer, size_t count);

//Helper functions
char** parse_args(int argc, char** argv);
verb check_args(char** args);
