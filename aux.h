#include "limit.h"

#include <stdbool.h>
#include <unistd.h>

bool is_prime(UINT_T num);
void *prime_count(void *handle);
unsigned long get_available_memory();
size_t get_stack_size();