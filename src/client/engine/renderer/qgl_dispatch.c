#include "gl_debug.h"
#include "gl_state.h"
#include "wgl_debug.h"

#include "../platform/dynamic_library_boundary.h"
#include "../platform/sdl_platform.h"

#include <string.h>

/* The original linker placed the 508 underlying driver pointers and the 508
 * public QGL/WGL dispatch pointers in one contiguous PE data range at
 * 0x04898c98..0x04899c74. Their source declarations remain individually
 * typed; the entry inventories keep definition, shutdown, and later loading
 * coverage synchronized without treating function pointers as object data. */
#define QGL_GL_ENTRY(type_, name_) \
    type_ rendererGl##name_##Driver; \
    type_ qgl##name_;
#include "qgl_gl_entries.h"
#undef QGL_GL_ENTRY

#define QGL_WGL_ENTRY(type_, name_) \
    type_ rendererWgl##name_##Driver; \
    type_ qwgl##name_;
#include "qgl_wgl_entries.h"
#undef QGL_WGL_ENTRY

void *rendererGlLibrary;

static renderer_wgl_proc_t QGL_SDLGetProcAddress(const char *name)
{
    renderer_wgl_proc_t function = NULL;

    CoduoSDL_GetOpenGLSymbol(name, &function, sizeof(function));
    return function;
}

/* Source: CoDUOMP.exe 0x004f4e00..0x004f5316.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_004f4e00_004f5317.mcode.
 * Role name: GLimp_Extensions calls this after accepting
 * GL_ARB_vertex_program. The original resolves these 62 entry points in the
 * exact inventory order and installs each result in both the driver and
 * public dispatch slots. */
void QGL_LoadARBVertexProgramFunctions(void)
{
#define QGL_ARB_VERTEX_PROGRAM_ENTRY(type_, name_) \
    rendererGl##name_##Driver = \
        (type_)qwglGetProcAddress("gl" #name_); \
    qgl##name_ = rendererGl##name_##Driver;
#include "qgl_arb_vertex_program_entries.h"
#undef QGL_ARB_VERTEX_PROGRAM_ENTRY
}

/* Source: CoDUOMP.exe 0x004d7000..0x004d880e.
 * The Windows routine explicitly nulls each of the 1,016 typed dispatch
 * globals after releasing opengl32.dll. The inventories expand to those same
 * assignments while retaining native function-pointer types on every host. */
void QGL_Shutdown(void)
{
    ri.Printf(R_PRINT_ALL, "...shutting down QGL\n");

    if (rendererGlLibrary != NULL) {
        ri.Printf(R_PRINT_ALL, "...unloading OpenGL DLL\n");
        coduomp_library_close(rendererGlLibrary);
    }
    rendererGlLibrary = NULL;

#define QGL_GL_ENTRY(type_, name_) \
    rendererGl##name_##Driver = NULL; \
    qgl##name_ = NULL;
#include "qgl_gl_entries.h"
#undef QGL_GL_ENTRY

#define QGL_WGL_ENTRY(type_, name_) \
    rendererWgl##name_##Driver = NULL; \
    qwgl##name_ = NULL;
#include "qgl_wgl_entries.h"
#undef QGL_WGL_ENTRY
}

/* Source: CoDUOMP.exe 0x004d8810..0x004db12e.
 * QGL_Init resolves exactly 336 OpenGL 1.1 and 16 core WGL names. Each result
 * is stored in both the underlying driver slot and its public dispatch slot.
 * Before those loads, it nulls the 143 non-core GL pairs; the 13 non-core WGL
 * pairs are left untouched. The literal strstr("dllname", ".dll") is an
 * original source quirk visible in the executable and intentionally retained. */
qboolean QGL_Init(const char *dllName)
{
    int32_t logLevel;

    (void)dllName;
    ri.Printf(
        R_PRINT_ALL,
        "...initializing QGL through the native SDL OpenGL context\n");

#define QGL_GL_ENTRY(type_, name_) \
    rendererGl##name_##Driver = NULL; \
    qgl##name_ = NULL;
#include "qgl_gl_entries.h"
#undef QGL_GL_ENTRY

#define QGL_WGL_ENTRY(type_, name_) \
    rendererWgl##name_##Driver = NULL; \
    qwgl##name_ = NULL;
#include "qgl_wgl_entries.h"
#undef QGL_WGL_ENTRY

#define QGL_CORE_GL_ENTRY(type_, name_) do { \
    CoduoSDL_GetOpenGLSymbol("gl" #name_, \
                             &rendererGl##name_##Driver, \
                             sizeof(rendererGl##name_##Driver)); \
    qgl##name_ = rendererGl##name_##Driver; \
} while (0);
#define QGL_CORE_WGL_ENTRY(type_, name_) do { \
    rendererWgl##name_##Driver = NULL; \
    qwgl##name_ = NULL; \
} while (0);
#include "qgl_core_entries.h"
#undef QGL_CORE_WGL_ENTRY
#undef QGL_CORE_GL_ENTRY

    rendererWglGetProcAddressDriver = QGL_SDLGetProcAddress;
    qwglGetProcAddress = QGL_SDLGetProcAddress;
    if (qglGetString == NULL || qglGetIntegerv == NULL) {
        ri.Printf(
            R_PRINT_ALL,
            "...native OpenGL context is missing core 1.1 entry points\n");
        QGL_Shutdown();
        return qfalse;
    }

    logLevel = r_logFile->integer;
    QGL_EnableErrorChecking(logLevel == 0 &&
                            r_debugGLErrors->integer != 0);
    QGL_EnableLogging(logLevel);
    return qtrue;
}
