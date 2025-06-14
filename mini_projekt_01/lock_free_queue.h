#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdint.h>

// lock free with hazard pointers
#define ACTIVE 1
#define INACTIVE 0
#define MAX_THREADS 32

typedef _Atomic(void*) atomic_void_ptr;

typedef struct lf_node {
    void *data;
    _Atomic(struct lf_node *) next;
} lf_node_t;

typedef struct {
     _Atomic(lf_node_t*) head;
     _Atomic(lf_node_t*) tail;
} lf_queue_t;

typedef struct hazard_record {
    struct hazard_record *next; 
    atomic_int active;
    atomic_void_ptr hazard_pointer;
} hazard_record_t;


hazard_record_t* acquire(void);
void release(hazard_record_t *record);
void retire(void *ptr);
void scan(void);

void lf_queue_init(lf_queue_t *queue);
void lf_enqueue(lf_queue_t *queue, void *data);
bool lf_dequeue(lf_queue_t *queue, void **data);
void lf_destroy(lf_queue_t *queue);
bool lf_is_empty(lf_queue_t *queue);