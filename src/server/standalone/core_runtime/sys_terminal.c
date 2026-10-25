#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#include "core_runtime_private.h"

#define SYS_ISATTY_TRUE 1
#define SYS_LINUX_TERMIOS_VERASE_INDEX 2
#define SYS_LINUX_TERMIOS_VEOF_INDEX 4
#define SYS_LINUX_TERMIOS_VTIME_INDEX 5
#define SYS_LINUX_TERMIOS_VMIN_INDEX 6
#define SYS_LINUX_TERMIOS_RAW_INPUT_MASK 48
#define SYS_LINUX_TERMIOS_NONCANONICAL_LOCAL_MASK 10
#define SYS_LINUX_TERMIOS_MIN_READ_BYTES 1
#define SYS_LINUX_TERMIOS_READ_TIMEOUT 0

void Sys_ShutdownTerminalConsole(void)
{
    if (sys_ttyConsoleActive != 0) {
        Com_Printf("Shutdown tty console\n");
        tcsetattr(SYS_STDIN_FILE_DESCRIPTOR, TCSADRAIN, &sys_ttyOriginalTermios);
    }
}

void Sys_InitTerminalConsole(void)
{
    struct termios terminalSettings;

    memset(&terminalSettings, 0, sizeof(terminalSettings));
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    ttycon = Cvar_Get("ttycon", "1", CVAR_NONE);
    if (ttycon == NULL || ttycon->value == 0.0f) {
        sys_ttyConsoleActive = 0;
        return;
    }

    if (isatty(SYS_STDIN_FILE_DESCRIPTOR) != SYS_ISATTY_TRUE) {
        Com_Printf("stdin is not a tty, tty console mode failed\n");
        Cvar_Set("ttycon", "0");
        sys_ttyConsoleActive = 0;
        return;
    }

    Com_Printf("Started tty console (use +set ttycon 0 to disable)\n");
    Sys_TTYResetLine(&sys_ttyCurrentLine);
    tcgetattr(SYS_STDIN_FILE_DESCRIPTOR, &sys_ttyOriginalTermios);
    /* Preserve this recovered boundary's validated input, state, and compatibility invariants. */
    sys_ttyEraseChar =
        (unsigned char)terminalSettings.c_cc[SYS_LINUX_TERMIOS_VERASE_INDEX];
    sys_ttyEofChar =
        (unsigned char)terminalSettings.c_cc[SYS_LINUX_TERMIOS_VEOF_INDEX];

    terminalSettings = sys_ttyOriginalTermios;
    terminalSettings.c_lflag &= ~SYS_LINUX_TERMIOS_NONCANONICAL_LOCAL_MASK;
    terminalSettings.c_iflag &= ~SYS_LINUX_TERMIOS_RAW_INPUT_MASK;
    terminalSettings.c_cc[SYS_LINUX_TERMIOS_VMIN_INDEX] =
        SYS_LINUX_TERMIOS_MIN_READ_BYTES;
    terminalSettings.c_cc[SYS_LINUX_TERMIOS_VTIME_INDEX] =
        SYS_LINUX_TERMIOS_READ_TIMEOUT;
    tcsetattr(SYS_STDIN_FILE_DESCRIPTOR, TCSADRAIN, &terminalSettings);
    sys_ttyConsoleActive = 1;
}
