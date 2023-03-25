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
#define RUNNING 3
#define WAITING 4

#define LOCK 1
#define UNLOCK 0

// in micro seconds (i.e. 10 ms)
#define QUANTUM 10000

// S is in intervals of QUANTUM
#define S 15

#define LEVELS 4

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
#include <time.h>

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
	// number of time quantums elapsed
	int elapsed;
	// TODO: delete this if not needed
	// thread address
	// TODO: delete this value if we don't need it
	worker_t *thread;
	// function thread was created with
	// TODO: delete this if not needed
	void* func;
	// holds id of thread waiting to exit
	worker_t wait_id;
	// holds mutex information
	struct worker_mutex_t* mutex;
	struct timespec start, tt_end;
	struct timespec rt_end;
} tcb; 

/* mutex struct definition */
typedef struct worker_mutex_t {
	// set to 0 for unlock, and 1 for lock
	int lock;
	// queue of threads waiting to access mutex
	struct queue* wait;
} worker_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

/* node struct for linked list*/
typedef struct node {
	tcb *block;
	struct node* next;
} node;

typedef struct mutex_node {
	struct worker_mutex_t* mutex;
	struct mutex_node* next;
} mutex_node;

typedef struct rv_node {
	void* rv;
	int id;
	struct rv_node* next;
} rv_node;

/* queue struct for runqueue*/
typedef struct queue {
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

static void schedule();
static void sched_psjf();
static void sched_mlfq();


/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void);

/* Initializes required contexts and data structures on library 
	first call.*/
void initialize();

/* node functions */
/* create linked list node */
node* node_create(tcb *block);

/* queue functions */
/* initialize queue */
queue* queue_init();

/* enqueue node */
void enqueue(queue* q, tcb *block);

/* dequeue node */
tcb* dequeue(queue* q);

/* prints out all nodes in queue from front to back */
void print_queue(queue* q);

/* initializes scheduler context */
void init_sched_ctx();

void handler(int signum);

/* initializes timer */
void init_timer();

/* Initializes mutex list*/
void init_mutexes();

/* Initializes return values list*/
void init_rvs();

/* finds if thread is in queue q or not */
int find_wait(worker_t find, queue* q);

/* Finds if thread is currently blocked and in mutex waitlist*/
int find_mutex_wait(worker_t find);

/* find the shortest job left until completion based on previous time elapsed */
tcb* find_shortest_job(queue* q);

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
