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
   
    //5
    char* input_5 = malloc(sizeof(char) * 4);
    input_5[3] = 0;
    empty_step(input_5);
    free(input_5);

    //6
    char* input_6_2 = malloc(5 * sizeof(char));
    void* input_6_1 = input_6_2;
    strcpy(input_6_2, "000u");
    two_step(input_6_1, input_6_2);
    free(input_6_2); 

    //7
    char* input_7 = malloc(5 * sizeof(char));
    three_step(input_7, input_7+2, input_7+4);
    free(input_7);

    //8
    char* input_8 = malloc(5 * sizeof(char));
    strcpy(input_8, "02:B");
    step_step_step(input_8, input_8, input_8);
    free(input_8);

    //9
    char* input_9 = malloc(sizeof(char));
    *input_9 = 90;
    it_may_be_odd(input_9, 90);
    free(input_9);

    //10
    char* input_10 = malloc(8 * sizeof(char));
    strcpy(input_10, " ,CS241");
    tok_step(input_10);
    free(input_10);

    //11
    char* input_11 = malloc(4 * sizeof(char));
    input_11[0] = 1;
    input_11[1] = 0;
    input_11[2] = 0;
    input_11[3] = 2;
    the_end(input_11, input_11);
    free(input_11); 
    return 0;
}
   
