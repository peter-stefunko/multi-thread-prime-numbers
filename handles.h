#ifndef HANDLES_H
#define HANDLES_H

#include "limit.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

struct prime_handle {
    atomic_uint *count;
    A_UINT_T *max_prime;
    UINT_T from;
    UINT_T to;
    UINT_T limit;
};

struct thread_handle {
    pthread_t t_id;
    sem_t *semaphore;
    struct prime_handle *handle;
    bool started;
};

#endif  // HANDLES_H
