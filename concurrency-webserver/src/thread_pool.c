#include "thread_pool.h"

// Initialize thread pool with workers number
thread_pool* init_thread_pool(int n) {
    thread_pool* pool = (thread_pool*)malloc(sizeof(thread_pool));
    if (pool == NULL) {
        fprintf(stderr, "ERROR: Memory allocation for thread pool.\n");
        return NULL;
    }

    pool->num_thd = n;
    pool->workers = malloc(n * sizeof(pthread_t));

    pthread_mutex_init(&pool->LOCK, NULL);
    pthread_cond_init(&pool->EMPTY, NULL);
    pthread_cond_init(&pool->FILL, NULL);

    return pool;
}

// Thread function for workers
void* thread_function(void *_arg) {
    thread_arg* arg = (thread_arg*)_arg;

    while (1) {
        int conn_fd = get_scheduler_work(arg->sch, arg->pool);
        request_handle(conn_fd);
	    close_or_die(conn_fd);
    }

    return NULL;
}

// Get to work
void start_thread_work(thread_pool* pool, scheduler* sch) {
    
    for (int i = 0; i < pool->num_thd; i++) {
        // Thread argument initalization
        thread_arg* arg = (thread_arg*)malloc(sizeof(thread_arg));
        if (arg == NULL) {
            fprintf(stderr, "ERROR: Memory allocation for thread arg %d.\n", i);
            exit(1);
        }
        arg->pool = pool;
        arg->sch = sch;

        int err = pthread_create(&pool->workers[i], NULL, thread_function, arg);
        if (err != 0) {
            fprintf(stderr, "ERROR: Start thread work.\n");
            exit(1);
        }
    }
}
