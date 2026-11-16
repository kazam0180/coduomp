#ifndef CODUOMP_CRT_MATH_DISPATCH_H
#define CODUOMP_CRT_MATH_DISPATCH_H

#include "compat/coduo_x87_math_ops.h"
#include <stdint.h>

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#define CODUOMP_CRT_INTEL 1
#include <emmintrin.h>
#include <cpuid.h>
#if defined(__GNUC__) || defined(__clang__)
#define CODUOMP_CRT_SSE2 __attribute__((target("sse2")))
#else
#define CODUOMP_CRT_SSE2
#endif
#else
#define CODUOMP_CRT_INTEL 0
#define CODUOMP_CRT_SSE2
#endif

#if CODUO_X87_MATH_AVAILABLE
#define CRT_AMD_VENDOR_B UINT32_C(0x68747541)
#define CRT_AMD_VENDOR_C UINT32_C(0x444d4163)
#define CRT_AMD_VENDOR_D UINT32_C(0x69746e65)
enum {
    CRT_CPU_FEATURES_LEAF = 1,
    CRT_CPU_SSE2_BIT = 26,
    CRT_CPU_EXTENDED_FAMILY = 15,
    CRT_MXCSR_MASKS = 0x1f80,
    CRT_X87_MASKS = 0x007f
};

/* NOT_FROM_ORIGINAL_SOURCE: preserve the original CPU exclusion. Supported
 * x86 operating systems provide SSE state management whenever SSE2 is present;
 * the per-call exception checks remain separate from CPU availability. */
static inline int coduomp_crt_cpu_sse2(void)
{
    unsigned eax, edx;
    unsigned vendorB, vendorC, vendorD;
    unsigned ebx, ecx;
    if (__get_cpuid_max(0, 0) < CRT_CPU_FEATURES_LEAF)
        return 0;
    __cpuid(0, eax, vendorB, vendorC, vendorD);
    __cpuid(CRT_CPU_FEATURES_LEAF, eax, ebx, ecx, edx);
    if ((edx & (1u << CRT_CPU_SSE2_BIT)) == 0)
        return 0;
    unsigned family = (eax >> 8) & 15;
    if (family == CRT_CPU_EXTENDED_FAMILY)
        family += (eax >> 20) & 255;
    return !(vendorB == CRT_AMD_VENDOR_B && vendorD == CRT_AMD_VENDOR_D && vendorC == CRT_AMD_VENDOR_C && family == CRT_CPU_EXTENDED_FAMILY);
}

/* NOT_FROM_ORIGINAL_SOURCE: query MXCSR only after CPU availability is known. */
static CODUOMP_CRT_SSE2 inline int coduomp_crt_sse_masks(void)
{
    return (_mm_getcsr() & CRT_MXCSR_MASKS) == CRT_MXCSR_MASKS;
}

/* NOT_FROM_ORIGINAL_SOURCE: the Windows math entries share these gates.
 * Rounding modes and sticky flags do not participate in dispatch. */
static inline int coduomp_crt_use_sse2_math(void)
{
    return coduomp_crt_cpu_sse2() && coduomp_crt_sse_masks() &&
        (coduomp_x87_control() & CRT_X87_MASKS) == CRT_X87_MASKS;
}
#endif

#endif
