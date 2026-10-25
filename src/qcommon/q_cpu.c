#define _POSIX_C_SOURCE 200809L

#include "q_cpu.h"

#if defined(__i386__) || defined(__x86_64__)
#include <x86intrin.h>
#else
#include <time.h>
#endif

/* Source: CoDUOMP.exe 0x00488350..0x00488354 and coduo_lnxded
 * 0x080b0876..0x080b0890. */
uint32_t rdtsc(void)
{
#if defined(__i386__) || defined(__x86_64__)
    return (uint32_t)__rdtsc();
#else
    struct timespec counter;

    (void)clock_gettime(CLOCK_MONOTONIC, &counter);
    return (uint32_t)((uint64_t)counter.tv_sec * UINT64_C(1000000000) +
                      (uint64_t)counter.tv_nsec);
#endif
}
