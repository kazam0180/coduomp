#include "q_shared.h"

enum {
    CODUOMP_LINUX_VIDEO_MEMORY_PROFILE_MB = 256
};

static int32_t coduomp_linux_detect_video_memory_mb(void)
{
    return 0;
}

int32_t Sys_GetVideoMemoryMB(void)
{
    const int32_t detectedMegabytes =
        coduomp_linux_detect_video_memory_mb();
    return detectedMegabytes > 0
               ? detectedMegabytes
               : CODUOMP_LINUX_VIDEO_MEMORY_PROFILE_MB;
}
