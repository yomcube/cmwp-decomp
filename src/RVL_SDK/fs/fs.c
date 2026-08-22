#include <revolution/fs.h>

#include <revolution/ios.h>
#include <revolution/ipc.h>
#include <revolution/os.h>

#include <string.h>

#define FS_DELETE(x)                                                                                                                                 \
    if (x) {                                                                                                                                         \
        iosFree(hId, (x));                                                                                                                           \
    }

#define FS_HEAP_SIZE 0x1500

enum {
    CB_STATE_NONE,
    CB_STATE_GET_STATS,
    CB_STATE_READ_DIR,
    CB_STATE_GET_ATTR,
    CB_STATE_GET_USAGE,
    CB_STATE_GET_FILE_STATS
};

enum {
    FS_IOCTL_FORMAT = 1,
    FS_IOCTL_GET_STATS = 2,
    FS_IOCTL_CREATE_DIR = 3,
    FS_IOCTL_READ_DIR = 4,
    FS_IOCTL_SET_ATTR = 5,
    FS_IOCTL_GET_ATTR = 6,
    FS_IOCTL_DELETE_PATH = 7,
    FS_IOCTL_RENAME_PATH = 8,
    FS_IOCTL_CREATE_FILE = 9,
    FS_IOCTL_SET_FILE_VER_CTRL = 10,
    FS_IOCTL_GET_FILE_STATS = 11,
    FS_IOCTLV_GET_USAGE = 12,
    FS_IOCTL_SHUTDOWN_FS = 13
};

typedef struct {
    u32 ownerId;             // 0x00
    u16 groupId;             // 0x04
    char path[FS_MAX_PATH];  // 0x06
    u8 ownerPerm;            // 0x46
    u8 groupPerm;            // 0x47
    u8 otherPerm;            // 0x48
    u8 attr;                 // 0x49
} FSFileIoctl;

typedef struct {
    char from[FS_MAX_PATH];  // 0x00
    char to[FS_MAX_PATH];    // 0x40
} FSRenameIoctl;

typedef struct {
    FSStats* statsOut;  // 0x00
} FSGetStatsAsyncCtx;

typedef struct {
    u32* fileCountOut;  // 0x00
} FSReadDirAsyncCtx;

typedef struct {
    u32* ownerIdOut;    // 0x00
    u16* groupIdOut;    // 0x04
    u32* attrOut;       // 0x08
    u32* ownerPermOut;  // 0x0C
    u32* groupPermOut;  // 0x10
    u32* otherPermOut;  // 0x14
} FSGetAttrAsyncCtx;

typedef struct {
    u32* blockCountOut;  // 0x00
    u32* fileCountOut;   // 0x04
} FSGetUsageAsyncCtx;

typedef struct {
    FSFileStats* statsOut;  // 0x00
} FSGetFileStatsAsyncCtx;

typedef struct {
    union {
        FSFileIoctl file;
        FSRenameIoctl rename;
        u8 work[0x100];
    } ioctl;  // 0x00

    FSAsyncCallback callback;  // 0x100
    void* callbackArg;         // 0x104
    u32 callbackState;         // 0x108

    union {
        FSGetStatsAsyncCtx getStats;
        FSReadDirAsyncCtx readDir;
        FSGetAttrAsyncCtx getAttr;
        FSGetUsageAsyncCtx getUsage;
        FSGetFileStatsAsyncCtx getFileStats;
    } ctx;  // 0x10C
} FSCommandBlock;

static BOOL __fsInitialized = FALSE;
static IOSFd __fsFd = IOS_INVALID_FD;
static char* __devfs = NULL;
static u32 _asynCnt = 0;

static IOSHeapId hId;

s32 ISFS_OpenLib() {
    static void* lo;
    static void* hi;

    s32 ret = FS_RESULT_OK;
    u8* base = NULL;

    if (!__fsInitialized) {
        lo = IPCGetBufferLo();
        hi = IPCGetBufferHi();
    }

    __devfs = (char*)OSRoundUp32B((u32)lo);

    if (!__fsInitialized && __devfs + FS_MAX_PATH > (char*)hi) {
        OSReport("APP ERROR: Not enough IPC arena\n");
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    strcpy(__devfs, "/dev/fs");
    __fsFd = IOS_Open(__devfs, 0);

    if (__fsFd < 0) {
        ret = __fsFd;
        goto exit;
    }

    base = (u8*)__devfs;

    if (!__fsInitialized && base + FS_MAX_PATH + FS_HEAP_SIZE > (u8*)hi) {
        OSReport("APP ERROR: Not enough IPC arena\n");
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    if (!__fsInitialized) {
        IPCSetBufferLo(base + FS_MAX_PATH + FS_HEAP_SIZE);
        __fsInitialized = TRUE;
    }

    hId = iosCreateHeap(base, FS_MAX_PATH + FS_HEAP_SIZE);
    if (hId < 0) {
        ret = IOS_ERROR_FAIL_ALLOC;
    }

exit:
    return ret;
}

// great function, guys
s32 ISFS_OpenLibEx() {
    s32 ret = FS_RESULT_OK;

    __fsInitialized = FALSE;
    ret = ISFS_OpenLib();

    return ret;
}

s32 ISFS_CloseLib() {
    s32 ret = FS_RESULT_OK;

    if (__fsFd < 0) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    IOS_Close(__fsFd);
    __fsFd = IOS_INVALID_FD;
    ret = iosDestroyHeap(hId);

exit:
    return ret;
}

static s32 _FSGetStatsCb(s32 result, void* arg);
static s32 _FSReadDirCb(s32 result, void* arg);
static s32 _FSGetAttrCb(s32 result, void* arg);
static s32 _FSGetUsageCb(s32 result, void* arg);
static s32 _FSGetFileStatsCb(s32 result, void* arg);

static s32 _isfsFuncCb(s32 result, void* arg) {
    FSCommandBlock* block;
    s32 ret = IOS_ERROR_OK;

    block = (FSCommandBlock*)arg;
    ret = result;

    if (ret >= IOS_ERROR_OK) {
        switch (block->callbackState) {
            case CB_STATE_GET_STATS: {
                _FSGetStatsCb(result, arg);
                break;
            }
            case CB_STATE_READ_DIR: {
                _FSReadDirCb(result, arg);
                break;
            }
            case CB_STATE_GET_ATTR: {
                _FSGetAttrCb(result, arg);
                break;
            }
            case CB_STATE_GET_USAGE: {
                _FSGetUsageCb(result, arg);
                break;
            }
            case CB_STATE_GET_FILE_STATS: {
                _FSGetFileStatsCb(result, arg);
                break;
            }
            default: {
                break;
            }
        }
    }

    _asynCnt = 0;
    if (block->callback) {
        block->callback(ret, block->callbackArg);
    }

    FS_DELETE(arg);

    return ret;
}

s32 ISFS_Format() {
    s32 ret = FS_RESULT_OK;

    if (__fsFd < 0) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    ret = IOS_Ioctl(__fsFd, FS_IOCTL_FORMAT, NULL, 0, NULL, 0);

exit:
    return ret;
}

s32 ISFS_FormatAsync(FSAsyncCallback callback, void* callbackArg) {
    s32 ret;

    FSCommandBlock* block;

    if (__fsFd < 0) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ret = IOS_IoctlAsync(__fsFd, FS_IOCTL_FORMAT, NULL, 0, NULL, 0, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_GetStats(FSStats* stats) {
    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block = NULL;

    if (__fsFd < 0 || !stats) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    block->ctx.getStats.statsOut = stats;

    ret = IOS_Ioctl(__fsFd, FS_IOCTL_GET_STATS, NULL, 0, (u8*)&block->ioctl.file, sizeof(FSStats));
    if (ret == IOS_ERROR_OK) {
        memcpy(stats, block->ioctl.work, sizeof(FSStats));
    }

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

static s32 _FSGetStatsCb(s32 result, void* arg) {
    FSCommandBlock* block = (FSCommandBlock*)arg;

    s32 ret = FS_RESULT_OK;

    if (result == IOS_ERROR_OK) {
        memcpy(block->ctx.getStats.statsOut, block->ioctl.work, sizeof(FSStats));
    }

    return ret;
}

s32 ISFS_GetStatsAsync(FSStats* stats, FSAsyncCallback callback, void* callbackArg) {
    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block;

    if (!stats) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_GET_STATS;

    block->ctx.getStats.statsOut = stats;

    ret = IOS_IoctlAsync(__fsFd, FS_IOCTL_GET_STATS, NULL, 0, (u8*)&block->ioctl.file, sizeof(FSStats), _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_CreateDir(const char* path, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm) {
    size_t len;

    s32 ret = IOS_ERROR_OK;

    FSCommandBlock* block = NULL;

    FSFileIoctl* ioctl;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    ioctl = &block->ioctl.file;

    memcpy(ioctl->path, path, len + 1);

    ioctl->attr = attr;
    ioctl->ownerPerm = ownerPerm;
    ioctl->groupPerm = groupPerm;
    ioctl->otherPerm = otherPerm;

    ret = IOS_Ioctl(__fsFd, FS_IOCTL_CREATE_DIR, (u8*)ioctl, sizeof(FSFileIoctl), NULL, 0);

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

s32 ISFS_CreateDirAsync(const char* path, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm, FSAsyncCallback callback, void* callbackArg) {
    size_t len;

    s32 ret = IOS_ERROR_OK;

    FSCommandBlock* block;

    FSFileIoctl* ioctl;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ioctl = &block->ioctl.file;

    memcpy(ioctl->path, path, len + 1);

    ioctl->attr = attr;
    ioctl->ownerPerm = ownerPerm;
    ioctl->groupPerm = groupPerm;
    ioctl->otherPerm = otherPerm;

    ret = IOS_IoctlAsync(__fsFd, FS_IOCTL_CREATE_DIR, (u8*)ioctl, sizeof(FSFileIoctl), NULL, 0, _isfsFuncCb, block);
exit:
    return ret;
}

s32 ISFS_ReadDir(const char* path, char* filesOut, u32* fileCountOut) {
    size_t len;
    u32 inCount, outCount;

    s32 ret = FS_RESULT_OK;

    IOSIoVector* vec = NULL;

    char* pathWork;
    u32* countWork;

    FSCommandBlock* block = NULL;

    if (path == NULL || fileCountOut == NULL || __fsFd < 0 || (u32)filesOut % 32 != 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    vec = (IOSIoVector*)block->ioctl.work;
    pathWork = (char*)OSRoundUp32B((u32)vec + (sizeof(IOSIoVector) * 4));
    memcpy(pathWork, path, len + 1);

    vec[0].base = (u8*)pathWork;
    vec[0].length = FS_MAX_PATH;

    countWork = (u32*)OSRoundUp32B((u32)pathWork + FS_MAX_PATH);
    vec[1].base = (u8*)countWork;
    vec[1].length = sizeof(u32);

    if (filesOut) {
        inCount = 2;
        outCount = 2;

        *countWork = *fileCountOut;

        vec[2].base = (u8*)filesOut;
        vec[2].length = *fileCountOut * FS_DIR_NAME_MAX;

        vec[3].base = (u8*)countWork;
        vec[3].length = sizeof(u32);
    } else {
        inCount = 1;
        outCount = 1;
    }

    ret = IOS_Ioctlv(__fsFd, FS_IOCTL_READ_DIR, inCount, outCount, vec);
    if (ret == IOS_ERROR_OK) {
        *fileCountOut = *countWork;
    }

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

static s32 _FSReadDirCb(s32 result, void* arg) {
    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block = (FSCommandBlock*)arg;

    u8* work;

    if (result == IOS_ERROR_OK) {
        u8* ioctlWork = block->ioctl.work;

        work = (u8*)OSRoundUp32B((u32)ioctlWork + (sizeof(IOSIoVector) * 4));
        work = (u8*)OSRoundUp32B((u32)work + FS_MAX_PATH);
        *block->ctx.readDir.fileCountOut = *(u32*)work;
    }

    return ret;
}

s32 ISFS_ReadDirAsync(const char* path, char* filesOut, u32* fileCountOut, FSAsyncCallback callback, void* callbackArg) {
    size_t len;
    u32 outCount, inCount;

    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block;

    IOSIoVector* vec;

    char* pathWork;
    u32* countWork;

    if (path == NULL || fileCountOut == NULL || __fsFd < 0 || (u32)filesOut % 32 != 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_READ_DIR;

    block->ctx.readDir.fileCountOut = fileCountOut;

    vec = (IOSIoVector*)block->ioctl.work;
    pathWork = (char*)OSRoundUp32B((u32)vec + (sizeof(IOSIoVector) * 4));
    memcpy(pathWork, path, len + 1);

    vec[0].base = (u8*)pathWork;
    vec[0].length = FS_MAX_PATH;

    countWork = (u32*)OSRoundUp32B((u32)pathWork + FS_MAX_PATH);
    vec[1].base = (u8*)countWork;
    vec[1].length = sizeof(u32);

    if (filesOut) {
        inCount = 2;
        outCount = 2;

        *countWork = *fileCountOut;

        vec[2].base = (u8*)filesOut;
        vec[2].length = *fileCountOut * FS_DIR_NAME_MAX;

        vec[3].base = (u8*)countWork;
        vec[3].length = sizeof(u32);
    } else {
        inCount = 1;
        outCount = 1;
    }

    ret = IOS_IoctlvAsync(__fsFd, FS_IOCTL_READ_DIR, inCount, outCount, vec, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_SetAttr(const char* path, u32 ownerId, u16 groupId, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm) {
    size_t len;

    s32 ret = IOS_ERROR_OK;

    FSCommandBlock* block = NULL;

    FSFileIoctl* ioctl;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    ioctl = &block->ioctl.file;

    memcpy(ioctl->path, path, len + 1);

    ioctl->ownerId = ownerId;
    ioctl->groupId = groupId;
    ioctl->attr = attr;
    ioctl->ownerPerm = ownerPerm;
    ioctl->groupPerm = groupPerm;
    ioctl->otherPerm = otherPerm;

    ret = IOS_Ioctl(__fsFd, FS_IOCTL_SET_ATTR, (u8*)ioctl, sizeof(FSFileIoctl), NULL, 0);

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

s32 ISFS_SetAttrAsync(const char* path, u32 ownerId, u16 groupId, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm, FSAsyncCallback callback,
                      void* callbackArg) {
    size_t len;

    s32 ret = IOS_ERROR_OK;

    FSCommandBlock* block;

    FSFileIoctl* ioctl;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ioctl = &block->ioctl.file;

    memcpy(ioctl->path, path, len + 1);

    ioctl->ownerId = ownerId;
    ioctl->groupId = groupId;
    ioctl->attr = attr;
    ioctl->ownerPerm = ownerPerm;
    ioctl->groupPerm = groupPerm;
    ioctl->otherPerm = otherPerm;

    ret = IOS_IoctlAsync(__fsFd, FS_IOCTL_SET_ATTR, (u8*)ioctl, sizeof(FSFileIoctl), NULL, 0, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_GetAttr(const char* path, u32* ownerIdOut, u16* groupIdOut, u32* attrOut, u32* ownerPermOut, u32* groupPermOut, u32* otherPermOut) {
    s32 ret = FS_RESULT_OK;

    FSFileIoctl* ioctl;

    size_t len;

    FSCommandBlock* block = NULL;

    u8* ioctlWork;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH || ownerIdOut == NULL || groupIdOut == NULL ||
        attrOut == NULL || ownerPermOut == NULL || groupPermOut == NULL || otherPermOut == NULL) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    ioctlWork = (u8*)block->ioctl.work;
    memcpy(ioctlWork, path, len + 1);
    ioctl = (FSFileIoctl*)OSRoundUp32B((u32)ioctlWork + FS_MAX_PATH);

    ret = IOS_Ioctl(__fsFd, FS_IOCTL_GET_ATTR, ioctlWork, FS_MAX_PATH, (u8*)ioctl, sizeof(FSFileIoctl));

    if (ret == IOS_ERROR_OK) {
        *ownerIdOut = ioctl->ownerId;
        *groupIdOut = ioctl->groupId;
        *attrOut = ioctl->attr;
        *ownerPermOut = ioctl->ownerPerm;
        *groupPermOut = ioctl->groupPerm;
        *otherPermOut = ioctl->otherPerm;
    }

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

static s32 _FSGetAttrCb(s32 result, void* arg) {
    s32 ret = result;

    if (result == IOS_ERROR_OK) {
        FSCommandBlock* block = (FSCommandBlock*)arg;
        FSFileIoctl* ioctl = (FSFileIoctl*)OSRoundUp32B((u32)block->ioctl.work + FS_MAX_PATH);

        *block->ctx.getAttr.ownerIdOut = ioctl->ownerId;
        *block->ctx.getAttr.groupIdOut = ioctl->groupId;
        *block->ctx.getAttr.attrOut = ioctl->attr;
        *block->ctx.getAttr.ownerPermOut = ioctl->ownerPerm;
        *block->ctx.getAttr.groupPermOut = ioctl->groupPerm;
        *block->ctx.getAttr.otherPermOut = ioctl->otherPerm;
    }

    return ret;
}

s32 ISFS_GetAttrAsync(const char* path, u32* ownerIdOut, u16* groupIdOut, u32* attrOut, u32* ownerPermOut, u32* groupPermOut, u32* otherPermOut,
                      FSAsyncCallback callback, void* callbackArg) {
    s32 ret = FS_RESULT_OK;

    FSFileIoctl* ioctl;

    size_t len;

    FSCommandBlock* block;

    u8* ioctlWork;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH || ownerIdOut == NULL || groupIdOut == NULL ||
        attrOut == NULL || ownerPermOut == NULL || groupPermOut == NULL || otherPermOut == NULL) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->ctx.getAttr.ownerIdOut = ownerIdOut;
    block->ctx.getAttr.groupIdOut = groupIdOut;
    block->ctx.getAttr.attrOut = attrOut;
    block->ctx.getAttr.ownerPermOut = ownerPermOut;
    block->ctx.getAttr.groupPermOut = groupPermOut;
    block->ctx.getAttr.otherPermOut = otherPermOut;

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_GET_ATTR;

    ioctlWork = block->ioctl.work;
    memcpy(ioctlWork, path, len + 1);

    ioctl = (FSFileIoctl*)OSRoundUp32B((u32)ioctlWork + FS_MAX_PATH);
    ret = IOS_IoctlAsync(__fsFd, FS_IOCTL_GET_ATTR, ioctlWork, FS_MAX_PATH, (u8*)ioctl, sizeof(FSFileIoctl), _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_Delete(const char* path) {
    s32 ret = FS_RESULT_OK;
    size_t len;
    FSCommandBlock* block = NULL;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    memcpy(block->ioctl.work, path, len + 1);

    ret = IOS_Ioctl(__fsFd, FS_IOCTL_DELETE_PATH, block->ioctl.work, FS_MAX_PATH, NULL, 0);

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

s32 ISFS_DeleteAsync(const char* path, FSAsyncCallback callback, void* callbackArg) {
    s32 ret = FS_RESULT_OK;

    size_t len;

    FSCommandBlock* block;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    memcpy(block->ioctl.work, path, len + 1);
    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ret = IOS_IoctlAsync(__fsFd, FS_IOCTL_DELETE_PATH, block->ioctl.work, FS_MAX_PATH, NULL, 0, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_Rename(const char* from, const char* to) {
    s32 ret = FS_RESULT_OK;

    size_t lenFrom, lenTo;

    FSCommandBlock* block = NULL;

    FSRenameIoctl* ioctl;

    if (from == NULL || to == NULL || __fsFd < 0 || (lenFrom = strnlen(from, FS_MAX_PATH)) == FS_MAX_PATH ||
        (lenTo = strnlen(to, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    ioctl = &block->ioctl.rename;
    memcpy(ioctl->from, from, lenFrom + 1);
    memcpy(ioctl->to, to, lenTo + 1);

    ret = IOS_Ioctl(__fsFd, FS_IOCTL_RENAME_PATH, (u8*)ioctl, sizeof(FSRenameIoctl), NULL, 0);

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

s32 ISFS_RenameAsync(const char* from, const char* to, FSAsyncCallback callback, void* callbackArg) {
    s32 ret = FS_RESULT_OK;

    size_t lenFrom, lenTo;

    FSCommandBlock* block;

    FSRenameIoctl* ioctl;

    if (from == NULL || to == NULL || __fsFd < 0 || (lenFrom = strnlen(from, FS_MAX_PATH)) == FS_MAX_PATH ||
        (lenTo = strnlen(to, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ioctl = &block->ioctl.rename;
    memcpy(ioctl->from, from, lenFrom + 1);
    memcpy(ioctl->to, to, lenTo + 1);

    ret = IOS_IoctlAsync(__fsFd, FS_IOCTL_RENAME_PATH, (u8*)ioctl, sizeof(FSRenameIoctl), NULL, 0, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_GetUsage(const char* path, u32* blockCountOut, u32* fileCountOut) {
    s32 ret = FS_RESULT_OK;

    u32* blockCountWork;
    u32* fileCountWork;
    char* pathWork;

    IOSIoVector* vec = NULL;

    FSCommandBlock* block = NULL;

    size_t len;

    if (path == NULL || __fsFd < 0 || blockCountOut == NULL || fileCountOut == NULL || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    vec = (IOSIoVector*)block->ioctl.work;
    pathWork = (char*)OSRoundUp32B((u32)vec + (sizeof(IOSIoVector) * 3));
    memcpy(pathWork, path, len + 1);

    vec[0].base = (u8*)pathWork;
    vec[0].length = FS_MAX_PATH;

    blockCountWork = (u32*)OSRoundUp32B((u32)pathWork + FS_MAX_PATH);
    fileCountWork = (u32*)OSRoundUp32B((u32)blockCountWork + sizeof(u32));

    vec[1].base = (u8*)blockCountWork;
    vec[1].length = sizeof(u32);

    vec[2].base = (u8*)fileCountWork;
    vec[2].length = sizeof(u32);

    ret = IOS_Ioctlv(__fsFd, FS_IOCTLV_GET_USAGE, 1, 2, vec);
    if (ret == IOS_ERROR_OK) {
        *blockCountOut = *blockCountWork;
        *fileCountOut = *fileCountWork;
    }

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

static s32 _FSGetUsageCb(s32 result, void* arg) {
    s32 ret = FS_RESULT_OK;
    FSCommandBlock* block = (FSCommandBlock*)arg;
    u8* work;

    if (result == IOS_ERROR_OK) {
        u8* ioctlWork = block->ioctl.work;
        work = (u8*)OSRoundUp32B((u32)ioctlWork + (sizeof(IOSIoVector) * 4));
        work = (u8*)OSRoundUp32B((u32)work + FS_MAX_PATH);
        *block->ctx.getUsage.blockCountOut = *(u32*)work;

        work = (u8*)OSRoundUp32B((u32)work + sizeof(u32));
        *block->ctx.getUsage.fileCountOut = *(u32*)work;
    }

    return ret;
}

s32 ISFS_GetUsageAsync(const char* path, u32* blockCountOut, u32* fileCountOut, FSAsyncCallback callback, void* callbackArg) {
    size_t len;

    s32 ret = FS_RESULT_OK;

    u32* blockCountWork;
    u32* fileCountWork;

    IOSIoVector* vec;

    FSCommandBlock* block;

    char* pathWork;

    if (path == NULL || __fsFd < 0 || blockCountOut == NULL || fileCountOut == NULL || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_GET_USAGE;

    block->ctx.getUsage.blockCountOut = blockCountOut;
    block->ctx.getUsage.fileCountOut = fileCountOut;

    vec = (IOSIoVector*)block->ioctl.work;
    pathWork = (char*)OSRoundUp32B((u32)vec + (sizeof(IOSIoVector) * 3));
    memcpy(pathWork, path, len + 1);

    vec[0].base = (u8*)pathWork;
    vec[0].length = FS_MAX_PATH;

    blockCountWork = (u32*)OSRoundUp32B((u32)pathWork + FS_MAX_PATH);
    fileCountWork = (u32*)OSRoundUp32B((u32)blockCountWork + sizeof(u32));

    vec[1].base = (u8*)blockCountWork;
    vec[1].length = sizeof(u32);

    vec[2].base = (u8*)fileCountWork;
    vec[2].length = sizeof(u32);

    ret = IOS_IoctlvAsync(__fsFd, FS_IOCTLV_GET_USAGE, 1, 2, vec, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_CreateFile(const char* path, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm) {
    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block = NULL;

    size_t len;

    FSFileIoctl* ioctl;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    ioctl = &block->ioctl.file;

    memcpy(ioctl->path, path, len + 1);

    ioctl->attr = attr;
    ioctl->ownerPerm = ownerPerm;
    ioctl->groupPerm = groupPerm;
    ioctl->otherPerm = otherPerm;

    ret = IOS_Ioctl(__fsFd, FS_IOCTL_CREATE_FILE, (u8*)ioctl, sizeof(FSFileIoctl), NULL, 0);

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

s32 ISFS_CreateFileAsync(const char* path, u32 attr, u32 ownerPerm, u32 groupPerm, u32 otherPerm, FSAsyncCallback callback, void* callbackArg) {
    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block;

    size_t len;

    FSFileIoctl* ioctl;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ioctl = &block->ioctl.file;

    memcpy(ioctl->path, path, len + 1);

    ioctl->attr = attr;
    ioctl->ownerPerm = ownerPerm;
    ioctl->groupPerm = groupPerm;
    ioctl->otherPerm = otherPerm;

    ret = IOS_IoctlAsync(__fsFd, FS_IOCTL_CREATE_FILE, (u8*)ioctl, sizeof(FSFileIoctl), NULL, 0, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_SetFileVersionControl(const char* path, u32 attr) {
    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block = NULL;

    size_t len;

    FSFileIoctl* ioctl;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    ioctl = &block->ioctl.file;

    memcpy(ioctl->path, path, len + 1);

    ioctl->attr = attr;

    ret = IOS_Ioctl(__fsFd, FS_IOCTL_SET_FILE_VER_CTRL, (u8*)ioctl, sizeof(FSFileIoctl), NULL, 0);

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

s32 ISFS_SetFileVersionControlAsync(const char* path, u32 attr, FSAsyncCallback callback, void* callbackArg) {
    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block;

    size_t len;

    FSFileIoctl* ioctl;

    if (path == NULL || __fsFd < 0 || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ioctl = &block->ioctl.file;

    memcpy(ioctl->path, path, len + 1);

    ioctl->attr = attr;

    ret = IOS_IoctlAsync(__fsFd, FS_IOCTL_SET_FILE_VER_CTRL, (u8*)ioctl, sizeof(FSFileIoctl), NULL, 0, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_Open(const char* path, s32 mode) {
    s32 ret = FS_RESULT_OK;

    size_t len;

    FSCommandBlock* block = NULL;

    if (path == NULL || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    memcpy(block->ioctl.work, path, len + 1);

    ret = IOS_Open((char*)block->ioctl.work, mode);

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

s32 ISFS_OpenAsync(const char* path, s32 mode, FSAsyncCallback callback, void* callbackArg) {
    s32 ret = FS_RESULT_OK;

    size_t len;

    FSCommandBlock* block;

    if (path == NULL || (len = strnlen(path, FS_MAX_PATH)) == FS_MAX_PATH) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    memcpy(block->ioctl.work, path, len + 1);

    ret = IOS_OpenAsync((char*)block->ioctl.work, mode, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_GetFileStats(s32 fd, FSFileStats* stats) {
    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block = NULL;

    if (stats == NULL || ((u32)stats & 31)) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    ret = IOS_Ioctl(fd, FS_IOCTL_GET_FILE_STATS, NULL, 0, (u8*)&block->ioctl.file, sizeof(FSFileStats));
    if (ret == IOS_ERROR_OK) {
        memcpy(stats, block->ioctl.work, sizeof(FSFileStats));
    }

exit:
    if (block) {
        FS_DELETE(block);
    }

    return ret;
}

static s32 _FSGetFileStatsCb(s32 result, void* arg) {
    FSCommandBlock* block = (FSCommandBlock*)arg;

    s32 ret = FS_RESULT_OK;

    if (result == IOS_ERROR_OK) {
        memcpy(block->ctx.getFileStats.statsOut, block->ioctl.work, sizeof(FSFileStats));
    }

    return ret;
}

s32 ISFS_GetFileStatsAsync(s32 fd, FSFileStats* stats, FSAsyncCallback callback, void* callbackArg) {
    FSCommandBlock* block;

    s32 ret = FS_RESULT_OK;

    if (stats == NULL || ((u32)stats & 31)) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_GET_FILE_STATS;

    block->ctx.getFileStats.statsOut = stats;

    ret = IOS_IoctlAsync(fd, FS_IOCTL_GET_FILE_STATS, NULL, 0, (u8*)&block->ioctl.file, sizeof(FSFileStats), _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_Seek(s32 fd, s32 offset, s32 mode) {
    s32 ret = FS_RESULT_OK;

    ret = IOS_Seek(fd, offset, mode);

    return ret;
}

s32 ISFS_SeekAsync(s32 fd, s32 offset, s32 mode, FSAsyncCallback callback, void* callbackArg) {
    s32 ret;

    FSCommandBlock* block;

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ret = IOS_SeekAsync(fd, offset, mode, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_Read(s32 fd, void* buf, u32 bufSize) {
    s32 ret = FS_RESULT_OK;

    if (buf == NULL || !OSIsAligned32B(buf)) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    ret = IOS_Read(fd, buf, bufSize);

exit:
    return ret;
}

s32 ISFS_ReadAsync(s32 fd, void* buf, u32 bufSize, FSAsyncCallback callback, void* callbackArg) {
    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block;

    if (buf == NULL || !OSIsAligned32B(buf)) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ret = IOS_ReadAsync(fd, buf, bufSize, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_Write(s32 fd, void* buf, u32 bufSize) {
    s32 ret = FS_RESULT_OK;

    if (buf == NULL || !OSIsAligned32B(buf)) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    ret = IOS_Write(fd, buf, bufSize);

exit:
    return ret;
}

s32 ISFS_WriteAsync(s32 fd, void* buf, u32 bufSize, FSAsyncCallback callback, void* callbackArg) {
    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block;

    if (buf == NULL || !OSIsAligned32B(buf)) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ret = IOS_WriteAsync(fd, buf, bufSize, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_Close(s32 fd) {
    s32 ret = FS_RESULT_OK;

    ret = IOS_Close(fd);

    return ret;
}

s32 ISFS_CloseAsync(s32 fd, FSAsyncCallback callback, void* callbackArg) {
    s32 ret;

    FSCommandBlock* block;

    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);
    if (!block) {
        ret = FS_RESULT_BUSY;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ret = IOS_CloseAsync(fd, _isfsFuncCb, block);

exit:
    return ret;
}

s32 ISFS_Shutdown() {
    s32 ret = FS_RESULT_OK;

    if (__fsFd < 0) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    ret = IOS_Ioctl(__fsFd, FS_IOCTL_SHUTDOWN_FS, NULL, 0, NULL, 0);

exit:
    return ret;
}

s32 ISFS_ShutdownAsync(FSAsyncCallback callback, void* callbackArg) {
    s32 ret = FS_RESULT_OK;

    FSCommandBlock* block;

    // @bug Memory allocation result is not validated
    block = (FSCommandBlock*)iosAllocAligned(hId, OSRoundUp32B(sizeof(FSCommandBlock)), 32);

    if (__fsFd < 0) {
        ret = FS_RESULT_INVALID;
        goto exit;
    }

    block->callback = callback;
    block->callbackArg = callbackArg;
    block->callbackState = CB_STATE_NONE;

    ret = IOS_IoctlAsync(__fsFd, FS_IOCTL_SHUTDOWN_FS, NULL, 0, NULL, 0, _isfsFuncCb, block);

exit:
    return ret;
}
