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

#define terminate_shell() shell_cleaner(); write_log(); return 0;
#define prompt_cleaner(a, b) free(a); free(b); a = NULL; b = NULL;
extern char * optarg;
extern int optind, opterr, optopt;

static char* HISTORY_FILE = NULL;
static char* COMMAND_FILE = NULL;
static char* HISTORY_PATH = NULL;
static char* COMMAND_PATH = NULL;
static FILE* HISTORY_FILE_POINTER = NULL;
static FILE* COMMAND_FILE_POINTER = NULL;

static vector* LOG = NULL;
static bool NO_FLAG = false;
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

int cmd_validator(char* cmd) {
	char* and_pos = strstr(cmd, "&&");
	char* or_pos = strstr(cmd, "||");
	char* semi_col_pos = strstr(cmd, ";");
	if (and_pos != NULL) {
		if (or_pos != NULL || semi_col_pos != NULL) {
			return -1;
		} else {
			return 1;
		}
	}
	if (or_pos != NULL) {
		if (and_pos != NULL || semi_col_pos != NULL) {
			return -1;
		} else {
			return 2;
		}
	}
	if (semi_col_pos != NULL) {
		if (and_pos != NULL || or_pos != NULL) {
			return -1;
		} else {
			return 3;
		}
	}
	return 0;
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
	while ((opt = getopt(argc, argv, "h:f:")) != -1) {
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

bool log_cmd(char* cmd) {
	if (LOG != NULL) {
		vector_push_back(LOG, cmd);
		return true;
	}
	return false;
}

void write_log() {
	char* cmd = NULL;
	if (HISTORY_FILE_POINTER != NULL) {
		void** _it = vector_begin(LOG);
		void** _end = vector_end(LOG);
		for( ; _it != _end; ++_it) {
			cmd = *_it;
			fprintf(HISTORY_FILE_POINTER, "%s\n", (char*) cmd);
		}
	}
	return;
}

void exec_cd(char* cmd) {
	size_t cmd_len = strlen(cmd);
	if (cmd_len <= 3) {
		print_no_directory("");
		return;
	}

	char* directory = cmd + 3;

	if (chdir(directory)) {
		print_no_directory(directory);
	}
}

void command_dispatcher(char* cmd, int logic_operator) {
	size_t cmd_len = strlen(cmd);
	if (cmd_len == 0) {
		return;
	}
	if (logic_operator == 0) { // NO LOGIC OPERATOR
		if (cmd_len > 1) {
			if (cmd[0] == 'c' && cmd[1] == 'd') {
				exec_cd(cmd);
			}
		} else {
			if (cmd[0] == '!') {
				
			}
			if (cmd[0] == '#') {
			}
		}			
	} else if (logic_operator == 1) { // AND
		//char* and_pos = strstr(cmd, "&&");


	} else if (logic_operator == 2) { // OR
		//char* or_pos = strstr(cmd, "&&");


	} else if (logic_operator == 3) { // SEMI COLON
		//char* semi_col_pos = strstr(cmd, ";");


	} else { //TODO DO nothing
	}
}
bool file_setup() {
	if (HISTORY_FILE != NULL) {
		if (access(HISTORY_FILE, R_OK | W_OK) != -1) {
			HISTORY_PATH = (*get_full_path)(HISTORY_FILE);
			HISTORY_FILE_POINTER = fopen(HISTORY_PATH, "a+");
		} else {
			print_history_file_error();
			HISTORY_FILE_POINTER = fopen(HISTORY_FILE, "a+");
			HISTORY_PATH = (*get_full_path)(HISTORY_FILE);
		}
		if (HISTORY_FILE_POINTER == NULL) { // fopen failed
			print_history_file_error();
			return false;
		}	 
	}
	if (COMMAND_FILE != NULL) {
		if (access(COMMAND_FILE, R_OK) != -1) {
			COMMAND_PATH = (*get_full_path)(COMMAND_FILE);
			COMMAND_FILE_POINTER = fopen(COMMAND_PATH, "r");
		} else {
			print_script_file_error();
			return false;
		}
		if (COMMAND_FILE_POINTER == NULL) { // fopen failed
			print_script_file_error();
			return false;
		}
	}
	return true;
}

bool log_setup() {
	LOG = string_vector_create();
	if (LOG != NULL) {
		return true;
	} else {
		return false;
	}	
}
			
	
int shell(int argc, char *argv[]) {
	if (!argc_validator(argc)) {
		print_usage();
		terminate_shell();
	}
	
	//expecting signal
	signal(SIGINT, signal_handler);
	signal(SIGCHLD, signal_handler);

	//get options
	if (argc == 1) {
		NO_FLAG = true;
	} else {	
		if (option_setup(argc, argv) == false) {
			print_usage();
			terminate_shell();
		}
	
		//setup files (ONLY IF there is -h or -f)
		if (file_setup() == false) {
		terminate_shell();
		}
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
			print_usage();
			prompt_cleaner(cmd, cwd);
			terminate_shell();	
		}
		//Change the newline char to NUL Char
		cmd[strlen(cmd) - 1] = '\0';
	
		//Validate cmd
		//0: NO Logic Operator
		//1: AND
		//2: OR
		//3: SEMICOLON
		//-1: INVALID
		int logic_operator = cmd_validator(cmd);

		//Dispatch Commands (w/ valid cmd)
		if (logic_operator == -1) {
			print_invalid_command(cmd);			
		} else {
			log_cmd(cmd);
			command_dispatcher(cmd, logic_operator);
		}

		//CLEAN UP - ready for next prompt
		prompt_cleaner(cmd, cwd);
	}
	exit(0);
}
