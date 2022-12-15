// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#include "util/utils.h"
#include "util/so_schedpreemt.h"
#include "util/so_scheduler.h"

extern so_scheduler_t schedpreemt;

so_thread_t *so_new_thread(so_handler *handler, uint32_t prio)
{
	so_thread_t *new_thread = malloc(sizeof(*new_thread));

	DIE(!new_thread, "malloc() failed");

	/*
	 * insert default values for the new thread
	 */
	new_thread->tid = INVALID_TID;
	new_thread->io_event = INVALID_IO;
	new_thread->priority = prio;
	new_thread->remaining_time = schedpreemt.time_slice;
	new_thread->handler = handler;

	int32_t rc = sem_init(&new_thread->running_thread, 0, 0);

	DIE(rc, "sem_init() failed");

	rc = pthread_create(&new_thread->tid, NULL, &so_thread_routine,
						(void *)new_thread);

	DIE(rc, "pthread_create() failed");

	return new_thread;
}

void *so_thread_routine(void *thread)
{
	so_thread_t *new_thread = (so_thread_t *)thread;

	int32_t rc = sem_wait(&new_thread->running_thread);

	DIE(rc, "sem_wait() failed");

	/*
	 * let the thread do its job, then mark it as terminated, regardless of
	 * whether it reached its time slice limit
	 */
	new_thread->handler(new_thread->priority);
	new_thread->state = TERMINATED;

	so_schedule();

	return NULL;
}

void so_schedule(void)
{
	/*
	 * when the priority queue is empty, we can pause the scheduler
	 */
	if (!schedpreemt.queue_size) {
		if (schedpreemt.current_thread->state == TERMINATED) {
			int32_t rc = sem_post(&schedpreemt.running_sched);

			DIE(rc, "sem_post() failed");
		}
		int32_t rc = sem_post(&schedpreemt.current_thread->running_thread);

		DIE(rc, "sem_post() failed");

		return;
	}

	/*
	 * when we don't have any thread running, or when the "running" thread
	 * became either TERMINATED or BLOCKED, we start the next thread based
	 * on its prio
	 */
	if (schedpreemt.current_thread == NULL ||
			schedpreemt.current_thread->state == BLOCKED ||
				schedpreemt.current_thread->state == TERMINATED) {
		schedpreemt.current_thread = so_peek();
		so_run_thread(schedpreemt.current_thread);
	} else if (so_peek()->priority > schedpreemt.current_thread->priority) {
		/*
		 * when it suddenly appears a thread with a priority greater that
		 * the one of the running thread, we yeet the running thread and
		 * run the more important thread instead
		 */
		so_new_thread_in_queue(schedpreemt.current_thread);
		schedpreemt.current_thread = so_peek();
		so_run_thread(schedpreemt.current_thread);
	} else if (schedpreemt.current_thread->remaining_time <= 0) {
		/*
		 * when the time slice of the current thread expired we start the
		 * thread with the greatest prio, be that the current thread or the
		 * one from the queue
		 */
		if (so_peek()->priority == schedpreemt.current_thread->priority) {
			so_new_thread_in_queue(schedpreemt.current_thread);
			schedpreemt.current_thread = so_peek();
			so_run_thread(schedpreemt.current_thread);
		} else {
			schedpreemt.current_thread->remaining_time = schedpreemt.time_slice;
			int32_t rc = sem_post(&schedpreemt.current_thread->running_thread);

			DIE(rc, "sem_post() failed");
		}
	} else {
		/*
		 * we release the thread, so it starts running
		 */
		int32_t rc = sem_post(&schedpreemt.current_thread->running_thread);

		DIE(rc, "sem_post() failed");
	}
}

void so_new_thread_in_queue(so_thread_t *thread)
{
	/*
	 * redimension the queue when its size reaches the capacity, by doubling it
	 */
	if (schedpreemt.queue_size >= schedpreemt.capacity_queue) {
		schedpreemt.capacity_queue *= 2;

		so_thread_t **tmp = realloc(schedpreemt.priority_queue,
						schedpreemt.capacity_queue * sizeof(so_thread_t *));

		DIE(!tmp, "realloc() failed");
		schedpreemt.priority_queue = tmp;
	}

	int32_t i = schedpreemt.queue_size - 1;

	/*
	 * while searching for the right spot of the new element to be inserted
	 * shift all current entries
	 */
	while (i >= 0 && thread->priority <=
						schedpreemt.priority_queue[i]->priority) {
		schedpreemt.priority_queue[i + 1] = schedpreemt.priority_queue[i];
		--i;
	}

	thread->state = READY;
	schedpreemt.priority_queue[i + 1] = thread;
	++schedpreemt.queue_size;
}

void so_new_thread_in_all_threads(so_thread_t *thread)
{
	/*
	 * resize the array of threads if needed
	 */
	if (schedpreemt.threads >= schedpreemt.capacity_threads) {
		schedpreemt.capacity_threads *= 2;

		so_thread_t **tmp = realloc(schedpreemt.all_threads,
						schedpreemt.capacity_threads * sizeof(so_thread_t *));

		DIE(!tmp, "realloc() failed");
		schedpreemt.all_threads = tmp;
	}

	/*
	 * add our thread on the last spot in the array, its priority is not
	 * important for this data structure, as we keep it only to free all
	 * threads at the end of the scheduling process
	 */
	schedpreemt.all_threads[schedpreemt.threads++] = thread;
}

void so_run_thread(so_thread_t *thread)
{
	/*
	 * set the given thread as running, give it a time slice, remove it from
	 * the priority queue and let it take control
	 */
	thread->state = RUNNING;
	thread->remaining_time = schedpreemt.time_slice;

	schedpreemt.priority_queue[--schedpreemt.queue_size] = NULL;

	int32_t rc = sem_post(&thread->running_thread);

	DIE(rc, "sem_post() failed");
}

so_thread_t *so_peek(void)
{
	return schedpreemt.priority_queue[schedpreemt.queue_size - 1];
}
