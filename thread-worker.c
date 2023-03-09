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
		// Create stack for thread
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

		// Set up linked list if head has not been initialized.
		if (head == NULL) {
			// initialize data structures
			head = (node*)malloc(sizeof(node));
			head->next = NULL;

			head->block = control_block;
		// Append to end of linked list if head has been initialized.
		} else {
			node* thread_node = (node*)malloc(sizeof(node));

			node* walk = head;
			while (walk->next != NULL) {
				walk = walk->next;
			}

			walk->next = thread_node;
			thread_node->next = NULL;
			thread_node->block = control_block;
		}

		if (DEBUG) {
			printf("---------------------------------------\n");
			node* walking = head;
			while (walking != NULL) {
				printf("walking id: %u\n", walking->block->id);
				printf("walking thread: %u\n", walking->block->thread);
				printf("walking status: %u\n", walking->block->status);
				printf("walking threadStack: %p\n", walking->block->threadStack);
				walking = walking->next;
			}
			printf("---------------------------------------\n");
		}

		// - allocate space of stack for this thread to run
		// after everything is set, push this thread into run queue and 
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

// YOUR CODE HERE

