/**
 * Shell Lab
 * CS 241 - Spring 2019
 */
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
 
#include "format.h"
#include "shell.h"
#include "vector.h"

#define terminate_shell() write_log(); shell_cleaner(); exit(0);
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
static bool H_FLAG = false;
static bool F_FLAG = false;
static bool LOGIC = false;

typedef struct process {
    char *command;
    pid_t pid;
} process;

int command_dispatcher(char*, int);

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
		while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {
			continue;
		}
		return;
	} else {
		return;
	}
}

void shell_cleaner() {
	if (HISTORY_FILE) {
		free(HISTORY_FILE);
		fclose(HISTORY_FILE_POINTER);
		HISTORY_FILE_POINTER = NULL;
		HISTORY_FILE = NULL;
	}
	if (COMMAND_FILE) {
		free(COMMAND_FILE);
		fclose(COMMAND_FILE_POINTER);
		COMMAND_FILE_POINTER = NULL;
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
	if (LOG) {
		vector_destroy(LOG);
		LOG = NULL;
	}
	return;
}

bool option_setup(int argc, char** argv) {
	int opt;
	while ((opt = getopt(argc, argv, "h:f:")) != -1) {
		if (opt == 'h') {
			H_FLAG = true;
			if (HISTORY_FILE == NULL && optarg != NULL) {
				HISTORY_FILE = strdup(optarg);
			} else {
				return false;
			}
		} else if (opt == 'f') {
			F_FLAG = true;
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
		//+1 for NUL Byte and +1 for adding newline char
		size_t cmd_len = strlen(cmd);
		char cmd_to_push[cmd_len + 1];
		strcpy(cmd_to_push, cmd);
		
		//FIX NUL Byte and newline
		cmd_to_push[cmd_len] = '\n';
		cmd_to_push[cmd_len + 1] = '\0';
	
		vector_push_back(LOG, cmd_to_push);
		return true;
	}
	return false;
}

void write_log() {
	if (H_FLAG == false) {
		return;
	}
	char* cmd = NULL;
	if (HISTORY_FILE_POINTER != NULL) {
		void** _it = vector_begin(LOG);
		void** _end = vector_end(LOG);
		for( ; _it != _end; ++_it) {
			cmd = *_it;
			fprintf(HISTORY_FILE_POINTER, "%s", (char*) cmd);
		}
	}
	vector_clear(LOG);
	fseek(HISTORY_FILE_POINTER, 0, SEEK_SET);
	return;
}

bool number_validator(char* cmd) {
	size_t i;
	for (i = 0; i < strlen(cmd); i++) {
		if (!isdigit(cmd[i])) {
			return false;
		}
	}
	return true;
}

bool exec_cd(char* cmd) {
	size_t cmd_len = strlen(cmd);
	if (cmd_len <= 3) {
		print_no_directory("");
		return false;
	}
	
	char* directory = cmd + 3;
	
	if (chdir(directory) == -1) {
		print_no_directory(directory);
		return false;
	}
	return true;
}

void print_current_shell_session_log() {
	char* cmd = NULL;
	void** _it = vector_begin(LOG);
	void** _end = vector_end(LOG);
	size_t cmd_line_number = 0;
	for( ; _it != _end; ++_it) {
		cmd = *_it;
		printf("%zu\t%s", cmd_line_number, (char*) cmd);
		cmd_line_number++;
	}
	return;
}	

void exec_print_history() {
	//!history is not stored in history
	if (H_FLAG) {
		write_log();
		char cmd_line[1024];
		size_t cmd_line_number = 0;
		while (fgets(cmd_line, sizeof(cmd_line), HISTORY_FILE_POINTER)) {
			printf("%zu\t%s", cmd_line_number, cmd_line);
			cmd_line_number++;
		}
	} else {
		print_current_shell_session_log();
	}
	return;	
}

size_t history_command_counter() {
	if (H_FLAG) {
		write_log();
		char cmd_line[1024];
		size_t cmd_line_count = 0;
		while (fgets(cmd_line, sizeof(cmd_line), HISTORY_FILE_POINTER)) {
			cmd_line_count++;
		}
		return cmd_line_count;
	} else {
		return vector_size(LOG);
	}
}
bool exec_nth_command(size_t cmd_line_number) {
	//#<n> is not stored in the history
	//But the executed command is 
	if (H_FLAG) {
		write_log();
		char cmd_line[1024];
		size_t curr_cmd_line = 0;
		bool INBOUND = false;
		while (fgets(cmd_line, sizeof(cmd_line), HISTORY_FILE_POINTER)) {
			if (curr_cmd_line == cmd_line_number) {
				cmd_line[strlen(cmd_line) - 1] = '\0';
				INBOUND = true;
				break;
			}
			curr_cmd_line++;
		}
		if (INBOUND == false) {
			return false;
		} else {
			int logic_operator = cmd_validator(cmd_line);
			puts(cmd_line);
			command_dispatcher(cmd_line, logic_operator);
			return true;
		}
	} else {
		char cmd_line[1024];
		void** _it = vector_begin(LOG);
		void** _end = vector_end(LOG);
		size_t curr_cmd_line = 0;
		bool INBOUND = false;
		for(; _it != _end; ++_it) {
			if (curr_cmd_line == cmd_line_number) {
				strcpy(cmd_line, *_it);
				cmd_line[strlen(cmd_line) - 1] = '\0';
				INBOUND = true;
				break;
			}
			curr_cmd_line++;
		}
		if (INBOUND == false) {
			return false;
		} else {
			int logic_operator = cmd_validator(cmd_line);
			puts(cmd_line);
			command_dispatcher(cmd_line, logic_operator);
			return true;
		}
	}
}

char** external_command_parser(char* cmd) {
	sstring* cmd_args = cstr_to_sstring(cmd);
	vector* parsed_cmd = sstring_split(cmd_args, ' ');
	free(cmd_args);
	cmd_args = NULL;
	char** parsed_args = malloc((vector_size(parsed_cmd) + 1) * sizeof(char*));
	void** _it = vector_begin(parsed_cmd);
	void** _end = vector_end(parsed_cmd);
	size_t i = 0;
	for (; _it != _end; _it++) {
		parsed_args[i] = strdup(*((char**)_it));
		i++;
	}
	vector_destroy(parsed_cmd);
	parsed_args[i] = NULL;
	return parsed_args;
}

bool exec_prefix_command(char* cmd) {
	size_t cmd_len = strlen(cmd);
	bool POUND_ONLY = false;
	if (cmd_len == 1) {
		POUND_ONLY = true;
	}
	char* command_prefix = cmd + 1;
	size_t prefix_len = strlen(command_prefix);	
	char cmd_line[1024] = { 0 };
	bool FOUND = false;
	if (H_FLAG) {
		write_log();
		vector* compiled_history = string_vector_create();
		while (fgets(cmd_line, sizeof(cmd_line), HISTORY_FILE_POINTER)) {
			vector_push_back(compiled_history, cmd_line);
		}
		if (vector_size(compiled_history) == 0) {
			vector_destroy(compiled_history);
			return false;
		}
		void** _it = vector_end(compiled_history) - 1;
		if (POUND_ONLY) {	
			strcpy(cmd_line, *((char**)_it));
			cmd_line[strlen(cmd_line) - 1] = '\0';
			puts(cmd_line);
			int logic_operator = cmd_validator(cmd_line);
			command_dispatcher(cmd_line, logic_operator);
			vector_destroy(compiled_history);
			return true;	
		}
		while (true) {
			if (_it == vector_begin(compiled_history)) {
				strcpy(cmd_line, *((char**)_it));
				cmd_line[strlen(cmd_line) - 1] = '\0';
				if (strncmp(command_prefix, cmd_line, prefix_len) == 0) {
					FOUND = true;
				}
				break;
			} else {
				strcpy(cmd_line, *((char**)_it));
				cmd_line[strlen(cmd_line) - 1] = '\0';
				//strncmp returns 0 if strs are same
				if (strncmp(command_prefix, cmd_line, prefix_len) == 0) {
					FOUND = true;
					break;
				}
			}
			_it--; // UPDATE WHILE LOOP
		}

		vector_destroy(compiled_history);
		if (FOUND == false) {
			return false;	
		} else {
			puts(cmd_line);
			int logic_operator = cmd_validator(cmd_line);
			command_dispatcher(cmd_line, logic_operator);
			return true;
		}
	} else { // H_FLAG == false
		if (vector_size(LOG) == 0) {
			return false;
		}
		void** _it = vector_end(LOG) - 1;
		if (POUND_ONLY) {
			strcpy(cmd_line, *((char**)_it));
			cmd_line[strlen(cmd_line) - 1] = '\0';
			puts(cmd_line);
			int logic_operator = cmd_validator(cmd_line);
			command_dispatcher(cmd_line, logic_operator);
			return true;
		}	
		while (true) {
			if (_it == vector_begin(LOG)) {
				strcpy(cmd_line, *((char**)_it));
				cmd_line[strlen(cmd_line) - 1] = '\0';
				if (strncmp(command_prefix, cmd_line, prefix_len) == 0) {
					FOUND = true;
				}
				break;
			} else {
				strcpy(cmd_line, *((char**)_it));
				cmd_line[strlen(cmd_line) - 1] = '\0';
				//strncmp returns 0 if strs are same
				if (strncmp(command_prefix, cmd_line, prefix_len) == 0) {
					FOUND = true;
					break;
				}
			}
			_it--;
		}
		if (FOUND == false) {
			return false;
		} else {
			puts(cmd_line);
			int logic_operator = cmd_validator(cmd_line);
			command_dispatcher(cmd_line, logic_operator);
			return true;
		}
	}
}	

int command_dispatcher(char* cmd, int logic_operator) {
	size_t cmd_len = strlen(cmd);
	if (cmd_len == 0) {
		return -1;
	}
	if (logic_operator == 0) { // NO LOGIC OPERATOR
		if (cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == 32) {
			if (!LOGIC) {
				log_cmd(cmd);
			}
			if (exec_cd(cmd) == false) {
				return -1;
			} else {
				return 1; //cd success
			}
		} else if (cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == '.') {
			print_invalid_command(cmd);
			return -1;
		} else if (cmd[0] == '!') {
			if (strstr(cmd, "history")) {//!history
				exec_print_history();
				return 1;
			} else {//!<prefix>
				if (exec_prefix_command(cmd) == false) {
					print_no_history_match();
					return -1;
				} else {
					return 1; //prefix success
				}
			}	
		} else if (cmd[0] == '#') {//#<n>
			if (number_validator(cmd + 1) == false) {
				print_invalid_index();
				return -1;
			} else {
				int cmd_line_number = atoi(cmd + 1);
				if (!exec_nth_command((size_t)cmd_line_number)) {
					print_invalid_index();
					return -1;
				}
				return 1; //nth exec success
			}
		} else if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't') { // exit
			return 0;
		} else {
			////////////////////////////////////////////////////
			/////////////////////EXTERNAL///////////////////////
			fflush(stdout); //flush
			int status;
			pid_t pid = fork();
			if (pid < 0) { // fork failed
				print_fork_failed();
				return -1;
			} else if (pid == 0) { //child process
				 
		}							
	} else if (logic_operator == 1) { // AND
		LOGIC = true;
		char* and_pos = strstr(cmd, "&&");
		char* second_cmd_start = and_pos + 3;
		size_t first_cmd_len = and_pos - cmd - 1;
		char first_cmd[1024];
		char second_cmd[1024];
		strncpy(first_cmd, cmd, first_cmd_len);
		first_cmd[first_cmd_len] = '\0';
		strcpy(second_cmd, second_cmd_start);
		if (first_cmd[0] == '!' || second_cmd[0] == '!') {
		} else if (first_cmd[0] == '#' || second_cmd[0] == '#') {//#<n>
			print_invalid_command(cmd);
			return -1;
		} else if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't') { // exit
			print_invalid_command(cmd);
			return -1;
		} else {//command chaining VALID
			log_cmd(cmd);
			if (command_dispatcher(first_cmd, 0) == 1) {
				if (command_dispatcher(second_cmd, 0) == 1) {
					LOGIC = false;
					return 1;
				} else {
					LOGIC = false;
					return -1;
				}
			} else {
				LOGIC = false;
				return -1;
			}
		}
	} else if (logic_operator == 2) { // OR
		LOGIC = true;
		char* or_pos = strstr(cmd, "||");
		char* second_cmd_start = or_pos + 3;
		size_t first_cmd_len = or_pos - cmd - 1;
		char first_cmd[1024];
		char second_cmd[1024];
		strncpy(first_cmd, cmd, first_cmd_len);
		first_cmd[first_cmd_len] = '\0';
		strcpy(second_cmd, second_cmd_start);
		if (first_cmd[0] == '!' || second_cmd[0] == '!') {
		} else if (first_cmd[0] == '#' || second_cmd[0] == '#') {//#<n>
			print_invalid_command(cmd);
			return -1;
		} else if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't') { // exit
			print_invalid_command(cmd);
			return -1;
		} else {//command chaining VALID
			log_cmd(cmd);
			if (command_dispatcher(first_cmd, 0) == -1) {
				if (command_dispatcher(second_cmd, 0) == -1) {
					LOGIC = false;
					return -1;
				} else {
					LOGIC = false;
					return 1;
				}
			} else {
				LOGIC = false;
				return 1;
			}
		}

	} else if (logic_operator == 3) { // SEMI COLON
		LOGIC = true;
		char* semi_col_pos = strstr(cmd, ";");
		char* second_cmd_start = semi_col_pos + 2;
		size_t first_cmd_len = semi_col_pos - cmd - 1;
		char first_cmd[1024];
		char second_cmd[1024];
		strncpy(first_cmd, cmd, first_cmd_len);
		first_cmd[first_cmd_len] = '\0';
		strcpy(second_cmd, second_cmd_start);
		if (first_cmd[0] == '!' || second_cmd[0] == '!') {
		} else if (first_cmd[0] == '#' || second_cmd[0] == '#') {//#<n>
			print_invalid_command(cmd);
			return -1;
		} else if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't') { // exit
			print_invalid_command(cmd);
			return -1;
		} else {//command chaining VALID
			log_cmd(cmd);
			if (command_dispatcher(first_cmd, 0) == -1 ||
			command_dispatcher(second_cmd, 0) == -1) {
				LOGIC = false;
				return -1;
			} else {
				LOGIC = false;
				return 1;
			}
		}

	} else { //TODO DO nothing
	}
	return 1;
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

	//Get ready to prompt
	log_setup();	
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
			//EOF is reached
			prompt_cleaner(cmd, cwd);
			terminate_shell();	
		}
		
		//Change the newline char to NUL char	
		cmd[strlen(cmd) - 1] = '\0';	
	
		//Validate cmd
		//0: NO Logic Operator
		//1: AND
		//2: OR
		//3: SEMICOLON
		//-1: INVALID
		int logic_operator = cmd_validator(cmd);
		int return_value = 1;
		//Dispatch Commands (w/ valid cmd)
		if (logic_operator == -1) {
			print_invalid_command(cmd);			
		} else {
			//command_dispatcher RETURN VALUE
			//1: EXEC SUCCEED
			//0: RECEIVED EXIT
			//-1: INPUT INVALID / NOTHING EXEC / NON-SIGNIFICANT ERROR 
			return_value = command_dispatcher(cmd, logic_operator);
		}
		
		//CLEAN UP - ready for next prompt
		prompt_cleaner(cmd, cwd);
		if (return_value == 0) {
			terminate_shell();
		} else {
		}
	}
	exit(0);
}
