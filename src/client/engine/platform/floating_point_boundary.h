#ifndef CODUOMP_FLOATING_POINT_BOUNDARY_H
#define CODUOMP_FLOATING_POINT_BOUNDARY_H

#if defined(_WIN32) || ((defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) || defined(__clang__)))
#if defined(_MSC_VER) && !defined(__clang__) && defined(_M_X64)
/* NOT_FROM_ORIGINAL_SOURCE: MSVC x64 emits SSE arithmetic and has no native
 * x87 precision-control path. Native arithmetic uses its binary64 long double
 * carrier and the source's explicit rounding boundaries. */
#define CODUOMP_NATIVE_X87_PRECISION_CONTROL 0
#define coduomp_restore_retail_x87_precision() ((void)0)
#else
#include <float.h>
#include <stdint.h>
#if defined(_WIN32) && UINTPTR_MAX != UINT32_MAX
#if defined(__clang__) && defined(_MSC_VER)
#if !defined(_M_X64) || LDBL_MANT_DIG != 64 || LDBL_MAX_EXP != 16384 || FLT_EVAL_METHOD != 0
#error "clang-cl requires x64 with 80-bit long double and source-type evaluation"
#endif
#elif !defined(__x86_64__) || defined(__SSE_MATH__) || !defined(__FLT_EVAL_METHOD__) || __FLT_EVAL_METHOD__ != 2
#error "The recovered Windows engine requires i686 or x86-64 GCC x87 code generation"
#endif
#endif
#define CODUOMP_NATIVE_X87_PRECISION_CONTROL 1
void coduomp_restore_retail_x87_precision(void);
#endif
#else
#define CODUOMP_NATIVE_X87_PRECISION_CONTROL 0
#define coduomp_restore_retail_x87_precision() ((void)0)
#endif

#endif
