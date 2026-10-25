#include "system_input.h"

#include "client/console.h"
#include "platform/crt_boundary.h"
#include "platform/sdl_platform.h"
#include "system_event.h"

#include <string.h>

cvar_t *in_mouse; /* original 0x048a5484 */
cvar_t *in_midi; /* original 0x048a5480 */
cvar_t *in_midiport; /* original 0x048a54a4 */
cvar_t *in_midichannel; /* original 0x048a5488 */
cvar_t *in_mididevice; /* original 0x048a54a0 */
cvar_t *in_joystick; /* original 0x048a548c */
cvar_t *in_joyBallScale; /* original 0x048a5494 */
cvar_t *in_debugjoystick; /* original 0x048a5490 */
cvar_t *joy_threshold; /* original 0x048a549c */

qboolean sysInputAppActive; /* original 0x048a54a8 */

static int32_t previousMouseButtons; /* original 0x009cdbbc */
static qboolean mouseActive;         /* original 0x009cdbc0 */
static qboolean mouseInitialized;    /* original 0x009cdbc4 */
static int32_t midiDeviceCount;      /* original 0x009cda50 */

static void IN_StartupMouse(void);
static void IN_StartupJoystick(void);
static void IN_JoyMove(void);
static void IN_StartupMIDI(void);
static void IN_ShutdownMIDI(void);
static void MidiInfo_f(void);

void IN_ActivateMouse(void)
{
    if (mouseInitialized == qfalse)
        return;
    if (in_mouse->integer == 0) {
        mouseActive = qfalse;
        return;
    }
    if (mouseActive == qfalse) {
        mouseActive = qtrue;
        CoduoSDL_SetRelativeMouse(qtrue);
    }
}

void IN_DeactivateMouse(void)
{
    if (mouseInitialized != qfalse && mouseActive != qfalse) {
        mouseActive = qfalse;
        CoduoSDL_SetRelativeMouse(qfalse);
    }
}

static void IN_StartupMouse(void)
{
    mouseInitialized = qfalse;
    if (in_mouse->integer == 0) {
        Com_Printf("Mouse control not active.\n");
        return;
    }
    mouseInitialized = qtrue;
}

void IN_MouseEvent(int32_t buttonMask)
{
    if (mouseInitialized == qfalse)
        return;

    for (int32_t button = 0; button < 3; ++button) {
        const int32_t buttonBit = 1 << button;
        if ((buttonMask & buttonBit) != 0 &&
            (previousMouseButtons & buttonBit) == 0) {
            Sys_QueEvent(sysMsgTime, SE_KEY, K_MOUSE1 + button,
                         qtrue, 0, NULL);
        }
        if ((buttonMask & buttonBit) == 0 &&
            (previousMouseButtons & buttonBit) != 0) {
            Sys_QueEvent(sysMsgTime, SE_KEY, K_MOUSE1 + button,
                         qfalse, 0, NULL);
        }
    }
    previousMouseButtons = buttonMask;
}

void IN_MouseMove(void)
{
}

static void IN_Startup(void)
{
    IN_StartupMouse();
    IN_StartupJoystick();
    IN_StartupMIDI();
    in_mouse->modified = qfalse;
    in_joystick->modified = qfalse;
}

void IN_Shutdown(void)
{
    IN_DeactivateMouse();
    IN_ShutdownMIDI();
    Cmd_RemoveCommand("midiinfo");
}

void IN_Restart(void)
{
    IN_Shutdown();
    IN_Init();
}

void IN_Init(void)
{
    in_midi = Cvar_Get("in_midi", "0", CVAR_ARCHIVE);
    in_midiport = Cvar_Get("in_midiport", "-1", CVAR_ARCHIVE);
    in_midichannel =
        Cvar_Get("in_midichannel", "-1", CVAR_ARCHIVE);
    in_mididevice = Cvar_Get("in_mididevice", "0", CVAR_ARCHIVE);
    Cmd_AddCommand("midiinfo", MidiInfo_f);

    in_mouse = Cvar_Get("in_mouse", "1",
                        CVAR_ARCHIVE | CVAR_LATCH);
    in_joystick = Cvar_Get("in_joystick", "0",
                           CVAR_ARCHIVE | CVAR_LATCH);
    in_joyBallScale =
        Cvar_Get("in_joyBallScale", "0.02", CVAR_ARCHIVE);
    in_debugjoystick =
        Cvar_Get("in_debugjoystick", "0", CVAR_TEMP);
    joy_threshold =
        Cvar_Get("joy_threshold", "0.15", CVAR_ARCHIVE);

    IN_Startup();
}

void IN_Activate(qboolean active)
{
    sysInputAppActive = active;
    if (active == qfalse)
        IN_DeactivateMouse();
}

void IN_Frame(void)
{
    IN_JoyMove();
    if (mouseInitialized == qfalse)
        return;

    if ((cls.keyCatchers & KEYCATCH_CONSOLE) != 0) {
        cvar_t *const fullscreen = Cvar_FindVar("r_fullscreen");
        if (fullscreen == NULL || fullscreen->value == 0.0f) {
            IN_DeactivateMouse();
            return;
        }
    }

    if (sysInputAppActive != qfalse) {
        IN_ActivateMouse();
        IN_MouseMove();
    } else {
        IN_DeactivateMouse();
    }
}

void IN_ClearStates(void)
{
    previousMouseButtons = 0;
}

static void IN_StartupJoystick(void)
{
}

static void IN_JoyMove(void)
{
}

static void MidiInfo_f(void)
{
    Com_Printf("\nMIDI control:       %s\n",
               in_midi->integer != 0 ? "enabled" : "disabled");
    Com_Printf("port:               %d\n", in_midiport->integer);
    Com_Printf("channel:            %d\n", in_midichannel->integer);
    Com_Printf("current device:     %d\n", in_mididevice->integer);
    Com_Printf("number of devices:  %d\n", midiDeviceCount);
}

static void IN_StartupMIDI(void)
{
    midiDeviceCount = 0;
}

static void IN_ShutdownMIDI(void)
{
    midiDeviceCount = 0;
}
