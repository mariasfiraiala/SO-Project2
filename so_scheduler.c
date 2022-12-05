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

    sem_init(&schedpreemt.running_sched, 0, 1);

    return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
    if (!func || priority > SO_MAX_PRIO)
        return INVALID_TID;

    if (!schedpreemt.threads) {
        sem_wait(&schedpreemt.running_sched);
    }

    so_thread_t *new = new_thread(func, priority);
    new_thread_in_all_threads(new);
    new_thread_in_queue(new);

    if (schedpreemt.current_thread)
        so_exec();
    else
        update_sched();

    return new->tid;
}

int so_wait(unsigned int io)
{
    if (io < 0 || io >= schedpreemt.events)
        return -1;

    schedpreemt.current_thread->state = BLOCKED;
    schedpreemt.current_thread->io_event = io;

    so_exec();

    return 0;
}

int so_signal(unsigned int io)
{
    if (io < 0 || io >= schedpreemt.events)
        return -1;

    int woken_up = 0;
    for (int i = 0; i < schedpreemt.threads; ++i)
        if (schedpreemt.all_threads[i]->io_event == io && schedpreemt.all_threads[i]->state == BLOCKED) {
            ++woken_up;
            schedpreemt.all_threads[i]->state = READY;
            schedpreemt.all_threads[i]->io_event = SO_MAX_NUM_EVENTS;
            new_thread_in_queue(schedpreemt.all_threads[i]);
        }

    so_exec();
    return woken_up;
}

void so_exec(void)
{
    --(schedpreemt.current_thread->remaining_time);
    update_sched();
    sem_wait(&schedpreemt.current_thread->running_thread);
}

void so_end(void)
{
    if (!schedpreemt.init)
        return;

    sem_wait(&schedpreemt.running_sched);

    for (int i = 0; i < schedpreemt.threads; ++i) {
        pthread_join(schedpreemt.all_threads[i]->tid, NULL);
    }

    for (int i = 0; i < schedpreemt.threads; ++i) {
        sem_destroy(&schedpreemt.all_threads[i]->running_thread);
        free(schedpreemt.all_threads[i]);
    }

    schedpreemt.init = 0;
    sem_destroy(&schedpreemt.running_sched);
    free(schedpreemt.all_threads);
    free(schedpreemt.priority_queue);
}