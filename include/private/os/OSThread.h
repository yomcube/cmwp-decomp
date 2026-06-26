#ifndef PRIVATE_OS_THREAD_H
#define PRIVATE_OS_THREAD_H

#include <revolution/os/OSThread.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void __OSThreadInit();
s32 __OSGetEffectivePriority(OSThread* thread);
void __OSPromoteThread(OSThread* thread, s32 priority);
void __OSReschedule();

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PRIVATE_OS_THREAD_H
