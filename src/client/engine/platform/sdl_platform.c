#include "sdl_platform.h"


#include "../client/console.h"
#include "../system_console.h"
#include "../system_event.h"
#include "../system_input.h"
#include "../renderer/platform_gamma.h"
#include "qcommon/q_memory.h"
#include "client/common/client_video_settings.h"

#include <SDL.h>
#if defined(__linux__)
#include <SDL_syswm.h>
#endif


#include <stdint.h>
#include <string.h>

/* NOT_FROM_ORIGINAL_SOURCE_STORAGE_FILE: all private globals in this
 * translation unit belong to the native SDL application shell. */
static SDL_Window *coduoSdlWindow;
static SDL_GLContext coduoSdlGlContext;
static qboolean coduoSdlInitialized;

enum {
    CODUO_SDL_EVENT_BATCH_COUNT = 256,
    CODUO_SDL_CHAR_PASTE = 22
};


/* NOT_FROM_ORIGINAL_SOURCE: SDL provides the native application, window,
 * OpenGL-context, and input boundary that Win32 supplied to the recovered
 * executable. Game events remain expressed through the original engine queue. */
qboolean CoduoSDL_Init(void)
{
    if (coduoSdlInitialized != qfalse)
        return qtrue;

    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "0");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS |
                 SDL_INIT_GAMECONTROLLER) != 0) {
        return qfalse;
    }

    coduoSdlInitialized = qtrue;
    SDL_StartTextInput();
    return qtrue;
}

/* NOT_FROM_ORIGINAL_SOURCE: releases the native resources owned by the SDL
 * application shell. The recovered engine still performs its own subsystem
 * shutdown before reaching this boundary. */
void CoduoSDL_Shutdown(void)
{
    if (coduoSdlInitialized == qfalse)
        return;

    CoduoSDL_DestroyOpenGLWindow();
    SDL_StopTextInput();
    SDL_Quit();
    coduoSdlInitialized = qfalse;
}

/* NOT_FROM_ORIGINAL_SOURCE: SDL owns the native clipboard integration on
 * macOS and Unix. Copy its temporary allocation into the malloc-owned domain
 * expected by Sys_GetClipboardData and the recovered field/UI callers. */
char *coduomp_sdl_get_clipboard_text_compat(void)
{
    char *const nativeText = SDL_GetClipboardText();
    if (nativeText == NULL)
        return NULL;

    char *const clipboardText = CopyStringInternal(nativeText);
    SDL_free(nativeText);
    return clipboardText;
}

/* NOT_FROM_ORIGINAL_SOURCE: native equivalent of the Win32 window, pixel
 * format, and WGL-context setup in GLW_CreateWindow/GLW_InitDriver. A legacy
 * compatibility context is required because the original renderer uses the
 * OpenGL fixed-function pipeline. The stock source uses only the original
 * boolean window-mode domain, leaving the added window modes unreachable. */
qboolean CoduoSDL_CreateOpenGLWindow(int32_t width, int32_t height,
                                     int32_t colorBits, int32_t depthBits,
                                     int32_t stencilBits,
                                     int32_t windowMode,
                                     int32_t displayIndex,
                                     int32_t refreshRate)
{
    const char *windowTitle = "Call of Duty: United Offensive Multiplayer";
    uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    qboolean allowHighDpi = qtrue;
    SDL_DisplayMode exclusiveMode = {0};
    int32_t windowX = SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex);
    int32_t windowY = SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex);

    if (CoduoSDL_Init() == qfalse)
        return qfalse;

    if (windowMode == CODUOMP_WINDOW_MODE_EXCLUSIVE) {
        SDL_DisplayMode requestedMode = {0};

        /* NOT_FROM_ORIGINAL_SOURCE: exclusive mode switches the selected
         * display to the requested resolution. Reject unavailable modes
         * instead of silently selecting a different output size or refresh. */
        requestedMode.w = width;
        requestedMode.h = height;
        requestedMode.refresh_rate = refreshRate;
        if (SDL_GetClosestDisplayMode(displayIndex, &requestedMode, &exclusiveMode) == NULL ||
            exclusiveMode.w != width || exclusiveMode.h != height ||
            (refreshRate > 0 && exclusiveMode.refresh_rate != refreshRate)) {
            SDL_SetError("Exclusive fullscreen mode %dx%d at %d Hz is unavailable on display %d",
                         width, height, refreshRate, displayIndex + 1);
            return qfalse;
        }
        /* SDL mode sizes are screen coordinates; do not double the drawable
         * on Retina after explicitly switching to the requested mode size. */
        allowHighDpi = qfalse;
    }

    if (allowHighDpi != qfalse)
        flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    CoduoSDL_DestroyOpenGLWindow();

    if (colorBits <= 0)
        colorBits = 24;
    if (depthBits <= 0)
        depthBits = colorBits > 16 ? 24 : 16;
    if (stencilBits < 0)
        stencilBits = 0;
    if (depthBits < 24)
        stencilBits = 0;

    SDL_GL_ResetAttributes();
    (void)SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    (void)SDL_GL_SetAttribute(SDL_GL_RED_SIZE, colorBits >= 24 ? 8 : 5);
    (void)SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, colorBits >= 24 ? 8 : 6);
    (void)SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, colorBits >= 24 ? 8 : 5);
    /* The retail PFD explicitly leaves cAlphaBits at zero. Cocoa may still
     * realize an 8-bit alpha channel, but it is not a renderer requirement. */
    (void)SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    (void)SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthBits);
    (void)SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencilBits);
    (void)SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    (void)SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    (void)SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

    if (windowMode == CODUOMP_WINDOW_MODE_FULLSCREEN) {
        /* COMPATIBILITY_PATCH (NOT_FROM_ORIGINAL_SOURCE): SDL desktop
         * fullscreen hides desktop UI without capturing the selected display
         * or changing its mode, so the other connected displays stay active. */
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else if (windowMode == CODUOMP_WINDOW_MODE_BORDERLESS) {
        SDL_Rect displayBounds;

        /* COMPATIBILITY_PATCH (NOT_FROM_ORIGINAL_SOURCE): borderless follows
         * the selected display's complete bounds without entering an OS
         * fullscreen state or changing its mode. */
        memset(&displayBounds, 0, sizeof(displayBounds));
        if (SDL_GetDisplayBounds(displayIndex, &displayBounds) != 0)
            return qfalse;
        windowX = displayBounds.x;
        windowY = displayBounds.y;
        width = displayBounds.w;
        height = displayBounds.h;
        flags |= SDL_WINDOW_BORDERLESS;
    }

    coduoSdlWindow = SDL_CreateWindow(
        windowTitle, windowX, windowY,
        width, height, flags);
    if (coduoSdlWindow == NULL)
        return qfalse;

    /* NOT_FROM_ORIGINAL_SOURCE: establish the display mode before entering
     * exclusive fullscreen so SDL owns restoration on focus loss, restart,
     * shutdown, and context-creation failure. Cocoa may capture all displays
     * when the selected monitor is the primary display. */
    if (windowMode == CODUOMP_WINDOW_MODE_EXCLUSIVE &&
        (SDL_SetWindowDisplayMode(coduoSdlWindow, &exclusiveMode) != 0 ||
         SDL_SetWindowFullscreen(coduoSdlWindow, SDL_WINDOW_FULLSCREEN) != 0)) {
        CoduoSDL_DestroyOpenGLWindow();
        return qfalse;
    }

    coduoSdlGlContext = SDL_GL_CreateContext(coduoSdlWindow);
    if (coduoSdlGlContext == NULL) {
        SDL_DestroyWindow(coduoSdlWindow);
        coduoSdlWindow = NULL;
        return qfalse;
    }

    if (SDL_GL_MakeCurrent(coduoSdlWindow, coduoSdlGlContext) != 0) {
        CoduoSDL_DestroyOpenGLWindow();
        return qfalse;
    }

    (void)SDL_GL_SetSwapInterval(0);
    /* COMPATIBILITY_PATCH (NOT_FROM_ORIGINAL_SOURCE): original
     * GLW_CreateWindow explicitly calls SetForegroundWindow and SetFocus at
     * CoDUOMP.exe 0x004f4742..0x004f4756. Re-establish focus after replacing
     * the SDL window, rebase the translated mouse state without treating the
     * focus click as gameplay input, then reproduce AppActivate so stale
     * engine key state cannot survive the replacement. */
    SDL_RaiseWindow(coduoSdlWindow);
    AppActivate(qtrue, qfalse);
    return qtrue;
}

void CoduoSDL_DestroyOpenGLWindow(void)
{
    if (coduoSdlGlContext != NULL) {
        SDL_GL_DeleteContext(coduoSdlGlContext);
        coduoSdlGlContext = NULL;
    }
    if (coduoSdlWindow != NULL) {
        /* COMPATIBILITY_PATCH (NOT_FROM_ORIGINAL_SOURCE): original
         * GLimp_Shutdown hides the window before DestroyWindow at
         * CoDUOMP.exe 0x004f7024..0x004f7034. The synchronous WM_ACTIVATE
         * path reaches AppActivate, whose first action at 0x0046fa95 is
         * Key_ClearStates. Perform that complete transition before SDL can
         * discard the old window's pending mouse-button release. */
        AppActivate(qfalse, qfalse);
        SDL_DestroyWindow(coduoSdlWindow);
        coduoSdlWindow = NULL;
    }
}

/* NOT_FROM_ORIGINAL_SOURCE: SDL numbers active displays from zero. Keep
 * invalid archived selections on the primary display instead of passing an
 * out-of-range index into several backend-specific queries. */
static int32_t coduomp_sdl_normalize_display_index_compat(
    int32_t displayIndex)
{
    const int32_t displayCount = SDL_GetNumVideoDisplays();

    return displayIndex >= 0 && displayIndex < displayCount
        ? displayIndex : 0;
}

/* NOT_FROM_ORIGINAL_SOURCE: exposes the active SDL output count to the
 * renderer and its separately linked graphics menu. */
int32_t coduomp_sdl_get_display_count_compat(void)
{
    const int32_t displayCount = SDL_GetNumVideoDisplays();

    return displayCount > 0 ? displayCount : 1;
}

void CoduoSDL_GetDesktopMode(int32_t displayIndex,
                             int32_t *width, int32_t *height,
                             int32_t *refreshRate)
{
    SDL_DisplayMode mode;

    displayIndex = coduomp_sdl_normalize_display_index_compat(displayIndex);
    memset(&mode, 0, sizeof(mode));
    if (SDL_GetCurrentDisplayMode(displayIndex, &mode) != 0) {
        *width = 0;
        *height = 0;
        *refreshRate = 0;
        return;
    }

    *width = mode.w;
    *height = mode.h;
    *refreshRate = mode.refresh_rate;
}


/* NOT_FROM_ORIGINAL_SOURCE: reports the selected display's hardware-sized
 * pixel mode. Desktop scaling remains a separate logical-window concern. */
qboolean coduomp_sdl_get_native_display_mode_compat(int32_t displayIndex,
                                                     int32_t *width,
                                                     int32_t *height,
                                                     int32_t *refreshRate)
{
    displayIndex = coduomp_sdl_normalize_display_index_compat(displayIndex);
    int32_t selectedWidth = 0;
    int32_t selectedHeight = 0;
    int32_t selectedRefreshRate = 0;
    const int modeCount = SDL_GetNumDisplayModes(displayIndex);

    /* SDL2 does not expose an output's preferred-mode marker. Select the
     * largest mode the active display actually enumerates instead of calling
     * the scaled/current desktop mode native. */
    for (int index = 0; index < modeCount; ++index) {
        SDL_DisplayMode mode;

        memset(&mode, 0, sizeof(mode));
        if (SDL_GetDisplayMode(displayIndex, index, &mode) != 0 ||
            mode.w <= 0 || mode.h <= 0) {
            continue;
        }
        if ((int64_t)mode.w * mode.h >
                (int64_t)selectedWidth * selectedHeight ||
            (mode.w == selectedWidth && mode.h == selectedHeight &&
             mode.refresh_rate > selectedRefreshRate)) {
            selectedWidth = mode.w;
            selectedHeight = mode.h;
            selectedRefreshRate = mode.refresh_rate;
        }
    }

    if (selectedWidth <= 0 || selectedHeight <= 0)
        return qfalse;
    *width = selectedWidth;
    *height = selectedHeight;
    *refreshRate = selectedRefreshRate;
    return qtrue;
}

/* NOT_FROM_ORIGINAL_SOURCE: distinguishes the selected macOS desktop's
 * logical coordinates, scaled backing surface, and physical-native panel
 * mode. Other SDL platforms report their current pixel desktop separately
 * from the largest enumerated hardware mode. */
qboolean coduomp_sdl_get_display_info_compat(
    int32_t displayIndex, coduomp_sdl_display_info_t *displayInfo)
{
    SDL_DisplayMode desktopMode;
    int32_t nativeRefreshRate = 0;

    if (displayInfo == NULL)
        return qfalse;
    memset(displayInfo, 0, sizeof(*displayInfo));
    displayIndex = coduomp_sdl_normalize_display_index_compat(displayIndex);
    memset(&desktopMode, 0, sizeof(desktopMode));
    if (SDL_GetCurrentDisplayMode(displayIndex, &desktopMode) != 0)
        return qfalse;

    displayInfo->logicalWidth = desktopMode.w;
    displayInfo->logicalHeight = desktopMode.h;
    displayInfo->backingWidth = desktopMode.w;
    displayInfo->backingHeight = desktopMode.h;
    displayInfo->refreshRate = desktopMode.refresh_rate;
    (void)coduomp_sdl_get_native_display_mode_compat(
        displayIndex, &displayInfo->nativeWidth,
        &displayInfo->nativeHeight, &nativeRefreshRate);
    if (displayInfo->refreshRate <= 0)
        displayInfo->refreshRate = nativeRefreshRate;

    return qtrue;
}

/* NOT_FROM_ORIGINAL_SOURCE: converts requested drawable pixels to SDL window
 * coordinates using the active macOS backing scale. Other SDL platforms use
 * pixel-sized window coordinates and leave the request unchanged. */
void coduomp_sdl_window_size_for_drawable_compat(int32_t displayIndex,
                                                  int32_t *width,
                                                  int32_t *height)
{
    (void)displayIndex;
    (void)width;
    (void)height;
}

/* NOT_FROM_ORIGINAL_SOURCE: reports whether the selected display exposes an
 * exact SDL fullscreen resolution. macOS display modes can include surfaces
 * larger than the physical panel, so cap candidates at the mode that Core
 * Graphics identifies as native before accepting SDL's mode entry. */
qboolean coduomp_sdl_display_mode_available_compat(int32_t displayIndex,
                                                    int32_t width,
                                                    int32_t height)
{
    if (width <= 0 || height <= 0)
        return qfalse;

    displayIndex = coduomp_sdl_normalize_display_index_compat(displayIndex);

    const int modeCount = SDL_GetNumDisplayModes(displayIndex);

    for (int index = 0; index < modeCount; ++index) {
        SDL_DisplayMode mode;

        memset(&mode, 0, sizeof(mode));
        if (SDL_GetDisplayMode(displayIndex, index, &mode) == 0 &&
            mode.w == width && mode.h == height) {
            return qtrue;
        }
    }
    return qfalse;
}

void CoduoSDL_GetFramebufferSize(int32_t *width, int32_t *height)
{
    if (coduoSdlWindow == NULL) {
        *width = 0;
        *height = 0;
        return;
    }
    SDL_GL_GetDrawableSize(coduoSdlWindow, width, height);
}

/* NOT_FROM_ORIGINAL_SOURCE: expose the SDL window's realized logical size for
 * resolution-independent console composition. On high-DPI macOS displays this
 * is measured in screen coordinates and can differ from drawable pixels. */
void coduomp_sdl_get_window_size_compat(int32_t *width, int32_t *height)
{
    if (coduoSdlWindow == NULL) {
        *width = 0;
        *height = 0;
        return;
    }
    SDL_GetWindowSize(coduoSdlWindow, width, height);
}

/* NOT_FROM_ORIGINAL_SOURCE: native display-gamma boundary used by the
 * recovered renderer's existing 256-entry RGB ramp. SDL selects the display
 * containing this window and maps the operation to the host window system. */
qboolean coduomp_sdl_get_window_gamma_ramp(uint16_t red[256],
                                           uint16_t green[256],
                                           uint16_t blue[256])
{
    if (coduoSdlWindow == NULL)
        return qfalse;

    return SDL_GetWindowGammaRamp(coduoSdlWindow, red, green, blue) == 0
               ? qtrue
               : qfalse;
}

/* NOT_FROM_ORIGINAL_SOURCE: install a renderer-generated gamma ramp on the
 * display containing the native game window. */
qboolean coduomp_sdl_set_window_gamma_ramp(const uint16_t red[256],
                                           const uint16_t green[256],
                                           const uint16_t blue[256])
{
    if (coduoSdlWindow == NULL)
        return qfalse;

    return SDL_SetWindowGammaRamp(coduoSdlWindow, red, green, blue) == 0
               ? qtrue
               : qfalse;
}

#if defined(__linux__)
/* NOT_FROM_ORIGINAL_SOURCE: expose the X11 handles already owned by SDL to
 * the compatibility-only XRandR gamma provider. No X11 types cross this
 * platform boundary, and Wayland windows fail the subsystem check. */
qboolean coduomp_sdl_get_x11_window_compat(
    void **display, unsigned long *window,
    int32_t *x, int32_t *y, int32_t *width, int32_t *height)
{
    SDL_SysWMinfo info;

    *display = NULL;
    *window = 0;
    *x = 0;
    *y = 0;
    *width = 0;
    *height = 0;
    if (coduoSdlWindow == NULL)
        return qfalse;

    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(coduoSdlWindow, &info) != SDL_TRUE ||
        info.subsystem != SDL_SYSWM_X11) {
        return qfalse;
    }

    *display = info.info.x11.display;
    *window = info.info.x11.window;
    SDL_GetWindowPosition(coduoSdlWindow, x, y);
    SDL_GetWindowSize(coduoSdlWindow, width, height);
    return qtrue;
}
#endif

/* NOT_FROM_ORIGINAL_SOURCE: retain SDL's diagnostic for a failed portable
 * display operation instead of reducing every backend failure to one generic
 * renderer message. */
const char *coduomp_sdl_error_compat(void)
{
    return SDL_GetError();
}

void CoduoSDL_GetOpenGLFormat(int32_t *colorBits, int32_t *depthBits,
                              int32_t *stencilBits)
{
    int redBits = 0;
    int greenBits = 0;
    int blueBits = 0;
    int alphaBits = 0;

    (void)SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &redBits);
    (void)SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &greenBits);
    (void)SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &blueBits);
    (void)SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &alphaBits);
    (void)SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, depthBits);
    (void)SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, stencilBits);
    *colorBits = redBits + greenBits + blueBits + alphaBits;
}

/* NOT_FROM_ORIGINAL_SOURCE: confines SDL's object-pointer carrier for OpenGL
 * entry points to the same typed-copy boundary used by the native loader. */
void CoduoSDL_GetOpenGLSymbol(const char *name, void *destination,
                              size_t destinationSize)
{
    void *symbol = SDL_GL_GetProcAddress(name);

    memset(destination, 0, destinationSize);
    if (destinationSize <= sizeof(symbol))
        memcpy(destination, &symbol, destinationSize);
}

void CoduoSDL_SwapWindow(void)
{
    if (coduoSdlWindow != NULL)
        SDL_GL_SwapWindow(coduoSdlWindow);
}

void CoduoSDL_SetSwapInterval(int32_t interval)
{
    if (coduoSdlGlContext != NULL)
        (void)SDL_GL_SetSwapInterval(interval);
}

void CoduoSDL_SetRelativeMouse(qboolean active)
{
    if (coduoSdlWindow == NULL)
        return;

    (void)SDL_SetRelativeMouseMode(
        active != qfalse ? SDL_TRUE : SDL_FALSE);
    SDL_SetWindowGrab(
        coduoSdlWindow, active != qfalse ? SDL_TRUE : SDL_FALSE);
}

qboolean CoduoSDL_HasOpenGLWindow(void)
{
    return coduoSdlWindow != NULL && coduoSdlGlContext != NULL;
}

/* NOT_FROM_ORIGINAL_SOURCE: portable replacement for the original Win32
 * MessageBoxA fatal-error boundary. */
void CoduoSDL_ShowErrorDialog(const char *message, const char *title)
{
    (void)SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR, title, message, coduoSdlWindow);
}

static int32_t CoduoSDL_MapKey(SDL_Keycode key)
{
    if (key >= SDLK_SPACE && key <= SDLK_z)
        return (int32_t)key;

    switch (key) {
    case SDLK_TAB: return K_TAB;
    case SDLK_RETURN: return K_ENTER;
    case SDLK_ESCAPE: return K_ESCAPE;
    case SDLK_BACKSPACE: return K_BACKSPACE;
    case SDLK_CAPSLOCK: return K_CAPSLOCK;
    case SDLK_PAUSE: return K_PAUSE;
    case SDLK_UP: return K_UPARROW;
    case SDLK_DOWN: return K_DOWNARROW;
    case SDLK_LEFT: return K_LEFTARROW;
    case SDLK_RIGHT: return K_RIGHTARROW;
    case SDLK_LALT:
    case SDLK_RALT: return K_ALT;
    case SDLK_LCTRL:
    case SDLK_RCTRL: return K_CTRL;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT: return K_SHIFT;
    case SDLK_LGUI:
    case SDLK_RGUI: return K_COMMAND;
    case SDLK_INSERT: return K_INS;
    case SDLK_DELETE: return K_DEL;
    case SDLK_PAGEDOWN: return K_PGDN;
    case SDLK_PAGEUP: return K_PGUP;
    case SDLK_HOME: return K_HOME;
    case SDLK_END: return K_END;
    case SDLK_F1: return K_F1;
    case SDLK_F2: return K_F2;
    case SDLK_F3: return K_F3;
    case SDLK_F4: return K_F4;
    case SDLK_F5: return K_F5;
    case SDLK_F6: return K_F6;
    case SDLK_F7: return K_F7;
    case SDLK_F8: return K_F8;
    case SDLK_F9: return K_F9;
    case SDLK_F10: return K_F10;
    case SDLK_F11: return K_F11;
    case SDLK_F12: return K_F12;
    case SDLK_KP_7: return K_KP_HOME;
    case SDLK_KP_8: return K_KP_UPARROW;
    case SDLK_KP_9: return K_KP_PGUP;
    case SDLK_KP_4: return K_KP_LEFTARROW;
    case SDLK_KP_5: return K_KP_5;
    case SDLK_KP_6: return K_KP_RIGHTARROW;
    case SDLK_KP_1: return K_KP_END;
    case SDLK_KP_2: return K_KP_DOWNARROW;
    case SDLK_KP_3: return K_KP_PGDN;
    case SDLK_KP_ENTER: return K_KP_ENTER;
    case SDLK_KP_0: return K_KP_INS;
    case SDLK_KP_PERIOD: return K_KP_DEL;
    case SDLK_KP_DIVIDE: return K_KP_SLASH;
    case SDLK_KP_MINUS: return K_KP_MINUS;
    case SDLK_KP_PLUS: return K_KP_PLUS;
    case SDLK_NUMLOCKCLEAR: return K_KP_NUMLOCK;
    case SDLK_KP_MULTIPLY: return K_KP_STAR;
    case SDLK_KP_EQUALS: return K_KP_EQUALS;
    default: return 0;
    }
}

static int32_t CoduoSDL_MapMouseButton(uint8_t button)
{
    switch (button) {
    case SDL_BUTTON_LEFT: return K_MOUSE1;
    case SDL_BUTTON_RIGHT: return K_MOUSE2;
    case SDL_BUTTON_MIDDLE: return K_MOUSE3;
    case SDL_BUTTON_X1: return K_MOUSE4;
    case SDL_BUTTON_X2: return K_MOUSE5;
    default: return 0;
    }
}


/* NOT_FROM_ORIGINAL_SOURCE: translates SDL's portable native events into the
 * recovered engine's original key/character/mouse event queue. */
void CoduoSDL_PumpEvents(void)
{
    SDL_Event event;

    if (coduoSdlInitialized == qfalse)
        return;

    /* NOT_FROM_ORIGINAL_SOURCE: keep each transfer batch small independently
     * of the ring capacity. Count produced engine events, not SDL records:
     * ignored records must not exhaust a batch with an empty engine queue.
     * Sys_GetEvent returns queued events immediately, and Com_EventLoop
     * resumes this pump after consuming them, before command creation. */
    const int32_t queueRemaining = coduomp_sys_event_queue_remaining();
    const int32_t batchReserve = queueRemaining > CODUO_SDL_EVENT_BATCH_COUNT
        ? queueRemaining - CODUO_SDL_EVENT_BATCH_COUNT : 0;

    for (;;) {
        /* NOT_FROM_ORIGINAL_SOURCE: one text event can expand to one engine
         * event per byte; all other translations emit at most two. Leave
         * unconsumed events in SDL when the engine queue needs draining.
         * Reconcile physical buttons only after the queued SDL transitions,
         * so a partial drain cannot reorder a later press or release. */
        if (coduomp_sys_event_queue_remaining() < batchReserve + SDL_TEXTINPUTEVENT_TEXT_SIZE)
            return;
        if (SDL_PollEvent(&event) == 0)
            break;

        /* NOT_FROM_ORIGINAL_SOURCE: SDL ticks and Sys_Milliseconds start at
         * different epochs. Key hold durations subtract event timestamps
         * from the engine frame clock, so preserve the event's age while
         * translating it into that clock's wrapping millisecond domain. */
        const uint32_t eventAge = SDL_GetTicks() - event.common.timestamp;
        sysMsgTime = (int32_t)(Sys_Milliseconds() - eventAge);

        switch (event.type) {
        case SDL_QUIT:
            Com_Quit_f();
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                Com_Quit_f();
            } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                coduomp_gamma_window_focus_changed(qtrue);
                AppActivate(qtrue, qfalse);
            } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                coduomp_gamma_window_focus_changed(qfalse);
                AppActivate(qfalse, qfalse);
                IN_ClearStates();
            }
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            const int32_t key = CoduoSDL_MapKey(event.key.keysym.sym);
            if (key != 0) {
                Sys_QueEvent(
                    sysMsgTime, SE_KEY, key,
                    event.type == SDL_KEYDOWN ? qtrue : qfalse,
                    0, NULL);
            }
            /* The original Win32 pump receives both WM_KEYDOWN/VK_BACK and
             * WM_CHAR/'\b'. SDL_TEXTINPUT does not report control characters,
             * so reproduce the missing character event at this platform
             * boundary. Field_KeyDownEvent deliberately does not edit on
             * K_BACKSPACE; Field_CharEvent performs the original deletion. */
            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_BACKSPACE) {
                Sys_QueEvent(
                    sysMsgTime, SE_CHAR, '\b', 0, 0, NULL);
            }
            /* COMPATIBILITY_PATCH (NOT_FROM_ORIGINAL_SOURCE): Win32 emits
             * character 22 for Ctrl-V, which the recovered field editor uses
             * to paste. SDL_TEXTINPUT omits shortcut control characters, so
             * synthesize that original event for Ctrl-V on Unix and for the
             * native Command-V chord on macOS. */
            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_v &&
                (event.key.keysym.mod & KMOD_CTRL) != 0) {
                Sys_QueEvent(
                    sysMsgTime, SE_CHAR, CODUO_SDL_CHAR_PASTE, 0, 0, NULL);
            }
            break;
        }

        case SDL_TEXTINPUT:
            for (const uint8_t *text =
                     (const uint8_t *)event.text.text;
                 *text != '\0'; ++text) {
                if (*text < 0x80U) {
                    Sys_QueEvent(
                        sysMsgTime, SE_CHAR, *text, 0, 0, NULL);
                }
            }
            break;

        case SDL_MOUSEMOTION:
            /* COMPATIBILITY_PATCH (NOT_FROM_ORIGINAL_SOURCE): SDL can report
             * desktop pointer motion after a fullscreen macOS window loses
             * focus. Win32 stops delivering the original window mouse
             * messages in that state. Preserve that boundary so inactive
             * motion cannot reach UI hover handling or gameplay input. */
            if (sysInputAppActive == qfalse)
                break;
            if (event.motion.xrel != 0 || event.motion.yrel != 0) {
                Sys_QueEvent(
                    sysMsgTime, SE_MOUSE,
                    event.motion.xrel, event.motion.yrel,
                    0, NULL);
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            if (sysInputAppActive == qfalse)
                break;
            const int32_t key =
                CoduoSDL_MapMouseButton(event.button.button);
            if (key != 0) {
                Sys_QueEvent(
                    sysMsgTime, SE_KEY, key,
                    event.type == SDL_MOUSEBUTTONDOWN ? qtrue : qfalse,
                    0, NULL);
            }
            break;
        }

        case SDL_MOUSEWHEEL: {
            if (sysInputAppActive == qfalse)
                break;
            int32_t wheelY = event.wheel.y;
            const int32_t key =
                wheelY > 0 ? K_MWHEELUP : K_MWHEELDOWN;
            if (wheelY != 0) {
                Sys_QueEvent(
                    sysMsgTime, SE_KEY, key, qtrue, 0, NULL);
                Sys_QueEvent(
                    sysMsgTime, SE_KEY, key, qfalse, 0, NULL);
            }
            break;
        }

        default:
            break;
        }
    }

}

