#include "thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

#define THREAD_STACK_SIZE (8 * 1024 * 1024)

static Thread			*thread_list = NULL;
static Thread			*current_thread = NULL;
static Thread			*main_thread = NULL;
static volatile bool	preempt = true;
static struct itimerval	timer;

static Thread *high_priority_head = NULL;
static Thread *low_priority_head = NULL;
static Thread *current_thread = NULL;

static void	timer_handler(int sig);
static void	thread_init_helper(ThreadFunc func, void* arg);
static void	schedule(void);

Thread	**thread_get_list(void)
{
	return (&thread_list);
}

Thread	**thread_get_current(void)
{
	return (&current_thread);
}

void	thread_set_current(Thread *thread)
{
	current_thread = thread;
}

static void	timer_handler(int sig)
{
	if (preempt)
	{
		thread_yield();
	}
}

static void	thread_init_helper(ThreadFunc func, void* arg)
{
	// TODO: Call the thread function with the provided argument and store the return value.
	// TODO: Mark the current thread as finished after the function completes.
	// TODO: Exit the thread and pass the return value to the thread_exit function.
	void *retval = func(arg);
	current_thread->finished = true;
	thread_exit(retval);
}

static void	schedule(void)
{

    Thread *start = current_thread;
    Thread **queue = (current_thread->priority == HIGH_PRIORITY) ? &high_priority_head : &low_priority_head;

    // First try high-priority threads
    if (high_priority_head && !high_priority_head->finished) {
        do {
            current_thread = current_thread->next;
            if (!current_thread->finished)
                break;
        } while (current_thread != start);
        swapcontext(&start->context, &current_thread->context);
        return;
    }

    // Then try low-priority threads
    if (low_priority_head && !low_priority_head->finished) {
        if (current_thread->priority == HIGH_PRIORITY)
            current_thread = low_priority_head;

        start = current_thread;
        do {
            current_thread = current_thread->next;
            if (!current_thread->finished)
                break;
        } while (current_thread != start);
        swapcontext(&start->context, &current_thread->context);
    }
}

void	thread_schedule(void)
{
	schedule();
}

int	thread_create(Thread **thread, ThreadFunc func, void* arg)
{
	// TODO: Initialize threading system if needed.
	// TODO: Set up thread context and link to main thread.
	// TODO: Mark thread as active and configure context for execution.
		// Use thread_init_helper
	// TODO: Add thread to circular list.
	// TODO: Update list pointers and return the new thread.

	if (!main_thread)
		thread_init();  // First call to thread_create

	Thread *t = malloc(sizeof(Thread));
	if (!t)
		return -1;

	t->stack = malloc(THREAD_STACK_SIZE);
	if (!t->stack) {
		free(t);
		return -1;
	}

	getcontext(&t->context);
	t->context.uc_stack.ss_sp = t->stack;
	t->context.uc_stack.ss_size = THREAD_STACK_SIZE;
	t->context.uc_link = NULL;
	t->finished = false;
	t->next = NULL;

	makecontext(&t->context, (void (*)())thread_init_helper, 2, func, arg);

	// Add to circular thread list
	if (!thread_list) {
		thread_list = t;
		t->next = t;
	} else {
		Thread *last = thread_list;
		while (last->next != thread_list)
			last = last->next;
		last->next = t;
		t->next = thread_list;
	}

	    // Determine priority from arg (assumes char* player name)
    char *name = (char *)arg;
    if (strcmp(name, "Player2") == 0 || strcmp(name, "Player4") == 0)
        t->priority = HIGH_PRIORITY;
    else
        t->priority = LOW_PRIORITY;

    // Insert into appropriate queue
    Thread **head = (t->priority == HIGH_PRIORITY) ? &high_priority_head : &low_priority_head;
    if (*head == NULL) {
        *head = t;
        t->next = t;
    } else {
        Thread *temp = *head;
        while (temp->next != *head)
            temp = temp->next;
        temp->next = t;
        t->next = *head;
    }


	if (new_thread)
		*new_thread = t;

	return 0;


}

void thread_yield(void)
{
	// Check if preemption is enabled.
	// Call the scheduler to switch to the next thread.
    schedule();
}

void thread_exit(void *retval)
{
	(void)retval;
	current_thread->finished = true;
	schedule();
}

Thread	*thread_self(void)
{
	return (current_thread);
}

void	thread_join(Thread *thread)
{
	// TODO: Wait for the given thread to finish execution.
	// TODO: Traverse the thread list to find the thread to be removed.
	// TODO: Remove the thread from the list and update the list pointers accordingly.

    while (!target->finished)
        thread_yield();

    // Remove from queue
    Thread **head = (target->priority == HIGH_PRIORITY) ? &high_priority_head : &low_priority_head;
    Thread *prev = *head;

    if (prev == target && prev->next == target) {
        *head = NULL;
    } else {
        while (prev->next != target)
            prev = prev->next;
        prev->next = target->next;
        if (*head == target)
            *head = target->next;
    }

    free(target->stack);
    free(target);
}

void thread_init(void)
{
	// TODO: Set up the main thread's context and mark it as not finished.
	// TODO: Link the main thread to itself and set it as the current thread.
	// TODO: Configure the signal handler for the timer to handle preemption.
	// TODO: Set up the timer to trigger periodically for thread scheduling.


	// Setup main thread context
	main_thread = malloc(sizeof(Thread));
	if (!main_thread)
		exit(1);

	getcontext(&main_thread->context);
	main_thread->stack = NULL;
	main_thread->finished = false;
	main_thread->next = main_thread;

	current_thread = main_thread;
	thread_list = main_thread;

	// Setup signal handler for preemption
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = timer_handler;
	sigaction(SIGVTALRM, &sa, NULL);

	// Setup timer for 100Hz (10ms)
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 10000;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 10000;

	setitimer(ITIMER_VIRTUAL, &timer, NULL);
	
}

void preempt_enable(void)
{
	preempt = true;
}

void preempt_disable(void)
{
	preempt = false;
}
