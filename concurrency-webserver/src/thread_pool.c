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
    // mutex lock for scheduler
    pthread_mutex_t MUTEX;

} thread_pool;

// Thread argument
typedef struct __thread_arg {
    // Index for debugging
    int index;
    // Listen fd
    int fd;
} thread_arg;

// Initialize thread pool with workers number
thread_pool* init_thread_pool(int n) {
    thread_pool* pool = (thread_pool*)malloc(sizeof(thread_pool));
    if (pool == NULL) {
        fprintf(stderr, "ERROR: Memory allocation for thread pool.\n");
        return NULL;
    }

    pool->num_thd = n;
    pool->workers = malloc(n * sizeof(pthread_t));

    pthread_mutex_init(&pool->MUTEX, NULL);

    return pool;
}

// Thread function for workers
void* thread_function(void *_arg) {
    thread_arg* arg = (thread_arg*)_arg;

    while (1) {
	    struct sockaddr_in client_addr;
	    int client_len = sizeof(client_addr);
	    int conn_fd = accept_or_die(arg->fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
        request_handle(conn_fd);
	    close_or_die(conn_fd);
    }

    return NULL;
}

// Get to work
void start_thread_work(thread_pool* pool, int listen_fd) {
    
    for (int i = 0; i < pool->num_thd; i++) {
        // Thread argument initalization
        thread_arg* arg = (thread_arg*)malloc(sizeof(thread_arg));
        if (arg == NULL) {
            fprintf(stderr, "ERROR: Memory allocation for thread arg %d.\n", i);
            exit(1);
        }
        arg->index = i;
        arg->fd = listen_fd;

        int err = pthread_create(&pool->workers[i], NULL, thread_function, arg);
        if (err != 0) {
            fprintf(stderr, "ERROR: Start thread work.\n");
            exit(1);
        }
    }
}
