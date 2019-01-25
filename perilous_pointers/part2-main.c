/**
 * Perilous Pointers Lab
 * CS 241 - Spring 2019
 */
 
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    //1
    first_step(81);

    //2
    int* input_2 = malloc(sizeof(int));
    *input_2 = 132;
    second_step(input_2);
    free(input_2);

    //3
    int** input_3 = malloc(sizeof(int*));
    *input_3 = malloc(sizeof(int));
    **input_3 = 8942;
    double_step(input_3);
    free(*input_3);
    free(input_3);
    
    //4
    int* input_4 = malloc(sizeof(int));
    *input_4 = 15; 
    strange_step(((char*) input_4) - 5);
    free(input_4);
    
    return 0;
}
