#ifndef PRIVATE_OS_RTC_H
#define PRIVATE_OS_RTC_H

#include <revolution/os/OSRtc.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL __OSGetRTC(u32* rtc);
BOOL __OSSetRTC(u32 rtc);
void __OSInitSram();
OSSram* __OSLockSram();
OSSramEx* __OSLockSramEx();
BOOL __OSUnlockSram(BOOL commit);
BOOL __OSUnlockSramEx(BOOL commit);
BOOL __OSSyncSram();
BOOL __OSCheckSram();
BOOL __OSReadROM(void* buffer, s32 length, s32 offset);
BOOL __OSReadROMAsync(void* buffer, s32 length, s32 offset, void (*callback)());
u8 __OSGetBootMode();
void __OSSetBootMode(u8 ntd);
BOOL __OSGetRTCFlags();
BOOL __OSClearRTCFlags();

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_OS_RTC_H
