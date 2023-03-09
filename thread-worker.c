// File:	thread-worker.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "thread-worker.h"

// Macro for stack size of each thread
#define STACK_SIZE SIGSTKSZ
#define DEBUG 1

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;

static int id = 0;
static node* head = NULL;
int runqueue_init = 0;
queue* runqueue;

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
		// TODO: MEMORY CLEANUP!!!
					
		// - create Thread Control Block (TCB)
		// Changes value for the caller as well. This is our thread ID.
		*thread = ++id;
		tcb *control_block = (tcb*)malloc(sizeof(tcb));
		control_block->id = *thread;
		// Sets to READY status
		control_block->status = READY;
		control_block->thread = thread;
		control_block->func = function;

		// - allocate space of stack for this thread to run
		void *stack = malloc(STACK_SIZE);
		if (stack == NULL) {
			perror("Failed to allocate stack");
			exit(1);
		}
		control_block->threadStack = stack;
		// - create and initialize the context of this worker thread
		ucontext_t ctx_main, ctx;
		if (getcontext(&ctx) < 0) {
			perror("getcontext");
			exit(1);
		}

		ctx.uc_link = &ctx_main;
		ctx.uc_stack.ss_sp = stack;
		ctx.uc_stack.ss_size = STACK_SIZE;
		ctx.uc_stack.ss_flags = 0;

		makecontext(&ctx, (void *)function, 0);

		// set context in TCB
		control_block->context = ctx;

		// after everything is set, push this thread into run queue and 
		// initialize runqueue if not initialized
		if (runqueue_init == 0) {
			runqueue = queue_init(); 
			runqueue_init = 1;
		}
		enqueue(runqueue, control_block);
		
		if (DEBUG) print_queue(runqueue);

		// - make it ready for the execution.

		// YOUR CODE HERE
	
    return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	// - every time a timer interrupt occurs, your worker thread library 
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	// if (sched == PSJF)
	//		sched_psjf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// - schedule policy
#ifndef MLFQ
	// Choose PSJF
#else 
	// Choose MLFQ
#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}


// Feel free to add any other functions you need
/* create linked list node */
node* node_create(tcb *block) {
	node* thread_node = (node*)malloc(sizeof(node));
	thread_node->next = NULL;
	thread_node->block = block;
	return thread_node;
}

/* initialize queue */
queue* queue_init() {
	queue* q = (queue*)malloc(sizeof(queue));
	q->front = NULL;
	q->back = NULL;
	return q;
}

/* enqueue node */
void enqueue(queue* q, tcb *block) {
	node* thread_node = node_create(block);
	// check for empty queue
	if (q->front == NULL) {
		q->front = thread_node;
		q->back = thread_node;
		return;
	}
	// add to back of queue
	q->back->next = thread_node;
	q->back = thread_node;
}

// /* dequeue node */
void dequeue(queue* q) {
	// check for empty queue
	if (q->front == NULL) return;

	node* hold = q->front;
	q->front = q->front->next;

	// check for newly empty queue
	if (q->front == NULL) q->back = NULL;

	// clear memory for dequeued node
	free(hold);
}

/* prints out all nodes in queue from front to back */
void print_queue(queue* q) {
	// check for empty queue
	if (q->front == NULL) {
		printf("Queue is Empty.\n");
		return;
	} 

	node* walk = q->front;
	printf("--------PRINTING_QUEUE--------\n");
	while(walk != NULL) {
		printf("Thread ID# = %u\n", walk->block->id);
		printf("Thread address = %u\n", walk->block->thread);
		printf("Thread status = %u\n", walk->block->status);
		printf("Thread stack = %p\n", walk->block->threadStack);
		printf("Thread function = %p\n", walk->block->func);
		printf("--------------------------\n");
		walk = walk->next;
	}
	printf("--------DONE_PRINTING--------\n");
}
