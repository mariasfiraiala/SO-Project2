// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#include "util/so_scheduler.h"
#include "util/so_schedpreemt.h"
#include "util/utils.h"

so_scheduler_t schedpreemt;

int so_init(unsigned int time_quantum, unsigned int io)
{
	if (schedpreemt.init == 1 || io > SO_MAX_NUM_EVENTS || time_quantum <= 0)
		return -1;

	schedpreemt.init = 1;
	schedpreemt.time_slice = time_quantum;
	schedpreemt.events = io;
	schedpreemt.threads = 0;
	schedpreemt.queue_size = 0;
	schedpreemt.capacity_threads = DEFAULT_SIZE;
	schedpreemt.capacity_queue = DEFAULT_SIZE;
	schedpreemt.current_thread = NULL;
	schedpreemt.all_threads = malloc(DEFAULT_SIZE * sizeof(so_thread_t *));
	DIE(!schedpreemt.all_threads, "malloc() failed");
	schedpreemt.priority_queue = malloc(DEFAULT_SIZE * sizeof(so_thread_t *));
	DIE(!schedpreemt.priority_queue, "malloc() failed");

	/*
	 * create semaphore for scheduler and let only one thread to use it
	 * sem_init(..., 0, 1) = sem_init(..., 0, 0)
	 *						 sem_post(...)
	 */
	int32_t rc = sem_init(&schedpreemt.running_sched, 0, 1);

	DIE(rc, "sem_init() failed");

	return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	if (!func || priority > SO_MAX_PRIO)
		return INVALID_TID;

	/*
	 * stop the sched if there is no thread in our "system"
	 */
	if (!schedpreemt.threads) {
		int32_t rc = sem_wait(&schedpreemt.running_sched);

		DIE(rc, "sem_wait() failed");
	}

	/*
	 * create and initialize new thread
	 */
	so_thread_t *new = new_thread(func, priority);

	/*
	 * add the newly created thread to our internal kitchen and in the prio
	 * queue as well
	 */
	new_thread_in_all_threads(new);
	new_thread_in_queue(new);

	/*
	 * if there is no running thread, we'll update the scheduler using
	 * round robin, otherwise we'll simulate its execution
	 */
	if (!schedpreemt.current_thread)
		update_sched();
	else
		so_exec();

	return new->tid;
}

int so_wait(unsigned int io)
{
	if (io < 0 || io >= schedpreemt.events)
		return -1;

	/*
	 * block the thread that wants to interract with an io device
	 */
	schedpreemt.current_thread->state = BLOCKED;
	schedpreemt.current_thread->io_event = io;

	so_exec();

	return 0;
}

int so_signal(unsigned int io)
{
	if (io < 0 || io >= schedpreemt.events)
		return -1;

	uint32_t woken_up = 0;

	/*
	 * go through all of our threads and wake up the ones that wait for a
	 * certain event
	 */
	for (int i = 0; i < schedpreemt.threads; ++i)
		if (schedpreemt.all_threads[i]->io_event == io) {
			++woken_up;
			schedpreemt.all_threads[i]->state = READY;
			schedpreemt.all_threads[i]->io_event = INVALID_IO;
			new_thread_in_queue(schedpreemt.all_threads[i]);
		}

	so_exec();
	return woken_up;
}

void so_exec(void)
{
	/*
	 * simulate doing work then block the thread from running, as we reach the
	 * end of our simulation
	 */
	so_thread_t *tmp = schedpreemt.current_thread;
	--(schedpreemt.current_thread->remaining_time);
	update_sched();

	int32_t rc = sem_wait(&tmp->running_thread);

	DIE(rc, "sem_wait() failed");
}

void so_end(void)
{
	if (!schedpreemt.init)
		return;

	int32_t rc = sem_wait(&schedpreemt.running_sched);

	DIE(rc, "sem_wait() failed");

	for (uint32_t i = 0; i < schedpreemt.threads; ++i) {
		rc = pthread_join(schedpreemt.all_threads[i]->tid, NULL);
		DIE(rc, "pthread_join() failed");
	}

	for (uint32_t i = 0; i < schedpreemt.threads; ++i) {
		rc = sem_destroy(&schedpreemt.all_threads[i]->running_thread);
		DIE(rc, "sem_destroy() failed");
		free(schedpreemt.all_threads[i]);
	}

	schedpreemt.init = 0;
	free(schedpreemt.all_threads);
	free(schedpreemt.priority_queue);
}
