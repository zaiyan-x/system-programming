/**
 * Mini Valgrind Lab
 * CS 241 - Spring 2019
 */
 
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Your tests here using malloc and free
	void *p1 = malloc(30);
	void *p2 = malloc(40);
	void *p3 = malloc(50);
	free(p2);
    return 0;
}
