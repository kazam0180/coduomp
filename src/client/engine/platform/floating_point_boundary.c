#include "floating_point_boundary.h"

#if CODUOMP_NATIVE_X87_PRECISION_CONTROL

#include <float.h>
#include <stdint.h>

enum {
    CODUOMP_X87_PRECISION_MASK = 0x0300,
    CODUOMP_X87_DOUBLE_PRECISION = 0x0200
};

/* NOT_FROM_ORIGINAL_SOURCE: the retail entry path pushes 1 at 0x0056fb55
 * before calling the CRT initializer at 0x0056eeee. Its pre-init pointer
 * 0x005c2fc8 selects 0x0056fc37, so the nonzero branch at 0x0056fc4b calls
 * 0x005783fc. That helper passes _PC_53/_MCW_PC to the control-word path whose
 * FLDCW is at 0x0057e212. The _fpreset path repeats the same operation at
 * 0x0057e043..0x0057e052, but has no direct caller or stored function-pointer
 * reference in the executable. MinGW starts i686 x87 in 64-bit extended
 * precision, so the recovered engine explicitly selects the retail process
 * mode at its startup boundary. */
void coduomp_restore_retail_x87_precision(void)
{
    uint16_t controlWord;

    /* NOT_FROM_ORIGINAL_SOURCE: GNU-compatible x86 ports can emit x87
     * operations without a Microsoft CRT precision-control path. Apply the
     * same 53-bit x87 control-word state directly on these hosts. */
    __asm__ volatile ("fnstcw %0" : "=m" (controlWord));
    controlWord = (uint16_t)((controlWord & (uint16_t)~CODUOMP_X87_PRECISION_MASK) |
                             CODUOMP_X87_DOUBLE_PRECISION);
    __asm__ volatile ("fldcw %0" : : "m" (controlWord));
}

#endif
