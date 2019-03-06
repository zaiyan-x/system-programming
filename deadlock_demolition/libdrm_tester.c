/**
 * Deadlock Demolition Lab
 * CS 241 - Spring 2019
 */
 
#include "libdrm.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//int a = 10000;
//drm_t * drm;
//void* decrement(void* arg) {
//	drm_wait(drm, arg);
//	a--;
//	drm_post(drm, arg);
//	return NULL;
//}

int main() {
    drm_t * drm = drm_init();
    // TODO your tests here
//	pthread_t tid1, tid2;
//	pthread_create(&tid1, NULL, decrement, (void*) &tid1);
//	pthread_create(&tid2, NULL, decrement, (void*) &tid2);
    drm_destroy(drm);
	
//	pthread_join(tid1, NULL);
//	pthread_join(tid2, NULL);
    return 0;
}
