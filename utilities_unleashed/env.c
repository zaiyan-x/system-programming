/**
 * Utilities Unleashed Lab
 * CS 241 - Spring 2019
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int ref_locator(char* str) {
	char* ref_ptr = strchr(str, '%');
	if (ref_ptr) {
		return ref_ptr - str;
	} else {
		return -1;
	}			
}

int equal_locator(char* str) {
	char* equal_ptr = strchr(str, '=');
	if (equal_ptr) {
		return (equal_ptr - str);
	} else {  
		return -1;
	}
}

char** arg_parser(char* str) {
	int ref_position = ref_locator(str);
	int equal_position = equal_locator(str);
	if (ref_position != -1 && equal_position != -1) {
		char** result = malloc(3 * sizeof(char*));
		result[0] = strdup("ref");
		result[1] = strndup(str, equal_position);
		result[2] = strdup(str + ref_position + 1);
		return result; 
	} else if (ref_position == -1 && equal_position != -1) {
		char** result = malloc(3 * sizeof(char*));
		result[0] = strdup("equ");
		result[1] = strndup(str, equal_position);
		result[2] = strdup(str + equal_position + 1);
		return result;
	} else {
		return NULL;
	}
}

bool arg_validator(int argc, char** argv) {
	size_t i;
	size_t j;
	size_t count_per = 0;
	size_t count_equ = 0;
	char* arg_cpy;
	for (i = 1; i < (size_t)argc; i++) {
		if (strstr(argv[i], "--") == NULL) {
			arg_cpy = argv[i];
			for (j = 0; arg_cpy[j]; j++) {
				if (arg_cpy[j] == '=') {
					count_equ++;
				} else if (arg_cpy[j] == '%') {
					count_per++;
				} else {
					continue;
				}
			}
			if (count_per > 1 || count_equ != 1) {
				return false;
			}
		} else {
			return true;
		}
		count_per = 0;
		count_equ = 0;
	}
	return false;
}

int main(int argc, char *argv[]) {
	bool valid = arg_validator(argc, argv);
	printf("is it valid?: %d\n", valid);
	size_t i;
	if (valid) {
		for (i = 1; i < (size_t) argc; i++) {
			char** parse_result = arg_parser(argv[i]);
			if (parse_result) {
				printf("ref?: %s\nkey: %s\nvalue: %s\n", parse_result[0], parse_result[1], parse_result[2]);
			}
		}
	}
	return 0;		
}
