#define _POSIX_C_SOURCE 200809L

#include "q_shared.h"

#include "system_localization.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)
#include <cpuid.h>
#include <x86intrin.h>
#endif

qboolean sysSseSupported; /* original 0x0489d460 */
double sysCpuFrequencyMHz; /* original 0x009cf1d8 */
int32_t sysPhysicalMemoryMB; /* original 0x009cf1e0 */
int32_t sysVideoMemoryMB; /* original 0x009cf1e4 */

enum {
    SYS_CPUID_VENDOR_LEAF = 0,
    SYS_CPUID_FEATURES_LEAF = 1,
    SYS_CPU_MEASUREMENT_MSEC = 250,
    SYS_BYTES_PER_MEGABYTE = 1024 * 1024,
    SYS_MIN_AVAILABLE_VIRTUAL_MEMORY = 128 * 1024 * 1024,
    SYS_PHYSICAL_MEMORY_CAP_MB = 1024,
    SYS_LOW_MEMORY_THRESHOLD_BYTES = 96 * 1024 * 1024
};

/* Exact .rdata qword at 0x005b9bd0: 0x3fdfffffff000000. */
static const double sysRoundHalfBias = 0.49999999906867743;

#if defined(__i386__) || defined(__x86_64__)
static void Sys_ReadCpuid(uint32_t leaf, uint32_t registers[4])
{
    __cpuid_count(
        leaf, 0, registers[0], registers[1],
        registers[2], registers[3]);
}

static qboolean Sys_CpuidSupported(void)
{
    return __get_cpuid_max(0, NULL) != 0 ? qtrue : qfalse;
}

static qboolean Sys_CpuHas3DNow(void)
{
    uint32_t registers[4];

    Sys_ReadCpuid(SYS_CPUID_VENDOR_LEAF, registers);
    Sys_ReadCpuid(UINT32_C(0x80000000), registers);
    if (registers[0] < UINT32_C(0x80000000))
        return qfalse;

    Sys_ReadCpuid(UINT32_C(0x80000001), registers);
    return (registers[3] & UINT32_C(0x80000000)) != 0
               ? qtrue
               : qfalse;
}

static qboolean Sys_CpuHasSse(void)
{
    uint32_t registers[4];

    Sys_ReadCpuid(SYS_CPUID_FEATURES_LEAF, registers);
    return (registers[3] & (UINT32_C(1) << 25)) != 0
               ? qtrue
               : qfalse;
}

static qboolean Sys_CpuHasMmx(void)
{
    uint32_t registers[4];

    Sys_ReadCpuid(SYS_CPUID_FEATURES_LEAF, registers);
    return (registers[3] & (UINT32_C(1) << 23)) != 0
               ? qtrue
               : qfalse;
}
#endif

sysCpuClass_t Sys_DetectCpuClass(void)
{
#if !defined(__i386__) && !defined(__x86_64__)
    return CPUID_GENERIC;
#else
    if (Sys_CpuidSupported() == qfalse)
        return CPUID_INTEL_UNSUPPORTED;
    if (Sys_CpuHasMmx() == qfalse)
        return CPUID_INTEL_PENTIUM;
    if (Sys_CpuHas3DNow() != qfalse)
        return CPUID_AMD_3DNOW;
    if (Sys_CpuHasSse() != qfalse)
        return CPUID_INTEL_KATMAI;
    return CPUID_INTEL_MMX;
#endif
}

int32_t Sys_RoundPositiveFloatToInt(float value)
{
    const double rounded =
        rint((double)value + sysRoundHalfBias);

    if (!(rounded >= (double)INT32_MIN &&
          rounded <= (double)INT32_MAX)) {
        return INT32_MIN;
    }
    return (int32_t)rounded;
}

double Sys_GetCpuFrequencyMHz(void)
{
#if defined(__i386__) || defined(__x86_64__)
    struct timespec delay = {
        0, SYS_CPU_MEASUREMENT_MSEC * 1000000L
    };
    struct timespec counterStart;
    struct timespec counterEnd;
    uint64_t cyclesStart;
    uint64_t cyclesEnd;
    double elapsedSeconds;

    clock_gettime(CLOCK_MONOTONIC, &counterStart);
    cyclesStart = __rdtsc();
    nanosleep(&delay, NULL);
    cyclesEnd = __rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &counterEnd);
    elapsedSeconds =
        (double)(counterEnd.tv_sec - counterStart.tv_sec) +
        (double)(counterEnd.tv_nsec - counterStart.tv_nsec) * 0.000000001;
    if (elapsedSeconds <= 0.0)
        return 0.0;
    return (double)(cyclesEnd - cyclesStart) * 0.000001 / elapsedSeconds;
#else
    return 0.0;
#endif
}

int32_t Sys_GetPhysicalMemoryMB(void)
{
    uint64_t totalPhysicalBytes;
    long pageCount;
    long pageSize;

    pageCount = sysconf(_SC_PHYS_PAGES);
    pageSize = sysconf(_SC_PAGESIZE);
    if (pageCount <= 0 || pageSize <= 0)
        totalPhysicalBytes = 0;
    else
        totalPhysicalBytes = (uint64_t)pageCount * (uint64_t)pageSize;

    const float physicalMegabytes =
        (float)totalPhysicalBytes * (1.0f / SYS_BYTES_PER_MEGABYTE);
    int32_t roundedMegabytes =
        Sys_RoundPositiveFloatToInt(physicalMegabytes);

    if (roundedMegabytes < 0 ||
        (uint64_t)roundedMegabytes * SYS_BYTES_PER_MEGABYTE !=
            totalPhysicalBytes ||
        roundedMegabytes > SYS_PHYSICAL_MEMORY_CAP_MB) {
        roundedMegabytes = SYS_PHYSICAL_MEMORY_CAP_MB;
    }
    return roundedMegabytes;
}

qboolean Sys_LowPhysicalMemory(void)
{
    return sysPhysicalMemoryMB <= SYS_LOW_MEMORY_THRESHOLD_BYTES
               ? qtrue
               : qfalse;
}

qboolean Sys_DetectSSESupport(void)
{
#if defined(__i386__) || defined(__x86_64__)
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;

    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx) != 0 &&
        (edx & bit_SSE) != 0) {
        __asm__ volatile("xorps %%xmm0, %%xmm0" : : : "xmm0");
        return qtrue;
    }
#endif
    return qfalse;
}
