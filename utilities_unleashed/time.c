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
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include "format.h"
int main(int argc, char *argv[]) {
    if (argc < 2) {
		print_time_usage();
		exit(1);
	}

	int status;
	pid_t pid = fork();
	if (pid < 0) { //fork failed
		print_fork_failed();
		exit(1);
	} else if (pid == 0) { //child process
		char* cmd = argv[1];
		char** cmd_arg = argv + 1;
		execvp(cmd, cmd_arg);
		print_exec_failed();
		exit(1);
	} else { //parent case
		struct timespec tp_start;
		struct timespec tp_end;
		int start = clock_gettime(CLOCK_MONOTONIC, &tp_start);
		waitpid(pid, &status, 0);
		int stop = clock_gettime(CLOCK_MONOTONIC, &tp_end);
		if (start == -1 || stop == -1) {
			print_time_usage();
			exit(1);
		}
		double duration = tp_end.tv_sec - tp_start.tv_sec + (tp_end.tv_nsec - tp_start.tv_nsec) / 1000000000.0;
		if (!status) {
			 display_results(argv, duration);
		}
	}

	return 0;
}
