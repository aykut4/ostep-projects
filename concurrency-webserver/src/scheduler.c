#include "scheduler.h"

// Function to initiate queue buffer inside scheduler
// Parameter n refers maximum size of buffer
queue* init_queue(int n) {
    queue* q = (queue*)malloc(sizeof(queue));
    if (q == NULL) {
        fprintf(stderr, "ERROR: Memory allocation queue buffer.\n");
        return NULL;
    }

    q->max_size = n;
    q->cur_size = 0;

    q->works = (work*)malloc(sizeof(work) * n);

    q->front = 0;
    q->rear = 0;
    
    return q;
}

int check_queue_full(queue* q) {
    return q->cur_size >= q->max_size;
}

int check_queue_empty(queue* q) {
    return q->cur_size == 0;
}

// Enqueue work for thread
void enqueue_work_fifo(queue* q, int fd) {

    assert(!check_queue_full(q));

    q->works[q->rear++].fd = fd;
    q->rear = q->rear % q->max_size;
    q->cur_size++;
}

// Dequeue work for thread
int dequeue_work_fifo(queue* q) {

    assert(!check_queue_empty(q));

    int work_fd = q->works[q->front++].fd;
    q->front = q->front % q->max_size;
    q->cur_size--;
    
    return work_fd;
}


// Main scheduler functions
scheduler* init_scheduler(int alg, int n) {

    scheduler* sch = (scheduler*)malloc(sizeof(scheduler));
    if (sch == NULL) {
        fprintf(stderr, "ERROR: Memory allocation for scheduler.\n");
        return NULL;
    }

    sch->schedalg = alg;
    sch->buf = init_queue(n);

    return sch;
}

int check_scheduler_full(scheduler* sch) {
    return check_queue_full(sch->buf);
} 

int check_scheduler_empty(scheduler* sch) {
    return check_queue_empty(sch->buf);
}

void insert_scheduler_work(scheduler* sch, thread_pool* pool, int fd) {
    pthread_mutex_lock(&pool->LOCK);

    while(check_scheduler_full(sch)) {
        pthread_cond_wait(&pool->FILL, &pool->LOCK);
    }

    if (sch->schedalg) {
        enqueue_work_fifo(sch->buf, fd);
    }

    pthread_cond_signal(&pool->EMPTY);
    pthread_mutex_unlock(&pool->LOCK);
}

int get_scheduler_work(scheduler* sch, thread_pool* pool) {
    pthread_mutex_lock(&pool->LOCK);

    while(check_scheduler_empty(sch)) {
        pthread_cond_wait(&pool->EMPTY, &pool->LOCK);
    }

    int conn_fd = -1;

    if (sch->schedalg) {
        conn_fd = dequeue_work_fifo(sch->buf);
    }

    pthread_cond_signal(&pool->FILL);
    pthread_mutex_unlock(&pool->LOCK);
    
    return conn_fd;
}