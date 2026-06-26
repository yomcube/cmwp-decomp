#ifndef REVOLUTION_OS_ALARM_H
#define REVOLUTION_OS_ALARM_H

#include <revolution/types.h>

#include <revolution/os/OSContext.h>
#include <revolution/os/OSTime.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OSAlarm OSAlarm;
typedef void (*OSAlarmHandler)(OSAlarm* alarm, OSContext* context);
struct OSAlarm {
    OSAlarmHandler handler;  // 0x00
    u32 tag;                 // 0x04
    OSTime fire;             // 0x08
    OSAlarm* prev;           // 0x0C
    OSAlarm* next;           // 0x10
    OSTime period;           // 0x18
    OSTime start;            // 0x20
    void* userData;          // 0x28
};

BOOL OSCheckAlarmQueue();

void OSInitAlarm();

void OSCreateAlarm(OSAlarm* alarm);

void OSSetAlarm(OSAlarm* alarm, OSTime tick, OSAlarmHandler handler);
void OSSetAbsAlarm(OSAlarm* alarm, OSTime time, OSAlarmHandler handler);
void OSSetPeriodicAlarm(OSAlarm* alarm, OSTime start, OSTime period, OSAlarmHandler handler);

void OSCancelAlarm(OSAlarm* alarm);

void OSSetAlarmTag(OSAlarm* alarm, u32 tag);

void OSCancelAlarms(u32 tag);

void* OSGetAlarmUserData(OSAlarm* tag);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_OS_ALARM_H
