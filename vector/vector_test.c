/*
*	vector Lab
*	CS 241 - Spring 2019
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "callbacks.h"
#include "vector.h"

// Test your vector here
int main() { 
	vector* test = vector_create(NULL, NULL, NULL);
	vector_resize(test, 13);
	vector_resize(test, 2);
	size_t cap = vector_capacity(test);
	size_t size = vector_size(test);
	printf("new cap: %zu\n", cap);
	printf("new size: %zu\n", size);
	vector_destroy(test);
}

