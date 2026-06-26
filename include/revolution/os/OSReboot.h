#ifndef REVOLUTION_OS_REBOOT_H
#define REVOLUTION_OS_REBOOT_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void OSSetSaveRegion(void* start, void* end);
void OSGetSaveRegion(void** start, void** end);
void OSGetSavedRegion(void** start, void** end);
void __OSReboot(u32 resetCode, u32 bootDol);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // REVOLUTION_OS_REBOOT_H
