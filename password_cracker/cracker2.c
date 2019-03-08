/**
 * Password Cracker Lab
 * CS 241 - Spring 2019
 */
 
#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include "queue.h"

#include <math.h>
#include <crypt.h>
#include <string.h>
#include <stdio.h>

#define MAX_PASSWORD 10000
#define STRING_LEN 32

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static pthread_barrier_t b;
static pthread_barrier_t B;
static size_t THREAD_TOTAL = 0;
static size_t TOTAL_TASK = 0;
static char* NAME[MAX_PASSWORD];
static char* HASH[MAX_PASSWORD];
static char* PASSWORD[MAX_PASSWORD];
static char CRACKED_PASSWORD[STRING_LEN];
static char CRACKED_NAME[STRING_LEN];
static size_t CURR_TOTAL_HASH = 0;
static size_t CURR_TASK = 0;
static bool FOUND_SYNC = false;
static bool TASK_FINISHED = false;
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

void heap_cleaner(size_t i) {
	free(NAME[i]);
	free(HASH[i]);
	free(PASSWORD[i]);
	NAME[i] = NULL;
	HASH[i] = NULL;
	PASSWORD[i] = NULL;
	return;
}

void* crack_password(void* thread_id) {
	//Initialize crypt and thread
	struct crypt_data cdata;
	cdata.initialized = 0;
	size_t tid = (size_t) thread_id;

	pthread_barrier_wait(&B); //Release MAIN to walk in for loop
	pthread_barrier_wait(&B); //waiting for Main commands
	/////////////////////
	//   START TASK    //
	/////////////////////
	while (true) {
		pthread_mutex_lock(&m);
		size_t curr_task = CURR_TASK;
		if (TASK_FINISHED) {
			pthread_mutex_unlock(&m);
			break;
		}
		pthread_mutex_unlock(&m);

		char name[STRING_LEN];
		char hash[STRING_LEN];
		char password[STRING_LEN];
		strcpy(name, NAME[curr_task]);
		strcpy(hash, HASH[curr_task]);
		strcpy(password, PASSWORD[curr_task]);
		
		char * unknown_start = strchr(password, '.');
		size_t known_length = unknown_start - password;
		size_t password_length = strlen(password);
		size_t unknown_length = password_length - known_length;
		long start_index = 0;
		long count = 0;
		getSubrange(unknown_length, THREAD_TOTAL, tid, &start_index, &count);

		//Get thread starting string
		setStringPosition(unknown_start, start_index);

		//STARTING CRACKING !
		pthread_mutex_lock(&m);
		v2_print_thread_start(tid, name, start_index, password);
		pthread_mutex_unlock(&m);

		pthread_barrier_wait(&b);

		long i;
		const char* hashed;
		size_t total_hashes = 0;
		bool FOUND = false;
		bool INTERUPT = false;	
		for (i = 0; i < count; i++) {
			pthread_mutex_lock(&m);
			if (FOUND_SYNC == true) {
				CURR_TOTAL_HASH += total_hashes;
				pthread_mutex_unlock(&m);
				INTERUPT = true;
				break;
			}
			pthread_mutex_unlock(&m);
			if (i > 0) {
				incrementString(unknown_start);
			}
//			puts(password);
			total_hashes++;
			hashed = crypt_r(password, "xx", &cdata);
			if (strcmp(hashed, hash) == 0) {
				FOUND = true;
				break;
			}
		}
		if (FOUND == true) {
			pthread_mutex_lock(&m);
			FOUND_SYNC = true;
			strcpy(CRACKED_PASSWORD, password);
			strcpy(CRACKED_NAME, name);
			CURR_TOTAL_HASH += total_hashes;
			v2_print_thread_result(tid, total_hashes, 0);
			pthread_mutex_unlock(&m);
		}
		if (FOUND == false && INTERUPT == false) {
			pthread_mutex_lock(&m);
			CURR_TOTAL_HASH += total_hashes;
			v2_print_thread_result(tid, total_hashes, 2);
			pthread_mutex_unlock(&m);
		}
		if (FOUND == false && INTERUPT == true) {
			pthread_mutex_lock(&m);
			CURR_TOTAL_HASH += total_hashes;
			v2_print_thread_result(tid, total_hashes, 1);
			pthread_mutex_unlock(&m);
		}
		pthread_barrier_wait(&B); //waiting for parents to print summary
		pthread_barrier_wait(&B); //waiting for parents to release
	}
	pthread_exit(NULL);
}

int start(size_t thread_count) {
	//Initialize
	pthread_barrier_init(&b, NULL, thread_count);
	pthread_barrier_init(&B, NULL, thread_count + 1);
	THREAD_TOTAL = thread_count;

	//Listening to inputs
	char * task_line = NULL;
	size_t task_line_size = 0;
	ssize_t len;
	while ((len = getline(&task_line, &task_line_size, stdin)) != -1) {
		task_line[len - 1] = '\0';
		NAME[TOTAL_TASK] = malloc(STRING_LEN * sizeof(char));
		HASH[TOTAL_TASK]  = malloc(STRING_LEN * sizeof(char));
		PASSWORD[TOTAL_TASK] = malloc(STRING_LEN * sizeof(char));
		task_line_parser(NAME[TOTAL_TASK], HASH[TOTAL_TASK], PASSWORD[TOTAL_TASK], task_line);
		TOTAL_TASK++;
		//Reset, accepting new stdin
		task_line = NULL;
		task_line_size = 0;	
	}
	//WARNING: Free the last call to getline
	free(task_line);
	task_line = NULL;

	//Start threads (stdin readin ends here)
	size_t i;
	pthread_t threads[thread_count];
	for (i = 0; i < thread_count; i++) {
		pthread_create(&threads[i], NULL, crack_password, (void*) (i + 1));
	}
	//Waiting for Child Thread Initialization
	pthread_barrier_wait(&B);
	for (i = 0; i <= TOTAL_TASK; i++) {
		if (i == TOTAL_TASK) {
			TASK_FINISHED = true;
			break;
		}
		double start_time = getTime();
		double start_cpu_time = getCPUTime();
		v2_print_start_user(NAME[i]);
		CURR_TASK = i;
		CURR_TOTAL_HASH = 0;
		pthread_barrier_wait(&B); //Releasing children to crack
		pthread_barrier_wait(&B); //waiting for children result
		if (FOUND_SYNC == true) {
			FOUND_SYNC = false;
			v2_print_summary(CRACKED_NAME, CRACKED_PASSWORD, CURR_TOTAL_HASH, getTime() - start_time, getCPUTime() - start_cpu_time, 0);
		} else { //FOUND_SYNC == false
			v2_print_summary(CRACKED_NAME, CRACKED_PASSWORD, CURR_TOTAL_HASH, getTime() - start_time, getCPUTime() - start_cpu_time, 1);
		}
		heap_cleaner(i);
	}
	pthread_barrier_wait(&B); //Releasing threads for the LAST time;	
	
	
	//Clean Up
	for (i = 0; i < thread_count; i++) {
		pthread_join(threads[i], NULL);
	}
	pthread_mutex_destroy(&m);
	pthread_barrier_destroy(&b);
	pthread_barrier_destroy(&B);
		
	return 0;
}
