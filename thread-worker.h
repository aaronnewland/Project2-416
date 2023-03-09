// File:	worker_t.h

// List all group member's name:
// username of iLab:
// iLab Server:

#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_WORKERS macro */
#define USE_WORKERS 1

// Define macros for thread status
// Ready, Sheduled, and Blocked
#define READY 0
#define SCHEDULED 1
#define BLOCKED 2

/* include lib header files that you need here: */
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <ucontext.h>
#include <sys/time.h>
#include <string.h>

typedef uint worker_t;

typedef struct TCB {
	/* add important states in a thread control block */
	// thread Id
	int id;
	// thread status
	int status;
	// thread context
	ucontext_t context;
	// thread stack
	void* threadStack;
	// thread priority, default 0
	int priority;
	// And more ...
	// thread parent context
	ucontext_t parent;
	// thread child context
	// TODO: delete this if not needed
	ucontext_t child;
	// thread address
	// TODO: delete this value if we don't need it
	worker_t *thread;
	// function thread was created with
	// TODO: delete this if not needed
	void* func;
} tcb; 

/* mutex struct definition */
typedef struct worker_mutex_t {
	/* add something here */

	// YOUR CODE HERE
} worker_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

/* node struct for linked list*/
typedef struct Node {
	tcb *block;
	struct Node* next;
} node;

/* queue struct for runqueue*/
typedef struct Queue {
	node *front, *back;
} queue;

/* Function Declarations: */

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level worker threads voluntarily */
int worker_yield();

/* terminate a thread */
void worker_exit(void *value_ptr);

/* wait for thread termination */
int worker_join(worker_t thread, void **value_ptr);

/* initial the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex);

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex);

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex);


/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void);

/* node functions */
/* create linked list node */
node* node_create(tcb *block);

/* queue functions */
/* initialize queue */
queue* queue_init();
/* enqueue node */
void enqueue(queue* q, tcb *block);
/* dequeue node */
void dequeue(queue* q);
/* prints out all nodes in queue from front to back */
void print_queue(queue* q);
void test_dequeue();

#ifdef USE_WORKERS
#define pthread_t worker_t
#define pthread_mutex_t worker_mutex_t
#define pthread_create worker_create
#define pthread_exit worker_exit
#define pthread_join worker_join
#define pthread_mutex_init worker_mutex_init
#define pthread_mutex_lock worker_mutex_lock
#define pthread_mutex_unlock worker_mutex_unlock
#define pthread_mutex_destroy worker_mutex_destroy
#endif

#endif
