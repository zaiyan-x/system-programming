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
	char** solution_1 = malloc(27 * sizeof(char*));
	solution_1[0] = "";
	solution_1[1] = "1\x02";
	solution_1[2] = "3";
	solution_1[3] = "4";
	solution_1[4] = "5";
	solution_1[5] = "6";
	solution_1[6] = "7";
	solution_1[7] = "8";
	solution_1[8] = "9";
	solution_1[9] = "10";
	solution_1[10] = "11";
	solution_1[11] = "12";
	solution_1[12] = "13";
	solution_1[13] = "14";
	solution_1[14] = "15";
	solution_1[15] = "16";
	solution_1[16] = "17";
	solution_1[17] = "18";
	solution_1[18] = "19";
	solution_1[19] = "20";
	solution_1[20] = "21";
	solution_1[21] = "22";
	solution_1[22] = "23";
	solution_1[23] = "weedIsNotLegalInUsa";
	solution_1[24] = "soYouHadBetterNotSmokeThat";
	solution_1[25] = "";
	solution_1[26] = NULL;

	char** user_solution_1 = (*camelCaser)(".1\2:3[4]5{6}7'8\"9;10?11/12,13+14-15!16#17@18(19)20^21&22%23$ Weed is not legal in USA. So you had better not smoke that.        .ignore this");

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
	(*destroy)(user_solution_1);
	free(solution_1);

	//Test 2
	char** solution_2 = (*camelCaser)(NULL);
	if (solution_2 != NULL) {
		return 0;
	}

	//Test 3
	char input_3[65];
	int j;
	for(j = 1; j < 32; j++) {
		input_3[j * 2 - 2] = j;
		input_3[j * 2 - 1] = '.';
	}
	input_3[62] = 127;
	input_3[63] = '.';
	input_3[64] = 0;

	char** user_solution_3 = (*camelCaser)(input_3);
	char** solution_3 = malloc(33 * sizeof(char*));
	solution_3[0] = "\x01";
	solution_3[1] = "\x02";
	solution_3[2] = "\x03";
	solution_3[3] = "\x04";
	solution_3[4] = "\x05";
	solution_3[5] = "\x06";
	solution_3[6] = "\a";
	solution_3[7] = "\b";
	solution_3[8] = "";
	solution_3[9] = "";
	solution_3[10] = "";
	solution_3[11] = "";
	solution_3[12] = "";
	solution_3[13] = "\x0E";
	solution_3[14] = "\x0F";
	solution_3[15] = "\x10";
	solution_3[16] = "\x11";
	solution_3[17] = "\x12";
	solution_3[18] = "\x13";
	solution_3[19] = "\x14";
	solution_3[20] = "\x15";
	solution_3[21] = "\x16";
	solution_3[22] = "\x17";
	solution_3[23] = "\x18";
	solution_3[24] = "\x19";
	solution_3[25] = "\x1A";
	solution_3[26] = "\x1B";
	solution_3[27] = "\x1C";
	solution_3[28] = "\x1D";
	solution_3[29] = "\x1E";
	solution_3[30] = "\x1F";
	solution_3[31] = "\x7F";
	solution_3[32] = NULL;

	if(cal_len_str_arr(user_solution_3) != cal_len_str_arr(solution_3)) {
		(*destroy) (user_solution_3);
		free(solution_3);
		return 0;
	}//Test amount of sentences

	curr_sentence = 0;
	while(user_solution_3[curr_sentence]) {
		printf("SOL:%d. %s\n",curr_sentence,  solution_3[curr_sentence]);
		printf("USER:%d. %s\n",curr_sentence,  user_solution_3[curr_sentence]);	
		if(strcmp(solution_3[curr_sentence], user_solution_3[curr_sentence])) {
			(*destroy)(user_solution_3);
			free(solution_3);
			return 0;
		}
		curr_sentence++;
	}
	(*destroy)(user_solution_3);
	free(solution_3);

	//Test 4
	char* input_4 = "The Heisenbug is an incredibal creature. Facenovel servers get their power from its indeterminism. Code smell can be ignored with INCREDIBLE use of air freshener. God objects are the new religion.";

	char** user_solution_4 = (*camelCaser)(input_4);
	char** solution_4 = malloc(5 * sizeof(char*));

	solution_4[0] = "theHeisenbugIsAnIncredibalCreature";
	solution_4[1] = "facenovelServersGetTheirPowerFromItsIndeterminism";
	solution_4[2] = "codeSmellCanBeIgnoredWithIncredibleUseOfAirFreshener";
	solution_4[3] = "godObjectsAreTheNewReligion";
	solution_4[4] = NULL;
	if(cal_len_str_arr(user_solution_4) != cal_len_str_arr(solution_4)) {
		(*destroy) (user_solution_4);
		free(solution_4);
		return 0;
	}//Test amount of sentences

	curr_sentence = 0;
	while(user_solution_4[curr_sentence]) {
		printf("SOL:%d. %s\n",curr_sentence,  solution_4[curr_sentence]);
		printf("USER:%d. %s\n",curr_sentence,  user_solution_4[curr_sentence]);	
		if(strcmp(solution_4[curr_sentence], user_solution_4[curr_sentence])) {
			(*destroy)(user_solution_4);
			free(solution_4);
			return 0;
		}
		curr_sentence++;
	}
	(*destroy)(user_solution_4);
	free(solution_4);

	//Test 5
	char* input_5 = "~!@#$%^&*()_+`-=[];',.{}|:*<>";
	char** user_solution_5 = (*camelCaser)(input_5);
	char** solution_5 = malloc(30 * sizeof(char*));

	for(j = 0; j < 29; j++) {
		solution_5[j] = ""; 
	}
	solution_5[29] = NULL;
	if(cal_len_str_arr(user_solution_5) != cal_len_str_arr(solution_5)) {
		(*destroy) (user_solution_5);
		free(solution_5);
		return 0;
	}//Test amount of sentences

	curr_sentence = 0;
	while(user_solution_5[curr_sentence]) {
		printf("SOL:%d. %s\n",curr_sentence,  solution_5[curr_sentence]);
		printf("USER:%d. %s\n",curr_sentence,  user_solution_5[curr_sentence]);	
		if(strcmp(solution_5[curr_sentence], user_solution_5[curr_sentence])) {
			(*destroy)(user_solution_5);
			free(solution_5);
			return 0;
		}
		curr_sentence++;
	}
	(*destroy)(user_solution_5);
	free(solution_5);


	
	return 1;
}
