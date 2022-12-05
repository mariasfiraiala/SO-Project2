#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "util/so_scheduler.h"
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
    schedpreemt.capacity = DEFAULT_SIZE;
    schedpreemt.current_thread = NULL;
    schedpreemt.all_threads = malloc(DEFAULT_SIZE * sizeof(*schedpreemt.all_threads));
    DIE(!schedpreemt.all_threads, "malloc() failed");
    schedpreemt.priority_queue = malloc(DEFAULT_SIZE * sizeof(*schedpreemt.priority_queue));
    DIE(!schedpreemt.priority_queue, "malloc() failed");

    return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{

}

int so_wait(unsigned int io)
{

}

int so_signal(unsigned int io)
{

}

void so_exec(void)
{

}

void so_end(void)
{

}