#include "../client_recovered.h"
#include "../globals.h"

#include <string.h>

/* This translation unit owns the cgame's movement-policy cvar mirrors. The
 * physics that consume them is recovered shared pmove code under src/bg, which
 * both the game module and cgame compile, so only the module-side storage and
 * its lifecycle live here. */

vmCvar_t bg_jumpSlowdownEnable;

void cgame_compat_reset_movement_state(void)
{
    memset(&bg_jumpSlowdownEnable, 0, sizeof(bg_jumpSlowdownEnable));
}

void cgame_compat_register_movement_cvars(void)
{
    trap_Cvar_Register(&bg_jumpSlowdownEnable, "jump_slowdownEnable", "1",
                       CVAR_ARCHIVE | CVAR_SYSTEMINFO);
}

void cgame_compat_update_movement_cvars(void)
{
    cgame_syscall(CG_CVAR_UPDATE, (intptr_t)&bg_jumpSlowdownEnable);
}
