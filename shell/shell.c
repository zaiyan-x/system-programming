/**
 * Shell Lab
 * CS 241 - Spring 2019
 */
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <dirent.h>
 
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"

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

static vector* PROC = NULL;
static vector* CMD = NULL;
typedef struct process {
    char *command;
    pid_t pid;
} process;

//FUNC DECLARATION
int command_dispatcher(char*, int);
void unlog_process(pid_t);

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
		pid_t child_pid;
		while ((child_pid = waitpid((pid_t)(-1), 0, WNOHANG)) > 0) {
			unlog_process(child_pid);
			continue;
		}
		return;
	} else {
		return;
	}
}

void child_process_reaper() {
	size_t i;
	pid_t child_pid;
	for (i = 0; i < vector_size(PROC); i++) {
		child_pid = (pid_t) (* (unsigned int*)vector_get(PROC, i));
		kill(child_pid, SIGKILL);
	}
	return;
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
	if (HISTORY_FILE_POINTER) {
		fclose(HISTORY_FILE_POINTER);
		HISTORY_FILE_POINTER = NULL;
	}
	if (COMMAND_FILE_POINTER) {
		fclose(COMMAND_FILE_POINTER);
		COMMAND_FILE_POINTER = NULL;
	}
	if (LOG) {
		vector_destroy(LOG);
		LOG = NULL;
	}
	child_process_reaper();
	if (PROC) {
		vector_destroy(PROC);
		PROC = NULL;
	}
	if (CMD) {
		vector_destroy(CMD);
		CMD = NULL;
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

size_t cmd_locator(unsigned pid) {
	size_t i = 0;
	for (i = 0; i < vector_size(PROC); i++) {
		if ((*(unsigned int *)vector_get(PROC, i)) == (unsigned int) pid) {
			break;
		}
	}
	return i;
}

/*
typedef struct process_info {
    int pid;
    long int nthreads;
    unsigned long int vsize;
    char state;
    char *start_str;
    char *time_str;
    char *command;
} process_info;
*/
void exec_ps() {
	size_t i;
	

	size_t PID_POS = 0;
	size_t NUM_THREADS_POS = 19;
	size_t VSIZE_POS = 22;
	size_t STATE_POS = 2;
	size_t START_POS = 21;
	size_t UTIME_POS = 13;
	size_t STIME_POS = 14;
	size_t CUTIME_POS = 15;
	size_t CSTIME_POS = 16;
	char curr_addr[PATH_MAX];

	//Time Char*
	char time_str[2048];
	char exec_time_str[2048];
	
	//FOR reading from PATH FILE
	char* curr_info = NULL; //TODO: free
	size_t curr_info_size = 0;
	
	FILE * proc_stat;
	process_info* pinfo = malloc(sizeof(process_info));
	print_process_info_header(); //PRINT HEADER
	for (i = 0; i < vector_size(PROC); i++) {
		unsigned curr = *(unsigned *)vector_get(PROC, i);
		sprintf(curr_addr, "/proc/%u/stat", curr);
		
		if (access(curr_addr, R_OK) == -1) { // can't access virtual /proc file
			return;
		}

		if ((proc_stat = fopen(curr_addr, "r")) == NULL) { //cannot open virtual file
			return;
		}
		
		getline(&curr_info, &curr_info_size, proc_stat);
		sstring* info_sstring = cstr_to_sstring(curr_info); //TODO: free
		
		//CLEAN UP curr_info
		free(curr_info);
		curr_info = NULL;	
			 
		vector* info_vector = sstring_split(info_sstring, ' '); //TODO: free

		//CLEAN UP info_sstring
		sstring_destroy(info_sstring);
		info_sstring = NULL;

		//PID
		pinfo->pid = atoi((char*)(vector_get(info_vector, PID_POS)));
		//NTHREADS
		pinfo->nthreads =  atol((char*)(vector_get(info_vector, NUM_THREADS_POS)));
		//VSIZE
		pinfo->vsize = strtoul((char*) (vector_get(info_vector, VSIZE_POS)), NULL, 0) / 1000;
		//STATE
		pinfo->state = * (char*) (vector_get(info_vector, STATE_POS));
		//START
		time_t time = strtoull((char*) (vector_get(info_vector, START_POS)), NULL, 0) / sysconf(_SC_CLK_TCK);
		struct tm * time_struct = localtime(&time);
		time_struct_to_string(time_str, sizeof(time_str), time_struct);
		pinfo->start_str = time_str;	


		//TIME
		time_t utime = strtoull((char*) (vector_get(info_vector, UTIME_POS)), NULL, 0) / sysconf(_SC_CLK_TCK);
		time_t stime = strtoull((char*) (vector_get(info_vector, STIME_POS)), NULL, 0) / sysconf(_SC_CLK_TCK);
		time_t cutime = strtoull((char*) (vector_get(info_vector, CUTIME_POS)), NULL, 0) / sysconf(_SC_CLK_TCK);
		time_t cstime = strtoull((char*) (vector_get(info_vector, CSTIME_POS)), NULL, 0) / sysconf(_SC_CLK_TCK);
		time_t total_exec_time = utime + stime + cutime + cstime;
		struct tm * total_exec_time_struct = localtime(&total_exec_time);
		time_struct_to_string(exec_time_str, sizeof(exec_time_str), total_exec_time_struct);
		pinfo->time_str = exec_time_str;

		//COMMAND
		pinfo->command = vector_get(CMD, cmd_locator((unsigned) pinfo->pid));


		//PRINT
		print_process_info(pinfo);	

		//CLEAN UP//
		vector_destroy(info_vector);
		fclose(proc_stat);
	}
	free(pinfo);
}

void print_current_shell_session_log() {
	char cmd[1024];
	void** _it = vector_begin(LOG);
	void** _end = vector_end(LOG);
	size_t cmd_line_number = 0;
	for( ; _it != _end; ++_it) {
		strcpy(cmd, *(char**)_it);
		cmd[strlen(cmd) - 1] = '\0';
		print_history_line(cmd_line_number, cmd);
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
			cmd_line[strlen(cmd_line) - 1] = '\0';
			print_history_line(cmd_line_number, cmd_line);
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
			print_command(cmd_line);
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
			print_command(cmd_line);
			command_dispatcher(cmd_line, logic_operator);
			return true;
		}
	}
} 

bool exec_pfd(pid_t pid) {
	//PRINT FD HEADER
	print_process_fd_info_header();
	
	char curr_addr[PATH_MAX];
	char curr_line[2048];
	size_t i;
	int curr_pid;
	DIR* curr_dir = NULL;
	struct dirent * dir;
	FILE* curr_fd_file = NULL;
	size_t fd_no = 0;
	size_t file_pos = 0;
	char realpath[PATH_MAX];
	ssize_t readlink_len = 0;
	for (i = 0; i < vector_size(PROC); i++) {
		curr_pid = *(int *) vector_get(PROC, i);
		if (curr_pid != pid) {
			continue;
		} else {
			sprintf(curr_addr, "/proc/%d/fdinfo", curr_pid);
			curr_dir = opendir(curr_addr);
			seekdir(curr_dir, 2);//SEEK DIR pointer to 2nd (excluding . and ..)	
			while ((dir = readdir(curr_dir)) != NULL) {
				fd_no = strtoul(dir->d_name, NULL, 0);
				sprintf(curr_addr, "/proc/%d/fdinfo/%zu", curr_pid, fd_no);
				curr_fd_file = fopen(curr_addr, "r");
				fgets(curr_line, sizeof(curr_line), curr_fd_file);
				sscanf(curr_line, "%*s %zd", &file_pos);
				sprintf(curr_addr, "/proc/%d/fd/%zu", curr_pid, fd_no);
				readlink_len = readlink(curr_addr, realpath, sizeof(realpath));
				if (readlink_len == -1) {
					break;
				} else {
					realpath[readlink_len] = '\0';
				}
				print_process_fd_info(fd_no, file_pos, realpath);
				//CLEAN UP
				fclose(curr_fd_file);	
			}
			//GET READY TO RETURN TO MOTHERSHIP
			closedir(curr_dir);
			return true;
		}
	}
	print_no_process_found((int) pid);
	return false;
}

char** external_command_parser(char* cmd) {
	sstring* cmd_args = cstr_to_sstring(cmd);
	free(cmd);
	cmd = NULL;
	vector* parsed_cmd = sstring_split(cmd_args, ' ');
	sstring_destroy(cmd_args);
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
	parsed_cmd = NULL;
	parsed_args[i] = NULL;
	return parsed_args;
}

void parsed_external_command_cleaner(char** cmd) {
	size_t i = 0;
	while(cmd[i] != NULL) {
		free(cmd[i]);
		cmd[i] = NULL;
		i++;
	}
	free(cmd);
	cmd = NULL;
	return;
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
			int logic_operator = cmd_validator(cmd_line);
			print_command(cmd_line);
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
			int logic_operator = cmd_validator(cmd_line);
			print_command(cmd_line);
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
			int logic_operator = cmd_validator(cmd_line);
			print_command(cmd_line);
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
			int logic_operator = cmd_validator(cmd_line);
			print_command(cmd_line);
			command_dispatcher(cmd_line, logic_operator);
			return true;
		}
	}
}

void unlog_process(pid_t pid) {
	size_t i;
	for (i = 0; i < vector_size(PROC); i++) {
		if ((*(unsigned int *)vector_get(PROC, i)) == (unsigned int) pid) {
			vector_erase(PROC, i);
			vector_erase(CMD, i);
		}
	}
}

int command_dispatcher(char* cmd, int logic_operator) {
	size_t cmd_len = strlen(cmd);
	if (cmd_len == 0) {
		return -1;
	}
	if (logic_operator == 0) { // NO LOGIC OPERATOR
		if (cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == ' ') {
			if (!LOGIC) {
				log_cmd(cmd);
			}
			if (exec_cd(cmd) == false) {
				return -1;
			} else {
				return 1; //cd success
			}
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
		} else if (cmd[0] == 'p' && cmd[1] == 's') {
			exec_ps();
			return 1;
		} else if (cmd[0] == 'p' && cmd[1] == 'f' && cmd[2] == 'd') {
			if (cmd[3] != ' ' || !number_validator(cmd + 4)) {
				print_invalid_command(cmd);
				return -1;
			}
			pid_t pid = atoi(cmd + 4);
			if (exec_pfd(pid)) {
				return 1; //pfd success!
			} else {
				return -1; //no pid found
			}	
		} else if (cmd[0] == 'k' && cmd[1] == 'i' && cmd[2] == 'l' && cmd[3] == 'l') { //kill 
		} else {
			////////////////////////////////////////////////////
			/////////////////////EXTERNAL///////////////////////
			if (!LOGIC) {
				log_cmd(cmd);
			}
			bool BACKGROUND = false;
			if (cmd[cmd_len - 1] == '&') {
				BACKGROUND = true;
			}
			int status;
			pid_t pid = fork();
			//REMEMBER CHLD PID
			vector_push_back(PROC, &pid);
			vector_push_back(CMD, cmd); 
			if (pid < 0) { // fork failed
				print_fork_failed();
				return -1;
			} else if (pid == 0) { //child process
				char* cmd_mod = malloc((cmd_len + 1) * sizeof(char));
				size_t i;
				for (i = 0; i < cmd_len; i++) {
					cmd_mod[i] = cmd[i];
				}
				cmd_mod[cmd_len] = '\0';
				if (BACKGROUND) {	
					if (cmd_mod[cmd_len - 1] == '&') {
						cmd_mod[cmd_len - 1] = '\0';
					}
					if (cmd_mod[cmd_len - 2] == ' ') {
						cmd_mod[cmd_len - 2] = '\0';
					}
				}
				char** parsed_cmd = external_command_parser(cmd_mod);
				print_command_executed(getpid());
				fsync(STDOUT_FILENO);
				execvp(parsed_cmd[0], parsed_cmd);
				parsed_external_command_cleaner(parsed_cmd);
				print_exec_failed(cmd);
				exit(1);
			} else { //parent case
				if (!BACKGROUND) {
					pid_t child_pid = waitpid(pid, &status, 0);
					unlog_process(child_pid);
					if (status != 0) {
						return -1;
					} else {
						return 1;
					}
				} else {
					if (setpgid(pid, pid) == -1) {
						print_setpgid_failed();
						exit(1);
					}
					return 1;
				}
			}	
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
		size_t first_cmd_len = semi_col_pos - cmd;
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
			int first_cmd_ret_val = command_dispatcher(first_cmd, 0);
			int second_cmd_ret_val = command_dispatcher(second_cmd, 0);
			if (first_cmd_ret_val == -1 || second_cmd_ret_val == -1) {
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
			
void process_vector_setup() {
	PROC = unsigned_int_vector_create();
	CMD = string_vector_create();
	return;
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
	//set up PROC
	process_vector_setup();

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
		if (F_FLAG) {
			if (getline(&cmd, &cmd_size, COMMAND_FILE_POINTER) == -1) {
				//EOF is reached
				prompt_cleaner(cmd, cwd);
				terminate_shell();
			}
		} else {
			if (getline(&cmd, &cmd_size, stdin) == -1) {
				//EOF is reached
				prompt_cleaner(cmd, cwd);
				terminate_shell();	
			}
		}
		//Change the newline char to NUL char	
		cmd[strlen(cmd) - 1] = '\0';	
		if (F_FLAG) {
			print_command(cmd);
		}	
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
