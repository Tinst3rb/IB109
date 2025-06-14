#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "posix_queue.h"


int posix_queue_init(posix_queue_t *queue) {
    if (queue == NULL) {
        return EINVAL;
    }
    
    int result = pthread_mutex_init(&queue->mutex, NULL);
    if (result != 0) {
        return result;
    }
    
    queue->head = NULL;
    queue->tail = NULL;
    
    return 0;
}

int posix_queue_enqueue(posix_queue_t *queue, void *data) {
    if (queue == NULL) {
        return EINVAL;
    }
    
    p_queue_node_t *new_node = malloc(sizeof(p_queue_node_t));
    if (new_node == NULL) {
        return ENOMEM;
    }
    
    new_node->data = data;
    new_node->next = NULL;
    
    pthread_mutex_lock(&queue->mutex);
    
    if (queue->tail == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
    
    pthread_mutex_unlock(&queue->mutex);
    
    return 0;
}

int posix_queue_dequeue(posix_queue_t *queue, void **data) {
    if (queue == NULL || data == NULL) {
        return EINVAL;
    }
    
    pthread_mutex_lock(&queue->mutex);
    
    if (queue->head == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return EAGAIN;
    }
    
    p_queue_node_t *node_to_remove = queue->head;
    *data = node_to_remove->data;
    
    queue->head = queue->head->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    
    pthread_mutex_unlock(&queue->mutex);
    
    free(node_to_remove);
    
    return 0;
}

bool posix_queue_is_empty(posix_queue_t *queue) {
    if (queue == NULL) {
        return true;
    }
    
    pthread_mutex_lock(&queue->mutex);
    bool empty = (queue->head == NULL);
    pthread_mutex_unlock(&queue->mutex);
    
    return empty;
}

void posix_queue_destroy(posix_queue_t *queue) {
    if (queue == NULL) {
        return;
    }
    
    pthread_mutex_lock(&queue->mutex);
    
    p_queue_node_t *current = queue->head;
    while (current != NULL) {
        p_queue_node_t *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    
    queue->head = NULL;
    queue->tail = NULL;
    
    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
}