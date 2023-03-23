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
static int interval = 0;
int init = 0;
ucontext_t sched_ctx;
// runqueue doubles as level 1 of the MLFQ
queue* runqueue;
tcb* running = NULL;
queue* queues[LEVELS - 1];

// TODO: delete probably
mutex_node* mutexes = NULL;

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
       // - create Thread Control Block (TCB)
       // - create and initialize the context of this worker thread
       // - allocate space of stack for this thread to run
       // after everything is set, push this thread into run queue and 
       // - make it ready for the execution.

		// initialize scheduler on first call to worker_create.
		if (init == 0) initialize();

		if (DEBUG && running != NULL) {
			printf("BEGINNING OF WORKER_CREATE: running = %u\n", running->id);
		}

		*thread = ++id;
		tcb *control_block = (tcb*)malloc(sizeof(tcb));
		control_block->id = *thread;
		control_block->status = READY;
		control_block->thread = thread;
		control_block->func = function;
		control_block->elapsed = 0;
		// Set defualt priority of 1
		control_block->priority = 1;

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

		// Determine argument value
		if (arg == NULL) {
			makecontext(&ctx, (void *)function, 0);
		} else {
			makecontext(&ctx, (void *)function, 1, arg);
		}
		

		// set context in TCB
		control_block->context = ctx;

		// push new worker thread onto runqueue
		enqueue(runqueue, control_block);

		if (DEBUG) print_queue(runqueue);

		// call scheduler
		tot_cntx_switches++;
		swapcontext(&running->context, &sched_ctx); 

		if (DEBUG && running != NULL) {
			printf("END OF WORKER_CREATE: running = %u\n", running->id);		
		}
		if (DEBUG) print_queue(runqueue);

    return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	if (init == 0) initialize();
	if (DEBUG) puts("in yield");
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	running->status = READY;
	// - switch from thread context to scheduler context
	tot_cntx_switches++;
	swapcontext(&running->context, &sched_ctx);
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	if (init == 0) initialize();
	// - de-allocate any dynamic memory created when starting this thread
	if (value_ptr != NULL) {
		// do stuff with saving the return value
	}

	free(running->threadStack);
	free(running);

	if (DEBUG) {
		printf("in exit: \n");
		print_queue(runqueue);
	}


	running = NULL;
	tot_cntx_switches++;
	setcontext(&sched_ctx);
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	if (init == 0) initialize();
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
	if (value_ptr != NULL) {
		// do stuff with saving the return value
	}

	if (DEBUG) {
		printf("IN WORKER_JOIN: thread = %u\n", thread);
		printf("IN WORKER_JOIN: running id = %u\n", running->id);
	}

	running->wait_id = thread;
	if (DEBUG) printf("IN WORKER_JOIN: running->wait_id = %u\n", running->wait_id);
	while (running->wait_id != -1) {

		// - schedule policy
		#ifndef MLFQ
			// thread waiting to join has terminiated
			if ((find_wait(thread, runqueue) == 0) && (find_mutex_wait(thread) == 0)) {
				running->wait_id = -1;
			// thread is still running
			} else {
				running->status = WAITING;
				tot_cntx_switches++;
				swapcontext(&running->context, &sched_ctx);
			}
		#else 
			// Choose MLFQ
			// thread waiting to join has terminiated
			if ((find_wait(thread, runqueue) == 0) && (find_mutex_wait(thread) == 0)
				&& (find_wait(thread, queues[0]) == 0) && (find_wait(thread, queues[1]) == 0) && (find_wait(thread, queues[2]) == 0))  {
				running->wait_id = -1;
			// thread is still running
			} else {
				running->status = WAITING;
				tot_cntx_switches++;
				swapcontext(&running->context, &sched_ctx);
			}
		#endif
	}
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	if (init == 0) initialize();
	if (mutexes->mutex == NULL) init_mutexes();
	//- initialize data structures for this mutex
	if (DEBUG) printf("mutex_create = %p\n", mutex);

	mutex_node *temp = (mutex_node*)malloc(sizeof(mutex_node));
	mutexes->mutex = mutex;
	mutexes->next = temp;
	temp->mutex = NULL;
	temp->next = NULL;

	//mutex = (worker_mutex_t*)malloc(sizeof(worker_mutex_t));
	// if (mutex == NULL) {
	// 	perror("Failed to allocate mutex");
	// 	exit(1);
	// }
	mutex->lock = 0;
	mutex->wait = queue_init();
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

		if (init == 0) initialize();
		if (DEBUG) {
			printf("BEGINNING mutex lock threadID: %u\n", running->id);
			printf("BEGINNING mutex lock: %u\n", mutex->lock);
		}
    
		while (__sync_lock_test_and_set(&mutex->lock, LOCK)) {
			if (DEBUG) puts("I'm in the while for mutx");
			running->status = BLOCKED;
			enqueue(mutex->wait, running);
			if (DEBUG) printf("WAITLIST LOCK: ");
			if (DEBUG) print_queue(mutex->wait);
			//swapcontext(&running->context, &sched_ctx);
			tot_cntx_switches++;
			setcontext(&sched_ctx);
		}

		if (DEBUG) printf("END mutex lock: %u\n", mutex->lock);
        


        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	if (init == 0) initialize();
	mutex->lock = UNLOCK;

	// if (running->status == BLOCKED) {
	// 	running->status = RUNNING;
	// }

	tcb* walk = dequeue(mutex->wait);
	while(walk != NULL) {
		walk->status = READY;
		// TODO: remove if we change prio elsewhere
		//walk->priority = 1;
		enqueue(runqueue, walk);
		walk = dequeue(mutex->wait);
	}

	if (DEBUG) printf("WAITLIST UNLOCK: ");
	if (DEBUG) print_queue(mutex->wait);

	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	if (init == 0) initialize();
	if (DEBUG)  printf("mutex to destroy = %p\n", mutex);
	// TODO: memory cleanup for freeing mutex if needed
	// - de-allocate dynamic memory created in worker_mutex_init
	// if(dequeue(mutex->wait) != NULL) {
	// 	// queue is not empty 
	// 	// handle this case
	// 	return 0;
	// }

	// queue is empty, destroy as normal
	free(mutex->wait);
	
	mutex_node* walk = mutexes;
	mutex_node* walk_next = mutexes->next;

	// only one mutex in list
	if (walk_next == NULL) {
		free(walk);
	}

	while (walk != NULL && walk_next != NULL) {
		if (walk_next->mutex == mutex) {
			if (DEBUG) puts("we got it");

			// end of list
			if (walk_next->next == NULL) {
				walk->next = NULL;
			// middle of list
			} else {
				walk->next = walk_next->next;
			}
			free(walk_next);
		}
		walk = walk->next;
		walk_next = walk_next->next;
	}

	//free(mutex);

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

// - schedule policy
#ifndef MLFQ
	// Choose PSJF
	sched_psjf();
#else 
	// Choose MLFQ
	sched_mlfq();
#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	if (DEBUG) printf("in PSJF schedule\n");

	if (running != NULL) {
		if (DEBUG) printf("running going into sched: %u\n", running->id);

		if (running->status == RUNNING) {
			running->status = READY;
		}

		if (running->status != BLOCKED) {
			enqueue(runqueue, running);	
		}		
			
	} else if (DEBUG)  puts("running going into sched: NULL");

	running = find_shortest_job(runqueue);

	if (DEBUG) printf("RUNNING AT END OF SCHED: %u\n", running->id);

	if (DEBUG) print_queue(runqueue);

	running->elapsed += 1;
	if (DEBUG) printf("RUNNING AT END OF SCHED: %u\n TIME CHUNKS ELAPSED: %u\n", running->id, running->elapsed);
	tot_cntx_switches++;
	if ((mutexes->mutex->wait != NULL) && (DEBUG)) {
		puts("mutex wait list:");
		print_queue(mutexes->mutex->wait);
	} else if (DEBUG) puts("mutex wait list is empty");
	setcontext(&running->context);
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	if (DEBUG) printf("in MLFQ schedule\n");

	// Boost priority for all threads in all levels
	if (interval == S) {
		for (int i = 0; i < (LEVELS - 1); i++) {
			tcb* temp = dequeue(queues[i]);
			while(temp != NULL) {
				temp->priority = 1;
				enqueue(runqueue, temp);
			}
		}
		// Reset time interval
		interval = 0;
	}

	if (running != NULL) {
		if (DEBUG) printf("running going into sched: %u\n", running->id);

		if (running->status == RUNNING) {
			running->status = READY;
		}

		if (running->status != BLOCKED) {
			switch (running->priority) {
				case 1:
					// runqueue
					enqueue(runqueue, running);
					break;
				case 2:
					// level 2
					enqueue(queues[0], running);
					break;
				case 3:
					// level 3
					enqueue(queues[1], running);
					break;
				case 4:
					// level4
					enqueue(queues[2], running);
					break;
				default:
					printf("ERROR: Erronous MLFQ level value\n");
			}
		}		
			
	} else if (DEBUG)  puts("running going into sched: NULL");

	//running = dequeue(runqueue);
	if (runqueue->front != NULL) {
		// enqueue from level 1 (runqueue)
		running = dequeue(runqueue);
	} else if (queues[0]->front != NULL) {
		// enqueue from level 2
		running = dequeue(queues[0]);
	} else if (queues[1]->front != NULL) {
		// enqueue from level 3
		running = dequeue(queues[1]);
	} else if (queues[2]->front != NULL) {
		// enqueue from level 4
		running = dequeue(queues[2]);
	}

	if (DEBUG) printf("RUNNING AT END OF SCHED: %u\n", running->id);

	if (DEBUG) print_queue(runqueue);

	running->elapsed++;
	if (running->priority < LEVELS) {
		running->priority++;
	}
	if (DEBUG) printf("RUNNING AT END OF SCHED: %u\n TIME CHUNKS ELAPSED: %u\n", running->id, running->elapsed);
	tot_cntx_switches++;
	if ((mutexes->mutex->wait != NULL) && (DEBUG)) {
		puts("mutex wait list:");
		print_queue(mutexes->mutex->wait);
	} else if (DEBUG) puts("mutex wait list is empty");
	setcontext(&running->context);
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}

/* Initializes required contexts and data structures on library 
	first call.*/
void initialize() {
	runqueue = queue_init();

	init_mutexes();

	// initialize bench_ctx
	ucontext_t bench_ctx;
	if (getcontext(&bench_ctx) < 0) {
		perror("getcontext");
		exit(1);
	}
	tcb *temp = (tcb*)malloc(sizeof(tcb));
	temp->id = id;
	temp->context = bench_ctx;
	temp->priority = 1;
	// push bench_ctx onto runqueue
	enqueue(runqueue, temp);
	// set running context to bench_ctx on first call
	running = dequeue(runqueue);
	running->status = RUNNING;

	init_sched_ctx();
	init_timer();

	// - schedule policy
	#ifndef MLFQ
	#else 
		// Choose MLFQ
		for (int i = 0; i < (LEVELS - 1); i++) {
			queues[i] = queue_init();
		}
	#endif

	init = 1;
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
		printf("Thread address = %p\n", walk->block->thread);
		printf("Thread status = %u\n", walk->block->status);
		printf("Thread stack = %p\n", walk->block->threadStack);
		printf("Thread function = %p\n", walk->block->func);
		printf("Thread elapsed time = %u\n", walk->block->elapsed);
		printf("Thread priority = %d\n", walk->block->priority);
		printf("--------------------------\n");
		walk = walk->next;
	}
	printf("--------DONE_PRINTING--------\n");
}

void handler(int signum) {
	if (DEBUG) puts("---DING DING TIMER ---");
	tot_cntx_switches++;
	#ifndef MLFQ
	#else 
		// Choose MLFQ
		interval++;
	#endif
	swapcontext(&running->context, &sched_ctx);
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

/* initializes timer */
void init_timer() {
	if (DEBUG) puts("in timer");
	struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &handler;
	sigaction (SIGPROF, &sa, NULL);

	struct itimerval timer;

	// Set up what the timer should reset to after the timer goes off
	timer.it_interval.tv_usec = QUANTUM; 
	timer.it_interval.tv_sec = 0;

	// Set up the current timer to go off in 1 useconds
	// Note: if both of the following values are zero
	//       the timer will not be active, and the timer
	//       will never go off even if you set the interval value
	timer.it_value.tv_usec = QUANTUM;
	timer.it_value.tv_sec = 0;

	// Set the timer up (start the timer)
	setitimer(ITIMER_PROF, &timer, NULL);
}

/* Initializes mutex list*/
void init_mutexes() {
	mutexes = (mutex_node*)malloc(sizeof(mutex_node));
	mutexes->mutex = NULL;
	mutexes->next = NULL;
}

/* finds if thread is in queue q or not */
int find_wait(worker_t find, queue* q) {
	node* walk = q->front;
		while(walk != NULL) {
			if (DEBUG) printf("WALKING: Thread ID# = %u\n", walk->block->id);
			if (DEBUG) printf("WALKING: Thread ID# = %u\n", find);

			// The thread is still running
			if (walk->block->id == find) {
				if (DEBUG) puts("we found it!!");
				return 1;
			}

			walk = walk->next;
		}
	// Thread "find" was not found in the given queue
	return 0;
}

/* Finds if thread is currently blocked and in mutex waitlist*/
int find_mutex_wait(worker_t find) {
	mutex_node* walk = mutexes;
	while(walk->mutex != NULL) {
		node* inside_walk = walk->mutex->wait->front;
		while(inside_walk != NULL) {
		// The thread is blocked by mutex
		if (inside_walk->block->id == find) {
			if (DEBUG) puts("(in mutex) we found it!!");
			return 1;
		}

		inside_walk = inside_walk->next;
		}
		walk = mutexes->next;
	}
	// Thread "find" was not found in any mutex wait list.
	return 0;
}

tcb* find_shortest_job(queue* q) {
	if (DEBUG) puts("----------- IN FIND SHORTEST JOB -----------");
	// walk through queue to find min job time
	node* walk = q->front;
	if (walk->next == NULL) {
		return dequeue(q);
	}
	int min = walk->block->elapsed;
	while(walk != NULL) {
		if (DEBUG) printf("WALKING: Thread ID# = %u\n", walk->block->id);
		if (DEBUG) printf("WALKING: Thread TIME ELAPSED = %u\n", walk->block->elapsed);

		// New minimum value is found
		if (walk->block->elapsed < min) {
			if (DEBUG) puts("updating minimum value");
			min = walk->block->elapsed;
		}
		walk = walk->next;
	}
	walk = q->front;
	node* walk_next = walk->next;

	// if head of list is min
	if (walk->block->elapsed == min) {
		return dequeue(q);
	}

	// finds first thread that is equal to min time value
	while(walk != NULL && walk_next != NULL) {
		if (DEBUG) printf("WALKING: Thread ID# = %u\n", walk->block->id);
		if (DEBUG) printf("WALKING: Thread TIME ELAPSED = %u\n", walk->block->elapsed);

		// Returns tcb with minimum time value
		if (walk_next->block->elapsed == min) {
			if (DEBUG) puts("updating minimum value");

			// end of list
			if (walk_next->next == NULL) {
				walk->next = NULL;
			// middle of list
			} else {
				walk->next = walk_next->next;
			}

			// free and return tcb for thread
			tcb *temp = walk_next->block;
			// clear memory for dequeued node
			free(walk_next);
			if (DEBUG) printf("SHORTEST JOB: Thread ID# = %u\n", temp->id);
			return temp;
		}
		walk = walk->next;
		walk_next = walk_next->next;
	}
	// if find_shortest_job fails, return NULL
	return NULL;
}