/**
 * Critical Concurrency Lab
 * CS 241 - Spring 2019
 * Callob with Eric Wang - wcwang2
 */

// this file is collaborated with Ryan Xu (zxu43) 

#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
	queue* q = malloc(sizeof(queue)); 
	q -> head = NULL; 
	q -> tail = NULL; 
	q -> size = 0; 
	q -> max_size = (max_size > 0) ? max_size : 0xfffff;  
	pthread_mutex_init(&(q -> m), NULL); 
	pthread_cond_init(&(q -> cv), NULL); 
    return q;
}

void queue_destroy(queue *this) {
    /* Your code here */
	queue_node* tempt = this -> head; 
	while (tempt) { 
		queue_node* q = tempt; 
		tempt = tempt -> next; 
		free(q); 
	} 
	pthread_mutex_destroy(&(this -> m)); 
	pthread_cond_destroy(&(this -> cv)); 
	free(this); 
}

void queue_push(queue *this, void *data) {
    /* Your code here */
	pthread_mutex_lock(&this -> m);
 while (this -> max_size > 0 && this -> size == this -> max_size) pthread_cond_wait(&this -> cv, &this -> m); 
	queue_node* q = malloc(sizeof(queue_node)); 
	q -> next = NULL; 
	q -> data = data; 
	//while (this -> max_size > 0 && this -> size == this -> max_size) pthread_cond_wait(&this -> cv, &this -> m); 
	if (this -> size == 0) { 
		this -> head = q; 
		this -> tail = q; 
	} 
	else { 
		this -> tail -> next = q; 
		this -> tail = q; 
	} 
	this -> size++; 
	if (this -> size == 1) pthread_cond_broadcast(&(this -> cv)); 
	pthread_mutex_unlock(&this -> m); 
}

void *queue_pull(queue *this) {
    /* Your code here */
    pthread_mutex_lock(&this -> m); 
	while (this -> size == 0) pthread_cond_wait(&this -> cv, &this -> m); 
	void* result = this -> head -> data; 
	queue_node* tempt = this -> head; 
	this -> head = this -> head -> next; 
	tempt -> next = NULL; 
	free(tempt); 
	if (this -> size == 1) { 
		this -> tail = NULL; 
	} 
	this -> size--; 
	if (this -> size == this -> max_size - 1) pthread_cond_broadcast(&(this -> cv)); 
	pthread_mutex_unlock(&this -> m); 
	return result; 
}

