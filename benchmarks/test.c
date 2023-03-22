#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"

#define THREAD_NUM 2
#define DEBUG 0
#define MAX_COUNT 20

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */
/*
pthread_t *thread;
int count1, count2, count3, count4;
count1 = 0;
count2 = 0;
count3 = 0;
count4 = 0;
pthread_mutex_t mutex;

void foo() {
    while (count1 < MAX_COUNT) {
        puts("foo");
        count1++;
    }

    //pthread_exit(NULL);
}

void bar() {
    while (count2 < MAX_COUNT) {
        puts("bar");
        count2++;
    }

    //pthread_exit(NULL);
}

int main(int argc, char **argv) {
    printf("mutex = %u\n", &mutex);
    // pthread_mutex_init(&mutex, NULL);

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

    //Destroy mutex.
    // pthread_mutex_destroy(&mutex);

    // Free memory on Heap
	free(thread);

	return 0;
}
*/

pthread_t t1, t2, t3, t4;
pthread_mutex_t mutex;
int x = 0;
int loop = 10000;

void *add_counter(void *arg) {

    int i;

    pthread_mutex_lock(&mutex); 
    for(i = 0; i < loop; i++){
        x = x + 1;
    }
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);

    return NULL;
}


int main(int argc, char *argv[]) {

    if(argc != 2){
        printf("Bad Usage: Must pass in a integer\n");
        exit(1);
    }

    loop = atoi(argv[1]);

    printf("Going to run four threads to increment x up to %d\n", 4 * loop);

    //Initialize mutex.
    if (DEBUG) printf("mutex = %p\n", &mutex);
    pthread_mutex_init(&mutex, NULL);

    //Create worker threads.
    pthread_create(&t1, NULL, add_counter, NULL);
    pthread_create(&t2, NULL, add_counter, NULL);
    pthread_create(&t3, NULL, add_counter, NULL);
    pthread_create(&t4, NULL, add_counter, NULL);

    if (DEBUG) {
        printf("thread create = %p\n", &t1);
        printf("thread2 = %u\n", t1);
        printf("--------------------------\n");

        printf("thread create = %p\n", &t2);
        printf("thread2 = %u\n", t2);
        printf("--------------------------\n");

        printf("thread create = %p\n", &t3);
        printf("thread2 = %u\n", t3);
        printf("--------------------------\n");

        printf("thread create = %p\n", &t4);
        printf("thread2 = %u\n", t4);
        printf("--------------------------\n");
    }

    //Join the threads
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    //Destroy mutex.
    pthread_mutex_destroy(&mutex);

    printf("Going to run four threads to increment x up to %d\n", 4 * loop);
    printf("The final value of x is %d\n", x);

    return 0;
}
