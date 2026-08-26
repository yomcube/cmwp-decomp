#ifndef REVOLUTION_FS_H
#define REVOLUTION_FS_H

#include <revolution/types.h>

#include <revolution/ios/iosctypes.h>
#include <revolution/ios/iostypes.h>

/**
 * https://wiibrew.org/wiki//dev/fs
 */

#define ISFS_ERROR_OK 0

#define ISFS_ERROR_INVALID -101
#define ISFS_ERROR_ACCESS -102
#define ISFS_ERROR_CORRUPT -103
#define ISFS_ERROR_NEEDFORMAT -104
#define ISFS_ERROR_EXISTS -105
#define ISFS_ERROR_NOEXISTS -106
#define ISFS_ERROR_MAXFILES -107
#define ISFS_ERROR_MAXBLOCKS -108
#define ISFS_ERROR_MAXFD -109
#define ISFS_ERROR_MAXDEPTH -110
#define ISFS_ERROR_OPENFD -111
#define ISFS_ERROR_BADBLOCK -112
#define ISFS_ERROR_ECC -113
#define ISFS_ERROR_ECC_CRIT -114
#define ISFS_ERROR_NOTEMPTY -115
#define ISFS_ERROR_AUTHENTICATION -116
#define ISFS_ERROR_UNKNOWN -117 /* ??? */
#define ISFS_ERROR_BUSY -118
#define ISFS_ERROR_FATAL_ERROR -119

#define ISFS_MAX_PATH 64
#define ISFS_INODE_NAMELEN 12
#define ISFS_FD_ENTRIES 16

#define ISFS_SEEK_BEG 0
#define ISFS_SEEK_CUR 1
#define ISFS_SEEK_END 2

#define ISFS_ACCESS_NONE 0
#define ISFS_ACCESS_READ (1 << 0)
#define ISFS_ACCESS_WRITE (1 << 1)
#define ISFS_ACCESS_RW (ISFS_ACCESS_READ | ISFS_ACCESS_WRITE)

typedef struct FSStats {
    u32 blockSize;  // 0x00

    u32 freeBlocks;  // 0x04
    u32 usedBlocks;  // 0x08

    u32 badBlocks;       // 0x0C
    u32 reservedBlocks;  // 0x10

    u32 freeInodes;  // 0x14
    u32 usedInodes;  // 0x18
} FSStats;

typedef struct FSFileStats {
    u32 length;  // 0x00
    u32 pos;     // 0x04
} FSFileStats;

typedef void (*FSAsyncCallback)(s32 result, void* arg);

s32 ISFS_OpenLib();
s32 ISFS_OpenLibEx();

s32 ISFS_CloseLib();

s32 ISFS_Format();
s32 ISFS_FormatAsync(FSAsyncCallback callback, void* callbackArg);

s32 ISFS_GetStats(FSStats* stats);
s32 ISFS_GetStatsAsync(FSStats* stats, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_CreateDir(const char* path, u32 attr, u32 ownerAcc, u32 groupAcc, u32 otherAcc);
s32 ISFS_CreateDirAsync(const char* path, u32 attr, u32 ownerAcc, u32 groupAcc, u32 otherAcc, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_ReadDir(const char* path, char* filesOut, u32* fileCountOut);
s32 ISFS_ReadDirAsync(const char* path, char* filesOut, u32* fileCountOut, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_SetAttr(const char* path, u32 ownerId, u16 groupId, u32 attr, u32 ownerAcc, u32 groupAcc, u32 otherAcc);
s32 ISFS_SetAttrAsync(const char* path, u32 ownerId, u16 groupId, u32 attr, u32 ownerAcc, u32 groupAcc, u32 otherAcc, FSAsyncCallback callback,
                      void* callbackArg);

s32 ISFS_GetAttr(const char* path, u32* ownerIdOut, u16* groupIdOut, u32* attrOut, u32* ownerAccOut, u32* groupAccOut, u32* otherAccOut);
s32 ISFS_GetAttrAsync(const char* path, u32* ownerIdOut, u16* groupIdOut, u32* attrOut, u32* ownerAccOut, u32* groupAccOut, u32* otherAccOut,
                      FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Delete(const char* path);
s32 ISFS_DeleteAsync(const char* path, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Rename(const char* from, const char* to);
s32 ISFS_RenameAsync(const char* from, const char* to, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_GetUsage(const char* path, u32* blockCountOut, u32* inodeCountOut);
s32 ISFS_GetUsageAsync(const char* path, u32* blockCountOut, u32* inodeCountOut, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_CreateFile(const char* path, u32 attr, u32 ownerAcc, u32 groupAcc, u32 otherAcc);
s32 ISFS_CreateFileAsync(const char* path, u32 attr, u32 ownerAcc, u32 groupAcc, u32 otherAcc, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_SetFileVersionControl(const char* path, u32 attr);
s32 ISFS_SetFileVersionControlAsync(const char* path, u32 attr, FSAsyncCallback callback, void* callbackArg);

IOSFd ISFS_Open(const char* path, s32 mode);
IOSFd ISFS_OpenAsync(const char* path, s32 mode, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_GetFileStats(IOSFd fd, FSFileStats* stats);
s32 ISFS_GetFileStatsAsync(IOSFd fd, FSFileStats* stats, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Seek(IOSFd fd, s32 offset, s32 mode);
s32 ISFS_SeekAsync(IOSFd fd, s32 offset, s32 mode, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Read(IOSFd fd, void* buf, u32 bufSize);
s32 ISFS_ReadAsync(IOSFd fd, void* buf, u32 bufSize, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Write(IOSFd fd, const void* buf, u32 bufSize);
s32 ISFS_WriteAsync(IOSFd fd, const void* buf, u32 bufSize, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Close(IOSFd fd);
s32 ISFS_CloseAsync(IOSFd fd, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Shutdown();
s32 ISFS_ShutdownAsync(FSAsyncCallback callback, void* callbackArg);

#endif  // REVOLUTION_FS_H
