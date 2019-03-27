/**
 * mapreduce Lab
 * CS 241 - Spring 2019
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
		pipe(mapper_pipe[i]); 
		descriptors_add(mapper_pipe[i][0]); 
		descriptors_add(mapper_pipe[i][1]); 
	} 
	
	// Create one input pipe for the reducer.
	int reducer_pipe[2]; 
	pipe(reducer_pipe); 
	descriptors_add(reducer_pipe[0]); 
	descriptors_add(reducer_pipe[1]); 
    
	// Open the output file.
	int output_fd = open(output_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); 
	descriptors_add(output_fd); 

    // Start a splitter process for each mapper.
	pid_t splitter_pids[mapper_count]; 
	for (int i = 0; i < mapper_count; i++) {  
		splitter_pids[i] = fork(); 
		if (splitter_pids[i] == 0) { // child
			dup2(mapper_pipe[i][1], 1); 
			descriptors_closeall();
			descriptors_destroy();
			int status = execlp("./splitter", "./splitter", input_file, mapper_count, i, NULL); 
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
			descriptors_closeall();
			descriptors_destroy();
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
		descriptors_closeall();
		descriptors_destroy();
		int status = execlp(reducer_executable, reducer_executable, NULL); 
		print_nonzero_exit_status(reducer_executable, status); 
		exit(1); 
	} 

	// Wait for the reducer to finish 
	else { // parent 
		descriptors_closeall();
		descriptors_destroy();
		int status; 
		waitpid(reducer_pid, &status, 0); 
	} 

	while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {
		continue;
	}
	
    // Count the number of lines in the output file.
	print_num_lines(output_file); 
    return 0;
}
