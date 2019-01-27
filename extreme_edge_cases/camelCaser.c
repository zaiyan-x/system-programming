/**
 * Extreme Edge Cases Lab
 * CS 241 - Spring 2019
 */
 
#include "camelCaser.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

char **camel_caser(const char *input_str) {
	//First check null
	if(!input_str) {
		return NULL;
	}

	//Count how many sentences we have (as to initialize array of char* later, we need to know the size)
	int num_of_sentence = 0;
	char const * curr_char = input_str;
	while(*++curr_char) {
		if(ispunct(*curr_char)) {
			num_of_sentence++;
		}
	}


	//malloc the space for the array of char pointers(char*)
	//And set the last element as NULL pointer 
	char** result = malloc((num_of_sentence + 1) * sizeof(char*));
	result[num_of_sentence] = NULL;
	
	
	//Count characters for each sentence and initialize string, or char array, for every sentence
   	int num_of_char = 0;
	int curr_sentence = 0;
	curr_char = input_str; //re-point to the input_str
	while(*++curr_char) {
		if(ispunct(*curr_char)) {
			result[curr_sentence] = malloc((num_of_char + 1) * sizeof(char));
			result[curr_sentence][num_of_char] = 0;
			num_of_char = 0; //reset character counter
			curr_sentence++; //move to next sentence
		} else if(isspace(*curr_char)) {
			continue;
		} else {
			num_of_char++;
		}
	}

	//Convert
	//1. if it encounters a space, then next non-space charcter should be capped
	//2. if it encounters a punctuation, then it should move to next sentence
		//it should not cap next word
		//increase iterator
	bool flag_cap = false;
	bool flag_fw = true;
	curr_sentence = 0;
	curr_char = input_str;
	num_of_char = 0;	
	while(*++curr_char) {
		//Exclude the case where input_str is not null but it only has space
		if(!result[curr_sentence]) {
			break;
		}
		if(isspace(*curr_char)) {
			flag_cap = true;
		} else if(ispunct(*curr_char)) {
			flag_fw = true;
			flag_cap = false;
			curr_sentence++;
			num_of_char = 0;
		} else {
			if(isalpha(*curr_char)) {
				if(flag_cap && !flag_fw) {
					result[curr_sentence][num_of_char] = toupper(*curr_char);
				} else {
					result[curr_sentence][num_of_char] = tolower(*curr_char);
				}
			} else {
				result[curr_sentence][num_of_char] = *curr_char;
			}
			num_of_char++;
			flag_cap = false;
			flag_fw = false;
		}
	}				

	return result;
}

void destroy(char **result) {
	char** temp = result;
	while(++temp) {
		free(*temp);
	}
	free(result);
}
