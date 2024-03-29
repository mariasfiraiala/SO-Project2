#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <semaphore.h>
#include <inttypes.h>
#include <errno.h>
#include "so_scheduler.h"

/* useful macro for handling error codes */
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while (0)

#define READY        1
#define RUNNING      2
#define BLOCKED      4
#define TERMINATED   8
#define DEFAULT_SIZE 16
#define INVALID_IO   -1

typedef struct {
	tid_t tid;
	uint32_t io_event;
	uint32_t priority;
	uint32_t state;
	uint32_t remaining_time;
	so_handler *handler;

	sem_t running_thread;
} so_thread_t;

typedef struct {
	uint8_t init;
	uint32_t time_slice;
	int32_t events;
	uint32_t threads;
	uint32_t queue_size;
	uint32_t capacity_threads;
	uint32_t capacity_queue;
	so_thread_t *current_thread;
	so_thread_t **all_threads;
	so_thread_t **priority_queue;

	sem_t running_sched;
} so_scheduler_t;

#endif
