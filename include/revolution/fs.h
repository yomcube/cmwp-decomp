#ifndef REVOLUTION_FS_H
#define REVOLUTION_FS_H

#include <revolution/types.h>

/**
 * https://wiibrew.org/wiki//dev/fs
 */

#define FS_RESULT_OK 0

#define FS_RESULT_INVALID -101
#define FS_RESULT_ACCESS -102
#define FS_RESULT_CORRUPT -103
#define FS_RESULT_EXISTS -104
#define FS_RESULT_MAXFILES -105
#define FS_RESULT_MAXBLOCKS -106
#define FS_RESULT_MAXFD -109
#define FS_RESULT_OPENFD -111
#define FS_RESULT_ECC_CRIT -112
#define FS_RESULT_NOTEMPTY -115
#define FS_RESULT_BUSY -118
#define FS_RESULT_FATAL_ERROR -119

#define FS_MAX_PATH 64
#define FS_DIR_NAME_MAX (12 + 1)

typedef struct FSStats {
    u32 clusterSize;
    u32 freeClusters;
    u32 usedClusters;
    u32 badClusters;
    u32 reservedClusters;
    u32 freeInodes;
    u32 usedInodes;
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

s32 ISFS_CreateDir(const char* path, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm);
s32 ISFS_CreateDirAsync(const char* path, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_ReadDir(const char* path, char* filesOut, u32* fileCountOut);
s32 ISFS_ReadDirAsync(const char* path, char* filesOut, u32* fileCountOut, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_SetAttr(const char* path, u32 ownerId, u16 groupId, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm);
s32 ISFS_SetAttrAsync(const char* path, u32 ownerId, u16 groupId, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm, FSAsyncCallback callback,
                      void* callbackArg);

s32 ISFS_GetAttr(const char* path, u32* ownerIdOut, u16* groupIdOut, u32* attrOut, u32* ownerPermOut, u32* groupPermOut, u32* otherPermOut);
s32 ISFS_GetAttrAsync(const char* path, u32* ownerIdOut, u16* groupIdOut, u32* attrOut, u32* ownerPermOut, u32* groupPermOut, u32* otherPermOut,
                      FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Delete(const char* path);
s32 ISFS_DeleteAsync(const char* path, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Rename(const char* from, const char* to);
s32 ISFS_RenameAsync(const char* from, const char* to, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_GetUsage(const char* path, u32* blockCountOut, u32* fileCountOut);
s32 ISFS_GetUsageAsync(const char* path, u32* blockCountOut, u32* fileCountOut, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_CreateFile(const char* path, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm);
s32 ISFS_CreateFileAsync(const char* path, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_SetFileVersionControl(const char* path, u32 attr);
s32 ISFS_SetFileVersionControlAsync(const char* path, u32 attr, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Open(const char* path, s32 mode);
s32 ISFS_OpenAsync(const char* path, s32 mode, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_GetFileStats(s32 fd, FSFileStats* stats);
s32 ISFS_GetFileStatsAsync(s32 fd, FSFileStats* stats, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Seek(s32 fd, s32 offset, s32 mode);
s32 ISFS_SeekAsync(s32 fd, s32 offset, s32 mode, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Read(s32 fd, void* buf, u32 bufSize);
s32 ISFS_ReadAsync(s32 fd, void* buf, u32 bufSize, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Write(s32 fd, void* buf, u32 bufSize);
s32 ISFS_WriteAsync(s32 fd, void* buf, u32 bufSize, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Close(s32 fd);
s32 ISFS_CloseAsync(s32 fd, FSAsyncCallback callback, void* callbackArg);

s32 ISFS_Shutdown();
s32 ISFS_ShutdownAsync(FSAsyncCallback callback, void* callbackArg);

#endif  // REVOLUTION_FS_H
