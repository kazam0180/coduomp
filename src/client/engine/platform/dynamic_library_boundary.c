#include "dynamic_library_boundary.h"

#include <string.h>

#include <dlfcn.h>
#include <errno.h>


/* NOT_FROM_ORIGINAL_SOURCE: portable boundary for the Win32
 * GetSystemDirectoryA import used by QGL_Init only to construct its diagnostic
 * library path. The actual LoadLibrary call receives the caller's unmodified
 * name. Native targets report their conventional system-library directory. */
void coduomp_system_library_directory(char *buffer, size_t bufferSize)
{
    const char *directory;
    size_t length;

    if (bufferSize == 0)
        return;

    directory = "/usr/lib";

    length = strlen(directory);
    if (length >= bufferSize)
        length = bufferSize - 1;
    memcpy(buffer, directory, length);
    buffer[length] = '\0';
}

/* NOT_FROM_ORIGINAL_SOURCE: portable boundary for the Win32 LoadLibraryA
 * import at CoDUOMP.exe 0x004d88bf. The retail call site performs no post-load
 * x87 adjustment, and retail cgame/UI/listen-server game process attach skips
 * each module's precision helper. MinGW i686's TLS callback calls __fpreset for
 * DLL_THREAD_ATTACH, not DLL_PROCESS_ATTACH, so this boundary must not alter
 * the caller thread's control word after a successful load. */
void *coduomp_library_open(const char *libraryName)
{
    return dlopen(libraryName, RTLD_NOW | RTLD_LOCAL);
}

/* NOT_FROM_ORIGINAL_SOURCE: typed function-pointer destinations cannot be
 * populated portably through an object-pointer cast. Copying the native
 * loader's symbol carrier into the caller's typed slot preserves its bits and
 * keeps that conversion confined to this explicit dynamic-library boundary. */
void coduomp_library_symbol(void *libraryHandle, const char *symbolName,
                            void *destination, size_t destinationSize)
{
    void *symbol = dlsym(libraryHandle, symbolName);

    memset(destination, 0, destinationSize);
    if (destinationSize <= sizeof(symbol))
        memcpy(destination, &symbol, destinationSize);
}

/* NOT_FROM_ORIGINAL_SOURCE: portable system boundary for the Win32
 * FreeLibrary import. QGL_Shutdown ignores the result at CoDUOMP.exe
 * 0x004d7030, while Sys_UnloadDll checks it at 0x0046b745. */
int32_t coduomp_library_close(void *libraryHandle)
{
    return dlclose(libraryHandle) == 0;
}

/* NOT_FROM_ORIGINAL_SOURCE: portable boundary for the Win32 GetLastError IAT
 * calls made by the checked WGL wrappers. Non-Windows platform loaders expose
 * their corresponding thread-local loader error through errno. */
uint32_t coduomp_platform_last_error(void)
{
    return (uint32_t)errno;
}
