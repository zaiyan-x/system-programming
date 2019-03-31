/**
 * Parallel Make Lab
 * CS 241 - Spring 2019
 */

#include <stdbool.h> 
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "set.h"

graph * G;
vector * GOAL_VECTOR;
pthread_mutex_t m_in;
pthread_mutex_t m_out;
pthread_cond_t cv;

int dispatch_task(char * rule);
rule_t * next_rule(rule_t * rule);
int execute_rule(rule_t * rule);

/*
 * This function checks if a string is indeed a file name (openable one).
 */
bool check_file(char * file_name) {
	FILE * file;
	if ((file = fopen(file_name, "r"))) {
		fclose(file);
		return true;
	} else {
		return false;
	}
}

/*
 * Recursively check descendents of current node are in a cycle
 * Return a boolean
 */
bool detect_cycle(char * node, set * visited_set) {
	if (set_contains(visited_set, node)) {
		return true;
	} else {
		set_add(visited_set, node);
		bool result = false;
		size_t i;
		vector * neighbor_vector = graph_neighbors(G, node);
		size_t neighbor_size = vector_size(neighbor_vector);
		char * curr_rule = NULL;
		for (i = 0; i < neighbor_size; i++) {
			curr_rule = vector_get(neighbor_vector, i);
			result = result || detect_cycle(curr_rule, visited_set);
			if (result) {
				vector_destroy(neighbor_vector);
				return true;
			}
			set_remove(visited_set, curr_rule);
		}
		vector_destroy(neighbor_vector);
		return false;
	}
}

/*
 * Separate Illegal and legal rules for later use.
 * This will modify inputs, illegal_rule_set, and legal_rule_set.
 * It will also call detect_cycle, which is a recursive function.
 */
void organize_rule(set * illegal_rule_set, 
				   set * legal_rule_set) {	
	//Acquire Goal Rule Vector
	vector * goal_rule_vector = graph_neighbors(G, "");
	
	//Loop thru the vector
	//1. For each iteration, a new 'visited_set' will be supplied to 'detect_cycle'.
	//	 Accordingly, it should be cleaned after each iteration.
	//2. Graph should not be changed.
	size_t i;
	bool is_cycle = false;
	char * curr_goal_rule = NULL;
	size_t goal_rule_size = vector_size(goal_rule_vector);
	set * visited_set = NULL;
	for (i = 0; i < goal_rule_size; i++) {
		visited_set = string_set_create();
		curr_goal_rule = vector_get(goal_rule_vector, i);
		is_cycle = detect_cycle(curr_goal_rule, visited_set);
		if (is_cycle) {
			set_add(illegal_rule_set, curr_goal_rule);
		} else {
			set_add(legal_rule_set, curr_goal_rule);
		}
		set_destroy(visited_set);
		visited_set = NULL;
	}
	
	//CLEAN UP
	vector_destroy(goal_rule_vector);
	return;
}

/*
 * state:
 * -1 = failed
 * 0  = default
 * 1  = IP
 * 2  = FINISHED
*/
rule_t * next_rule(rule_t * curr_rule) {
	//Satisfy Base case first
	pthread_mutex_lock(&m_in);
	int curr_rule_state = curr_rule->state;
	pthread_mutex_unlock(&m_in);
	if (curr_rule_state != 0) { //base case: found one which's been taken care of
		return NULL;
	}

	//Base case passed - There is at least curr_rule can be run
	//Check children first
	rule_t * next = NULL;
	vector * child_vector = graph_neighbors(G, curr_rule->target);
	size_t child_size = vector_size(child_vector);
	//ZERO CHILDREN
	if (child_size == 0) {
		next = curr_rule;
		return next;
	}
	//ELSE Check all children
	size_t i;
	bool child_all_done = true;
	bool child_failed = false;
	rule_t * child_rule = NULL;
	int child_state = 0;
	for (i = 0; i < child_size; i++) {
		child_rule = graph_get_vertex_value(G, vector_get(child_vector, i));
		pthread_mutex_lock(&m_in);
		child_state = child_rule->state;
		pthread_mutex_unlock(&m_in);
		if (child_state == 0) {
			next = next_rule(child_rule);
			child_all_done = false;
			if (next != 0) {
				break;
			}
		} else if (child_state == 1) {
			child_all_done = false;
		} else if (child_state == -1) {
			child_failed = true;
		} else { //child_state == 2
		 	//Do Nothing
		}
	}
	if (child_all_done) {
		if (child_failed == true) {
			curr_rule->state = -1;
		} else {
			next = curr_rule;
		}
	}
	return next;
}

/*
 * Fetch next rule with state of 0.
*/
rule_t * fetch_rule() {
	rule_t * curr_goal = NULL;
	rule_t * rule = NULL;
	size_t i;
	for (i = 0; i < vector_size(GOAL_VECTOR); i++) {
		curr_goal = graph_get_vertex_value(G, vector_get(GOAL_VECTOR, i));
		pthread_mutex_lock(&m_in);
		if (curr_goal->state) {
			pthread_mutex_unlock(&m_in);
			vector_erase(GOAL_VECTOR, i);
			if (vector_size(GOAL_VECTOR) == 0) {
				pthread_cond_broadcast(&cv);
				pthread_mutex_unlock(&m_out);
				pthread_exit(0);
			}	
			return fetch_rule();
		}
		pthread_mutex_unlock(&m_in);
		rule = next_rule(curr_goal);
		if (rule != NULL) {
			rule->state = 1;
			break;
		}
		//rule is NULL
	}
	return rule;
}

/*
 * Threads working environment.
 * After being created, threads will enter here.
*/
void * make_worker(void * null) {
	while (true) {	
		pthread_mutex_lock(&m_out);
		if (vector_size(GOAL_VECTOR) == 0) {
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&m_out);
			pthread_exit(0);
		}
		rule_t * curr_rule = NULL;
		while (true) {
			curr_rule = fetch_rule();
			if (curr_rule != NULL) {
				break;
			}
			if (vector_size(GOAL_VECTOR) == 0) {
				pthread_cond_broadcast(&cv);
				pthread_mutex_unlock(&m_out);
				pthread_exit(0);
			}
			pthread_cond_wait(&cv, &m_out);
		}
		pthread_mutex_unlock(&m_out);
		execute_rule(curr_rule);
		pthread_cond_broadcast(&cv);
	}
} 


/*
 * execute current rule.
 * return value:
 * /1/ means rule commands are all successes.
 * /-1/ means rule commands failed sometime.
*/
int execute_rule(rule_t * curr_rule) {
	char * rule = curr_rule->target;
	bool is_file = check_file(rule);
	bool is_newer = false;
	size_t i;
	if (is_file) {//current rule is a file
		vector * child_vector = graph_neighbors(G, rule);
		size_t child_size = vector_size(child_vector);
		char * curr_rule = NULL;
		for (i = 0; i < child_size; i++) {
			curr_rule = vector_get(child_vector, i);
			if (check_file(curr_rule)) { //curr child is also a file
				struct stat child_stat;
				struct stat parent_stat;
				stat(curr_rule, &child_stat);
				stat(rule, &parent_stat);
				if (difftime(parent_stat.st_mtime, child_stat.st_mtime) > 0) {
					continue;
				} else {
					is_newer = true;
				}
			}
		}
		vector_destroy(child_vector);
	}
	rule_t * curr_rule_t = (rule_t *) graph_get_vertex_value(G, rule);
	int retval = 0;
	if (is_file == false || (is_file && is_newer)) {
		char * curr_cmd = NULL;
		rule_t * curr_rule_t = (rule_t *) graph_get_vertex_value(G, rule);
		vector * cmd_vector = curr_rule_t->commands;
		size_t cmd_size = vector_size(cmd_vector);
		for (i = 0; i < cmd_size; i++) {
			curr_cmd = vector_get(cmd_vector, i);
			if (system(curr_cmd) != 0) { //system() failed
				retval = -1;
				break;
			}
		}
		if (retval == -1) { //fail
		} else { //success
			retval = 2;
		}
		pthread_mutex_lock(&m_in);
		curr_rule_t->state = retval;
		pthread_mutex_unlock(&m_in);
		return retval;
	} else {
		retval = 2;
		pthread_mutex_lock(&m_in);
		curr_rule_t->state = retval;
		pthread_mutex_lock(&m_in);
		return retval;
	}
}


int parmake(char *makefile, size_t num_threads, char **targets) {
	//Initiate Graph - TO CLEAR
	G = parser_parse_makefile(makefile, targets);

	//Initiate Sets - TO CLEAR
	set * illegal_rule_set = string_set_create();
	set * legal_rule_set = string_set_create();	
	organize_rule(illegal_rule_set, legal_rule_set);
	
	//Reporting
	SET_FOR_EACH(illegal_rule_set, curr_rule, {print_cycle_failure(curr_rule);});

	//Prepare stuff
	GOAL_VECTOR = string_vector_create();
	SET_FOR_EACH(legal_rule_set, curr_rule, {vector_push_back(GOAL_VECTOR, curr_rule);});
	pthread_cond_init(&cv, NULL);
	pthread_mutex_init(&m_out, NULL);
	pthread_mutex_init(&m_in, NULL);

	//Set worker threads
	size_t i;
	pthread_t threads[num_threads];
	for (i = 0; i < num_threads; i++) {
		pthread_create(&threads[i], NULL, make_worker, NULL);
	}

	//Wair for all workers
	for (i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}

	//CLEAN UP
	set_destroy(illegal_rule_set);
	set_destroy(legal_rule_set);
	vector_destroy(GOAL_VECTOR);
	graph_destroy(G);
	return 0;
}
