#include "system_console.h"

#include "system_localization.h"
#include "system_platform.h"
#include "system_input.h"
#include "system_event.h"
#include "client/cgame.h"
#include "client/console.h"
#include "platform/crt_boundary.h"
#include "renderer/renderer_cvars.h"
#include "renderer/platform_display.h"
#include "renderer/platform_gamma.h"
#include "client/common/client_video_settings.h"
#include "server/server.h"
#include "sound/sound_system.h"
#include "qcommon/q_string.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


enum {
    SYS_CONSOLE_PRINT_SOURCE_LIMIT = 16383,
    SYS_CONSOLE_PRINT_BUFFER_CAPACITY = 32768,
    SYS_CONSOLE_TEXT_RETAIN_LIMIT = 16384,
    SYS_CONSOLE_APPEND_POSITION = 65535,
    SYS_CONSOLE_SCROLL_TO_END_LINES = 65535,
    SYS_CONSOLE_SELECT_ALL_END = -1,
    SYS_CONSOLE_ERROR_TEXT_CAPACITY = 512,
    SYS_CONSOLE_INPUT_CAPACITY = 512,
    SYS_CONSOLE_INPUT_APPEND_LIMIT = 507,
    SYS_CONSOLE_HISTORY_COUNT = 64,
    SYS_CONSOLE_COPY_BUTTON_ID = 1,
    SYS_CONSOLE_QUIT_BUTTON_ID = 2,
    SYS_CONSOLE_CLEAR_BUTTON_ID = 3,
    SYS_CONSOLE_HIDDEN = 0,
    SYS_CONSOLE_VISIBLE = 1,
    SYS_CONSOLE_MINIMIZED = 2,
    SYS_ERROR_DIALOG_TYPE = 16
};

/* COMPATIBILITY_PATCH (NOT_FROM_ORIGINAL_SOURCE): modern clients should
 * retain normal Windows task switching unless the user disables it. */
#define CODUOMP_WIN_ALLOW_ALT_TAB_DEFAULT "1"

void *sysApplicationInstance; /* original 0x0489bb8c */
void *sysConsoleWindow;       /* original 0x009d5950 */
void *sysConsoleOutputWindow; /* original 0x009d5954 */
void *sysConsoleErrorWindow;  /* original 0x009d5964 */
void *sysConsoleFont;         /* original 0x009d597c */
void *sysConsoleInputWindow;  /* original 0x009d5984 */
qboolean sysWindowActive;     /* original 0x0489bb90 */
qboolean sysWindowMinimized;  /* original 0x0489bb94 */

static int32_t sysConsoleVisibility; /* original 0x009d5f88 */
static qboolean sysConsoleQuitOnClose; /* original 0x009d5f8c */
static char sysConsoleErrorText[SYS_CONSOLE_ERROR_TEXT_CAPACITY];
static qboolean winAltTabDisabled; /* original 0x009d5fa8 */
/* Original queued input is stored at 0x009d5b88; Sys_ConsoleInput transfers
 * all queued lines to the separate return buffer at 0x009d5d88.
 * The returned buffer remains valid until the next nonempty input poll. */
static char sysConsoleInputLine[SYS_CONSOLE_INPUT_CAPACITY];
static char sysConsoleReturnedLine[SYS_CONSOLE_INPUT_CAPACITY];


/* Source: CoDUOMP.exe 0x0046f310..0x0046f673.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046f310_0046f674.mcode, its literal
 * window/class strings, and PE imports identifying every Win32 operation.
 * Name and no-argument signature: exact same-module Mac symbol
 * Sys_CreateConsole. */
void Sys_CreateConsole(void)
{
    /* NOT_FROM_ORIGINAL_SOURCE: modern non-Windows application shells do
     * not create the original Win32 edit-control console. */
}

/* Source: CoDUOMP.exe 0x0046f680..0x0046f6b5.
 * Name: established id-engine Win32 console boundary. The compiler inlines
 * the same body into Sys_Quit at 0x0046b4fd..0x0046b531. */
void Sys_DestroyConsole(void)
{
}

/* Source: CoDUOMP.exe 0x0046f6c0..0x0046f72f.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046f6c0_0046f730.mcode.
 * Name and two-argument signature: exact same-module Mac symbol
 * Sys_ShowConsole. */
void Sys_ShowConsole(int32_t visibility, qboolean quitOnClose)
{
    sysConsoleQuitOnClose = quitOnClose;
    if (visibility == sysConsoleVisibility)
        return;

    sysConsoleVisibility = visibility;
    if (sysConsoleWindow == NULL)
        return;

    /* NOT_FROM_ORIGINAL_SOURCE: no separate native console window exists
     * until the application shell supplies one. */
    (void)visibility;
}

/* Source: CoDUOMP.exe 0x0046f760..0x0046f8e3.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046f760_0046f8e4.mcode.
 * Name and argument: exact same-module Mac symbol Sys_Print. The Windows
 * console keeps only the last 16383 source bytes, expands line endings to
 * CRLF, and strips id color escapes before appending to its edit control. */
void Sys_Print(const char *text)
{
    /* NOT_FROM_ORIGINAL_SOURCE: native terminal output corresponding to the
     * Win32 edit-control console boundary. */
    fputs(text, stderr);
}

/* Source: CoDUOMP.exe 0x0046f730..0x0046f759.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046f730_0046f75a.mcode.
 * Name and return contract: exact same-module Mac symbol Sys_ConsoleInput.
 * The complete NUL-terminated queue is copied before the producer buffer's
 * first byte is cleared. */
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


/* Source: CoDUOMP.exe 0x0046f9a0..0x0046fa10.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046f9a0_0046fa11.mcode.
 * Provisional Windows-family name: disables Alt-Tab once, using an Alt+Tab
 * hotkey reservation on NT and SPI_SETSCREENSAVERRUNNING on older Windows. */
void WIN_DisableAltTab(void)
{
    if (winAltTabDisabled != qfalse)
        return;

    winAltTabDisabled = qtrue;
}

/* Source: CoDUOMP.exe 0x0046fa20..0x0046fa8c.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046fa20_0046fa8d.mcode.
 * Provisional Windows-family name: exact inverse of WIN_DisableAltTab. */
void WIN_EnableAltTab(void)
{
    if (winAltTabDisabled == qfalse)
        return;

    winAltTabDisabled = qfalse;
}

/* Source: CoDUOMP.exe 0x0046fa90..0x0046fadf.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046fa90_0046fae0.mcode.
 * Name and two Boolean roles: established Win32 id-engine window boundary;
 * the WM_ACTIVATE caller proves active comes from LOWORD(wParam) !=
 * WA_INACTIVE and minimized from HIWORD(wParam). */
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

/* Source: CoDUOMP.exe 0x0046fae0..0x0046fb4c.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046fae0_0046fb4d.mcode and the original
 * byte tables at 0x005ca528 and 0x005ca64c.
 * Name: established id-engine Win32 keyboard boundary. keyData is the low
 * 32-bit Win32 key-message lParam; its scan code and extended-key bit choose
 * between the original two translation-table columns. */

/* Source: CoDUOMP.exe 0x0046f8f0..0x0046f99f.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046f8f0_0046f9a0.mcode.
 * Provisional Windows-source name: installs fatal text in the console,
 * replaces its input control with a static error control, and also presents
 * the localized modal error dialog. */
void Sys_SetErrorText(const char *text)
{
    strncpy(sysConsoleErrorText, text,
            SYS_CONSOLE_ERROR_TEXT_CAPACITY - 1);
    sysConsoleErrorText[SYS_CONSOLE_ERROR_TEXT_CAPACITY - 1] = '\0';

    /* NOT_FROM_ORIGINAL_SOURCE: native application-shell error presentation
     * is intentionally separate from the terminal Sys_Print boundary. */
    (void)text;
}
