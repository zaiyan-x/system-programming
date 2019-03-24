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

int dispatch_task(char * rule);

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
	for (i = 0; i < vector_size(goal_rule_vector); i++) {
		set * visited_set = string_set_create();
		curr_goal_rule = vector_get(goal_rule_vector, i);
		is_cycle = detect_cycle(curr_goal_rule, visited_set);
		if (is_cycle) {
			set_add(illegal_rule_set, curr_goal_rule);
		} else {
			set_add(legal_rule_set, curr_goal_rule);
		}
		set_destroy(visited_set);
	}
	
	//CLEAN UP
	vector_destroy(goal_rule_vector);
	return;
}



int execute_leaf(rule_t * rule) {
	size_t i;
	char * curr_cmd = NULL;
	vector * cmd_vec = rule->commands;
	size_t cmd_vec_size = vector_size(cmd_vec);
	int retval = 0;
	for (i = 0; i < cmd_vec_size; i++) {
		curr_cmd = vector_get(cmd_vec, i);
		if (system(curr_cmd) != 0) { //system() failed
			retval = -1;
			break;
		}
	}
	if (retval == -1) { //fail
	} else { //success
		retval = 1;
	}
	
	//CLEAN UP
	vector_destroy(cmd_vec);
	return retval;		
}

int execute_command_rule(char * rule_vertex) {
	//Prepare things
	rule_t * rule = (rule_t *) graph_get_vertex_value(G, rule_vertex);
	//Check if the rule is already executed
	if (rule->state == 1) {
		return 1;
	}
	//Keeps preparing
	vector * neighbor_vector = graph_neighbors(G, rule_vertex);
	size_t neighbor_size = vector_size(neighbor_vector);
	int retval = 0;
	//Check if it is leaf
	if (neighbor_size == 0) { //leaf call
		retval = execute_leaf(rule);
		rule->state = retval;
	} else { //non-leaf call
		size_t i;
		char * curr_rule = NULL;
		for (i = 0; i < neighbor_size; i++) {
			curr_rule = vector_get(neighbor_vector, i);
			retval = dispatch_task(curr_rule);
			if (retval == -1) { //descendents failed
				break;
			}
		}
		rule->state = retval;
	}
		
	//CLEAN UP
	vector_destroy(neighbor_vector);
	return retval;
}

int execute_file_rule(char * rule_vertex) {
	//Prepare things
	rule_t * rule = (rule_t *) graph_get_vertex_value(G, rule_vertex);
	vector * neighbor_vector = graph_neighbors(G, rule_vertex);
	size_t neighbor_size = vector_size(neighbor_vector);
	int retval = 0;
	
	//Check if it is leaf
	if (neighbor_size == 0) { //leaf call
		retval = execute_leaf(rule);
		rule->state = retval;
	} else { //non-leaf call
		size_t i;
		char * curr_rule = NULL;
		for (i = 0; i < neighbor_size; i++) {
			curr_rule = vector_get(neighbor_vector, i);
			if (check_file(curr_rule)) { //curr child is also a file
				struct stat child_stat;
				struct stat parent_stat;
				stat(curr_rule, &child_stat);
				stat(rule_vertex, &parent_stat);
				if (difftime(parent_stat.st_mtime, child_stat.st_mtime) > 0) {
					retval = 1;
					continue; //skip
				} else {
					retval = dispatch_task(curr_rule);
					if (retval == -1) { //descendents failed
						break;
					}
				}
			} else {
				retval = dispatch_task(curr_rule);
				if (retval == -1) { //descendents failed
					break;
				}
			}
		}
		rule->state = retval;
	}
	//CLEAN UP
	vector_destroy(neighbor_vector);
	return retval;
}

/*
 * Separate rule into either command or file
 */
int dispatch_task(char * rule) {
	if (check_file(rule)) {
		return execute_file_rule(rule);
	} else {
		return execute_command_rule(rule);
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

	//Ready to make
	SET_FOR_EACH(legal_rule_set, curr_rule, {dispatch_task(curr_rule);});
	
	//CLEAN UP
	set_destroy(illegal_rule_set);
	set_destroy(legal_rule_set);
	graph_destroy(G);
	return 0;
}
