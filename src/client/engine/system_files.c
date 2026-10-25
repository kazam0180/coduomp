#include "q_shared.h"
#include "platform/crt_boundary.h"

#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum {
    SYS_FILE_CASE_COMPARE_LIMIT = 99999,
    SYS_LISTFILES_MAX_COUNT = 4095,
    SYS_LISTFILES_STACK_CAPACITY = SYS_LISTFILES_MAX_COUNT + 1
};

static char sysCurrentWorkingDirectory[MAX_OSPATH];
static char sysDefaultHomePath[MAX_OSPATH];

void Sys_Mkdir(const char *path)
{
    (void)coduomp_crt_mkdir(path);
}

const char *Sys_Cwd(void)
{
    if (coduomp_crt_getcwd(sysCurrentWorkingDirectory,
                           sizeof(sysCurrentWorkingDirectory) - 1u) == NULL) {
        sysCurrentWorkingDirectory[0] = '.';
        sysCurrentWorkingDirectory[1] = '\0';
    }
    sysCurrentWorkingDirectory[
        sizeof(sysCurrentWorkingDirectory) - 1u] = '\0';
    return sysCurrentWorkingDirectory;
}

const char *Sys_DefaultCDPath(void)
{
    return "";
}

const char *Sys_DefaultBasePath(void)
{
    return Sys_Cwd();
}

const char *Sys_DefaultHomePath(void)
{
    const char *const home = getenv("HOME");
    if (home == NULL || home[0] == '\0')
        return NULL;

    const char *const xdgDataHome = getenv("XDG_DATA_HOME");
    if (xdgDataHome != NULL && xdgDataHome[0] == '/') {
        Com_sprintf(sysDefaultHomePath, sizeof(sysDefaultHomePath),
                    "%s/opencoduo", xdgDataHome);
    } else {
        Com_sprintf(sysDefaultHomePath, sizeof(sysDefaultHomePath),
                    "%s/.local/share/opencoduo", home);
    }

    return sysDefaultHomePath;
}

const char *Sys_DefaultInstallPath(void)
{
    return Sys_Cwd();
}

static qboolean Sys_IsManagedDirectoryName(const char *name)
{
    return (Q_stricmpn(name, ".", SYS_FILE_CASE_COMPARE_LIMIT) == 0 ||
            Q_stricmpn(name, "..", SYS_FILE_CASE_COMPARE_LIMIT) == 0 ||
            Q_stricmpn(name, "CVS", SYS_FILE_CASE_COMPARE_LIMIT) == 0)
               ? qtrue
               : qfalse;
}

static qboolean Sys_PosixPathIsDirectory(const char *directory,
                                         const char *name)
{
    char path[MAX_OSPATH];
    struct stat status;

    Com_sprintf(path, sizeof(path), "%s/%s", directory, name);
    return stat(path, &status) == 0 && S_ISDIR(status.st_mode) ? qtrue
                                                               : qfalse;
}

static qboolean Sys_PosixNameHasExtension(const char *name,
                                          const char *extension)
{
    const size_t nameLength = strlen(name);
    const size_t extensionLength = strlen(extension);

    if (extensionLength == 0)
        return qtrue;
    if (nameLength < extensionLength)
        return qfalse;
    return Q_stricmpn(name + nameLength - extensionLength, extension,
                      SYS_FILE_CASE_COMPARE_LIMIT) == 0
               ? qtrue
               : qfalse;
}

void Sys_ListFilteredFiles(const char *directory, const char *subdirectory,
                           const char *filter, char **list,
                           int32_t *numFiles)
{
    char searchPath[MAX_OSPATH];
    char childSubdirectory[MAX_OSPATH];
    char relativePath[MAX_OSPATH];

    if (*numFiles >= SYS_LISTFILES_MAX_COUNT)
        return;

    DIR *handle;

    if (subdirectory[0] != '\0') {
        Com_sprintf(searchPath, sizeof(searchPath), "%s/%s", directory,
                    subdirectory);
    } else {
        Com_sprintf(searchPath, sizeof(searchPath), "%s", directory);
    }

    handle = opendir(searchPath);
    if (handle == NULL)
        return;

    for (;;) {
        const struct dirent *entry = readdir(handle);

        if (entry == NULL)
            break;
        if (Sys_PosixPathIsDirectory(searchPath, entry->d_name) != qfalse &&
            Sys_IsManagedDirectoryName(entry->d_name) == qfalse) {
            if (subdirectory[0] != '\0') {
                Com_sprintf(childSubdirectory, sizeof(childSubdirectory),
                            "%s/%s", subdirectory, entry->d_name);
            } else {
                Com_sprintf(childSubdirectory, sizeof(childSubdirectory),
                            "%s", entry->d_name);
            }
            Sys_ListFilteredFiles(directory, childSubdirectory, filter, list,
                                  numFiles);
        }

        if (*numFiles >= SYS_LISTFILES_MAX_COUNT)
            break;

        Com_sprintf(relativePath, sizeof(relativePath), "%s/%s",
                    subdirectory, entry->d_name);
        if (Com_FilterPath(filter, relativePath, qfalse) != qfalse) {
            list[*numFiles] = CopyStringInternal(relativePath);
            ++*numFiles;
        }
    }

    closedir(handle);
}

char **Sys_ListFiles(const char *directory, const char *extension,
                     const char *filter, int32_t *numFiles,
                     qboolean wantDirectories)
{
    char *list[SYS_LISTFILES_STACK_CAPACITY];
    int32_t count = 0;

    if (filter != NULL) {
        Sys_ListFilteredFiles(directory, "", filter, list, &count);
    } else {
        qboolean listDirectories = wantDirectories;

        if (extension == NULL)
            extension = "";
        if (extension[0] == '/' && extension[1] == '\0') {
            extension = "";
            listDirectories = qtrue;
        }

        DIR *handle = opendir(directory);

        if (handle == NULL) {
            *numFiles = 0;
            return NULL;
        }

        for (;;) {
            const struct dirent *entry = readdir(handle);
            qboolean isDirectory;

            if (entry == NULL)
                break;
            isDirectory =
                Sys_PosixPathIsDirectory(directory, entry->d_name);
            if ((listDirectories != qfalse) != (isDirectory != qfalse))
                continue;
            if (isDirectory != qfalse &&
                Sys_IsManagedDirectoryName(entry->d_name) != qfalse)
                continue;
            if (Sys_PosixNameHasExtension(entry->d_name, extension) ==
                qfalse) {
                continue;
            }
            list[count++] = CopyStringInternal(entry->d_name);
            if (count == SYS_LISTFILES_MAX_COUNT)
                break;
        }

        closedir(handle);
    }

    list[count] = NULL;
    *numFiles = count;
    if (count == 0)
        return NULL;

    char **result = Z_MallocInternal(
        ((size_t)count + 1U) * sizeof(result[0]));
    memcpy(result, list, (size_t)count * sizeof(result[0]));
    result[count] = NULL;
    return result;
}

void Sys_FreeFileList(char **list)
{
    if (list == NULL)
        return;

    for (char **entry = list; *entry != NULL; ++entry)
        Z_FreeInternal(*entry);
    Z_FreeInternal(list);
}
