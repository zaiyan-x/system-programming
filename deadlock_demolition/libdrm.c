/**
 * Deadlock Demolition Lab
 * CS 241 - Spring 2019
 * Collab with Eric Wang - wcwang2 FUCK YOU 374
 */
 
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

struct drm_t {
	pthread_mutex_t m; //m: a wrapped mutex
	pthread_t * tid; //tid: the thread hold it
};

//global variable
graph * RAG = NULL;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

drm_t *drm_init() {
	pthread_mutex_lock(&m);
    if (RAG == NULL) {
		RAG = shallow_graph_create();
	}
	pthread_mutex_unlock(&m);
	drm_t * drm = malloc(sizeof(struct drm_t));
	pthread_mutex_init(&drm->m, NULL); 
	drm->tid = NULL;
	graph_add_vertex(RAG, drm);
	return drm;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
	pthread_mutex_lock(&m);
	void * curr_node = (void*) (*thread_id);
	if (!graph_contains_vertex(RAG, curr_node)) {
		pthread_mutex_unlock(&m);
		return 0;
	} else {
		if (graph_adjacent(RAG, drm, curr_node)) {
			graph_remove_edge(RAG, drm, curr_node);
			pthread_mutex_unlock(&drm->m);
			free(drm->tid);
			drm->tid = NULL;
		}
		pthread_mutex_unlock(&m);
		return 1;
	}
}

void graph_reset() {
	vector * vertex_vector = graph_vertices(RAG);
	size_t i;
	for (i = 0; i < vector_size(vertex_vector); i++) {
		graph_set_vertex_value(RAG, vector_get(vertex_vector, i), "n");
	}
	return;
}

bool detect_cycle(void* node) {
	char * value = (char *) graph_get_vertex_value(RAG, node);
	if (*value == 'y') {
		return true;
	} else {
		bool result = false;
		size_t i;
		vector * neighbor_vector = graph_neighbors(RAG, node);
		graph_set_vertex_value(RAG, node, "y");
		for (i = 0; i < vector_size(neighbor_vector); i++) {
			result = result || detect_cycle(vector_get(neighbor_vector, i));
			if (result) {
				return true;
			}
		}
		return false;
	}	
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
	pthread_mutex_lock(&m);
	void* curr_node = (void*) (*thread_id);
	//FIRST add thread_id if it is present
	if (!graph_contains_vertex(RAG, curr_node)) {
		graph_add_vertex(RAG, curr_node);
	}
	if (graph_adjacent(RAG, drm, curr_node)) {
		pthread_mutex_unlock(&m);
		return 0;
	}
	graph_add_edge(RAG, curr_node, drm);
	graph_reset();
	if (detect_cycle(curr_node)) {
		graph_remove_edge(RAG, curr_node, drm);
		pthread_mutex_unlock(&m);
		return 0;
	} else {
		pthread_mutex_unlock(&m);
		pthread_mutex_lock(&drm->m);
		pthread_mutex_lock(&m);
		drm->tid = malloc(sizeof(pthread_t));
		*drm->tid = *thread_id;
		graph_remove_edge(RAG, curr_node, drm);
		graph_add_edge(RAG, drm, curr_node);
		pthread_mutex_unlock(&m);
		return 1;
	}			
}

void drm_destroy(drm_t *drm) {
	pthread_mutex_lock(&m);
	if (graph_contains_vertex(RAG, drm)) {
		vector* neighbor_vector = graph_neighbors(RAG, drm);
		vector* antineighbor_vector = graph_antineighbors(RAG, drm);
		size_t i;
		for (i = 0; i < vector_size(neighbor_vector); i++) {
			graph_remove_edge(RAG, drm, vector_get(neighbor_vector, i));
		}
		for (i = 0; i < vector_size(antineighbor_vector); i++) {
			graph_remove_edge(RAG, vector_get(antineighbor_vector, i), drm);
		}	
		graph_remove_vertex(RAG, drm);
		free(drm->tid);
		drm->tid = NULL;
		free(drm);
		vector_destroy(neighbor_vector);
		vector_destroy(antineighbor_vector);
	}
	pthread_mutex_unlock(&m);
    return;
}
