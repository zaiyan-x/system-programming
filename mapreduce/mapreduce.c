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

static int STDIN = 0;
static int STDOUT = 1;

int main(int argc, char **argv) {
	
	if (argc != 6) { 
		print_usage(); 
		return 0; 
	} 
    char* input_file = argv[1]; 
	char* output_file = argv[2]; 
	char* mapper_executable = argv[3]; 
	char* reducer_executable = argv[4]; 
	int mapper_count; 
	sscanf(argv[5], "%d", &mapper_count); 
	
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
	FILE* f_output = fopen(output_file, "w"); 
	descriptors_add(fileno(f_output)); 

    // Start a splitter process for each mapper.
	pid_t splitter_pids[mapper_count]; 
	for (int i = 0; i < mapper_count; i++) {  
		splitter_pids[i] = fork(); 
		if (splitter_pids[i] == 0) { // child
			dup2(mapper_pipe[i][1], 1); 
			close(mapper_pipe[i][0]); 
			int status = execlp("./splitter", "./splitter", input_file, mapper_count, i, NULL); 
			if (status < 0) print_nonzero_exit_status("./splitter", status); 
			exit(1); 
		} 
		else { // parent 
			int status; 
			waitpid(splitter_pids[i], &status, 0); 
			close(mapper_pipe[i][1]); 
		} 
	} 
    
	// Start all the mapper processes.
	pid_t mapper_pids[mapper_count]; 
	for (int i = 0; i < mapper_count; i++) { 
		mapper_pids[i] = fork(); 
		if (mapper_pids[i] == 0) { // child 
			dup2(mapper_pipe[i][0], 0); 
			dup2(reducer_pipe[1], 1); 
			close(reducer_pipe[0]); 
			close(mapper_pipe[i][1]); 
			int status = execlp(mapper_executable, mapper_executable, NULL); 
			if (status < 0) print_nonzero_exit_status(mapper_executable, status); 
			exit(1); 
		} 
		else { // parent 
			int status; 
			waitpid(mapper_pids[i], &status, 0); 
			close(mapper_pipe[i][0]); 
		} 
	} 

    // Start the reducer process.
	pid_t reducer_pid = fork(); 
	if (reducer_pid == 0) { // child 
		close(reducer_pipe[1]); 
		dup2(reducer_pipe[0], 0); 
		dup2(fileno(f_output), 1); 
		int status = execlp(reducer_executable, reducer_executable, NULL); 
		if (status < 0) print_nonzero_exit_status(reducer_executable, status); 
		exit(1); 
	} 

	// Wait for the reducer to finish 
	else { // parent 
		close(reducer_pipe[0]); 
		close(reducer_pipe[1]); 
		int status; 
		waitpid(reducer_pid, &status, 0); 
		fclose(f_output); 
	} 
	
    // Count the number of lines in the output file.
	print_num_lines(output_file); 
	descriptors_closeall(); 
	descriptors_destroy(); 
    return 0;
}
