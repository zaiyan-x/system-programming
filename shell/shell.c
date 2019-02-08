/**
 * Shell Lab
 * CS 241 - Spring 2019
 */
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
 
#include "format.h"
#include "shell.h"
#include "vector.h"

#define terminate_shell shell_cleaner(); return 0;
extern char * optarg;
extern int optind, opterr, optopt;

typedef struct process {
    char *command;
    pid_t pid;
} process;



bool arg_validator(int argc, char** argv) {

	return true;
}



void signal_handler(int signal) {

}

void shell_cleaner() {
	return;
}
	
int shell(int argc, char *argv[]) {
	if (!arg_validator(argc, argv)) {
		terminate_shell
	}

	pid_t pid = getpid();

	char* cmd = NULL;
	size_t cmd_size = 0;
	char* cwd = NULL;
	size_t cwd_size = 0;
	while (true) {
		if ((cwd = getcwd(cwd, cwd_size))) {
			print_prompt(cwd, pid);
		}
		if (getline(&cmd, &cmd_size, stdin) == -1) {
			terminate_shell			
		}
	}
	return 0;
}
