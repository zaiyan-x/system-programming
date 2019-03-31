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
	vector * vector_1 = vector_create(string_copy_constructor,
									  string_destructor,
									  string_default_constructor);
	
	vector_push_back(vector_1, (void*) "Hello World!");
	
	puts(vector_get(vector_1, 0));	 	
}

