#include "handles.h"
#include "limit.h"

#include <err.h>
#include <math.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

bool is_prime(UINT_T num) {
    UINT_T root = sqrt(num);
    for (UINT_T d = 2; d <= root; d++) {
        if (num % d == 0)
            return false;
    }
    return true;
}

void *prime_count(void *handle) {
    struct thread_handle *th = handle;
    th->started = true;
    struct prime_handle *ph = th->handle;

    for (UINT_T n = ph->from; n < ph->to && n <= ph->limit; n++) {
        if (!is_prime(n))
            continue;

        (*ph->count)++;
        if (n > *(ph->max_prime)) {
            *(ph->max_prime) = n;
        }
    }

    free(ph);
    th->handle = NULL;

    if (sem_post(th->semaphore) == -1)
        err(-1, "prime_count: sem_post");
    return NULL;
}

unsigned long get_available_memory() {
#ifdef __linux__
    return sysconf(_SC_AVPHYS_PAGES) * sysconf(_SC_PAGESIZE);
#elif __APPLE__
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics64_data_t vm_stat;

    if (host_statistics64(mach_host_self(), HOST_VM_INFO, (host_info_t)&vm_stat, &count) != KERN_SUCCESS) {
        perror("get_available_memory: host_statistics64");
        return 0;
    }

    return (vm_stat.free_count + vm_stat.inactive_count) * sysconf(_SC_PAGESIZE);
#endif
}

size_t get_stack_size() {
    struct rlimit limit;
    if (getrlimit(RLIMIT_STACK, &limit) == -1) {
        perror("get_stack_size: getrlimit");
        return 0;
    }
    return limit.rlim_cur;
}
