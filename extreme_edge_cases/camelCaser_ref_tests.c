/**
 * Extreme Edge Cases Lab
 * CS 241 - Spring 2019
 */
 
#include "camelCaser.h"
#include "camelCaser_ref_utils.h"
#include <unistd.h>

int main() {
    // Enter the string you want to test with the reference here
    /* char input[65];

    int j = 0;
    for(j=1;j<32;j++){
    	input[j*2-2]=j;
    	input[j*2-1]='.';
    }
    input[62] = 127;
    input[63] = '.';
    input[64] = 0; */

    /*char input[3];
    input[0] = 0;
    input[1] = '.';
    input[2] = 0;*/

    char* input = "~!@#$%^&*()_+`-=[];',.{}|:*<>";
    // This function prints the reference implementation output on the terminal
    print_camelCaser(input);

    // Enter your expected output as you would in camelCaser_utils.c
    char **output = NULL;
    // This function compares the expected output you provided against
    // the output from the reference implementation
    // The function returns 1 if the outputs match, and 0 if the outputs
    // do not match
    return check_output(input, output);
}
