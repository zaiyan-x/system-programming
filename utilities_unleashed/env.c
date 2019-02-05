/**
 * Utilities Unleashed Lab
 * CS 241 - Spring 2019
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "format.h"
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
	} else if (strstr(str, "--")) {
		char** result = malloc(3 * sizeof(char*));
		result[0] = strdup("dash");
		result[1] = NULL;
		result[2] = NULL;
		return result;
	} else {
		return NULL;
	}
}

bool arg_validator(int argc, char** argv) {
	if (argc < 3) {
		return false;
	}
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
	if (!valid) {
		print_env_usage();
		exit(1);
	}
	int status;
	pid_t pid = fork();
	if (pid < 0) { //Failed
		print_fork_failed();
		exit(1);
	} else if (pid == 0) { // child case
		char** arg;
		size_t i;
		for (i = 1; i < (size_t) argc; i++) {
			arg = arg_parser(argv[i]);	
			if (!strcmp(arg[0], "ref")) {
				char* env_val = getenv(arg[2]);
				if (!env_val) {
					env_val = "";
				}
				setenv(arg[1], env_val, 1);
			} else if (!strcmp(arg[0], "equ")) {
				setenv(arg[1], arg[2], 1);
			} else if (!strcmp(arg[0], "dash")) {
				break;
			} else {
			}
		}
		char* cmd = argv[i + 1];
		printf("%s", cmd);
		char** cmd_arg = argv + i + 1;
		execvp(cmd, cmd_arg);
		print_exec_failed();
		exit(1);
			
	} else { //parent case
		waitpid(pid, &status, 0);
	}

	return 0;		
}
