#ifndef PRIVATE_OS_BOOT_INFO_H
#define PRIVATE_OS_BOOT_INFO_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/* uhhh gonna be here for now */

typedef struct DVDDiskID {
    char gameName[4];
    char company[2];
    u8 diskNumber;
    u8 gameVersion;
    u8 streaming;
    u8 streamingBufSize;
    u8 padding[22];
} DVDDiskID;

typedef struct OSBootInfo_s {
    DVDDiskID DVDDiskID;
    u32 magic;
    u32 version;
    u32 memorySize;
    u32 consoleType;
    void* arenaLo;
    void* arenaHi;
    void* FSTLocation;
    u32 FSTMaxLength;
} OSBootInfo;

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PRIVATE_OS_BOOT_INFO_H
