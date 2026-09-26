#include "server_client_maintenance.h"

#include "qcommon/qcommon_limits.h"
#include "qcommon/qcommon_runtime_types.h"
#include "server_client_message.h"
#include "math/q_math.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
    SERVER_MAX_PING = 999,
    SERVER_TIMEOUT_SECONDS_TO_MSEC = 1000,
    SERVER_TIMEOUT_DROP_SCANS = 5
};

extern serverStatic_t svs;
extern cvar_t *sv_maxclients;
extern cvar_t *sv_timeout;
extern cvar_t *sv_zombietime;
extern cvar_t *cl_paused;
extern cvar_t *sv_paused;
extern cvar_t *sv_botRandomPing;
extern cvar_t *sv_botRandomPingJitter;
extern cvar_t *sv_botPings;

#define SERVER_BOT_PING_POOL_CAPACITY 64
#define SERVER_BOT_PING_FALLBACK 80

static int32_t botPingPool[SERVER_BOT_PING_POOL_CAPACITY];
static int32_t botPingPoolCount = 0;
static int32_t botPingPoolModificationCount = -1;
static int32_t botBasePing[MAX_CLIENTS];
static int8_t botPingSign[MAX_CLIENTS];

static void refreshBotPingPool(void)
{
    if (sv_botPings == NULL || sv_botPings->string == NULL) {
        if (botPingPoolCount == 0) {
            botPingPool[0] = SERVER_BOT_PING_FALLBACK;
            botPingPoolCount = 1;
        }
        return;
    }
    if (sv_botPings->modificationCount == botPingPoolModificationCount) {
        return;
    }
    botPingPoolModificationCount = sv_botPings->modificationCount;

    botPingPoolCount = 0;
    const char *scan = sv_botPings->string;
    while (*scan != '\0' && botPingPoolCount < SERVER_BOT_PING_POOL_CAPACITY) {
        while (*scan == ' ' || *scan == '\t') {
            ++scan;
        }
        if (*scan == '\0') {
            break;
        }
        if (*scan == ';') {
            ++scan;
            continue;
        }
        const int32_t value = atoi(scan);
        while (*scan != '\0' && *scan != ';') {
            ++scan;
        }
        if (*scan == ';') {
            ++scan;
        }
        if (value < 1 || value > SERVER_MAX_PING) {
            continue;
        }
        botPingPool[botPingPoolCount++] = value;
    }
    if (botPingPoolCount == 0) {
        botPingPool[0] = SERVER_BOT_PING_FALLBACK;
        botPingPoolCount = 1;
    }
    memset(botBasePing, 0, sizeof(botBasePing));
}

void Com_DPrintf(const char *format, ...);
/*
 * Complete periodic client ping, timeout, and pause-maintenance cluster
 * shared by the Windows client engine and Linux dedicated engine:
 *
 * Function                    Windows       Linux
 * SV_CalcPings                0x00462d50    0x08094cad
 * SV_CheckTimeouts            0x00462ec0    0x08094e0d
 * SV_CheckPaused              0x00462fb0    0x08094f3a
 *
 * The client-state gates, 32-frame ping sample, 999 clamp, timeout/zombie
 * aging, five-scan grace period, and single-connected-client pause rule agree.
 * The supporting Mac client exports the same canonical names.
 */

/* Source: CoDUOMP.exe 0x00462d50..0x00462e54.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00462d50_00462e55.mcode.
 * Name and source structure: exact same-module Mac symbol SV_CalcPings. */
void SV_CalcPings(void)
{
    client_t *client = svs.clients;
    for (int32_t clientNum = 0;
         clientNum < sv_maxclients->integer;
         ++clientNum, ++client) {

        if (client->isTestClient && sv_botRandomPing->integer > 0) {
            const int32_t botNum = (int32_t)(client - svs.clients);
            refreshBotPingPool();
            if (botBasePing[botNum] <= 0
                || botBasePing[botNum] >= SERVER_MAX_PING
                || client->ping <= 0
                || client->ping >= SERVER_MAX_PING) {
                botBasePing[botNum] = botPingPool[irand(0, botPingPoolCount - 1)];
                botPingSign[botNum] = 1;
            }
            botPingSign[botNum] = -botPingSign[botNum];
            client->ping = botBasePing[botNum] + botPingSign[botNum]
                * irand(0, sv_botRandomPingJitter->integer);
            if (client->ping < 1)
                client->ping = 1;
            else if (client->ping > SERVER_MAX_PING)
                client->ping = SERVER_MAX_PING;
            continue;
        }
        if (client->state != CS_ACTIVE || client->gentity == NULL) {
            client->ping = SERVER_MAX_PING;
            continue;
        }

        int32_t pingTotal = 0;
        int32_t pingSamples = 0;
        for (int32_t frameIndex = 0;
             frameIndex < SERVER_CLIENT_SNAPSHOT_FRAME_COUNT;
             ++frameIndex) {
            const clientSnapshot_t *const frame =
                &client->snapshotFrames[frameIndex];
            if (frame->messageAcknowledgedTime > 0) {
                ++pingSamples;
                pingTotal += frame->messageAcknowledgedTime -
                             frame->messageSentTime;
            }
        }

        if (pingSamples == 0) {
            client->ping = SERVER_MAX_PING;
        } else {
            client->ping = pingTotal / pingSamples;
            if (client->ping > SERVER_MAX_PING)
                client->ping = SERVER_MAX_PING;
        }
    }
}

/* Source: CoDUOMP.exe 0x00462ec0..0x00462faf.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00462ec0_00462fb0.mcode.
 * Name and source structure: exact same-module Mac symbol SV_CheckTimeouts. */
void SV_CheckTimeouts(void)
{
    const int32_t timeoutLimit =
        svs.realTime -
        sv_timeout->integer * SERVER_TIMEOUT_SECONDS_TO_MSEC;
    const int32_t zombieLimit =
        svs.realTime -
        sv_zombietime->integer * SERVER_TIMEOUT_SECONDS_TO_MSEC;

    client_t *client = svs.clients;
    for (int32_t clientNum = 0;
         clientNum < sv_maxclients->integer;
         ++clientNum, ++client) {
        if (client->lastPacketTime > svs.realTime)
            client->lastPacketTime = svs.realTime;

        if (client->isTestClient != qfalse)
            continue;

        if (client->state == CS_ZOMBIE &&
            client->lastPacketTime < zombieLimit) {
            Com_DPrintf("Going from CS_ZOMBIE to CS_FREE for %s\n",
                        client->name);
            client->state = CS_FREE;
        } else if (client->state >= CS_CONNECTED &&
                   client->lastPacketTime < timeoutLimit) {
            ++client->timeoutCount;
            if (client->timeoutCount > SERVER_TIMEOUT_DROP_SCANS) {
                SV_DropClient(client, "EXE_TIMEDOUT");
                client->state = CS_FREE;
            }
        } else {
            client->timeoutCount = 0;
        }
    }
}

/* Source: CoDUOMP.exe 0x00462fb0..0x00463004.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_00462fb0_00463005.mcode.
 * Name and source structure: exact same-module Mac symbol SV_CheckPaused. */
qboolean SV_CheckPaused(void)
{
    if (cl_paused->integer == 0)
        return qfalse;

    int32_t connectedClients = 0;
    client_t *client = svs.clients;
    for (int32_t clientNum = 0;
         clientNum < sv_maxclients->integer;
         ++clientNum, ++client) {
        if (client->state >= CS_CONNECTED)
            ++connectedClients;
    }

    if (connectedClients > 1) {
        sv_paused->integer = 0;
        return qfalse;
    }

    sv_paused->integer = 1;
    return qtrue;
}
