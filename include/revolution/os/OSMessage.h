#ifndef _DOLPHIN_OSMESSAGE_H_
#define _DOLPHIN_OSMESSAGE_H_

#include <revolution/os/OSThread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OSMessageQueue {
    OSThreadQueue queueSend;     // 0x00
    OSThreadQueue queueReceive;  // 0x08
    void* msgArray;              // 0x10
    s32 msgCount;                // 0x14
    s32 firstIndex;              // 0x18
    s32 usedCount;               // 0x1C
} OSMessageQueue;

#define OS_MESSAGE_BLOCK 0x01

void OSInitMessageQueue(OSMessageQueue* mq, void* msgArray, s32 msgCount);
BOOL OSSendMessage(OSMessageQueue* mq, void* msg, s32 flags);
BOOL OSReceiveMessage(OSMessageQueue* mq, void* msg, s32 flags);
BOOL OSJamMessage(OSMessageQueue* mq, void* msg, s32 flags);

#ifdef __cplusplus
}
#endif

#endif  // _DOLPHIN_OSMESSAGE_H_
