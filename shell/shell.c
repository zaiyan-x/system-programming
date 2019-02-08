/**
 * Shell Lab
 * CS 241 - Spring 2019
 */
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
 
#include "format.h"
#include "shell.h"
#include "vector.h"

#define terminate_shell() shell_cleaner(); return 0;
#define prompt_cleaner(a, b) free(a); free(b); a = NULL; b = NULL;
extern char * optarg;
extern int optind, opterr, optopt;

static char* HISTORY_FILE = NULL;
static char* COMMAND_FILE = NULL;
static char* HISTORY_PATH = NULL;
static char* COMMAND_PATH = NULL;

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
	if (signal == SIGINT) {
		return;
	} else if (signal == SIGCHLD) {
		// TODO
	} else {
		return;
	}
}

void shell_cleaner() {
	if (HISTORY_FILE) {
		free(HISTORY_FILE);
		HISTORY_FILE = NULL;
	}
	if (COMMAND_FILE) {
		free(COMMAND_FILE);
		COMMAND_FILE = NULL;
	}
	if (HISTORY_PATH) {
		free(HISTORY_PATH);
		HISTORY_PATH = NULL;
	}
	if (COMMAND_PATH) {
		free(COMMAND_PATH);
		COMMAND_PATH = NULL;
	}
	return;
}

bool option_setup(int argc, char** argv) {
	int opt;
	while ((opt = getopt(argc, argv, "h;f:")) != -1) {
		if (opt == 'h') {
			if (HISTORY_FILE == NULL && optarg != NULL) {
				HISTORY_FILE = strdup(optarg);
			} else {
				return false;
			}
		} else if (opt == 'f') {
			if (COMMAND_FILE == NULL && optarg != NULL) {
				COMMAND_FILE = strdup(optarg);
			} else {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}

void command_dispatcher(char* cmd) {

}

void file_setup() {
	if (HISTORY_FILE != NULL) {
		if (access(HISTORY_FILE, R_OK | W_OK) != -1) {
			HISTORY_PATH = (*get_full_path)(HISTORY_FILE);
			//TODO	
	
int shell(int argc, char *argv[]) {
	if (!argc_validator(argc)) {
		print_usage();
		terminate_shell();
	}
	
	//expecting signal
	signal(SIGINT, signal_handler);
	signal(SIGCHLD, signal_handler);

	//get options
	if (option_setup(argc, argv) == false) {
		print_usage();
		terminate_shell();
	}	
	puts(HISTORY_FILE);
	puts(COMMAND_FILE);	
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
		//Change the newline char to NUL Char
		cmd[strlen(cmd) - 1] = '\0';
	
		//Process commands		
		command_processor(cmd);

		//CLEAN UP - ready for next prompt
		prompt_cleaner(cmd, cwd);
	}
	return 0;
}
