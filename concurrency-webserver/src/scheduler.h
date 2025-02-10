#ifndef __SCHEDULER_H_
#define __SCHEDULER_H_

// Work for threads
typedef struct __work {
    int fd;
} work;

// FIFO scheduler
typedef struct __queue {
    // Maximum size of buffer
    int max_size;
    // Current size of buffer
    int cur_size;
    // Array of works
    work* works;
    // Position of queue
    int front;
    int rear;
} queue;

// Main scheduler
typedef struct __scheduler {
    // Scheduler algorithm FIFO 1, SFF 0
    int schedalg;

    // Buffer queue
    queue* buf;

} scheduler;

#include "thread_pool.h"

queue* init_queue(int n);
void enqueue_work_fifo(queue* fifo, int fd);
int dequeue_work_fifo(queue* fifo);
scheduler* init_scheduler(int alg, int n);
void insert_scheduler_work(scheduler* sch, thread_pool* pool, int fd);
int get_scheduler_work(scheduler* sch, thread_pool* pool);

#endif //__SCHEDULER_H_