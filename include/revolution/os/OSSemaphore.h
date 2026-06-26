#ifndef REVOLUTION_OS_SEMAPHORE_H
#define REVOLUTION_OS_SEMAPHORE_H

#include <revolution/types.h>

#include <revolution/os/OSThread.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct OSSemaphore {
    s32 count;
    OSThreadQueue queue;
} OSSemaphore;

void OSInitSemaphore(OSSemaphore* sem, s32 count);
s32 OSWaitSemaphore(OSSemaphore* sem);
s32 OSTryWaitSemaphore(OSSemaphore* sem);
s32 OSSignalSemaphore(OSSemaphore* sem);
s32 OSGetSemaphoreCount(OSSemaphore* sem);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // REVOLUTION_OS_SEMAPHORE_H
