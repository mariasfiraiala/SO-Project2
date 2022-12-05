#include <stdlib.h>
#include <pthread.h>

#include "util/utils.h"
#include "util/so_schedpreemt.h"
#include "util/so_scheduler.h"

extern so_scheduler_t schedpreemt;

static so_thread_t *new_thread(so_handler *handler, uint32_t prio)
{
    so_thread_t *new_thread = malloc(sizeof(*new_thread));
    DIE(!new_thread, "malloc() failed");

    new_thread->tid = INVALID_TID;
    new_thread->io_event = SO_MAX_NUM_EVENTS;
    new_thread->priority = prio;
    new_thread->state = NEW;
    new_thread->remaining_time = schedpreemt.time_slice;
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

    return NULL;
}

void update_sched(void)
{
    // TODO stop sched

    // when we don't have any thread running, or when the "running" thread became either TERMINATED or BLOCKED, we start the next thread based on its prio
    if (schedpreemt.current_thread == NULL || schedpreemt.current_thread->state == BLOCKED || schedpreemt.current_thread->state == TERMINATED) {
        schedpreemt.current_thread = schedpreemt.priority_queue[schedpreemt.queue_size - 1];
        run(schedpreemt.current_thread);
    } else if (schedpreemt.priority_queue[schedpreemt.queue_size - 1]->priority > schedpreemt.current_thread->priority){
        new_thread_in_queue(schedpreemt.current_thread);
        schedpreemt.current_thread = schedpreemt.priority_queue[schedpreemt.queue_size - 1];
        run(schedpreemt.current_thread);
    } else if (schedpreemt.current_thread->remaining_time <= 0) {
        // when the time slice of the current thread expired we start the thread with the greatest prio, be that the current thread or the one from the queue
        if (schedpreemt.priority_queue[schedpreemt.queue_size - 1]->priority >= schedpreemt.current_thread->priority) {
            new_thread_in_queue(schedpreemt.current_thread);
            schedpreemt.current_thread = schedpreemt.priority_queue[schedpreemt.queue_size - 1];
            run(schedpreemt.current_thread);
        } else {
            schedpreemt.current_thread->remaining_time = schedpreemt.time_slice;
        }
    }
}

static void new_thread_in_queue(so_thread_t *thread)
{
    
}