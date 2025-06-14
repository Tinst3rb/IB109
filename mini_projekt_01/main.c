#include <stdio.h>
#include <stdlib.h>
#include "test.h"

#define UCO 524725

int main() {
    posix_queue_t posix_queue;
    lf_queue_t lf_queue;
    
    double time_posix = test_queue(&posix_queue,
                                   posix_enqueue_wrapper,
                                   posix_dequeue_wrapper,
                                   posix_init_wrapper,
                                   posix_destroy_wrapper,
                                   posix_is_empty_wrapper);
    
    if (time_posix < 0) {
        return 1;
    }
    
    double time_lockfree = test_queue(&lf_queue,
                                      lf_enqueue_wrapper,
                                      lf_dequeue_wrapper,
                                      lf_init_wrapper,
                                      lf_destroy_wrapper,
                                      lf_is_empty_wrapper);
    
    if (time_lockfree < 0) {
        return 1;
    }
    
    int diff_percentage = (int)((time_lockfree / time_posix) * 100);
    
    printf("%d\n", UCO);
    printf("%d\n", diff_percentage);
    
    return 0;
}