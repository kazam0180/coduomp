#define _POSIX_C_SOURCE 200809L

#include "q_shared.h"

#include <limits.h>
#include <math.h>
#include <time.h>

qboolean sysMillisecondsInitialized; /* original 0x0389fddc */
uint32_t sysMillisecondsBase;         /* original 0x0489bc3c */
static char sysDateTimeStamp[16];     /* original 0x009d0504..0x009d0513 */

enum {
    SYS_DATE_BUFFER_BYTES = 10,
    SYS_TIME_BUFFER_BYTES = 7,
    SYS_TIME_BUFFER_OFFSET = 9
};

static uint32_t Sys_RawMilliseconds(void)
{
    struct timespec now;

    (void)clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint32_t)((uint64_t)now.tv_sec * UINT64_C(1000) +
                      (uint64_t)now.tv_nsec / UINT64_C(1000000));
}

uint32_t Sys_Milliseconds(void)
{
    if (sysMillisecondsInitialized == qfalse) {
        sysMillisecondsBase = Sys_RawMilliseconds();
        sysMillisecondsInitialized = qtrue;
    }

    return Sys_RawMilliseconds() - sysMillisecondsBase;
}

char *Sys_DateTimeStamp(void)
{
    const time_t now = time(NULL);
    struct tm localTime;

    if (localtime_r(&now, &localTime) == NULL) {
        sysDateTimeStamp[0] = '\0';
        return sysDateTimeStamp;
    }
    (void)strftime(
        sysDateTimeStamp, SYS_DATE_BUFFER_BYTES,
        "%Y%m%d-", &localTime);
    (void)strftime(
        &sysDateTimeStamp[SYS_TIME_BUFFER_OFFSET],
        SYS_TIME_BUFFER_BYTES, "%H%M%S", &localTime);
    return sysDateTimeStamp;
}

int32_t Sys_RoundFloatToInt(float value)
{
    const double rounded = rint((double)value);

    if (!(rounded >= (double)INT32_MIN &&
          rounded <= (double)INT32_MAX)) {
        return INT32_MIN;
    }
    return (int32_t)rounded;
}

void Sys_SnapVector(vec3_t vector)
{
    for (int32_t component = 0; component < 3; ++component) {
        vector[component] =
            (float)Sys_RoundFloatToInt(vector[component]);
    }
}
