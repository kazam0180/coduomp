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
    GLIMP_GAMMA_ENTRY_COUNT = 256
};

renderer_wgl_device_context_t rendererWin32DeviceContext;

uint16_t rendererOriginalGammaRamp
    [GLIMP_GAMMA_CHANNEL_COUNT][GLIMP_GAMMA_ENTRY_COUNT];

static uint16_t coduompCurrentGammaRamp
    [GLIMP_GAMMA_CHANNEL_COUNT][GLIMP_GAMMA_ENTRY_COUNT];
static qboolean coduompCurrentGammaRampValid;
static qboolean coduompGammaWindowActive = qtrue;

static coduomp_gamma_mode_t coduomp_gamma_mode_compat(void)
{
    if (r_gammaMode == NULL)
        return CODUOMP_GAMMA_MODE_AUTOMATIC;
    const int32_t mode = r_gammaMode->integer;
    if (mode <= CODUOMP_GAMMA_MODE_DISABLED)
        return CODUOMP_GAMMA_MODE_DISABLED;
    if (mode >= CODUOMP_GAMMA_MODE_SOFTWARE)
        return CODUOMP_GAMMA_MODE_SOFTWARE;
    return CODUOMP_GAMMA_MODE_AUTOMATIC;
}

static qboolean coduomp_gamma_native_allowed_compat(void)
{
    return coduomp_gamma_mode_compat() == CODUOMP_GAMMA_MODE_AUTOMATIC &&
           r_ignorehwgamma->integer == 0;
}

qboolean coduomp_gamma_texture_fallback_enabled_compat(void)
{
    if (coduomp_gamma_mode_compat() == CODUOMP_GAMMA_MODE_DISABLED)
        return qfalse;
    return coduomp_output_gamma_texture_fallback_active_compat();
}

qboolean coduomp_gamma_output_available(void)
{
    return glConfig.deviceSupportsGamma != qfalse ||
                   coduomp_output_gamma_software_active_compat() != qfalse
               ? qtrue
               : qfalse;
}

static uint16_t GLimp_ExpandGammaByte(uint8_t value)
{
    return (uint16_t)(((uint16_t)value << 8) | value);
}

void RestoreSystemGammas(void)
{
    coduompCurrentGammaRampValid = qfalse;
    GLimp_RestoreGamma();
}

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

void GLimp_RestoreGamma(void)
{
    if (coduomp_output_gamma_xrandr_active_compat() != qfalse) {
        coduomp_output_gamma_restore_xrandr_compat();
        return;
    }

    if (glConfig.deviceSupportsGamma == qfalse)
        return;

    (void)coduomp_sdl_set_window_gamma_ramp(
        rendererOriginalGammaRamp[0],
        rendererOriginalGammaRamp[1],
        rendererOriginalGammaRamp[2]);
}
