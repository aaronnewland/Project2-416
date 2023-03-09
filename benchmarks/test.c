#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"

#define THREAD_NUM 4
#define DEBUG 1

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

pthread_t *thread;

void foo() {
    while (1) {
        puts("foo");
    }
    return;
}

void bar() {
    while (1) {
        puts("bar");
    }
    return;
}

int main(int argc, char **argv) {
	// initialize pthread_t
	thread = (pthread_t*)malloc(THREAD_NUM*sizeof(pthread_t));

    for (int i = 0; i < THREAD_NUM - 1; ++i) {
        if (DEBUG) {
            printf("thread create = %u\n", &thread[i]);
            printf("thread2 = %u\n", thread[i]);
            printf("--------------------------\n");
        }
        
        pthread_create(&thread[i], NULL, &foo, NULL);
    }
    pthread_create(&thread[THREAD_NUM-1], NULL, &bar, NULL);
    if (DEBUG) {
        // prints out all threads and their IDs
        printf("--------------------------\n");
        for (int i = 0; i < THREAD_NUM; ++i) {
            printf("thread = %u\n", &thread[i]);
            printf("thread2 = %u\n", thread[i]);
        }
    }

    // Free memory on Heap
	free(thread);

	return 0;
}
