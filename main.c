#include "aux.h"
#include "handles.h"
#include "limit.h"

#include <err.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#ifdef __APPLE__
    #include <sys/sysctl.h>
    #include <mach/mach.h>
#endif

int main() {
    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        err(-1, "main: clock_gettime start");
    }

    unsigned long available_memory;
    size_t stack_size;

    if ((available_memory = get_available_memory()) == 0
        || (stack_size = get_stack_size()) == 0)
            return -1;

    int total_threads = available_memory / stack_size;

    struct thread_handle *t_handles = calloc(total_threads, sizeof(struct thread_handle));
    if (!t_handles)
        err(-1, "main: calloc t_handles");

    UINT_T limit = LIMIT;
    UINT_T step = (limit + total_threads - 1) / total_threads;
    atomic_uint count = 0;
    A_UINT_T max_prime = 0;
    int handled = 0;
    int rv = -1;

    int max_concurrent = sysconf(_SC_NPROCESSORS_ONLN);
    sem_t *semaphore = NULL;
    const char *sem_name = "/primes.semaphore";

    if ((semaphore = sem_open(sem_name, O_CREAT, 0644, max_concurrent)) == SEM_FAILED) {
        free(t_handles);
        err(-1, "main: sem_open");
    }

    while (handled < total_threads) {
        if (sem_wait(semaphore) == -1) {
            perror("main: sem_wait");
            goto end;
        }

        struct prime_handle *ph = calloc(1, sizeof(struct prime_handle));
        if (!ph) {
            perror("main: calloc prime_handle");
            goto end;
        }

        ph->count = &count;
        ph->max_prime = &max_prime;
        ph->from = handled * step + 2;
        ph->to = ph->from + step;
        ph->limit = limit;

        struct thread_handle *th = &t_handles[handled];
        th->handle = ph;
        th->semaphore = semaphore;
        th->started = false;

        if (pthread_create(&th->t_id, NULL, prime_count, th) != 0) {
            perror("main: pthread_create");
            free(ph);
            goto end;
        }

        handled++;
    }

    rv = 0;

end:
    for (int i = 0; i < handled; i++) {
        struct thread_handle *th = &t_handles[i];
        if (th->started && pthread_join(th->t_id, NULL) != 0) {
            perror("main: pthread_join");
            rv = -1;
        }
        if (!th->started && th->handle)
            free(th->handle);
    }

    if (sem_close(semaphore) == -1)
        perror("main: sem_close");
    if (sem_unlink(sem_name) == -1)
        perror("main: sem_unlink");
    free(t_handles);

    double runtime = -1;
    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
        perror("main: clock_gettime end");
    else
        runtime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("rv = %d\n", rv);
    printf("runtime = %.6f seconds\n\n", runtime);

    printf("range = [2, %lu]\n", (unsigned long)limit);
    printf("calcs = %d\n", total_threads);
    printf("step  = %lu\n\n", (unsigned long)step);

    printf("count   = %d\n", count);
    printf("largest = %d\n\n", max_prime);

    printf("available memory = %lu\n", available_memory);
    printf("stack size       = %lu\n", stack_size);
    printf("total threads    = %d\n", total_threads);
    printf("handled threads  = %d\n", handled);
    return rv;
}
