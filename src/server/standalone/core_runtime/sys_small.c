#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

#include "core_runtime_private.h"

#define SYS_CONFIGURE_PLATFORM_STUB_RESULT ((int32_t)0)
#define SYS_DEFAULT_USERNAME "player"

void Sys_LoadingKeepAlive(void)
{
}

qboolean Sys_InfoChanged(void)
{
    return SYS_CONFIGURE_PLATFORM_STUB_RESULT;
}

qboolean
Sys_ConfigureChecksumChanged(int32_t checksum)
{
    (void)checksum;
    return SYS_CONFIGURE_PLATFORM_STUB_RESULT;
}

const char *Sys_GetCurrentUser(void)
{
    struct passwd *entry;

    entry = getpwuid(getuid());
    if (entry == NULL) {
        return SYS_DEFAULT_USERNAME;
    }

    return entry->pw_name;
}
