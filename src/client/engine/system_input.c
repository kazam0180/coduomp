#include "system_input.h"

#include "client/console.h"
#include "platform/crt_boundary.h"
#include "system_event.h"

#include <string.h>

#include "platform/sdl_platform.h"

enum {
    IN_MOUSE_BUTTON_COUNT = 3,
    IN_JOYSTICK_DIRECTION_COUNT = 16,
    IN_JOYSTICK_DIGITAL_AXIS_COUNT = 4,
    IN_JOYSTICK_BALL_AXIS_COUNT = 6,
    IN_JOYSTICK_AXIS_CENTER = 32768,
    IN_JOYSTICK_POV_FORWARD_BIT = 1 << 12,
    IN_JOYSTICK_POV_BACKWARD_BIT = 1 << 13,
    IN_JOYSTICK_POV_RIGHT_BIT = 1 << 14,
    IN_JOYSTICK_POV_LEFT_BIT = 1 << 15,
    IN_MIDI_DEVICE_CAPACITY = 8,
    IN_MIDI_KEY_OFFSET = 179,
    IN_MIDI_FIRST_KEY = 239,
    IN_MIDI_LAST_KEY = 255,
    IN_MIDI_STATUS_TYPE_MASK = 0xf0,
    IN_MIDI_CHANNEL_MASK = 0x0f,
    IN_MIDI_NOTE_ON = 0x90,
    IN_MIDI_NOTE_OFF = 0x80,
    IN_MIDI_DATA_BYTE_MASK = 0xff,
    IN_RAW_INPUT_GENERIC_DESKTOP_USAGE_PAGE = 1,
    IN_RAW_INPUT_MOUSE_USAGE = 2
};

cvar_t *in_mouse; /* original 0x048a5484 */
cvar_t *in_midi; /* original 0x048a5480 */
cvar_t *in_midiport; /* original 0x048a54a4 */
cvar_t *in_midichannel; /* original 0x048a5488 */
cvar_t *in_mididevice; /* original 0x048a54a0 */
cvar_t *in_joystick; /* original 0x048a548c */
cvar_t *in_joyBallScale; /* original 0x048a5494 */
cvar_t *in_debugjoystick; /* original 0x048a5490 */
cvar_t *joy_threshold; /* original 0x048a549c */

/* Set by the platform window/event layer. The original Win32 activation
 * handler at 0x0046fa90 writes this process-local state. */
qboolean sysInputAppActive; /* original 0x048a54a8 */

static int32_t windowCenterX;       /* original 0x009cdbb8 */
static int32_t windowCenterY;       /* original 0x009cda48 */
static int32_t previousMouseButtons;/* original 0x009cdbbc */
static qboolean mouseActive;        /* original 0x009cdbc0 */
static qboolean mouseInitialized;   /* original 0x009cdbc4 */
static int32_t midiDeviceCount;     /* original 0x009cda50 */


static void IN_StartupMouse(void);
static void IN_StartupJoystick(void);
static void IN_JoyMove(void);
static void IN_StartupMIDI(void);
static void IN_ShutdownMIDI(void);
static void MidiInfo_f(void);


/* Source: CoDUOMP.exe 0x004699b0..0x00469a72.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_004699b0_00469a73.mcode.
 * Role name: this is the Win32 cursor-capture body reached by
 * IN_ActivateMouse. */
static void IN_ActivateWin32Mouse(void)
{
    CoduoSDL_SetRelativeMouse(qtrue);
    windowCenterX = 0;
    windowCenterY = 0;
}

/* Source: CoDUOMP.exe 0x00469a80..0x00469aaa.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00469a80_00469aab.mcode.
 * Role name: inverse Win32 cursor-capture body used by every mouse
 * deactivation path. */
static void IN_DeactivateWin32Mouse(void)
{
    CoduoSDL_SetRelativeMouse(qfalse);
}

/* Source: CoDUOMP.exe 0x00469ab0..0x00469af5.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00469ab0_00469af6.mcode.
 * Role name: obtains the Win32 cursor displacement and recenters it. */
static void IN_Win32Mouse(int32_t *deltaX, int32_t *deltaY)
{
    /* NOT_FROM_ORIGINAL_SOURCE: non-Windows backends enqueue their native
     * relative-motion events directly. */
    *deltaX = 0;
    *deltaY = 0;
}

/* Source: CoDUOMP.exe 0x00469b00..0x00469b37.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00469b00_00469b38.mcode.
 * Name: established id-engine platform input interface. */
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
        IN_ActivateWin32Mouse();
    }
}

/* Source: CoDUOMP.exe 0x00469b40..0x00469b61, recovered from an executable
 * gap after repairing the missing Ghidra function entry.
 * Name: established id-engine platform input interface. */
void IN_DeactivateMouse(void)
{
    if (mouseInitialized != qfalse && mouseActive != qfalse) {
        mouseActive = qfalse;
        IN_DeactivateWin32Mouse();
    }
}

/* Source: CoDUOMP.exe 0x00469b70..0x00469b9e, recovered from an executable
 * gap after repairing the missing Ghidra function entry.
 * Name: established id-engine platform input interface. */
static void IN_StartupMouse(void)
{
    mouseInitialized = qfalse;
    if (in_mouse->integer == 0) {
        Com_Printf("Mouse control not active.\n");
        return;
    }
    mouseInitialized = qtrue;
}

/* Source: CoDUOMP.exe 0x00469ba0..0x00469c1e.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00469ba0_00469c1f.mcode.
 * Name and button-mask contract: established Win32 id-engine input
 * interface; the original handles the first three mouse buttons. */
void IN_MouseEvent(int32_t buttonMask)
{
    if (mouseInitialized == qfalse)
        return;

    for (int32_t button = 0; button < IN_MOUSE_BUTTON_COUNT; ++button) {
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

/* Source: CoDUOMP.exe 0x00469c20..0x00469c75.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00469c20_00469c76.mcode.
 * Name: established id-engine platform input interface. The compiler inlined
 * IN_Win32Mouse into this body in the shipped executable. */
void IN_MouseMove(void)
{
    int32_t deltaX;
    int32_t deltaY;

    IN_Win32Mouse(&deltaX, &deltaY);
    if (deltaX != 0 || deltaY != 0)
        Sys_QueEvent(0, SE_MOUSE, deltaX, deltaY, 0, NULL);
}

/* Source: CoDUOMP.exe 0x00469c80..0x00469cd3.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00469c80_00469cd4.mcode.
 * Name: established id-engine platform input interface. The compiler inlined
 * IN_StartupMouse into the shipped body. */
static void IN_Startup(void)
{
    IN_StartupMouse();
    IN_StartupJoystick();
    IN_StartupMIDI();
    in_mouse->modified = qfalse;
    in_joystick->modified = qfalse;
}

/* Source: CoDUOMP.exe 0x00469ce0..0x00469d2e.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00469ce0_00469d2f.mcode.
 * Name: established id-engine platform input interface. */
void IN_Shutdown(void)
{
    IN_DeactivateMouse();
    IN_ShutdownMIDI();
    Cmd_RemoveCommand("midiinfo");
}

/* Source: CoDUOMP.exe 0x0046bdf0..0x0046bdf9, recovered from the executable
 * gap immediately before Sys_Init.
 * Name: established id-engine input command. The original is the two-call
 * shutdown/reinitialize sequence registered as "in_restart" by Sys_Init. */
void IN_Restart(void)
{
    IN_Shutdown();
    IN_Init();
}

/* Source: CoDUOMP.exe 0x00469d30..0x00469e12.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00469d30_00469e13.mcode.
 * Name: established id-engine platform input interface. */
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

/* Source: CoDUOMP.exe 0x00469e20..0x00469e4a, recovered from an executable
 * gap after repairing the missing Ghidra function entry.
 * Name: established id-engine platform input interface. The original receives
 * active in EAX and inlines IN_DeactivateMouse. */
void IN_Activate(qboolean active)
{
    sysInputAppActive = active;
    if (active == qfalse)
        IN_DeactivateMouse();
}

/* Source: CoDUOMP.exe 0x00469e50..0x00469ebe.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00469e50_00469ebf.mcode.
 * Name: established id-engine platform input interface. */
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

/* Source: CoDUOMP.exe 0x00469ec0..0x00469eca, recovered from an executable
 * gap after repairing the missing Ghidra function entry.
 * Name: established id-engine platform input interface. */
void IN_ClearStates(void)
{
    previousMouseButtons = 0;
}

/* Source: CoDUOMP.exe 0x00469ed0..0x0046a067.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00469ed0_0046a068.mcode.
 * Name: established id-engine WinMM input interface. */
static void IN_StartupJoystick(void)
{
    /* NOT_FROM_ORIGINAL_SOURCE: WinMM joystick discovery has no direct
     * non-Windows equivalent; a native input backend will enqueue events. */
}


/* Source: CoDUOMP.exe 0x0046a0c0..0x0046a3c5.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046a0c0_0046a3c6.mcode.
 * Name: established id-engine WinMM input interface. */
static void IN_JoyMove(void)
{
    /* NOT_FROM_ORIGINAL_SOURCE: native backends provide joystick events
     * directly rather than polling WinMM here. */
}


/* Source: CoDUOMP.exe 0x0046a4d0..0x0046a602.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046a4d0_0046a603.mcode.
 * Name: command string and behavior establish the console command role. */
static void MidiInfo_f(void)
{
    Com_Printf("\nMIDI control:       %s\n",
               in_midi->integer != 0 ? "enabled" : "disabled");
    Com_Printf("port:               %d\n", in_midiport->integer);
    Com_Printf("channel:            %d\n", in_midichannel->integer);
    Com_Printf("current device:     %d\n", in_mididevice->integer);
    Com_Printf("number of devices:  %d\n", midiDeviceCount);

}

/* Source: CoDUOMP.exe 0x0046a610..0x0046a6e0.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046a610_0046a6e1.mcode.
 * Name: established id-engine WinMM input interface. */
static void IN_StartupMIDI(void)
{
    /* NOT_FROM_ORIGINAL_SOURCE: native MIDI integration is deferred to the
     * platform input backend; the original implementation is WinMM-only. */
    midiDeviceCount = 0;
}

/* Source: CoDUOMP.exe 0x0046a6f0..0x0046a710, recovered from an executable
 * gap after repairing the missing Ghidra function entry.
 * Name: established id-engine WinMM input interface. */
static void IN_ShutdownMIDI(void)
{
    /* NOT_FROM_ORIGINAL_SOURCE: no WinMM handle exists on native platforms. */
    midiDeviceCount = 0;
}
