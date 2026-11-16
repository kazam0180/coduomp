#include <stdint.h>
#include <string.h>

#include "../abi/ui_module_abi.h"

enum {
    UI_GAMMA_WINDOW_CAPACITY = 4,
    UI_WINDOW_TITLE_BYTES = 33,
    UI_WINDOW_TEXT_CAPACITY = 1024
};

typedef void *uiNativeWindow_t;
typedef intptr_t uiNativeWindowParam_t;
#define UI_WINDOW_CALLBACK qboolean

#if defined(__GNUC__)
#define UI_STORAGE_USED __attribute__((used))
#else
#define UI_STORAGE_USED
#endif

// Source: uo_ui_mp_x86.dll data 0x40041800..0x40041813.
// These slots retain the windows hidden by UI_HideGameWindow.
static uint32_t ui_hiddenWindowCount UI_STORAGE_USED; // 0x40041800
// 0x40041804
static uiNativeWindow_t ui_hiddenWindows[UI_GAMMA_WINDOW_CAPACITY]
    UI_STORAGE_USED;

/* NOT_FROM_ORIGINAL_SOURCE: restore the PE loader's empty hidden-window list
 * at each portable dllEntry lifecycle boundary. */
void ui_compat_reset_window_state(void)
{
    ui_hiddenWindowCount = 0;
    memset(ui_hiddenWindows, 0, sizeof(ui_hiddenWindows));
}

// Source: uo_ui_mp_x86.dll 0x40001000..0x400010ab
// Evidence: cgame_mp/mcode/uo_ui_mp_x86/FUN_40001000_400010ab.mcode
UI_WINDOW_CALLBACK UI_HideGameWindow(uiNativeWindow_t window,
                                     uiNativeWindowParam_t unused)
{
    // NOT_FROM_ORIGINAL_SOURCE: portable replacement for the Win32 callback.
    (void)window;
    (void)unused;
    return qtrue;
}

// Source: uo_ui_mp_x86.dll 0x400010b0..0x40001146
// Evidence: cgame_mp/mcode/uo_ui_mp_x86/FUN_400010b0_40001146.mcode
void UI_RestoreDesktopDisplay(void)
{
    // NOT_FROM_ORIGINAL_SOURCE: non-Windows restoration is engine-owned.
}
