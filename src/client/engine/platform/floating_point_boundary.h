#ifndef CODUOMP_FLOATING_POINT_BOUNDARY_H
#define CODUOMP_FLOATING_POINT_BOUNDARY_H

#if (defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) || defined(__clang__))
#include <float.h>
#include <stdint.h>
#define CODUOMP_NATIVE_X87_PRECISION_CONTROL 1
void coduomp_restore_retail_x87_precision(void);
#else
#define CODUOMP_NATIVE_X87_PRECISION_CONTROL 0
#define coduomp_restore_retail_x87_precision() ((void)0)
#endif

#endif
