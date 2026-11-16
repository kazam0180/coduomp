#include "system_event.h"

#include "networking/net_channel.h"
#include "system_console.h"

#include <stdlib.h>
#include <string.h>

#include "platform/sdl_platform.h"

enum {
    SYS_EVENT_QUEUE_MASK = SYS_EVENT_QUEUE_COUNT - 1
};

/* Original queue storage is 0x0489bc60..0x0489d45f, followed by the consumer
 * index at 0x0489d464. Native records widen with their payload pointer. */
static sysEvent_t eventQueue[SYS_EVENT_QUEUE_COUNT];
static int32_t eventHead; /* original 0x0489bc40 */
static int32_t eventTail; /* original 0x0489d464 */
static uint8_t sysPacketBuffer[MAX_MSGLEN];
                                /* original 0x0489d480..0x048a547f */

int32_t sysMsgTime; /* original 0x0489bc2c */

/* NOT_FROM_ORIGINAL_SOURCE: SDL can provide many motion reports per frame.
 * Let its translator yield before overflowing the engine's fixed queue,
 * retaining unconsumed reports in SDL until the engine drains its events. */
int32_t coduomp_sys_event_queue_remaining(void)
{
    const uint32_t queued = (uint32_t)eventHead - (uint32_t)eventTail;
    return queued < SYS_EVENT_QUEUE_COUNT ? SYS_EVENT_QUEUE_COUNT - (int32_t)queued : 0;
}

/* Source: CoDUOMP.exe 0x0046ba10..0x0046bac0.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046ba10_0046bac1.mcode.
 * Name and signature: exact same-module Mac symbol Sys_QueEvent. */
void Sys_QueEvent(int32_t time, sysEventType_t type,
                  int32_t value, int32_t value2,
                  int32_t payloadLength, void *payload)
{
    sysEvent_t *event = &eventQueue[eventHead & SYS_EVENT_QUEUE_MASK];

    if (eventHead - eventTail >= SYS_EVENT_QUEUE_COUNT) {
        Com_Printf("Sys_QueEvent: overflow\n");
        if (event->payload != NULL) {
            /* The PE calls its statically linked MSVC free entry at
             * 0x0056d9cf; native libc supplies that same ownership boundary. */
            free(event->payload);
        }
        ++eventTail;
    }

    ++eventHead;
    if (time == 0)
        time = (int32_t)Sys_Milliseconds();

    event->time = time;
    event->type = type;
    event->value = value;
    event->value2 = value2;
    event->payloadLength = payloadLength;
    event->payload = payload;
}

/* Source: CoDUOMP.exe 0x0046bad0..0x0046bb16.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046bad0_0046bb17.mcode.
 * Provisional role name: releases every owned payload still queued at the
 * native system-event boundary. */
void Sys_ClearEventQueue(void)
{
    while (eventTail < eventHead) {
        sysEvent_t *const event =
            &eventQueue[eventTail & SYS_EVENT_QUEUE_MASK];
        ++eventTail;
        if (event->payload != NULL)
            free(event->payload);
    }
}

/* Source: CoDUOMP.exe 0x0046bb20..0x0046bb7a.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046bb20_0046bb7b.mcode and PE imports
 * identifying PeekMessageA, GetMessageA, TranslateMessage, DispatchMessageA,
 * and SetThreadExecutionState.
 * Name: exact same-module Mac symbol Sys_PumpEvents. The Windows routine
 * services at most one pending WM_POWERBROADCAST message per call, then keeps
 * the display awake. */
void Sys_PumpEvents(void)
{
    CoduoSDL_PumpEvents();
}

/* Source: CoDUOMP.exe 0x0046bb80..0x0046bde6.
 * Evidence: coduomp/mcode/CoDUOMP/FUN_0046bb80_0046bde7.mcode.
 * Name and structure-return signature: exact same-module Mac symbol
 * Sys_GetEvent. The machine code proves the two queue checks, unrestricted
 * Win32 message drain, newline removal from console events, and the packet
 * payload layout `netadr_t` followed by unread message bytes. */
sysEvent_t Sys_GetEvent(void)
{
    if (eventHead > eventTail) {
        const sysEvent_t event =
            eventQueue[eventTail & SYS_EVENT_QUEUE_MASK];
        ++eventTail;
        return event;
    }

    CoduoSDL_PumpEvents();
    if (eventHead > eventTail) {
        const sysEvent_t event =
            eventQueue[eventTail & SYS_EVENT_QUEUE_MASK];
        ++eventTail;
        return event;
    }

    char *const consoleLine = Sys_ConsoleInput();
    if (consoleLine != NULL) {
        const int32_t payloadLength =
            (int32_t)strlen(consoleLine) + 1;
        char *const payload = Z_MallocInternal(
            (size_t)payloadLength);

        /* Sys_ConsoleInput includes a trailing newline. The original copies
         * through the preceding character, replaces that newline with NUL,
         * and retains strlen(line)+1 as the owned payload allocation/length. */
        strncpy(payload, consoleLine, (size_t)payloadLength - 2);
        payload[payloadLength - 2] = '\0';
        Sys_QueEvent(
            0, SE_CONSOLE, 0, 0, payloadLength, payload);
    }

    msg_t packetMessage;
    netadr_t packetAddress;
    MSG_Init(
        &packetMessage, sysPacketBuffer,
        (int32_t)sizeof(sysPacketBuffer));

    if (Sys_GetPacket(&packetAddress, &packetMessage) != qfalse) {
        const int32_t unreadLength =
            packetMessage.cursize - packetMessage.readcount;
        const int32_t payloadLength =
            (int32_t)sizeof(packetAddress) + unreadLength;
        uint8_t *const payload = Z_MallocInternal(
            (size_t)payloadLength);

        memcpy(payload, &packetAddress, sizeof(packetAddress));
        memcpy(
            payload + sizeof(packetAddress),
            packetMessage.data + packetMessage.readcount,
            (size_t)unreadLength);
        Sys_QueEvent(
            0, SE_PACKET, 0, 0, payloadLength, payload);
    }

    if (eventHead > eventTail) {
        const sysEvent_t event =
            eventQueue[eventTail & SYS_EVENT_QUEUE_MASK];
        ++eventTail;
        return event;
    }

    sysEvent_t event = {0};
    /* NOT_FROM_ORIGINAL_SOURCE: native monotonic-clock counterpart to the
     * original direct WinMM timeGetTime call. */
    event.time = (int32_t)Sys_Milliseconds();
    return event;
}
