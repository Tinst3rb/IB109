#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "test.h"

// POSIX wrappers
int posix_enqueue_wrapper(void *queue, void *data) {
    return posix_queue_enqueue((posix_queue_t *)queue, data);
}

int posix_dequeue_wrapper(void *queue, void **data) {
    int result = posix_queue_dequeue((posix_queue_t *)queue, data);
    return (result == EAGAIN) ? 1 : result;
}

int posix_init_wrapper(void *queue) {
    return posix_queue_init((posix_queue_t *)queue);
}

void posix_destroy_wrapper(void *queue) {
    posix_queue_destroy((posix_queue_t *)queue);
}

bool posix_is_empty_wrapper(void *queue) {
    return posix_queue_is_empty((posix_queue_t *)queue);
}

// Lock-free wrappers
int lf_enqueue_wrapper(void *queue, void *data) {
    lf_enqueue((lf_queue_t *)queue, data);
    return 0;
}

int lf_dequeue_wrapper(void *queue, void **data) {
    return lf_dequeue((lf_queue_t *)queue, data) ? 0 : 1;
}

int lf_init_wrapper(void *queue) {
    lf_queue_init((lf_queue_t *)queue);
    return 0;
}

void lf_destroy_wrapper(void *queue) {
    lf_destroy((lf_queue_t *)queue);
}

bool lf_is_empty_wrapper(void *queue) {
    return lf_is_empty((lf_queue_t *)queue);
}

// Test
void *producer_function(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int *data = malloc(sizeof(int));
        if (data == NULL) {
            continue;
        }
        
        *data = (int)pthread_self() + i;
        
        int result = args->enqueue_fn(args->queue, data);
        if (result != 0) {
            free(data);
        }
    }
    return NULL;
}

void *consumer_function(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    int empty_attempts = 0;
    
    while (empty_attempts < 100) {
        void *data;
        
        int result = args->dequeue_fn(args->queue, &data);
        
        if (result == 0) {
            free(data);
            empty_attempts = 0;
        } else {
            args->is_empty_fn(args->queue);
            empty_attempts++;
        }
    }
    
    args->is_empty_fn(args->queue);
    
    return NULL;
}

double get_time_diff(struct timeval start, struct timeval end) {
    return (double)(end.tv_sec - start.tv_sec) + 
           (double)(end.tv_usec - start.tv_usec) / 1000000.0;
}

double test_queue(void *queue, 
                  enqueue_func_t enqueue_fn,
                  dequeue_func_t dequeue_fn,
                  init_func_t init_fn,
                  destroy_func_t destroy_fn,
                  is_empty_func_t is_empty_fn) {
    
    pthread_t producers[PRODUCERS];
    pthread_t consumers[CONSUMERS];
    
    if (init_fn && init_fn(queue) != 0) {
        return -1.0;
    }
    
    if (!is_empty_fn(queue)) {
        return -1.0;
    }
    
    thread_args_t args = {
        .queue = queue,
        .enqueue_fn = enqueue_fn,
        .dequeue_fn = dequeue_fn,
        .is_empty_fn = is_empty_fn 
    };
    
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    for (int i = 0; i < PRODUCERS; i++) {
        if (pthread_create(&producers[i], NULL, producer_function, &args) != 0) {
            return -1.0;
        }
    }
    
    for (int i = 0; i < CONSUMERS; i++) {
        if (pthread_create(&consumers[i], NULL, consumer_function, &args) != 0) {
            return -1.0;
        }
    }
    
    for (int i = 0; i < PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    
    for (int i = 0; i < CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    gettimeofday(&end_time, NULL);
    double elapsed_time = get_time_diff(start_time, end_time);
    
    if (destroy_fn) {
        destroy_fn(queue);
    }
    
    return elapsed_time;
}