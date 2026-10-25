#include "system_platform.h"

#include "client/common/client_legacy_crt.h"
#include "client/cgame.h"
#include "filesystem/filesystem.h"
#include "qcommon/game_module_abi_types.h"
#include "networking/net_address.h"
#include "platform/dynamic_library_boundary.h"
#include "platform/sdl_platform.h"
#include "renderer/backend.h"
#include "renderer/renderer_api.h"
#include "system_console.h"
#include "system_event.h"
#include "system_fatal.h"
#include "system_input.h"
#include "system_localization.h"
#include "ui/ui_module_loader.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum {
    SYS_FUNCTION_SCAN_LIMIT = 1024,
    SYS_ERROR_TEXT_CAPACITY = 4096,
    SYS_CONSOLE_VISIBLE = 1,
    SYS_USER_NAME_CAPACITY = 1024,
    SYS_PROCESSOR_X86 = 1
};

void *sysSplashWindow;
qboolean sysCheckCrashOrRerun;
uint32_t sysExecutableChecksum;
static char sysCurrentUser[SYS_USER_NAME_CAPACITY];

void Sys_Init(void)
{
    sysCpuClass_t cpuClass;

    Cmd_AddCommand("in_restart", IN_Restart);
    Cmd_AddCommand("net_restart", NET_Restart_f);

    (void)Cvar_Get("sys_cpustring", "detect", 0);
    const char *const configuredCpu = Cvar_VariableString("sys_cpustring");
    if (Q_stricmp(configuredCpu, "detect") == 0) {
        Com_Printf("...detecting CPU, found ");
        cpuClass = Sys_DetectCpuClass();
        switch (cpuClass) {
        case CPUID_GENERIC:
            (void)Cvar_Set2("sys_cpustring", "generic", qtrue);
            break;
        case CPUID_AXP:
            (void)Cvar_Set2("sys_cpustring", "Alpha AXP", qtrue);
            break;
        case CPUID_INTEL_UNSUPPORTED:
            (void)Cvar_Set2(
                "sys_cpustring", "x86 (pre-Pentium)", qtrue);
            break;
        case CPUID_INTEL_PENTIUM:
            (void)Cvar_Set2(
                "sys_cpustring", "x86 (P5/PPro, non-MMX)", qtrue);
            break;
        case CPUID_INTEL_MMX:
            (void)Cvar_Set2(
                "sys_cpustring", "x86 (P5/Pentium2, MMX)", qtrue);
            break;
        case CPUID_INTEL_KATMAI:
            (void)Cvar_Set2(
                "sys_cpustring", "Intel Pentium III", qtrue);
            break;
        case CPUID_AMD_3DNOW:
            (void)Cvar_Set2("sys_cpustring", "AMD w/ 3DNow!", qtrue);
            break;
        default:
            Com_Error(
                ERR_FATAL, "\x15Unknown cpu type %d\n", cpuClass);
            break;
        }
    } else {
        Com_Printf("...forcing CPU type to ");
        if (Q_stricmp(configuredCpu, "generic") == 0)
            cpuClass = CPUID_GENERIC;
        else if (Q_stricmp(configuredCpu, "x87") == 0)
            cpuClass = CPUID_INTEL_PENTIUM;
        else if (Q_stricmp(configuredCpu, "mmx") == 0)
            cpuClass = CPUID_INTEL_MMX;
        else if (Q_stricmp(configuredCpu, "3dnow") == 0)
            cpuClass = CPUID_AMD_3DNOW;
        else if (Q_stricmp(configuredCpu, "PentiumIII") == 0)
            cpuClass = CPUID_INTEL_KATMAI;
        else if (Q_stricmp(configuredCpu, "axp") == 0)
            cpuClass = CPUID_AXP;
        else {
            Com_Printf(
                "WARNING: unknown sys_cpustring '%s'\n", configuredCpu);
            cpuClass = CPUID_GENERIC;
        }
    }

    Cvar_SetValue("sys_cpuid", (float)cpuClass);
    Com_Printf("%s\n", Cvar_VariableString("sys_cpustring"));
    Com_Printf(
        "Measured CPU speed is %.2lf GHz\n",
        sysCpuFrequencyMHz * 0.001);
    Com_Printf(
        "System memory is %i MB (capped at 1 GB)\n",
        sysPhysicalMemoryMB);
    Com_Printf("Video card memory is %i MB\n", sysVideoMemoryMB);
    Com_Printf(
        "Streaming SIMD Extensions (SSE) %ssupported\n",
        sysSseSupported != qfalse ? "" : "not ");
    Com_Printf("\n");
    (void)Cvar_Set2("username", Sys_GetCurrentUser(), qtrue);
    IN_Init();
}

static const uint8_t *coduomp_resolve_function_entry(
    const uint8_t *function)
{
    int32_t displacement;

    if (function[0] != 0xe9)
        return function;

    memcpy(&displacement, &function[1], sizeof(displacement));
    return function + 5 + displacement;
}

static int32_t coduomp_function_comparison_length(
    const uint8_t *function)
{
    int32_t offset;

    for (offset = 0; offset < SYS_FUNCTION_SCAN_LIMIT; ++offset) {
        if (function[offset] == 0xc3 &&
            function[offset + 1] == 0x90 &&
            function[offset + 2] == 0x90) {
            break;
        }
    }
    return offset + 2;
}

void Sys_CreateCrashMarker(void)
{
    const int marker =
        open("crash", O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (marker != -1)
        close(marker);
}

void Sys_CreateSplashWindow(void)
{
}

void Sys_DestroySplashWindow(void)
{
}

void Sys_ShowSplashWindow(void)
{
}

void Sys_HideSplashWindow(void)
{
}

qboolean Sys_FunctionsMatch(const uint8_t *leftFunction,
                            const uint8_t *rightFunction)
{
    const uint8_t *left =
        coduomp_resolve_function_entry(leftFunction);
    const uint8_t *right =
        coduomp_resolve_function_entry(rightFunction);
    const int32_t length =
        coduomp_function_comparison_length(left);

    for (int32_t offset = 0; offset < length; ++offset) {
        if (left[offset] == 0xe8) {
            int32_t leftDisplacement;
            int32_t rightDisplacement;
            memcpy(&leftDisplacement, &left[offset + 1],
                   sizeof(leftDisplacement));
            memcpy(&rightDisplacement, &right[offset + 1],
                   sizeof(rightDisplacement));

            const uintptr_t leftTarget =
                (uintptr_t)&left[offset + 5] +
                (intptr_t)leftDisplacement;
            const uintptr_t rightTarget =
                (uintptr_t)&right[offset + 5] +
                (intptr_t)rightDisplacement;
            if (leftTarget == rightTarget) {
                offset += 4;
                continue;
            }
        }

        if (left[offset] != right[offset])
            return qfalse;
    }

    return qtrue;
}

uint32_t Sys_FunctionChecksum(const uint8_t *function)
{
    const uint8_t *entry =
        coduomp_resolve_function_entry(function);
    return Com_BlockChecksum(
        entry, coduomp_function_comparison_length(entry));
}

qboolean Sys_CheckCrashOrRerun(void)
{
    return sysCheckCrashOrRerun;
}

void Sys_Shutdown(void)
{
    Sys_DestroySplashWindow();
    IN_Shutdown();
    CL_ShutdownCGame();
    CL_ShutdownUI();
    CL_ShutdownRef();
}

void Sys_UnableToLoadDLLError(void)
{
    Com_Error(ERR_FATAL, "%s\n",
              Sys_LocalizeString("WIN_UNABLE_LOAD_DLL_BODY"));
}

void Sys_StartProcess(const char *executableName, qboolean doExit)
{
    char workingDirectory[MAX_OSPATH];
    char executablePath[2 * MAX_OSPATH];

    if (getcwd(workingDirectory, sizeof(workingDirectory)) == NULL) {
        Com_Error(ERR_FATAL,
                  "EXE_ERR_COULDNT_START_PROCESS\x15'%s'\n%s\n%08x",
                  executableName, strerror(errno), (uint32_t)errno);
    }
    Com_sprintf(executablePath, sizeof(executablePath), "%s/%s",
                workingDirectory, executableName);

    const pid_t child = fork();
    if (child == 0) {
        execl(executablePath, executablePath, (char *)NULL);
        _exit(127);
    }
    if (child < 0) {
        Com_Error(ERR_FATAL,
                  "EXE_ERR_COULDNT_START_PROCESS\x15'%s/%s'\n%s\n%08x",
                  workingDirectory, executableName, strerror(errno),
                  (uint32_t)errno);
    }

    if (doExit != qfalse)
        Cbuf_AddText("quit\n");
}

void Sys_OpenURL(const char *url, qboolean doExit)
{
    const pid_t child = fork();
    if (child == 0) {
        execlp("xdg-open", "xdg-open", url, (char *)NULL);
        _exit(127);
    }
    if (child < 0)
        Com_Error(1, "EXE_ERR_COULDNT_OPEN_URL\x15%s", url);

    if (doExit != qfalse)
        Cbuf_AddText("quit\n");
}

void Sys_LoadingKeepAlive(void)
{
}

void coduomp_loading_keepalive(void)
{
}

_Noreturn void Sys_Error(const char *format, ...)
{
    char errorText[SYS_ERROR_TEXT_CAPACITY];
    va_list arguments;

    va_start(arguments, format);
    (void)vsnprintf(errorText, sizeof(errorText), format, arguments);
    va_end(arguments);

    Sys_Shutdown();
    RestoreSystemGammas();
    Sys_Print(errorText);
    Sys_Print("\n");

    Com_Quit_f();
}

_Noreturn void Sys_Quit(void)
{
    IN_Shutdown();
    Sys_ShutdownLocalization();
    Cvar_Shutdown();
    Cmd_Shutdown();
    Com_InitPushEvent();
    Sys_ClearEventQueue();
    Key_Shutdown();
    CoduoSDL_Shutdown();
    exit(EXIT_SUCCESS);
}

void Sys_Warning(const char *text)
{
    Sys_Print(text);
}

qboolean Sys_ScanForInstallMedia(void)
{
    return qfalse;
}

int32_t Sys_MonkeyShouldBeSpanked(void)
{
    return 0;
}

int32_t Sys_GetProcessorId(void)
{
    return SYS_PROCESSOR_X86;
}

char *Sys_GetCurrentUser(void)
{
    const char *const user = getenv("USER");
    if (user != NULL)
        Q_strncpyz(sysCurrentUser, user, sizeof(sysCurrentUser));
    else
        sysCurrentUser[0] = '\0';

    if (sysCurrentUser[0] == '\0')
        memcpy(sysCurrentUser, "player", sizeof("player"));
    return sysCurrentUser;
}

char *Sys_GetClipboardData(void)
{
    char *const clipboardText = coduomp_sdl_get_clipboard_text_compat();
    if (clipboardText != NULL)
        (void)strtok(clipboardText, "\n\r\b");
    return clipboardText;
}

int32_t Sys_UnloadDll(void *libraryHandle)
{
    if (libraryHandle == NULL)
        return qfalse;

    if (coduomp_library_close(libraryHandle) != 0)
        return qtrue;

    Com_Error(ERR_FATAL, "\x15"
              "Sys_UnloadDll FreeLibrary failed");
    return qfalse;
}

static qboolean Sys_FilesMatch(const char *virtualPath, const char *osPath)
{
    void *virtualBytes;
    const int32_t virtualSize = FS_ReadFile(virtualPath, &virtualBytes);
    if (virtualSize == -1)
        return qfalse;

    FILE *const file = fopen(osPath, "rb");
    if (file == NULL) {
        FS_FreeFile(virtualBytes);
        return qfalse;
    }

    fseek(file, 0, SEEK_END);
    const int32_t osSize = (int32_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    qboolean matches = qfalse;
    if (osSize > 0) {
        const size_t transferSize = (size_t)(uint32_t)osSize;
        void *const osBytes = Z_MallocInternal(transferSize);
        const size_t bytesRead = fread(osBytes, 1, transferSize, file);
        if (bytesRead == transferSize && osSize == virtualSize &&
            memcmp(virtualBytes, osBytes, transferSize) == 0) {
            matches = qtrue;
        }
        free(osBytes);
    }

    fclose(file);
    FS_FreeFile(virtualBytes);
    return matches;
}

static qboolean coduomp_is_reconstructed_client_module(
    const char *moduleName)
{
    if (strcmp(moduleName, "cgame") == 0 ||
        strcmp(moduleName, "ui") == 0) {
        return qtrue;
    }
    return qfalse;
}

void *Sys_LoadDll(const char *moduleName, char *loadedPath,
                  vmMain_t *vmMain,
                  coduo_module_syscall_t moduleSyscall)
{
    char dllName[MAX_QPATH];
    char pureIdentityName[MAX_QPATH];
    char dllPath[MAX_OSPATH];
    const char *const emptyString = "";
    const cvar_t *cvar;
    const char *basePath;
    const char *baseGame;
    const char *game;
    const char *loadedFile;
    const qboolean reconstructedClientModule =
        coduomp_is_reconstructed_client_module(moduleName);
    void *libraryHandle;

    loadedPath[0] = '\0';
    Com_sprintf(pureIdentityName, sizeof(pureIdentityName),
                "uo_%s_mp_x86.dll", moduleName);
#if defined(__i386__)
    Com_sprintf(
        dllName, sizeof(dllName), "uo_%s_mp_x86.so", moduleName);
#else
    Com_sprintf(
        dllName, sizeof(dllName), "uo_%s_mp_x86_64.so", moduleName);
#endif

    cvar = Cvar_FindVar("fs_basepath");
    basePath = cvar != NULL ? cvar->string : emptyString;
    (void)Cvar_FindVar("fs_cdpath");
    cvar = Cvar_FindVar("fs_basegame");
    baseGame = cvar != NULL ? cvar->string : emptyString;
    cvar = Cvar_FindVar("fs_game");
    game = cvar != NULL ? cvar->string : emptyString;

    FS_BuildOSPath_Internal(basePath, game, dllName, dllPath, qfalse);

    if (Q_strncmp(moduleName, "game", 4) != 0) {
        int32_t fileHandle;
        qboolean packagedCopyMatches = qfalse;
        const char *const packagedModuleName =
            reconstructedClientModule != qfalse
                ? pureIdentityName
                : dllName;

        fs_fileAccessed = 1;
        if (FS_FOpenFileRead(packagedModuleName,
                             &fileHandle, qfalse) >= 0) {
            pack_t *const pack = fs_handleFiles[fileHandle].zipArchive;
            FS_FCloseFile(fileHandle);
            if (pack != NULL &&
                reconstructedClientModule == qfalse) {
                FS_BuildOSPath_Internal(
                    basePath, pack->pakGamename, dllName, dllPath, qfalse);
                packagedCopyMatches = Sys_FilesMatch(dllName, dllPath);
            }
        }

        if (packagedCopyMatches == qfalse) {
            if (cl_connectedToPureServer != qfalse &&
                reconstructedClientModule == qfalse) {
                Com_Error(1, "EXE_ERR_GAME_FAILED_PURE_CHECK\x15%s",
                          dllName);
            }
            FS_BuildOSPath_Internal(
                basePath,
                reconstructedClientModule != qfalse
                    ? "main"
                    : game,
                dllName, dllPath, qfalse);
        }
    }

    libraryHandle = NULL;
    if (libraryHandle == NULL) {
        loadedFile = dllPath;
        libraryHandle = coduomp_library_open(loadedFile);
    }
    if (libraryHandle == NULL) {
        FS_BuildOSPath_Internal(
            basePath, baseGame, dllName, dllPath, qfalse);
        libraryHandle = coduomp_library_open(dllPath);
    }
    if (libraryHandle == NULL) {
        FS_BuildOSPath_Internal(
            basePath, "main", dllName, dllPath, qfalse);
        libraryHandle = coduomp_library_open(dllPath);
    }
    if (libraryHandle == NULL) {
        loadedFile = dllName;
        libraryHandle = coduomp_library_open(loadedFile);
    }
    if (libraryHandle == NULL)
        return NULL;

    strncpy(loadedPath, loadedFile, 63);
    loadedPath[63] = '\0';

    coduo_dll_entry_t dllEntry = NULL;
    coduomp_library_symbol(
        libraryHandle, "dllEntry", &dllEntry, sizeof(dllEntry));
    coduomp_library_symbol(
        libraryHandle, "vmMain", vmMain, sizeof(*vmMain));
    if (*vmMain == NULL || dllEntry == NULL) {
        (void)coduomp_library_close(libraryHandle);
        return NULL;
    }

#if UINTPTR_MAX == UINT32_MAX
    dllEntry(moduleSyscall);
#else
    if (strcmp(moduleName, "game") == 0) {
        game_dll_entry_t gameDllEntry;
        game_syscall_t gameSyscall;

        _Static_assert(sizeof(gameDllEntry) == sizeof(dllEntry),
                       "native dllEntry pointer size mismatch");
        _Static_assert(sizeof(gameSyscall) == sizeof(moduleSyscall),
                       "native syscall pointer size mismatch");
        memcpy(&gameDllEntry, &dllEntry, sizeof(gameDllEntry));
        memcpy(&gameSyscall, &moduleSyscall, sizeof(gameSyscall));
        gameDllEntry(gameSyscall);
    } else {
        dllEntry(moduleSyscall);
    }
#endif
    return libraryHandle;
}

uint32_t Sys_GetExecutableChecksum(const void *imageBase)
{
    (void)imageBase;
    return 0;
}
