/**
 * Vector Lab
 * CS 241 - Spring 2019
 */
 
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    vector* str_vec;
};

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
	sstring* result = malloc(sizeof(sstring));
	result->str_vec = vector_create(char_copy_constructor, char_destructor, char_default_constructor);
	
	char curr;
	size_t i;
	for (i = 0; i < strlen(input); i++) {
		curr = input[i];
		vector_push_back(result->str_vec, &curr);
	}	
    return result;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
	size_t size = vector_size(input->str_vec); 
    char* result = malloc((size + 1) * sizeof(char));
	size_t i;
	for (i = 0; i < size; i++) {
		result[i] = *((char*) vector_get(input->str_vec, i));
	}
	result[size] = '\0';
	return result;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
	size_t new_size = vector_size(this->str_vec) + vector_size(addition->str_vec);
	vector_reserve(this->str_vec, new_size);
	size_t i;
	for (i = 0; i < vector_size(addition->str_vec); i++) {
		vector_push_back(this->str_vec, vector_get(addition->str_vec, i));
	}
	return new_size;
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
	vector* result = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
	bool more_del = true;
	size_t i;
	char temp_char[2];
	sstring* temp = cstr_to_sstring("");
	for (i = 0; i < vector_size(this->str_vec); i++) {
		if ((*(char*)vector_get(this->str_vec, i)) == delimiter && !more_del) {
			char* str_to_push = sstring_to_cstr(temp);
			vector_push_back(result, str_to_push);
			free(str_to_push);
			sstring_destroy(temp);
			more_del = true;
			temp = cstr_to_sstring("");
		} else if ((*(char*)vector_get(this->str_vec, i)) == delimiter && more_del) {
			more_del = true;
			vector_push_back(result, "");
		} else {
			more_del = false;
			temp_char[0] = *(char*)vector_get(this->str_vec, i);
			temp_char[1] = '\0';
			sstring* sstr_to_append = cstr_to_sstring(temp_char);
			sstring_append(temp, sstr_to_append);
			sstring_destroy(sstr_to_append);
		}
	}
	char* str_to_push = sstring_to_cstr(temp);
	vector_push_back(result, str_to_push);
	free(str_to_push);
	sstring_destroy(temp);
    return result;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
	size_t target_length = strlen(target);
	size_t sub_length = strlen(substitution);
	size_t new_size = vector_size(this->str_vec) - target_length + sub_length;
	vector_reserve(this->str_vec, new_size);

	char curr;
	bool found = false;
	bool engaged = false;
	
	size_t i;
	size_t j = vector_size(this->str_vec);
	size_t k = 0;

	size_t target_end = strlen(target) - 1;
	size_t found_position = 0;

	for (i = offset; i < j; i++) {
		curr = *(char*) vector_get(this->str_vec, i);
		if (engaged == false) {
			if (curr != target[0]) {
				continue;
			} else {
				engaged = true;
				k++;
			}
		} else {
			if (curr != target[k]) {
				engaged = false;
				k = 0;
			} else {
				if (k == target_end) {
					found = true;
					found_position = i;
					break;
				} else if (k < target_end) {
					k++;
				} else {
				}
			}
		}
	}

	if (found == false) {
		return (int) found;
	}
	//Replace	
	size_t start_position = found_position - target_length + 1; 
	size_t end_position;
	size_t fix_position;	
	if (sub_length >= target_length) { // extend
		end_position = start_position + sub_length - 1;
		j = target_length;
	} else {
		end_position = start_position + target_length - 1;
		j = sub_length;
		fix_position = start_position + sub_length;
	}
	
	k = 0;

	for (i = start_position; i <= end_position; i++) {
		if (j > 0) {
			vector_set(this->str_vec, i, &substitution[k]);
			k++;
			j--;
		} else {
			if (sub_length < target_length) {
				vector_erase(this->str_vec, fix_position);
			} else {
				vector_insert(this->str_vec, i, &substitution[k]);
				k++;
			}
		}
	}
    return (int) found;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    return NULL;
}

void sstring_destroy(sstring *this) {
    // your code goes here
	vector_destroy(this->str_vec);
	this->str_vec = NULL;
	free(this);
	this = NULL;
	return;
}
