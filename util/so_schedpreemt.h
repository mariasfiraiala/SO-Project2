#ifndef SO_SCHEDPREEMT_H_
#define SO_SCHEDPREEMT_H_

#include "utils.h"

static so_thread_t *new_thread(so_handler *handler, uint32_t prio);

static void new_thread_in_queue(so_thread_t *thread);

static void update_sched(void);

static void *thread_routine(void *thread);

#endif