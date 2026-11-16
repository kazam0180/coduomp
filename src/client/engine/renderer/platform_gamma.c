#include "backend.h"

#include "platform_gamma.h"
#include "renderer_cvars.h"
#include "wgl_debug.h"
#include "output_gamma_compat.h"

#include "../platform/sdl_platform.h"

#include <stdint.h>
#include <string.h>


enum {
    GLIMP_GAMMA_CHANNEL_COUNT = 3,
    GLIMP_GAMMA_ENTRY_COUNT = 256,
    GLIMP_W2K_CLAMP_ENTRY_COUNT = 128,
    GLIMP_W2K_CLAMP_BASE = 128,
    GLIMP_W2K_MAXIMUM_RAMP_VALUE = 254,
    RESTORE_GAMMA_WINDOW_CAPACITY = 5
};

renderer_wgl_device_context_t rendererWin32DeviceContext;

/* Original 0x0389f6e0..0x0389fce0. Retail GLimp_InitGamma captures the
 * desktop device's ramp here; the multi-monitor compatibility path captures
 * the game display's ramp so shutdown restores the same hardware target. */
uint16_t rendererOriginalGammaRamp
    [GLIMP_GAMMA_CHANNEL_COUNT][GLIMP_GAMMA_ENTRY_COUNT];

/* NOT_FROM_ORIGINAL_SOURCE_STORAGE: SDL's gamma API changes the complete
 * display on macOS. Retain the current game ramp so native focus transitions
 * can expose system UI under the captured desktop ramp and restore the game
 * mapping without recalculating renderer state. */
static uint16_t coduompCurrentGammaRamp
    [GLIMP_GAMMA_CHANNEL_COUNT][GLIMP_GAMMA_ENTRY_COUNT];
static qboolean coduompCurrentGammaRampValid;
static qboolean coduompGammaWindowActive = qtrue;

/* NOT_FROM_ORIGINAL_SOURCE: return the latched compatibility policy selected
 * at renderer startup. Registration clamps the archived cvar to this enum. */
static coduomp_gamma_mode_t coduomp_gamma_mode_compat(void)
{
    if (r_gammaMode == NULL)
        return CODUOMP_GAMMA_MODE_AUTOMATIC;

    switch (r_gammaMode->integer) {
    case CODUOMP_GAMMA_MODE_DISABLED:
        return CODUOMP_GAMMA_MODE_DISABLED;
    case CODUOMP_GAMMA_MODE_SOFTWARE:
        return CODUOMP_GAMMA_MODE_SOFTWARE;
    default:
        return CODUOMP_GAMMA_MODE_AUTOMATIC;
    }
}

/* NOT_FROM_ORIGINAL_SOURCE: native ramps are considered only in automatic
 * mode, and the original r_ignorehwgamma cvar remains a permanent veto. */
static qboolean coduomp_gamma_native_allowed_compat(void)
{
    return coduomp_gamma_mode_compat() == CODUOMP_GAMMA_MODE_AUTOMATIC &&
                   r_ignorehwgamma->integer == 0
               ? qtrue
               : qfalse;
}

/* NOT_FROM_ORIGINAL_SOURCE: retain the recovered texture-upload fallback as
 * the final automatic provider. Disabled and forced full-frame software modes
 * must not silently substitute this visibly different, limited correction. */
qboolean coduomp_gamma_texture_fallback_enabled_compat(void)
{
    return coduomp_gamma_mode_compat() == CODUOMP_GAMMA_MODE_AUTOMATIC
               ? qtrue
               : qfalse;
}

/* NOT_FROM_ORIGINAL_SOURCE: report whether the final output has either the
 * recovered native ramp or the compatibility provider's exact table lookup.
 * Renderer table policy does not need to know which output boundary won. */
qboolean coduomp_gamma_output_available(void)
{
    return glConfig.deviceSupportsGamma != qfalse ||
                   coduomp_output_gamma_software_active_compat() != qfalse
               ? qtrue
               : qfalse;
}


static uint16_t GLimp_ExpandGammaByte(uint8_t value)
{
    /* NOT_FROM_ORIGINAL_SOURCE: source-level expression for the repeated
     * byte-to-WORD expansion in GLimp_SetGamma's unrolled PE loop. The native
     * SDL display boundary consumes the same 16-bit ramp representation. */
    return (uint16_t)(((uint16_t)value << 8) | value);
}

/* Source: CoDUOMP.exe 0x004010b0..0x00401145.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_004010b0_00401146.mcode.
 * Name: exact same-module Mac symbol RestoreSystemGammas. The Windows body
 * resets the display mode, normalizes matching thread-window styles through
 * its callback, then installs an exact 0x0000,0x0101,...,0xffff linear ramp
 * on the desktop device context. */
void RestoreSystemGammas(void)
{
    /* NOT_FROM_ORIGINAL_SOURCE: the normal native restore occurs in
     * GLimp_Shutdown while SDL still owns the window/display association. */
}

/* Source: CoDUOMP.exe 0x00523c80..0x00523d65.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00523c80_00523d66.mcode.
 * Name and no-argument source boundary: exact same-module Mac symbol
 * GLimp_InitGamma. The PE imports prove the GetDesktopWindow/GetDC,
 * GetDeviceGammaRamp, and ReleaseDC sequence. */
void GLimp_InitGamma(void)
{
    glConfig.deviceSupportsGamma = qfalse;

    coduompCurrentGammaRampValid = qfalse;

    if (coduomp_gamma_mode_compat() == CODUOMP_GAMMA_MODE_DISABLED) {
        ri.Printf(R_PRINT_ALL,
                  "Gamma correction is disabled by r_gammaMode 0.\n");
        return;
    }

    coduomp_output_gamma_initialize_compat();

    if (coduomp_gamma_mode_compat() == CODUOMP_GAMMA_MODE_SOFTWARE) {
        (void)coduomp_output_gamma_activate_software_compat(
            "Full-frame software gamma is forced by r_gammaMode 2");
        return;
    }

    if (r_ignorehwgamma->integer != 0) {
        (void)coduomp_output_gamma_activate_software_compat(
            "Native display gamma is disabled by r_ignorehwgamma");
        return;
    }

    /* NOT_FROM_ORIGINAL_SOURCE: SDL supplies the native display/window
     * boundary corresponding to the original desktop-device gamma calls. */
    glConfig.deviceSupportsGamma = coduomp_sdl_get_window_gamma_ramp(
        rendererOriginalGammaRamp[0],
        rendererOriginalGammaRamp[1],
        rendererOriginalGammaRamp[2]);
    if (glConfig.deviceSupportsGamma == qfalse) {
        glConfig.deviceSupportsGamma =
            coduomp_output_gamma_try_xrandr_compat(
                rendererOriginalGammaRamp);
        if (glConfig.deviceSupportsGamma == qfalse) {
            (void)coduomp_output_gamma_activate_software_compat(
                "SDL could not read a native display gamma ramp");
        }
    }

    if (glConfig.deviceSupportsGamma == qfalse)
        return;

    if ((uint8_t)(rendererOriginalGammaRamp[0][255] >> 8) <=
            (uint8_t)(rendererOriginalGammaRamp[0][0] >> 8) ||
        (uint8_t)(rendererOriginalGammaRamp[1][255] >> 8) <=
            (uint8_t)(rendererOriginalGammaRamp[1][0] >> 8) ||
        (uint8_t)(rendererOriginalGammaRamp[2][255] >> 8) <=
            (uint8_t)(rendererOriginalGammaRamp[2][0] >> 8)) {
        glConfig.deviceSupportsGamma = qfalse;
        ri.Printf(R_PRINT_WARNING,
                  "WARNING: device has broken gamma support, generated "
                  "gamma.dat\n");
        glConfig.deviceSupportsGamma =
            coduomp_output_gamma_try_xrandr_compat(
                rendererOriginalGammaRamp);
        if (glConfig.deviceSupportsGamma == qfalse) {
            (void)coduomp_output_gamma_activate_software_compat(
                "The native display returned a broken gamma ramp");
        }
    }

    /* Some drivers report a saturated restoration ramp. The PE tests the
     * high byte of channel 0, entry 181 and replaces entries 0..254 with the
     * exact linear values below, deliberately retaining entry 255. */
    if ((uint8_t)(rendererOriginalGammaRamp[0][181] >> 8) == 255) {
        ri.Printf(R_PRINT_WARNING,
                  "WARNING: suspicious gamma tables, using linear ramp for "
                  "restoration\n");
        for (int32_t entry = 0; entry < 255; ++entry) {
            const uint16_t linearValue = (uint16_t)(entry << 8);
            for (int32_t channel = 0;
                 channel < GLIMP_GAMMA_CHANNEL_COUNT; ++channel) {
                rendererOriginalGammaRamp[channel][entry] = linearValue;
            }
        }
    }
}

/* Source: CoDUOMP.exe 0x00523d70..0x0052401c.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00523d70_0052401d.mcode.
 * Name and three 256-byte channel arguments: exact same-module Mac symbol
 * GLimp_SetGamma plus the R_SetColorMappings call at 0x0050a837. */
void GLimp_SetGamma(const uint8_t red[GLIMP_GAMMA_ENTRY_COUNT],
                    const uint8_t green[GLIMP_GAMMA_ENTRY_COUNT],
                    const uint8_t blue[GLIMP_GAMMA_ENTRY_COUNT])
{
    if (glConfig.deviceSupportsGamma == qfalse ||
        coduomp_gamma_native_allowed_compat() == qfalse) {
        return;
    }

    uint16_t gammaRamp
        [GLIMP_GAMMA_CHANNEL_COUNT][GLIMP_GAMMA_ENTRY_COUNT];
    for (int32_t entry = 0; entry < GLIMP_GAMMA_ENTRY_COUNT; ++entry) {
        gammaRamp[0][entry] = GLimp_ExpandGammaByte(red[entry]);
        gammaRamp[1][entry] = GLimp_ExpandGammaByte(green[entry]);
        gammaRamp[2][entry] = GLimp_ExpandGammaByte(blue[entry]);
    }


    for (int32_t channel = 0;
         channel < GLIMP_GAMMA_CHANNEL_COUNT; ++channel) {
        for (int32_t entry = 1;
             entry < GLIMP_GAMMA_ENTRY_COUNT; ++entry) {
            if (gammaRamp[channel][entry] <
                gammaRamp[channel][entry - 1]) {
                gammaRamp[channel][entry] =
                    gammaRamp[channel][entry - 1];
            }
        }
    }

    /* NOT_FROM_ORIGINAL_SOURCE: apply the original renderer's computed ramp
     * through SDL to the display that owns the native game window. */
    memcpy(coduompCurrentGammaRamp, gammaRamp,
           sizeof(coduompCurrentGammaRamp));
    coduompCurrentGammaRampValid = qtrue;
    if (coduompGammaWindowActive != qfalse) {
        qboolean gammaSet;

        if (coduomp_output_gamma_xrandr_active_compat() != qfalse) {
            gammaSet = coduomp_output_gamma_set_xrandr_compat(gammaRamp);
        } else {
            gammaSet = coduomp_sdl_set_window_gamma_ramp(
                gammaRamp[0], gammaRamp[1], gammaRamp[2]);
            if (gammaSet == qfalse) {
                Com_Printf("SDL_SetWindowGammaRamp failed: %s\n",
                           coduomp_sdl_error_compat());
                if (coduomp_output_gamma_try_xrandr_compat(
                        rendererOriginalGammaRamp) != qfalse) {
                    gammaSet = coduomp_output_gamma_set_xrandr_compat(
                        gammaRamp);
                }
            }
        }
        if (gammaSet == qfalse) {
            glConfig.deviceSupportsGamma = qfalse;
            (void)coduomp_output_gamma_activate_software_compat(
                "SDL and native X11 gamma are unavailable");
        }
    }
}


/* COMPATIBILITY_PATCH (NOT_FROM_ORIGINAL_SOURCE): SDL implements window gamma
 * with a display-wide ramp on macOS. Restore the captured desktop mapping
 * while another application or a system overlay has focus, then reapply the
 * last renderer mapping when the game regains focus. This keeps the original
 * live r_gamma behavior without brightening unrelated macOS UI. */
void coduomp_gamma_window_focus_changed(qboolean active)
{
    coduompGammaWindowActive = active;

    if (coduomp_output_gamma_xrandr_active_compat() != qfalse) {
        if (active != qfalse &&
            coduompCurrentGammaRampValid != qfalse &&
            coduomp_gamma_native_allowed_compat() != qfalse) {
            (void)coduomp_output_gamma_set_xrandr_compat(
                coduompCurrentGammaRamp);
        } else {
            coduomp_output_gamma_restore_xrandr_compat();
        }
        return;
    }

    if (glConfig.deviceSupportsGamma == qfalse)
        return;

    if (active != qfalse &&
        coduompCurrentGammaRampValid != qfalse &&
        coduomp_gamma_native_allowed_compat() != qfalse) {
        (void)coduomp_sdl_set_window_gamma_ramp(
            coduompCurrentGammaRamp[0],
            coduompCurrentGammaRamp[1],
            coduompCurrentGammaRamp[2]);
    } else {
        (void)coduomp_sdl_set_window_gamma_ramp(
            rendererOriginalGammaRamp[0],
            rendererOriginalGammaRamp[1],
            rendererOriginalGammaRamp[2]);
    }
}

/* Source: CoDUOMP.exe 0x00524020..0x00524054.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00524020_00524055.mcode.
 * Provisional role name: retained out-of-line body of the gamma restoration
 * helper inlined by the Windows compiler into GLimp_Shutdown at 0x004f6f51.
 * Retail restores the desktop ramp; the compatibility branch restores the
 * game monitor through the same display-specific DC used for capture. */
void GLimp_RestoreGamma(void)
{

    if (coduomp_output_gamma_xrandr_active_compat() != qfalse) {
        coduomp_output_gamma_restore_xrandr_compat();
        return;
    }

    if (glConfig.deviceSupportsGamma == qfalse)
        return;

    /* NOT_FROM_ORIGINAL_SOURCE: restore the ramp captured before the native
     * renderer installed its game-specific gamma mapping. */
    (void)coduomp_sdl_set_window_gamma_ramp(
        rendererOriginalGammaRamp[0],
        rendererOriginalGammaRamp[1],
        rendererOriginalGammaRamp[2]);
}
