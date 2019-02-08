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

#define terminate_shell() shell_cleaner(); return 0;
#define prompt_cleaner(a, b) free(a); free(b); a = NULL; b = NULL;
extern char * optarg;
extern int optind, opterr, optopt;

char* HISTORY_FILE = NULL;
char* COMMAND_FILE = NULL;
char* HISTORY_FILE_PATH = NULL;
char* COMMAND_FILE_PATH = NULL;

typedef struct process {
    char *command;
    pid_t pid;
} process;



bool argc_validator(int argc) {
	if (argc == 1 || argc == 3 || argc == 5) {
		return true;
	} else {
		return false;
	}
}



void signal_handler(int signal) {

}

void shell_cleaner() {
	return;
}

void option_parser(int argc, char** argv) {

}
	
int shell(int argc, char *argv[]) {
	if (!argc_validator(argc)) {
		print_usage();
		terminate_shell();
	}
	
	//expecting signal
	signal(SIGINT, signal_handler);
	signal(SIGCHLD, signal_handler);

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
			print_usage();
			terminate_shell();	
		}



		//CLEAN UP - ready for next prompt
		prompt_cleaner(cmd, cwd);
	}
	return 0;
}
