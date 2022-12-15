#ifndef SO_SCHEDPREEMT_H_
#define SO_SCHEDPREEMT_H_

#include "utils.h"

so_thread_t *so_new_thread(so_handler *handler, uint32_t prio);

void so_new_thread_in_queue(so_thread_t *thread);

void so_new_thread_in_all_threads(so_thread_t *thread);

void so_schedule(void);

void *so_thread_routine(void *thread);

void so_run_thread(so_thread_t *thread);

so_thread_t *so_peek(void);

#endif
