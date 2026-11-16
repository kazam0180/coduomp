
#include "platform/crt_boundary.h"
#include "platform/sdl_platform.h"
#include "platform/floating_point_boundary.h"
#include "q_shared.h"
#include "system_info.h"
#include "system_input.h"
#include "system_localization.h"
#include "system_platform.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if   defined(__linux__)
#include <sys/resource.h>
#endif

enum {
    SYS_NATIVE_COMMAND_LINE_CAPACITY = 8192
};

#if defined(__linux__)
enum {
    SYS_LINUX_OPEN_FILE_TARGET = 4096
};

/* NOT_FROM_ORIGINAL_SOURCE: Linux sessions can inherit a low descriptor soft
 * limit. Keep room for archives and filesystem restarts without lowering an
 * existing limit or changing the inherited hard limit. */
static void coduomp_linux_raise_open_file_limit(void)
{
    struct rlimit limits;
    rlim_t target = SYS_LINUX_OPEN_FILE_TARGET;

    if (getrlimit(RLIMIT_NOFILE, &limits) != 0) {
        perror("CoDUOMP: cannot read open-file limit");
        return;
    }
    if (limits.rlim_cur >= target)
        return;
    if (limits.rlim_max < target)
        target = limits.rlim_max;
    if (limits.rlim_cur >= target)
        return;

    limits.rlim_cur = target;
    if (setrlimit(RLIMIT_NOFILE, &limits) != 0)
        perror("CoDUOMP: cannot raise open-file limit");
}
#endif

/* NOT_FROM_ORIGINAL_SOURCE: constructs the mutable command string consumed by
 * Com_ParseCommandLine from the native argc/argv process interface. Quoting is
 * retained for arguments containing whitespace so startup cvar values survive
 * the source-level conversion. */
static qboolean Sys_BuildNativeCommandLine(
    int argc, char **argv, char *commandLine, size_t commandLineSize)
{
    size_t cursor = 0;

    commandLine[0] = '\0';
    for (int argumentIndex = 1;
         argumentIndex < argc; ++argumentIndex) {
        const char *argument = argv[argumentIndex];
        const qboolean quote =
            strpbrk(argument, " \t") != NULL ? qtrue : qfalse;
        const size_t argumentLength = strlen(argument);
        const size_t required =
            argumentLength + (quote != qfalse ? 2U : 0U) +
            (cursor != 0 ? 1U : 0U);

        if (required >= commandLineSize - cursor)
            return qfalse;
        if (cursor != 0)
            commandLine[cursor++] = ' ';
        if (quote != qfalse)
            commandLine[cursor++] = '"';
        memcpy(&commandLine[cursor], argument, argumentLength);
        cursor += argumentLength;
        if (quote != qfalse)
            commandLine[cursor++] = '"';
        commandLine[cursor] = '\0';
    }
    return qtrue;
}

/* NOT_FROM_ORIGINAL_SOURCE: native process entry point corresponding to the
 * recovered WinMain. Subsystem initialization and the frame loop preserve the
 * original ordering; SDL replaces only the Win32 application shell. */
int main(int argc, char **argv)
{
    char commandLine[SYS_NATIVE_COMMAND_LINE_CAPACITY];
    char workingDirectory[MAX_OSPATH];

    coduomp_restore_retail_x87_precision();

#if   defined(__linux__)
    coduomp_linux_raise_open_file_limit();
#endif

    if (Sys_BuildNativeCommandLine(
            argc, argv, commandLine, sizeof(commandLine)) == qfalse) {
        fputs("CoDUOMP: command line is too long\n", stderr);
        return EXIT_FAILURE;
    }

    (void)Sys_InitLocalization();
    Sys_InitHardwareInfo();
    sysExecutableChecksum = Sys_GetExecutableChecksum(NULL);

    if (CoduoSDL_Init() == qfalse) {
        fputs("CoDUOMP: SDL initialization failed\n", stderr);
        Sys_ShutdownLocalization();
        return EXIT_FAILURE;
    }
    (void)atexit(CoduoSDL_Shutdown);

    (void)Sys_Milliseconds();
    Com_Init(commandLine);
    (void)coduomp_crt_getcwd(
        workingDirectory, sizeof(workingDirectory));
    Com_Printf("Working directory: %s\n", workingDirectory);

    for (;;) {
        IN_Frame();
        Com_Frame();
    }
}

