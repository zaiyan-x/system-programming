/**
 * Charming Chatroom
 * CS 241 - Spring 2019
 * Collab with Eric Wang - wcwang2
 */


#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

#define MAX_CLIENTS 8

void *process_client(void *p);

static volatile int serverSocket;
static volatile int endSession;

static volatile int clientsCount;
static volatile int clients[MAX_CLIENTS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Signal handler for SIGINT.
 * Used to set flag to end server.
 */
void close_server() {
    endSession = 1;
    // add any additional flags here you want.
}

/**
 * Cleanup function called in main after `run_server` exits.
 * Server ending clean up (such as shutting down clients) should be handled
 * here.
 */
void cleanup() {
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown():");
    }
    close(serverSocket);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            if (shutdown(clients[i], SHUT_RDWR) != 0) {
                perror("shutdown(): ");
            }
            close(clients[i]);
        }
    }
}

/**
 * Sets up a server connection.
 * Does not accept more than MAX_CLIENTS connections.  If more than MAX_CLIENTS
 * clients attempts to connects, simply shuts down
 * the new client and continues accepting.
 * Per client, a thread should be created and 'process_client' should handle
 * that client.
 * Makes use of 'endSession', 'clientsCount', 'client', and 'mutex'.
 *
 * port - port server will run on.
 *
 * If any networking call fails, the appropriate error is printed and the
 * function calls exit(1):
 *    - fprtinf to stderr for getaddrinfo
 *    - perror() for any other call
 */
void run_server(char *port) {
	int fd = socket(AF_INET, SOCK_STREAM, 0); 
	int val = 1; 
	int fail = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)); 
	if (fail) { 
		perror("Failed!\n"); 
		exit(1); 
	} 
	struct addrinfo hints; 
	struct addrinfo* result; 
	memset(&hints, 0, sizeof(struct addrinfo)); 
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_STREAM; 
	hints.ai_flags = AI_PASSIVE; 
	fail = getaddrinfo(NULL, port, &hints, &result); 
	if (fail) { 
		perror("Failed!\n"); 
		exit(1); 
	} 
	if (bind(fd, result -> ai_addr, result -> ai_addrlen)) { 
		perror("Failed!\n"); 
		exit(1); 
	} 
	if (listen(fd, 10)) { 
		perror("Failed!\n"); 
		exit(1); 
	} 
	for (size_t i = 0; i < MAX_CLIENTS; i++) clients[i] = -1; 
	pthread_t workers[MAX_CLIENTS]; 
	serverSocket = fd; 
	while (!endSession) { 
		int client_fd = accept(fd, NULL, NULL); 
		if (client_fd == -1) break; 
		long l; 
		pthread_mutex_lock(&mutex); 
		for (l = 0; l < MAX_CLIENTS; l++) { 
			if (clients[l] == -1) break; 
		} 
		if (l == MAX_CLIENTS) { 
			if (shutdown(client_fd, SHUT_RDWR)) perror("Failed!\n"); 
			close(client_fd); 
			continue; 
		} 
		clients[l] = client_fd; 
		clientsCount++; 
		pthread_mutex_unlock(&mutex); 
		pthread_create(&workers[l], NULL, process_client, (void*)l); 
	} 
	freeaddrinfo(result); 
}

/**
 * Broadcasts the message to all connected clients.
 *
 * message  - the message to send to all clients.
 * size     - length in bytes of message to send.
 */
void write_to_clients(const char *message, size_t size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            ssize_t retval = write_message_size(size, clients[i]);
            if (retval > 0) {
                retval = write_all_to_socket(clients[i], message, size);
            }
            if (retval == -1) {
                perror("write(): ");
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * Handles the reading to and writing from clients.
 *
 * p  - (void*)intptr_t index where clients[(intptr_t)p] is the file descriptor
 * for this client.
 *
 * Return value not used.
 */
void *process_client(void *p) {
    pthread_detach(pthread_self());
    intptr_t clientId = (intptr_t)p;
    ssize_t retval = 1;
    char *buffer = NULL;

    while (retval > 0 && endSession == 0) {
        retval = get_message_size(clients[clientId]);
        if (retval > 0) {
            buffer = calloc(1, retval);
            retval = read_all_from_socket(clients[clientId], buffer, retval);
        }
        if (retval > 0)
            write_to_clients(buffer, retval);

        free(buffer);
        buffer = NULL;
    }

    printf("User %d left\n", (int)clientId);
    close(clients[clientId]);

    pthread_mutex_lock(&mutex);
    clients[clientId] = -1;
    clientsCount--;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s <port>\n", argv[0]);
        return -1;
    }

    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = close_server;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    run_server(argv[1]);
    cleanup();
    pthread_exit(NULL);
}
