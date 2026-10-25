#include "filesystem.h"

#include "qcommon/com_sprintf.h"
#include "qcommon/q_string.h"

#include <dirent.h>
#include <string.h>

enum {
    FS_DIRECTORY_NAME_COMPARE_LIMIT = 99999
};

/*
 * Source: CoDUOMP.exe 0x00468dc0..0x00468eaf and coduo_lnxded
 * 0x080c82c8..0x080c835b.  The Windows body ignores the three managed names
 * ".", "..", and "CVS" with case-insensitive comparisons; Linux ignores only
 * "." and ".." with exact comparisons.
 */
#if defined(WINDOWS_BEHAVIOR)
qboolean FS_DirectoryHasNonDotEntries(const char *path)
{
    DIR *const directory = opendir(path);

    if (directory == NULL)
        return qfalse;

    for (;;) {
        const struct dirent *const entry = readdir(directory);
        if (entry == NULL)
            break;
        if (Q_stricmpn(entry->d_name, ".",
                       FS_DIRECTORY_NAME_COMPARE_LIMIT) != 0 &&
            Q_stricmpn(entry->d_name, "..",
                       FS_DIRECTORY_NAME_COMPARE_LIMIT) != 0 &&
            Q_stricmpn(entry->d_name, "CVS",
                       FS_DIRECTORY_NAME_COMPARE_LIMIT) != 0) {
            (void)closedir(directory);
            return qtrue;
        }
    }

    (void)closedir(directory);
    return qfalse;
}
#else
qboolean FS_DirectoryHasNonDotEntries(const char *path)
{
    DIR *const directory = opendir(path);
    qboolean hasEntries = qfalse;

    if (directory == NULL)
        return qfalse;

    for (;;) {
        const struct dirent *const entry = readdir(directory);
        if (entry == NULL)
            break;
        if (strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {
            hasEntries = qtrue;
            break;
        }
    }

    (void)closedir(directory);
    return hasEntries;
}
#endif
