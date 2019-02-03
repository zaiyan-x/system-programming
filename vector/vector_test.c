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
	// Initialize
	vector* test_vector = vector_create(int_copy_constructor, int_destructor, int_default_constructor);

	// TEST 1
	int iter;
	int a = 10;
	for (iter = 0; iter < 1000; iter++) {
		vector_push_back(test_vector, &a);
	}	
	vector_destroy(test_vector);
	printf("TEST1 COMPLETE with SIZE: %zu and CAP %zu.\n", vector_size(test_vector), vector_capacity(test_vector));

	// TEST 2	
	test_vector = vector_create(int_copy_constructor, int_destructor, int_default_constructor);


	return 0;
}

