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

	//Test 6
/*	char input[121];
	//1
        input[0] = 97;
        input[1] = 46;
        input[2] = 1;
        input[3] = 32;
        input[4] = 33;
        //2
        input[5] = 97;
        input[6] = 46;
        input[7] = 32;
        input[8] = 1;
        input[9] = 33;
        //3
        input[10] = 97;
        input[11] = 1;
        input[12] = 46;
        input[13] = 32;
        input[14] = 33;
        //4
        input[15] = 97;
        input[16] = 1;
        input[17] = 32;
	input[18] = 46;
        input[19] = 33;
        //5
        input[20] = 97;
        input[21] = 32;
        input[22] = 46;
        input[23] = 1;
        input[24] = 33;
        //6
        input[25] = 97;
        input[26] = 32;
        input[27] = 1;
        input[28] = 46;
        input[29] = 33;
        //7
        input[30] = 46;
        input[31] = 97;
        input[32] = 1;
        input[33] = 32;
        input[34] = 33;
        //8
        input[35] = 46;
	input[36] = 97;
        input[37] = 32;
        input[38] = 1;
        input[39] = 33;
        //9
        input[40] = 46;
        input[41] = 1;
        input[42] = 97;
        input[43] = 32;
        input[44] = 33;
        //10
        input[45] = 46;
        input[46] = 1;
        input[47] = 32;
        input[48] = 97;
        input[49] = 33;
        //11
        input[50] = 46;
        input[51] = 32;
        input[52] = 97;
        input[53] = 1;
        input[54] = 33;
	//12
        input[55] = 46;
        input[56] = 32;
        input[57] = 1;
        input[58] = 97;
        input[59] = 33;
        //13
        input[60] = 1;
        input[61] = 97;
        input[62] = 46;
        input[63] = 32;
        input[64] = 33;
        //14
        input[65] = 1;
        input[66] = 97;
        input[67] = 32;
        input[68] = 46;
        input[69] = 33;
        //15
        input[70] = 1;
        input[71] = 46;
        input[72] = 97;
        input[73] = 32;
	input[74] = 33;
        //16
        input[75] = 1;
        input[76] = 46;
        input[77] = 32;
        input[78] = 97;
        input[79] = 33;
        //17
        input[80] = 1;
        input[81] = 32;
        input[82] = 97;
        input[83] = 46;
        input[84] = 33;
        //18
        input[85] = 1;
        input[86] = 32;
        input[87] = 46;
        input[88] = 97;
        input[89] = 33;
        //19
        input[90] = 32;
        input[91] = 97;
        input[92] = 46;
	input[93] = 1;
        input[94] = 33;
        //20
        input[95] = 32;
        input[96] = 97;
        input[97] = 1;
        input[98] = 46;
        input[99] = 33;
        //21
        input[100] = 32;
        input[101] = 46;
        input[102] = 97;
        input[103] = 1;
        input[104] = 33;
        //22
        input[105] = 32;
        input[106] = 46;
        input[107] = 1;
        input[108] = 97;
        input[109] = 33;
        //23
        input[110] = 32;
        input[111] = 1;
	input[112] = 97;
        input[113] = 46;
        input[114] = 33;
        //24
        input[115] = 32;
        input[116] = 1;
        input[117] = 46;
        input[118] = 97;
        input[119] = 33;

        //END char
        input[120] = 0;	*/
	char* input_6 = "a.\001 !a. \001!a\001. !a\001 .!a .\001!a \001.!.a\001 !.a \001!.\001a !.\001 a!. a\001!. \001a!\001a. !\001a .!\001.a !\001. a!\001 a.!\001 .a! a.\001! a\001.! .a\001! .\001a! \001a.! \001.a!";
	char** user_solution_6 = (*camelCaser)(input_6);
	char** solution_6 = malloc(48 * sizeof(char*));

	solution_6[0] = "a";
	solution_6[1] = "\001";
	solution_6[2] = "a";
	solution_6[3] = "\001";
	solution_6[4] = "a\001";
	solution_6[5] = "";
	solution_6[6] = "a\001";
	solution_6[7] = "";
	solution_6[8] = "a";
	solution_6[9] = "\001";
	solution_6[10] = "a\001";
	solution_6[11] = "";
	solution_6[12] = "";
	solution_6[13] = "a\001";
	solution_6[14] = "";
	solution_6[15] = "a\001";
	solution_6[16] = "";
	solution_6[17] = "\001a";
	solution_6[18] = "";	
	solution_6[19] = "\001A";
	solution_6[20] = "";
	solution_6[21] = "a\001";
	solution_6[22] = "";
	solution_6[23] = "\001a";
	solution_6[24] = "\001a";
	solution_6[25] = "";
	solution_6[26] = "\001a";
	solution_6[27] = "";
	solution_6[28] = "\001";	
	solution_6[29] = "a";
	solution_6[30] = "\001";
	solution_6[31] = "a";
	solution_6[32] = "\001A";
	solution_6[33] = "";
	solution_6[34] = "\001";
	solution_6[35] = "a";
	solution_6[36] = "a";	
	solution_6[37] = "\001";
	solution_6[38] = "a\001";
	solution_6[39] = "";
	solution_6[40] = "";
	solution_6[41] = "a\001";
	solution_6[42] = "";
	solution_6[43] = "\001a";
	solution_6[44] = "\001a";
	solution_6[45] = "";
	solution_6[46] = "\001";
	solution_6[47] = "a";
	solution_6[48] = NULL;
	if(cal_len_str_arr(user_solution_6) != cal_len_str_arr(solution_6)) {
		(*destroy) (user_solution_6);
		free(solution_6);
		return 0;
	}//Test amount of sentences

	curr_sentence = 0;
	while(user_solution_6[curr_sentence]) {
		printf("SOL:%d. %s\n",curr_sentence,  solution_6[curr_sentence]);
		printf("USER:%d. %s\n",curr_sentence,  user_solution_6[curr_sentence]);	
		if(strcmp(solution_6[curr_sentence], user_solution_6[curr_sentence])) {
			(*destroy)(user_solution_6);
			free(solution_6);
			return 0;
		}
		curr_sentence++;
	}
	(*destroy)(user_solution_6);
	free(solution_6);

	//Test 7
	char* input_7 = "1Hello       World     .";
	char** user_solution_7 = (*camelCaser)(input_7);
	char** solution_7 = malloc(2 * sizeof(char*));
	solution_7[0] = "1helloWorld";
	solution_7[1] = NULL;
	if(cal_len_str_arr(user_solution_7) != cal_len_str_arr(solution_7)) {
		(*destroy) (user_solution_7);
		free(solution_6);
		return 0;
	}//Test amount of sentences

	curr_sentence = 0;
	while(user_solution_7[curr_sentence]) {
		printf("SOL:%d. %s\n",curr_sentence,  solution_7[curr_sentence]);
		printf("USER:%d. %s\n",curr_sentence,  user_solution_7[curr_sentence]);	
		if(strcmp(solution_7[curr_sentence], user_solution_7[curr_sentence])) {
			(*destroy)(user_solution_7);
			free(solution_7);
			return 0;
		}
		curr_sentence++;
	}
	(*destroy)(user_solution_7);
	free(solution_7);


	//Test 8
	char* input_8 = "dEAL wItH ThIs Mother*";
	char** user_solution_8 = (*camelCaser) (input_8);
	char** solution_8 = malloc(2 * sizeof(char*));
	solution_8[0] = "dealWithThisMother";
	solution_8[1] = NULL;
	if(cal_len_str_arr(user_solution_8) != cal_len_str_arr(solution_8)) {
		(*destroy) (user_solution_8);
		free(solution_8);
		return 0;
	}//Test amount of sentences

	curr_sentence = 0;
	while(user_solution_8[curr_sentence]) {
		printf("SOL:%d. %s\n",curr_sentence,  solution_8[curr_sentence]);
		printf("USER:%d. %s\n",curr_sentence,  user_solution_8[curr_sentence]);	
		if(strcmp(solution_8[curr_sentence], user_solution_8[curr_sentence])) {
			(*destroy)(user_solution_8);
			free(solution_8);
			return 0;
		}
		curr_sentence++;
	}
	(*destroy)(user_solution_8);
	free(solution_8);


	//Test 9
	char* input_9 = "2HELLO WORLD     FUCK.";
	char** user_solution_9 = (*camelCaser) (input_9);
	char** solution_9 = malloc(2 * sizeof(char*));
	solution_9[0] = "2helloWorldFuck";
	solution_9[1] = NULL;
	if(cal_len_str_arr(user_solution_9) != cal_len_str_arr(solution_9)) {
		(*destroy) (user_solution_9);
		free(solution_9);
		return 0;
	}//Test amount of sentences

	curr_sentence = 0;
	while(user_solution_9[curr_sentence]) {
		printf("SOL:%d. %s\n",curr_sentence,  solution_9[curr_sentence]);
		printf("USER:%d. %s\n",curr_sentence,  user_solution_9[curr_sentence]);	
		if(strcmp(solution_9[curr_sentence], user_solution_9[curr_sentence])) {
			(*destroy)(user_solution_9);
			free(solution_9);
			return 0;
		}
		curr_sentence++;
	}
	(*destroy)(user_solution_9);
	free(solution_9);

	return 1;
}
