/**
 * Savvy Scheduler
 * CS 241 - Spring 2019
 * Collab with Eric Wang - wcwang2
 */
#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print_functions.h"
typedef struct _job_info {
    int id;
	int priority;

	int start_time;
	int end_time;
	int run_time;
	int arrival_time;
	int remaining_time;
	int rr_queue_time;
	int current_start_time;
} job_info;

//static job* current_job;
//static int current_job_start_time;
static int total_job;
priqueue_t pqueue;
scheme_t pqueue_scheme;
comparer_t comparision_func;

//Time globals
static int wait_time;
static int turnaround_time;
static int response_time;

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any set up code you may need here
	total_job = 0;

	wait_time = 0;
	turnaround_time = 0;
	response_time = 0;
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    job* job_a = (job*) a;
	job* job_b = (job*) b;
	job_info* info_a = (job_info*) job_a->metadata;
	job_info* info_b = (job_info*) job_b->metadata;
	if (info_a->arrival_time < info_b->arrival_time) {
		return -1;
	} else if (info_a->arrival_time > info_b->arrival_time) {
		return 1;
	} else {
		return 0;
	}
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
	job* job_a = (job*) a;
	job* job_b = (job*) b;
	job_info* info_a = (job_info*) job_a->metadata;
	job_info* info_b = (job_info*) job_b->metadata;
	if (info_a->priority < info_b->priority) {
		return -1;
	} else if (info_a->priority > info_b->priority) {
		return 1;
	} else {
		return break_tie(a,b);
	}	
}

//Compare remaining time
int comparer_psrtf(const void *a, const void *b) {
	job* job_a = (job*) a;
	job* job_b = (job*) b;
	job_info* info_a = (job_info*) job_a->metadata;
	job_info* info_b = (job_info*) job_b->metadata;
	if (info_a->remaining_time < info_b->remaining_time) {
		return -1;
	} else if (info_a->remaining_time > info_b->remaining_time) {
		return 1;
	} else {
		return break_tie(a, b);
	}
}

int comparer_rr(const void *a, const void *b) {
	job* job_a = (job*) a;
	job* job_b = (job*) b;
	job_info* info_a = (job_info*) job_a->metadata;
	job_info* info_b = (job_info*) job_b->metadata;
	if (info_a->rr_queue_time < info_b->rr_queue_time) {
		return -1;
	} else if (info_a->rr_queue_time > info_b->rr_queue_time) {
		return 1;
	} else {
		return break_tie(a, b);
	}
}

int comparer_sjf(const void *a, const void *b) {
	job* job_a = (job*) a;
	job* job_b = (job*) b;
	job_info* info_a = (job_info*) job_a->metadata;
	job_info* info_b = (job_info*) job_b->metadata;
	if (info_a->run_time < info_b->run_time) {
		return -1;
	} else if (info_a->run_time > info_b->run_time) {
		return 1;
	} else {
		return break_tie(a, b);
	}
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
	job_info * new_job_info = malloc(sizeof(job_info));
	memset(new_job_info, 0, sizeof(job_info));

	new_job_info->id = job_number;
	new_job_info->priority = sched_data->priority;
	new_job_info->run_time = sched_data->running_time;
	new_job_info->remaining_time = sched_data->running_time;
	new_job_info->rr_queue_time = -1;
	new_job_info->arrival_time = time;
	new_job_info->end_time = -1;
	new_job_info->start_time = -1;
	new_job_info->current_start_time = -1;

	//Store info into job
	newjob->metadata = (void*) new_job_info;

	//Push newjob into heap
	priqueue_offer(&pqueue, newjob);
	
	//Do record
	total_job++;
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
	//Check if rotation is needed
	if (pqueue_scheme == FCFS || pqueue_scheme == PRI || pqueue_scheme == SJF) {
		if (job_evicted != NULL) {
			return job_evicted;
		}
	}

	if (job_evicted != NULL) {
		job_info* job_evicted_info = (job_info*) job_evicted->metadata;

		//Update current job remaining time
		job_evicted_info->remaining_time -= (time - job_evicted_info->current_start_time);

		//Update current job rr_queue_time
		job_evicted_info->rr_queue_time = time;
		
		//Push current job back to the queue
		priqueue_offer(&pqueue, job_evicted);
	}

	//Flush
	job * priority_job = (job*) priqueue_poll(&pqueue);
	if (priority_job == NULL) {
		return NULL;
	}
	job_info* priority_info = (job_info*) priority_job->metadata;
	
	if (priority_info->start_time == -1) {
		priority_info->start_time = time;
	}
	priority_info->current_start_time = time;
	return priority_job;
}

void scheduler_job_finished(job *job_done, double time) {
	if (job_done == NULL) {
		return;
	}
	
	job_info* info = (job_info*) job_done->metadata;
	info->end_time = time;
	turnaround_time += info->end_time - info->arrival_time;
	response_time += info->start_time - info->arrival_time;
	wait_time += info->end_time - info->arrival_time - info->run_time;

	//free malloc
	free(info);
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
	return 1.0 * wait_time / total_job;
}

double scheduler_average_turnaround_time() {
    return 1.0 * turnaround_time / total_job;
}

double scheduler_average_response_time() {
    return 1.0 * response_time / total_job;
}

void scheduler_show_queue() {
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}
