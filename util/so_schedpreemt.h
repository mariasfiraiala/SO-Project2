#ifndef SO_SCHEDPREEMT_H_
#define SO_SCHEDPREEMT_H_

#include "utils.h"

so_thread_t *new_thread(so_handler *handler, uint32_t prio);

void new_thread_in_queue(so_thread_t *thread);

void new_thread_in_all_threads(so_thread_t *thread);

void update_sched(void);

void *thread_routine(void *thread);

void run(so_thread_t *thread);

so_thread_t *peek(void);

#endif
