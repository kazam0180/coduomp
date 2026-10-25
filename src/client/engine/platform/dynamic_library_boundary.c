#include "dynamic_library_boundary.h"

#include <string.h>
#include <dlfcn.h>
#include <errno.h>

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

void *coduomp_library_open(const char *libraryName)
{
    return dlopen(libraryName, RTLD_NOW | RTLD_LOCAL);
}

void coduomp_library_symbol(void *libraryHandle, const char *symbolName,
                            void *destination, size_t destinationSize)
{
    void *symbol = dlsym(libraryHandle, symbolName);

    memset(destination, 0, destinationSize);
    if (destinationSize <= sizeof(symbol))
        memcpy(destination, &symbol, destinationSize);
}

int32_t coduomp_library_close(void *libraryHandle)
{
    return dlclose(libraryHandle) == 0;
}

uint32_t coduomp_platform_last_error(void)
{
    return (uint32_t)errno;
}
