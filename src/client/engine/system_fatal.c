#include "system_fatal.h"

#include "platform/sdl_platform.h"
#include "system_localization.h"
#include "system_platform.h"

#include <stdlib.h>

enum {
    SYS_ERROR_DIALOG_TYPE = 16,
    SYS_FATAL_EXIT_STATUS = -1
};

void Sys_ShowErrorDialog(const char *message, const char *title,
                         uint32_t dialogType)
{
    (void)dialogType;
    CoduoSDL_ShowErrorDialog(message, title);
}

void Sys_OutOfMemory(void)
{
    const char *title;
    const char *message;

    Sys_Shutdown();
    title = Sys_LocalizeString("WIN_OUT_OF_MEM_TITLE");
    message = Sys_LocalizeString("WIN_OUT_OF_MEM_BODY");
    Sys_ShowErrorDialog(message, title, SYS_ERROR_DIALOG_TYPE);
    exit(SYS_FATAL_EXIT_STATUS);
}
