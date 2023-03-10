#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <ucontext.h>
#include <sys/time.h>
#include <string.h>

#define STACK_SIZE SIGSTKSZ

// Declare contex globally
static ucontext_t ctx1, ctx2;
static int count = 0;

void handler(int signum) {
    puts("SWITCHING");
    count++;
    if (count % 2 == 1) {
        if (swapcontext(&ctx1, &ctx2) == -1) {
            perror("swapcontext");
        }
    } else {
        if (swapcontext(&ctx2, &ctx1) == -1) {
            perror("swapcontext");
        }
    }
}

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

    ucontext_t ctx_main;

    if (getcontext(&ctx1) < 0) {
        perror("getcontext");
        exit(1);
    }
    if (getcontext(&ctx2) < 0) {
        perror("getcontext");
        exit(1);
    }

    // Allocate stack space
    void *stack1 = malloc(STACK_SIZE);
    void *stack2 = malloc(STACK_SIZE);

    if (stack1 == NULL || stack2 == NULL) {
        perror("Failed to allocate stack");
        exit(1);
    }

    // Set up context
    // ctx1 to foo() and ctx2 to bar()
    ctx1.uc_link = &ctx_main;
    ctx1.uc_stack.ss_sp = stack1;
    ctx1.uc_stack.ss_size = STACK_SIZE;
    ctx1.uc_stack.ss_flags = 0;

    makecontext(&ctx1, (void *)&foo, 0);

    ctx2.uc_link = &ctx1;
    ctx2.uc_stack.ss_sp = stack2;
    ctx2.uc_stack.ss_size = STACK_SIZE;
    ctx2.uc_stack.ss_flags = 0;

    makecontext(&ctx2, (void *)&bar, 0);

    //swapcontext(&ctx_main, &ctx1);
    
    // Use sigaction to register signal handler
	struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &handler;
	sigaction (SIGPROF, &sa, NULL);

	// Create timer struct
	struct itimerval timer;

	// Set up what the timer should reset to after the timer goes off
	timer.it_interval.tv_usec = 1; 
	timer.it_interval.tv_sec = 0;

	// Set up the current timer to go off in 1 second
	// Note: if both of the following values are zero
	//       the timer will not be active, and the timer
	//       will never go off even if you set the interval value
	timer.it_value.tv_usec = 1;
	timer.it_value.tv_sec = 0;

	// Set the timer up (start the timer)
	setitimer(ITIMER_PROF, &timer, NULL);

    swapcontext(&ctx_main, &ctx1);

    return 0;
}
