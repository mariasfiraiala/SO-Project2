#include <stdlib.h>
#include <pthread.h>

#include "util/utils.h"
#include "util/so_schedpreemt.h"
#include "util/so_scheduler.h"

extern so_scheduler_t so_schedpreemt;

static so_thread_t *new_thread(so_handler *handler, uint32_t prio)
{
    so_thread_t *new_thread = malloc(sizeof(*new_thread));
    DIE(!new_thread, "malloc() failed");

    new_thread->tid = INVALID_TID;
    new_thread->io_event = SO_MAX_NUM_EVENTS;
    new_thread->priority = prio;
    new_thread->state = NEW;
    new_thread->remaining_time = so_schedpreemt.time_slice;
    new_thread->handler = handler;

    int rc = pthread_create(&new_thread->tid, NULL, &thread_routine, (void *)new_thread);
    DIE(rc, "pthread_create() failed");

    return new_thread;
}

static void *thread_routine(void *thread)
{
    so_thread_t *new_thread = (so_thread_t *)thread;

    new_thread->handler(new_thread->priority);
    new_thread->state = TERMINATED;

    update_sched();
}

void update_sched(void)
{
    
}