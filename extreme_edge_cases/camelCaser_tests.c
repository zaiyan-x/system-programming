/**
 * Extreme Edge Cases Lab
 * CS 241 - Spring 2019
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int cal_len_str_arr(char** input_array) {
	int result = 0;
	while(*input_array) {
		result++;
		input_array++;
	}
	printf("%d\n", result + 1);
	return result + 1;
}


/*
 * Testing function for various implementations of camelCaser.
 *
 * @param  camelCaser   A pointer to the target camelCaser function.
 * @param  destroy      A pointer to the function that destroys camelCaser
 * output.
 * @return              Correctness of the program (0 for wrong, 1 for correct).
 */
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
	// TODO: Return 1 if the passed in function works properly; 0 if it doesn't.
	// Test 1
	char** solution_1 = malloc(9 * sizeof(char*));
	solution_1[0] = "";
	solution_1[1] = "";
	solution_1[2] = "";
	solution_1[3] = "";
	solution_1[4] = "weedIsNotLegalInUsa";
	solution_1[5] = "soYou";
	solution_1[6] = "dBetterNotSmokeThat";
	solution_1[7] = "";
	solution_1[8] = NULL;

	char** user_solution_1 = (*camelCaser)(".###Weed is not legal in USA.      So you'd bettter not smoke that.        .         ");

	if(cal_len_str_arr(user_solution_1) != cal_len_str_arr(solution_1)) {
		(*destroy) (user_solution_1);
		free(solution_1);
		return 0;
	}//Test amount of sentences

	int curr_sentence = 0;
	while(user_solution_1[curr_sentence]) {
	printf("SOL:%d. %s\n",curr_sentence,  solution_1[curr_sentence]);
	printf("USER:%d. %s\n",curr_sentence,  user_solution_1[curr_sentence]);	
		if(strcmp(solution_1[curr_sentence], user_solution_1[curr_sentence])) {
			(*destroy)(user_solution_1);
			free(solution_1);
			return 0;
		}
	curr_sentence++;
	}
    return 1;
}
