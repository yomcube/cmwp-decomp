#ifndef REVOLUTION_OS_STOPWATCH_H
#define REVOLUTION_OS_STOPWATCH_H

#include <revolution/types.h>

#include <revolution/os/OSTime.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct OSStopwatch {
    char* name;
    u32 hits;
    OSTime total;
    OSTime min;
    OSTime max;
    OSTime last;
    BOOL running;
    u32 _padding;
} OSStopwatch;

void OSInitStopwatch(OSStopwatch* sw, char* name);
void OSStartStopwatch(OSStopwatch* sw);
void OSStopStopwatch(OSStopwatch* sw);
OSTime OSCheckStopwatch(OSStopwatch* sw);
void OSResetStopwatch(OSStopwatch* sw);
void OSDumpStopwatch(OSStopwatch* sw);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // REVOLUTION_OS_STOPWATCH_H
