#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"

#define THREAD_NUM 2
#define DEBUG 1
#define MAX_COUNT 10

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

pthread_t *thread;
int count1, count2, count3, count4;
count1 = 0;
count2 = 0;
count3 = 0;
count4 = 0;

void foo() {
    while (count1 < MAX_COUNT) {
        puts("foo");
        count1++;
    }

    pthread_exit(NULL);
}

void bar() {
    while (count2 < MAX_COUNT) {
        puts("bar");
        count2++;
    }

    pthread_exit(NULL);
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
    if (DEBUG) {
        printf("thread create = %u\n", &thread[THREAD_NUM-1]);
        printf("thread2 = %u\n", thread[THREAD_NUM-1]);
        printf("--------------------------\n");
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

    pthread_join(thread[0], NULL);
    pthread_join(thread[1], NULL);

    // Free memory on Heap
	free(thread);

	return 0;
}
