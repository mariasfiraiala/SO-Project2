#ifndef UTILS_H_
#define UTILS_H_

/* useful macro for handling error codes */
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while(0)

typedef struct {
    tid_t tid;
    uint32_t io_event;
    uint32_t priority;
    uint32_t state;
    uint32_t remaining_time;
    so_handler *handler;

} so_thread_t

typedef struct {
    uint8_t init;
    uint32_t time_slice;
    uint32_t events;
    uint32_t threads;
    uint32_t queue_size;
    so_thread_t *current_thread;
    so_thread_t **all_threads;
    so_thread_t **priority_queue;

} so_scheduler_t

#endif
