#include "platform/crt_boundary.h"
#include "platform/sdl_platform.h"
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

enum {
    SYS_NATIVE_COMMAND_LINE_CAPACITY = 8192
};

/* Constructs the mutable command string consumed by
 * Com_ParseCommandLine from the native argc/argv process interface. Quoting is
 * added around arguments that contain whitespace so tokenization preserves the
 * original argument boundaries. */
static qboolean Sys_BuildNativeCommandLine(
    int argc, char **argv, char *commandLine, size_t capacity)
{
    size_t cursor = 0;

    if (capacity == 0)
        return qfalse;
    commandLine[0] = '\0';

    for (int index = 1; index < argc; ++index) {
        const char *const argument = argv[index];
        const size_t argumentLength = strlen(argument);
        const qboolean quote =
            strpbrk(argument, " \t\r\n") != NULL ? qtrue : qfalse;
        const size_t addedLength =
            (cursor != 0 ? 1 : 0) +
            (quote != qfalse ? 2 : 0) +
            argumentLength;

        if (cursor + addedLength >= capacity)
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

int main(int argc, char **argv)
{
    char commandLine[SYS_NATIVE_COMMAND_LINE_CAPACITY];
    char workingDirectory[MAX_OSPATH];

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
