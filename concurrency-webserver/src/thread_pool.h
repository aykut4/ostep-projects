#ifndef __THREAD_POOL_H_
#define __THREAD_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "request.h"
#include "io_helper.h"

// Thread pool struct
typedef struct __thread_pool {
    // Number of total workers
    int num_thd;
    // Worker threads
    pthread_t* workers;
    // Mutex lock for scheduler
    pthread_mutex_t LOCK;
    // Condition variable for scheduler
    pthread_cond_t EMPTY;
    pthread_cond_t FILL;

} thread_pool;

#include "scheduler.h"

// Thread argument
typedef struct __thread_arg {
    // Thread pool
    thread_pool* pool;
    // Scheduler
    scheduler* sch;
} thread_arg;

thread_pool* init_thread_pool(int n);
void* thread_function(void *_arg);
void start_thread_work(thread_pool* pool, scheduler* sch);

#endif // __THREAD_POOL_H_