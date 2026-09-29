#ifndef SHARED_SERVER_FRAME_H
#define SHARED_SERVER_FRAME_H

#include "qcommon/server_runtime_types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void SV_RunFrame(void);
void SV_BotUserMove(client_t *client);
qboolean SV_SetBotInput(int32_t clientNum, int32_t buttons, int32_t wbuttons,
                        int32_t weapon, int32_t forwardMove,
                        int32_t rightMove, int32_t upMove);
void SV_RunBotFrame(void);
void SV_Frame(int32_t msec);

#ifdef __cplusplus
}
#endif

#endif
