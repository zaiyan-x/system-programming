/**
 * Teaching Threads Lab
 * CS 241 - Spring 2019
 * COLLAB with Eric Wang - wcwang2 :)
 */
 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct task_{ 
    int* list; 
    int len; 
    reducer reduce_fun; 
    int base_case; 
    int* result_arr; 
    int thread_index; 
} task; 

/* You should create a start routine for your threads. */
void* start_routine(void* data) { 
    task* ptr = (task*) data; 
    (ptr -> result_arr)[ptr -> thread_index] = reduce(ptr -> list, ptr -> len, ptr -> reduce_fun, ptr -> base_case); 
    return NULL; 
} 

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */ 
    if (!list) return 0; 
    if (num_threads > list_len) num_threads = list_len; 

    int a = list_len / num_threads; 
    int b = list_len % num_threads; 

    int result[num_threads]; 
    task task_arr[num_threads]; 
    pthread_t threads[num_threads]; 

    for (size_t i = 0; i < num_threads; i++) { 
	// list 
	task_arr[i].list = list + a * i; 

	//len 
	if (i != num_threads - 1) task_arr[i].len = a; 
	else task_arr[i].len = a + b; 
	
	// reducer 
	task_arr[i].reduce_fun = reduce_func; 

	// base 
	task_arr[i].base_case = base_case; 

	// thread_index 
	task_arr[i].thread_index = i; 

	// result_arr
	task_arr[i].result_arr = result; 

	// create 
	pthread_create(&threads[i], NULL, &start_routine, &task_arr[i]); 
    } 
    for (size_t i = 0; i < num_threads; i++) { 
	pthread_join(threads[i], NULL); 
    } 
    return reduce(result, num_threads, reduce_func, base_case);
}
