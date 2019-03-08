/**
 * Password Cracker Lab
 * CS 241 - Spring 2019
 */
 
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "queue.h"

#include <math.h>
#include <crypt.h>
#include <string.h>
#include <stdio.h>

#define MAX_PASSWORD 10000
//#define NUM_OF_LOWER 26
#define STRING_LEN 32

static queue * TASK_Q = NULL;
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static size_t FINISHED = 0;
static size_t NUM_OF_LOWER = 26;
static size_t ERROR = 0;
static size_t TOTAL_TASK = 0;
/*
 * task_line_parser parse and store info into separated stacks
 * It will free task_line, which is allocated by getline
*/
void task_line_parser(char * name, char * hash, char * password, char * task_line) {
	char * target_pos = NULL;
	char * curr_pos = task_line;
	size_t cpy_len = 0;
	target_pos = strchr(curr_pos, ' ');
	cpy_len = target_pos - curr_pos;
	strncpy(name, curr_pos, cpy_len);
	name[cpy_len] = '\0';
	curr_pos = target_pos + 1;
	
	target_pos = strchr(curr_pos, ' ');
	cpy_len = target_pos - curr_pos;
	strncpy(hash, curr_pos, cpy_len);
	hash[cpy_len] = '\0';
	curr_pos = target_pos + 1;
	
	target_pos = strchr(curr_pos, '\0');
	cpy_len = target_pos - curr_pos;
	strncpy(password, curr_pos, cpy_len);
	password[cpy_len] = '\0';
	
	free(task_line);
	task_line = NULL;
}

void* crack_password (void* thread_id) {
	//Initialize crypt
	struct crypt_data cdata;
	cdata.initialized = 0;
	size_t tid = (size_t) thread_id;
	while (true) {
		//CRITICAL SECTION STARTS
		//Check if it is finished
		pthread_mutex_lock(&m);
		if (FINISHED) {
			pthread_mutex_unlock(&m);
			break;
		}

		//Fetching from queue and Set things up
		char * task_line = (char *) queue_pull(TASK_Q);
		if (task_line == NULL) { //lock and change FINISHED to 1
			FINISHED = 1;
			pthread_mutex_unlock(&m);
			break;
		}
		pthread_mutex_unlock(&m);
		//CRITICAL SECTION ENDS
		char name[STRING_LEN];
		char hash[STRING_LEN];
		char password[STRING_LEN];
		task_line_parser(name, hash, password, task_line);


		//thread_start
		v1_print_thread_start(tid, name);

		//start cracking password
		char * unknown_start = strchr(password, '.');
		size_t known_length = unknown_start - password;
		size_t password_length = strlen(password);
		size_t unknown_length = password_length - known_length;
		double num_of_comb = pow(NUM_OF_LOWER, unknown_length);
		memset(unknown_start, 'a', unknown_length);
//		size_t curr_num_pos;
//		size_t curr_alpha_padding;
//		size_t curr_total;
		const char * hashed;
		size_t i;	
		//mega test
//		printf("name is %s\n hash is %s\n password is %s\n known_length is %zu\n password_length is%zu\n unknown_length is%zu\n", name, hash, password, known_length, password_length, unknown_length); 
//		printf("num_of_comb %f\n", num_of_comb);	
		size_t FOUND = 0;
		for (i = 0; i < num_of_comb; i++) {
//			curr_num_pos = password_length - 1;
//			curr_total = i;
//			while (curr_total > 0 && curr_num_pos >= known_length) {
//				curr_alpha_padding = curr_total % NUM_OF_LOWER;
//				printf("curr_alpha_padding is %zu\n", curr_alpha_padding);
//				password[curr_num_pos] += curr_alpha_padding;
//				curr_num_pos--;
//				curr_total = (curr_total - curr_alpha_padding) / NUM_OF_LOWER;
//			}
			if (i > 0) {
				incrementString(unknown_start);
			}			
//			puts(password);
			hashed = crypt_r(password, "xx", &cdata);
			if (strcmp(hashed, hash) == 0) {
				v1_print_thread_result(tid, name, password, i + 1, getThreadCPUTime(), 0);
				FOUND = 1;
				break;
			}
//			memset(unknown_start, 'a', unknown_length);
		}
		if (FOUND == 0) {
			pthread_mutex_lock(&m);
			ERROR++;
			pthread_mutex_unlock(&m);
			v1_print_thread_result(tid, name, password, num_of_comb, getThreadCPUTime(), 1);
		}
	}
	pthread_exit(NULL);
}


int start(size_t thread_count) {
	//Data Initialization
	TASK_Q = queue_create(MAX_PASSWORD);
	
	//Listening to inputs
	char * task_line = NULL;
	size_t task_line_size = 0;
	ssize_t len;
	while ((len = getline(&task_line, &task_line_size, stdin)) != -1) {
		task_line[len - 1] = '\0';
		queue_push(TASK_Q, task_line);
		TOTAL_TASK++;
		//Reset, accepting new stdin
		task_line = NULL;
		task_line_size = 0;	
	}
	//WARNING: Free the last call to getline
	free(task_line);
	task_line = NULL;
	
	queue_push(TASK_Q, NULL);
	
	//Start threads (stdin read-in ends here)
	size_t i;
	pthread_t threads[thread_count];
	for (i = 0; i < thread_count; i++) {
		pthread_create(&threads[i], NULL, crack_password, (void*) (i + 1));
	}
	
	//Clean up
	for (i = 0; i < thread_count; i++) {
		pthread_join(threads[i], NULL);
	}
	pthread_mutex_destroy(&m);
	queue_destroy(TASK_Q);
	v1_print_summary(TOTAL_TASK - ERROR, ERROR);
	return 0;
}
