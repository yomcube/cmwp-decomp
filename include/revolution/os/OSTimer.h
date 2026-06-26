#ifndef REVOLUTION_OS_TIMER_H
#define REVOLUTION_OS_TIMER_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef void (*OSTimerCallback)();

OSTimerCallback OSSetTimerCallback(OSTimerCallback callback);
void OSInitTimer(u32 time, u32 mode);
void OSStartTimer();
void OSStopTimer();

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // REVOLUTION_OS_TIMER_H
