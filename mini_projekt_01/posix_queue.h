#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct p_queue_node {
    void *data;
    struct p_queue_node *next;
} p_queue_node_t;

typedef struct posix_queue {
    p_queue_node_t *head;
    p_queue_node_t *tail;
    pthread_mutex_t mutex;
} posix_queue_t;

int posix_queue_init(posix_queue_t *queue);
int posix_queue_enqueue(posix_queue_t *queue, void *data);
int posix_queue_dequeue(posix_queue_t *queue, void **data);
void posix_queue_destroy(posix_queue_t *queue);
bool posix_queue_is_empty(posix_queue_t *queue);