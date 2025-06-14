#ifndef TEST_H
#define TEST_H

#include <sys/time.h>
#include <stdbool.h>
#include "posix_queue.h"
#include "lock_free_queue.h"

#define PRODUCERS 4
#define CONSUMERS 2
#define ITEMS_PER_PRODUCER 1000

typedef int (*enqueue_func_t)(void *queue, void *data);
typedef int (*dequeue_func_t)(void *queue, void **data);
typedef int (*init_func_t)(void *queue);
typedef void (*destroy_func_t)(void *queue);
typedef bool (*is_empty_func_t)(void *queue);

typedef struct {
    void *queue;
    enqueue_func_t enqueue_fn;
    dequeue_func_t dequeue_fn;
    is_empty_func_t is_empty_fn;
} thread_args_t;

double test_queue(void *queue, 
                  enqueue_func_t enqueue_fn,
                  dequeue_func_t dequeue_fn,
                  init_func_t init_fn,
                  destroy_func_t destroy_fn,
                  is_empty_func_t is_empty_fn);

int posix_enqueue_wrapper(void *queue, void *data);
int posix_dequeue_wrapper(void *queue, void **data);
int posix_init_wrapper(void *queue);
void posix_destroy_wrapper(void *queue);
bool posix_is_empty_wrapper(void *queue);

int lf_enqueue_wrapper(void *queue, void *data);
int lf_dequeue_wrapper(void *queue, void **data);
int lf_init_wrapper(void *queue);
void lf_destroy_wrapper(void *queue);
bool lf_is_empty_wrapper(void *queue);

#endif