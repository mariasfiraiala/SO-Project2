Copyright 2022 Maria Sfiraiala (maria.sfiraiala@stud.acs.upb.ro)

# Preemptive Scheduler - Project2

## Descripiton

The project aims to simulate a naive preemptive scheduler that would apply the Round Robin algorithm and manage a one core processor, ergo, besides our scheduler's thread, only one other thread should be running at a moment of time.
Implementation follows the sequence of events:

1. `so_init`: initialize the scheduler with default values, prepare the priority queue and the array of threads that would mantain them until the final step

2. `so_fork`: fork threads, get them in the queue and in the internal array of threads

3. `so_wait`: block the thread that starts interacting with an io event

4. `so_signal`: wake up all threads waiting for the respons of an io device

5. `so_exec`: simulate running the thread, each call mirrors instructions made during a clock interval

6. `so_end`: join all threads (from our internal thread array) and free them

## Observations Regarding the Project

Interesting project however made me feel like the stupidest person ever; got me questioning my reasoning (for choosing UPB) and my knowledge regarding scheduling.

I guess it did its job right, as I definetely understand threads and synchronization a lot more.
