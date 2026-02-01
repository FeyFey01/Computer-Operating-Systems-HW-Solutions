#include "semaphore.h"
#include "thread.h"

void	sem_init(Semaphore* sem, int value)
{
	// TODO: Initialize the semaphore with the given value.
	sem->count = value;
	queue_init(&sem->waiting_queue);
}

void	sem_wait(Semaphore* sem)
{
	// TODO: Implement the semaphore wait (P) operation. 
	__disable_irq();  // Atomic section (optional depending on your threading model)

	sem->count--;
	if (sem->count < 0) {
		// No resources available; block current thread
		Thread *current = thread_current();
		queue_enqueue(&sem->waiting_queue, current);
		thread_block();  // Thread will block here
	}

	__enable_irq();  // Exit atomic section
}

void	sem_post(Semaphore* sem)
{
	// TODO: Implement the semaphore signal (V) operation.
	__disable_irq();  // Atomic section

	sem->count++;
	if (sem->count <= 0) {
		// There are threads waiting, unblock one
		Thread *next = (Thread *)queue_dequeue(&sem->waiting_queue);
		if (next) {
			thread_unblock(next);
		}
	}

	__enable_irq();  // Exit atomic section
}

void	sem_destroy(Semaphore* sem)
{
	// TODO: Implement the semaphore destroy operation.
	queue_destroy(&sem->waiting_queue);
}
