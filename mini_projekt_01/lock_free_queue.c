#include "lock_free_queue.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>

static _Atomic(hazard_record_t*) hazard_list = NULL;
static atomic_int hazard_count = 0;

__thread struct retired_node {
    void *ptr;
    struct retired_node *next;
} *retired_list = NULL;

__thread int retired_count = 0;
#define RETIREMENT_THRESHOLD 64

int ptr_compare(const void *a, const void *b) {
    void *ptr_a = *(void**)a;
    void *ptr_b = *(void**)b;
    return (ptr_a < ptr_b) ? -1 : (ptr_a > ptr_b) ? 1 : 0;
}

// hazard pointer

hazard_record_t* acquire() {
    // Purpose: Get a hazard pointer record to publish a pointer

    // Scan global list for an inactive record
    _Atomic(hazard_record_t *)record = atomic_load(&hazard_list);
    while (record != NULL) {

        // Try to claim it with CAS on active field
        int expected = INACTIVE;
        if (atomic_compare_exchange_weak_explicit(&(record->active), &expected, ACTIVE,
                                        memory_order_acquire, memory_order_relaxed)) {
            return record;
        }

        record = record->next;
    }

    // If no free records, allocate new one and add to head
    hazard_record_t *new_record = malloc(sizeof(hazard_record_t));
    atomic_store(&new_record->active, ACTIVE);
    atomic_store(&new_record->hazard_pointer, NULL);

    _Atomic(hazard_record_t *)current_head;
    do {
        current_head = atomic_load(&hazard_list);
        new_record->next = current_head;
        // CAS updates current_head if it fails
    } while (!atomic_compare_exchange_weak(&hazard_list, &current_head, new_record));

    atomic_fetch_add(&hazard_count, 1);

    // Return the acquired record
    return new_record;
}

void release(hazard_record_t *record) {
    // Purpose: Return a hazard pointer record when done
    if (record == NULL) return;
    atomic_store_explicit(&record->hazard_pointer, NULL, memory_order_release);
    atomic_store_explicit(&record->active, INACTIVE, memory_order_release);
}

void retire(void *ptr) {
    // Purpose: Mark pointer for eventual deletion
    if (ptr == NULL) return;

    // Add pointer to thread's retirement list
    struct retired_node *node = malloc(sizeof(struct retired_node));
    node->ptr = ptr;
    node->next = retired_list;
    retired_list = node;
    retired_count++;

    // If list gets too long, call Scan()
    if (retired_count >= RETIREMENT_THRESHOLD){
        scan();
    }
}

void scan() {
    // Purpose: Actually free memory that's no longer hazardous

    // Collect all currently published hazard pointers
    void **hazards = malloc(MAX_THREADS * sizeof(void *));
    int hazards_count = 0;

    _Atomic(hazard_record_t *)record = atomic_load(&hazard_list);
    while (record != NULL && hazards_count < MAX_THREADS) 
    {
        if (atomic_load_explicit(&record->active, memory_order_acquire)) {
            void *hazard_pointer = atomic_load_explicit(&record->hazard_pointer, memory_order_acquire);
            if (hazard_pointer != NULL) {
                hazards[hazards_count++] = hazard_pointer;
            }
        }

        record = record->next;
    }

    // Sort them for fast lookup
    qsort(hazards, hazards_count, sizeof(void *), ptr_compare);

    // For each retired pointer, check if it's hazardous
    struct retired_node **current = &retired_list;
    int new_retired_count = 0; 
    while (*current != NULL) {
        if (bsearch(&(*current)->ptr, hazards, hazards_count, sizeof(void *), ptr_compare) == NULL) {
            struct retired_node *to_free = *current;
            *current = (*current)->next;
            free(to_free->ptr);
            free(to_free);
        } else {
            current = &(*current)->next;
            new_retired_count++;
        }
    }

    // Free non-hazardous pointers, keep hazardous ones
    retired_count = new_retired_count;
    free(hazards);

}


void lf_queue_init(lf_queue_t *queue) {
    lf_node_t *dummy = malloc(sizeof(lf_node_t));
    dummy->data = NULL;
    atomic_store(&dummy->next, NULL);

    atomic_store(&queue->head, dummy);
    atomic_store(&queue->tail, dummy);
}

void lf_enqueue(lf_queue_t *queue, void *data) {
    lf_node_t *new_node = malloc(sizeof(lf_node_t));
    new_node->data = data;
    atomic_store(&new_node->next, NULL);

    lf_node_t *tail, *next;

    while (1) {
        tail = atomic_load(&queue->tail);
        next = atomic_load(&tail->next);
        
        if (tail == atomic_load(&queue->tail)) {
            if (next == NULL) {
                if (atomic_compare_exchange_weak_explicit(&tail->next, &next, new_node,
                                        memory_order_release, memory_order_relaxed)) {
                    break;
                }
            } else {
                atomic_compare_exchange_weak(&queue->tail, &tail, next);
            }
        }
    }

    atomic_compare_exchange_weak(&queue->tail, &tail, new_node);
}

bool lf_dequeue(lf_queue_t *queue, void **data) {

    hazard_record_t *hazard_ptr_head = acquire();
    hazard_record_t *hazard_ptr_next = acquire();

    lf_node_t *head, *tail, *next;

    while (1) {
        head = atomic_load(&queue->head);
        atomic_store(&hazard_ptr_head->hazard_pointer, head);

        if (head != atomic_load(&queue->head)) {
            continue;
        }

        tail = atomic_load(&queue->tail);
        next = atomic_load(&head->next);

        atomic_store(&hazard_ptr_next->hazard_pointer, next);

        if (head != atomic_load(&queue->head)) {
            continue;
        }

        if (head == tail) {
            if (next == NULL) {
                release(hazard_ptr_head);
                release(hazard_ptr_next);

                return false;
            }

            atomic_compare_exchange_weak(&queue->tail, &tail, next);
        } else {
            if (next == NULL) {
                continue;
            }

            *data = next->data;

            if (atomic_compare_exchange_weak_explicit(&queue->head, &head, next, 
                                        memory_order_release, memory_order_relaxed)) {
                release(hazard_ptr_head);
                release(hazard_ptr_next);
                retire(head);
                return true;
            }
        }

    }
}

void lf_destroy(lf_queue_t *queue){
    
    void *data = NULL;

    while (lf_dequeue(queue, &data)) {};

    lf_node_t *dummy = atomic_load(&queue->head);
    free(dummy);

    scan();
}

bool lf_is_empty(lf_queue_t *queue){
    hazard_record_t *hazard_ptr_head = acquire();
    lf_node_t *head, *tail, *next;
    bool result;
    
    while (1) {
        head = atomic_load(&queue->head);
        atomic_store(&hazard_ptr_head->hazard_pointer, head);
        
        // Verify head didn't change
        if (head != atomic_load(&queue->head)) {
            continue;
        }
        
        tail = atomic_load(&queue->tail);
        next = atomic_load(&head->next);
        
        // Double-check head consistency
        if (head != atomic_load(&queue->head)) {
            continue;
        }
        
        if (head == tail) {
            if (next == NULL) {
                result = true;  // Empty
                break;
            } else {
                // Help advance lagging tail
                atomic_compare_exchange_weak(&queue->tail, &tail, next);
                continue;
                // Continue loop to re-check
            }
        } else {
            result = false;  // Not empty
            break;
        }
    }
    
    release(hazard_ptr_head);
    return result;
}