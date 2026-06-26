#ifndef PRIVATE_OS_TIME_H
#define PRIVATE_OS_TIME_H

#include <revolution/os/OSTime.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

OSTime __OSGetSystemTime();
OSTime __OSTimeToSystemTime(OSTime time);

#define __OSSystemTime (OSTime*)0x800030D8

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PRIVATE_OS_TIME_H
