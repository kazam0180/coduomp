#include "system_console.h"

#include "system_input.h"
#include "system_localization.h"

#include <stdio.h>
#include <string.h>

enum {
    SYS_CONSOLE_ERROR_TEXT_CAPACITY = 512,
    SYS_CONSOLE_INPUT_CAPACITY = 512
};

void *sysApplicationInstance;
void *sysConsoleWindow;
void *sysConsoleOutputWindow;
void *sysConsoleErrorWindow;
void *sysConsoleFont;
void *sysConsoleInputWindow;
qboolean sysWindowActive;
qboolean sysWindowMinimized;

static qboolean sysConsoleQuitOnClose;
static char sysConsoleErrorText[SYS_CONSOLE_ERROR_TEXT_CAPACITY];
static char sysConsoleInputLine[SYS_CONSOLE_INPUT_CAPACITY];
static char sysConsoleReturnedLine[SYS_CONSOLE_INPUT_CAPACITY];

void Sys_CreateConsole(void)
{
}

void Sys_DestroyConsole(void)
{
}

void Sys_ShowConsole(int32_t visibility, qboolean quitOnClose)
{
    sysConsoleQuitOnClose = quitOnClose;
    (void)visibility;
}

void Sys_Print(const char *text)
{
    fputs(text, stderr);
}

char *Sys_ConsoleInput(void)
{
    if (sysConsoleInputLine[0] == '\0')
        return NULL;

    char *source = sysConsoleInputLine;
    char *destination = sysConsoleReturnedLine;
    do {
        *destination++ = *source;
    } while (*source++ != '\0');
    sysConsoleInputLine[0] = '\0';
    return sysConsoleReturnedLine;
}

void AppActivate(qboolean active, qboolean minimized)
{
    sysWindowMinimized = minimized;
    Key_ClearStates();

    if (active != qfalse && minimized == qfalse) {
        sysWindowActive = qtrue;
        sysInputAppActive = qtrue;
        return;
    }

    sysWindowActive = qfalse;
    sysInputAppActive = qfalse;
    IN_DeactivateMouse();
}

void Sys_SetErrorText(const char *text)
{
    strncpy(sysConsoleErrorText, text,
            SYS_CONSOLE_ERROR_TEXT_CAPACITY - 1);
    sysConsoleErrorText[SYS_CONSOLE_ERROR_TEXT_CAPACITY - 1] = '\0';
}
