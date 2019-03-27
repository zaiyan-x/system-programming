/**
 * mapreduce Lab
 * CS 241 - Spring 2019
 * Collab with Eric Wang - wcwang2 :))))
 */
 
#include "utils.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/wait.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv) {
	
	if (argc != 6) { 
		print_usage(); 
		return 0; 
	} 
    char* input_file = argv[1]; 
	char* output_file = argv[2]; 
	char* mapper_executable = argv[3]; 
	char* reducer_executable = argv[4]; 
	int mapper_count = atoi(argv[5]); 
	
	// Create an input pipe for each mapper.
	int mapper_pipe[mapper_count][2]; 
    for (int i = 0; i < mapper_count; i++) { 
		pipe2(mapper_pipe[i], O_CLOEXEC); 
		descriptors_add(mapper_pipe[i][0]); 
		descriptors_add(mapper_pipe[i][1]); 
	} 
	
	// Create one input pipe for the reducer.
	int reducer_pipe[2]; 
	pipe2(reducer_pipe, O_CLOEXEC); 
	descriptors_add(reducer_pipe[0]); 
	descriptors_add(reducer_pipe[1]); 
    
	// Open the output file.
	int output_fd = open(output_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); 
	descriptors_add(output_fd); 

    // Start a splitter process for each mapper.
	for (int i = 0; i < mapper_count; i++) {  
		pid_t splitter_pid = fork(); 
		if (splitter_pid == 0) { // child
			dup2(mapper_pipe[i][1], 1);
			char num_of_mapper[10];
			snprintf(num_of_mapper, 10, "%d", mapper_count);
			char current_mapper[10];
			snprintf(current_mapper, 10, "%d", i); 
			int status = execlp("./splitter", "./splitter", input_file, num_of_mapper, current_mapper, NULL); 
			print_nonzero_exit_status("./splitter", status); 
			exit(1); 
		} 
	} 
    
	// Start all the mapper processes.
	pid_t mapper_pids[mapper_count]; 
	for (int i = 0; i < mapper_count; i++) { 
		mapper_pids[i] = fork(); 
		if (mapper_pids[i] == 0) { // child 
			dup2(mapper_pipe[i][0], 0); 
			dup2(reducer_pipe[1], 1); 
			int status = execlp(mapper_executable, mapper_executable, NULL); 
			print_nonzero_exit_status(mapper_executable, status); 
			exit(1); 
		} 
	} 

    // Start the reducer process.
	pid_t reducer_pid = fork(); 
	if (reducer_pid == 0) { // child 
		dup2(reducer_pipe[0], 0); 
		dup2(output_fd, 1); 
		int status = execlp(reducer_executable, reducer_executable, NULL); 
		print_nonzero_exit_status(reducer_executable, status); 
		exit(1); 
	} 

	// Wait for the reducer to finish 

	descriptors_closeall();
	descriptors_destroy();

	while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {
		continue;
	}
	
    // Count the number of lines in the output file.
	print_num_lines(output_file); 
    return 0;
}
