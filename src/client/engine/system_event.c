#include "system_event.h"

#include "networking/net_channel.h"
#include "platform/sdl_platform.h"
#include "system_console.h"

#include <stdlib.h>
#include <string.h>

enum {
    SYS_EVENT_QUEUE_MASK = SYS_EVENT_QUEUE_COUNT - 1
};

static sysEvent_t eventQueue[SYS_EVENT_QUEUE_COUNT];
static int32_t eventHead;
static int32_t eventTail;
static uint8_t sysPacketBuffer[MAX_MSGLEN];

int32_t sysMsgTime;

void Sys_QueEvent(int32_t time, sysEventType_t type,
                  int32_t value, int32_t value2,
                  int32_t payloadLength, void *payload)
{
    sysEvent_t *event = &eventQueue[eventHead & SYS_EVENT_QUEUE_MASK];

    if (eventHead - eventTail >= SYS_EVENT_QUEUE_COUNT) {
        Com_Printf("Sys_QueEvent: overflow\n");
        if (event->payload != NULL) {
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

void Sys_PumpEvents(void)
{
    CoduoSDL_PumpEvents();
}

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
    event.time = (int32_t)Sys_Milliseconds();
    return event;
}
