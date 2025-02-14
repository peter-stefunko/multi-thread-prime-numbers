#ifndef LIMIT_H
#define LIMIT_H

#include <limits.h>
#include <stdatomic.h>

#define LIMIT 25000000

#if LIMIT <= UINT_MAX
    #define UINT_T unsigned
    #define A_UINT_T atomic_uint
#elif LIMIT <= ULONG_MAX
    #define UINT_T unsigned long
    #define A_UINT_T atomic_ulong
#endif

#endif  // LIMIT_H
