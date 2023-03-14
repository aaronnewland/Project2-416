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


// INITAILIZE ALL YOUR OTHER VARIABLES HERE
static int id = 0;
int init = 0;
ucontext_t sched_ctx;
queue* runqueue;
tcb* running = NULL;


/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
       // - create Thread Control Block (TCB)
       // - create and initialize the context of this worker thread
       // - allocate space of stack for this thread to run
       // after everything is set, push this thread into run queue and 
       // - make it ready for the execution.

		// initialize scheduler on first call to worker_create.
		if (init == 0) {
			runqueue = queue_init();

			// initialize bench_ctx
			ucontext_t bench_ctx;
			if (getcontext(&bench_ctx) < 0) {
				perror("getcontext");
				exit(1);
			}
			tcb *temp = (tcb*)malloc(sizeof(tcb));
			temp->id = id;
			temp->context = bench_ctx;
			// push bench_ctx onto runqueue
			enqueue(runqueue, temp);
			// set running context to bench_ctx on first call
			running = dequeue(runqueue);
			running->status = RUNNING;

			init_sched_ctx();
			init = 1;
		}

		if (DEBUG) {
			if (running != NULL) {
				printf("BEGINNING OF WORKER_CREATE: running = %u\n", running->id);
			}
		}

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
		ucontext_t ctx;
		if (getcontext(&ctx) < 0) {
			perror("getcontext");
			exit(1);
		}

		ctx.uc_link = &sched_ctx;
		ctx.uc_stack.ss_sp = stack;
		ctx.uc_stack.ss_size = STACK_SIZE;
		ctx.uc_stack.ss_flags = 0;

		makecontext(&ctx, (void *)function, 0);

		// set context in TCB
		control_block->context = ctx;

		// push new worker thread onto runqueue
		enqueue(runqueue, control_block);

		if (DEBUG) print_queue(runqueue);

		// dummy FCFS
		// call scheduler
		swapcontext(&running->context, &sched_ctx); 

		if (DEBUG) {
			if (running != NULL) {
				printf("END OF WORKER_CREATE: running = %u\n", running->id);
			}
		}
		if (DEBUG) print_queue(runqueue);

    return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	if (DEBUG) puts("in yield");
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	running->status = READY;
	// - switch from thread context to scheduler context
	swapcontext(&running->context, &sched_ctx);
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread
	if (value_ptr != NULL) {
		// do stuff with saving the return value
	}

	free(running->threadStack);
	free(running);

	printf("in exit: \n");
	if (DEBUG) print_queue(runqueue);

	running = NULL;
	setcontext(&sched_ctx);
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	if (value_ptr != NULL) {
		// do stuff with saving the return value
	}

	if (DEBUG) {
		printf("IN WORKER_JOIN: thread = %u\n", thread);
		printf("IN WORKER_JOIN: running id = %u\n", running->id);
	}

	running->wait_id = thread;
	while (running->wait_id != -1) {
		// thread waiting to join has terminiated
		if (find_wait(thread) == 1) {
			running->wait_id = -1;
		// thread is still running
		} else {
			running->status = WAITING;
			swapcontext(&running->context, &sched_ctx);
		}
	}


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
	printf("in schedule\n");

	if (running != NULL) {
		printf("running going into sched: %u\n", running->id);

		if (running->status == RUNNING) {
			running->status = READY;
		}

		enqueue(runqueue, running);		
	} else puts("running going into sched: NULL");

	running = dequeue(runqueue);

	printf("RUNNING AT END OF SCHED: %u\n", running->id);

	setcontext(&running->context);

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
tcb* dequeue(queue* q) {
	// check for empty queue
	if (q->front == NULL) return NULL;

	node* hold = q->front;
	q->front = q->front->next;

	// check for newly empty queue
	if (q->front == NULL) q->back = NULL;

	tcb *temp = hold->block;

	// clear memory for dequeued node
	free(hold);
	return temp;
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

/* initializes scheduler context */
void init_sched_ctx() {
		// TODO: memory cleanup for sched_ctx stack
		// - allocate space of stack for this thread to run
		void *stack = malloc(STACK_SIZE);
		if (stack == NULL) {
			perror("Failed to allocate stack");
			exit(1);
		}
		// - create and initialize the context of this worker thread
		if (getcontext(&sched_ctx) < 0) {
			perror("getcontext");
			exit(1);
		}

		sched_ctx.uc_link = NULL;
		sched_ctx.uc_stack.ss_sp = stack;
		sched_ctx.uc_stack.ss_size = STACK_SIZE;
		sched_ctx.uc_stack.ss_flags = 0;

		makecontext(&sched_ctx, (void *)&schedule, 0);

		return;
}

int find_wait(worker_t find){
	node* walk = runqueue->front;
		while(walk != NULL) {
			printf("WALKING: Thread ID# = %u\n", walk->block->id);

			// The thread is still running
			if (walk->block->id == find) {
				puts("we found it!!");
				return 0;
			}

			walk = walk->next;
		}
	return 1;
}