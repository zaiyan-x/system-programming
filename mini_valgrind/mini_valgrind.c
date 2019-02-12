/**
 * Mini Valgrind Lab
 * CS 241 - Spring 2019
 */
 
#include "mini_valgrind.h"
#include <stdio.h>
#include <string.h>

meta_data* head = NULL;
size_t total_memory_requested = 0;
size_t total_memory_freed = 0;
size_t invalid_addresses = 0;

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
	if (request_size == 0) {
		return NULL;
	}
	meta_data* alloc_block = malloc(sizeof(meta_data) + request_size);
	if (alloc_block == NULL) {
		return NULL;
	}
	alloc_block->filename = filename;
	alloc_block->request_size = request_size;
	alloc_block->instruction = instruction;
	alloc_block->next = head;
	head = alloc_block;
	total_memory_requested += request_size;
    return (alloc_block + 1);
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
	if (num_elements == 0 || element_size == 0) {
		return NULL;
	}
	meta_data* alloc_block = (meta_data*)  mini_malloc(num_elements * element_size, filename, instruction);
	if (alloc_block == NULL) {
		return NULL;
	}
	memset(alloc_block, 0, num_elements * element_size);
    return alloc_block;
}

void *mini_realloc(void *ptr, size_t request_size, const char *filename,
                   void *instruction) {
    if (ptr == NULL) {
		return mini_malloc(request_size, filename, instruction);
    }
	if (request_size == 0) {
		mini_free(ptr);
		return NULL;
	}
	meta_data* block_metadata = (meta_data*) ptr - sizeof(meta_data);
	meta_data* curr = head;
	meta_data* prev = NULL;	
	while (curr != NULL && curr != block_metadata) {
		prev = curr;
		curr = curr->next;
	}
	if (curr == NULL) {//NO SPECIFIED PTR FOUND IN LINKED LIST
		invalid_addresses++; //INVALID++
		return NULL;
	}
	
	int realloc_size = ((int) curr->request_size) - ((int) request_size);
	meta_data* realloc_block = realloc(curr, request_size + sizeof(meta_data));
	curr = NULL; //defunct curr ptr	
	//UPDATE SIZE
	realloc_block->request_size = request_size;
	realloc_block->filename = filename;
	realloc_block->instruction = instruction;
	
	if (prev == NULL) { //WHILE LOOP PREV NOT CHANGED = (head = curr)
		head = realloc_block;
	} else {
		prev->next = realloc_block;
	}

	//UPDATE GLOBALs
	if (realloc_size < 0) {//shrink
		total_memory_requested -= realloc_size;
	} else if (realloc_size > 0) {
		total_memory_freed += realloc_size;
	} else { //NOTHING HAPPENED
		//DO NOTHING
	}

	return (realloc_block + 1);
}

void mini_free(void *ptr) {
    if (ptr == NULL) {
		return;
	}
	if (head == NULL) {
		invalid_addresses++;
		return;
	}
	meta_data* block_metadata = (meta_data*) ptr - sizeof(meta_data);
	if (block_metadata == head) {
		total_memory_freed += head->request_size;
		head = head->next;
		free(block_metadata);
		block_metadata = NULL;
		return;
	}
	meta_data* curr = head;
	meta_data* prev = NULL;
	while (curr != NULL && curr != block_metadata) {
		prev = curr;
		curr = curr->next;
	}
	if (curr == NULL) {//WHILE NO CHANGE
		invalid_addresses++;
	} else {
		total_memory_freed += block_metadata->request_size;
		prev->next = block_metadata->next;
		free(block_metadata);
		block_metadata = NULL;
	}
	return;
}
