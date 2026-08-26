#include <private/nand.h>
#include <revolution/nand.h>

#include <revolution/os.h>

#include <revolution/fs.h>
#include <revolution/ios.h>

#include <stdio.h>
#include <string.h>

static void nandSplitPerm(u8 perm, u32* ownerAcc, u32* groupAcc, u32* othersAcc) NO_INLINE;
static void nandGetStatusCallback(s32 result, void* arg);
static void nandGetFileStatusAsyncCallback(s32 result, void* arg);
static BOOL nandInspectPermission(u8 perm);
#if SDK_VERSION >= 20091211
static BOOL nandCheckPathName(const char* path);
#endif

static s32 nandCreate(const char* path, u8 perm, u8 attr, NANDCommandBlock* block, BOOL asyncFlag, BOOL privateFlag) {
    char absPath[NAND_MAX_PATH] = "";
    u32 ownerAcc = 0, groupAcc = 0, othersAcc = 0;

    ASSERTMSGLINE(254, path, "NULL pointer is detected.");

#if SDK_VERSION >= 20091211
    if (!nandCheckPathName(path)) {
        return ISFS_ERROR_INVALID;
    }
#endif

    nandGenerateAbsPath(absPath, path);

    if (!privateFlag && nandIsPrivatePath(absPath)) {
        return ISFS_ERROR_ACCESS;
    }

    if (!nandInspectPermission(perm)) {
        return ISFS_ERROR_INVALID;
    }

    nandSplitPerm(perm, &ownerAcc, &groupAcc, &othersAcc);

    if (asyncFlag) {
        return ISFS_CreateFileAsync(absPath, attr, ownerAcc, groupAcc, othersAcc, nandCallback, block);
    } else {
        return ISFS_CreateFile(absPath, attr, ownerAcc, groupAcc, othersAcc);
    }
}

s32 NANDCreate(const char* path, u8 perm, u8 attr) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandCreate(path, perm, attr, NULL, FALSE, FALSE));
}

s32 NANDPrivateCreate(const char* path, u8 perm, u8 attr) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandCreate(path, (u8)perm, (u8)attr, NULL, FALSE, TRUE));
}

s32 NANDCreateAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandCreate(path, perm, attr, block, TRUE, FALSE));
}

s32 NANDPrivateCreateAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandCreate(path, (u8)perm, (u8)attr, block, TRUE, TRUE));
}

static s32 nandDelete(const char* path, NANDCommandBlock* block, BOOL asyncFlag, BOOL privateFlag) {
    char absPath[NAND_MAX_PATH] = "";

    nandGenerateAbsPath(absPath, path);

    if (!privateFlag && nandIsPrivatePath(absPath)) {
        return ISFS_ERROR_ACCESS;
    }

    if (asyncFlag) {
        return ISFS_DeleteAsync(absPath, nandCallback, block);
    } else {
        return ISFS_Delete(absPath);
    }
}

s32 NANDDelete(const char* path) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandDelete(path, NULL, FALSE, FALSE));
}

s32 NANDPrivateDelete(const char* path) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandDelete(path, NULL, FALSE, TRUE));
}

s32 NANDDeleteAsync(const char* path, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandDelete(path, block, TRUE, FALSE));
}

s32 NANDPrivateDeleteAsync(const char* path, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandDelete(path, block, TRUE, TRUE));
}

s32 NANDRead(NANDFileInfo* info, void* buf, u32 bufLength) {
    ASSERTMSGLINE(412, info, "*info is NULL pointer!");
    ASSERTMSGLINE(413, buf, "*buf is NULL pointer!");
    ASSERTMSGLINE(414, OSIsAligned32B(buf), "Buffer must be 32 bytes aligned.");
    ASSERTMSGLINE(415, OSIsAligned32B(bufLength), "Buffer length must be multiples of 32 bytes.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(ISFS_Read(info->fileDescriptor, buf, bufLength));
}

s32 NANDReadAsync(NANDFileInfo* info, void* buf, u32 bufLength, NANDAsyncCallback callback, NANDCommandBlock* block) {
    ASSERTMSGLINE(428, info, "*info is NULL pointer!");
    ASSERTMSGLINE(429, buf, "*buf is NULL pointer!");
    ASSERTMSGLINE(430, OSIsAligned32B(buf), "Buffer must be 32 bytes aligned.");
    ASSERTMSGLINE(431, OSIsAligned32B(bufLength), "Buffer length must be multiples of 32 bytes.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(ISFS_ReadAsync(info->fileDescriptor, buf, bufLength, nandCallback, block));
}

s32 NANDWrite(NANDFileInfo* info, const void* buf, u32 bufLength) {
    ASSERTMSGLINE(448, info, "*info is NULL pointer!");
    ASSERTMSGLINE(449, buf, "*buf is NULL pointer!");
    ASSERTMSGLINE(450, OSIsAligned32B(buf), "Buffer must be 32 bytes aligned.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(ISFS_Write(info->fileDescriptor, buf, bufLength));
}

s32 NANDWriteAsync(NANDFileInfo* info, const void* buf, u32 bufLength, NANDAsyncCallback callback, NANDCommandBlock* block) {
    ASSERTMSGLINE(463, info, "*info is NULL pointer!");
    ASSERTMSGLINE(464, buf, "*buf is NULL pointer!");
    ASSERTMSGLINE(465, OSIsAligned32B(buf), "Buffer must be 32 bytes aligned.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(ISFS_WriteAsync(info->fileDescriptor, buf, bufLength, nandCallback, block));
}

static s32 nandSeek(s32 fd, s32 offset, s32 whence, NANDCommandBlock* block, BOOL asyncFlag) {
    s32 isfsSeek = -1;

    switch (whence) {
        case NAND_SEEK_BEG: {
            isfsSeek = ISFS_SEEK_BEG;
            break;
        }
        case NAND_SEEK_CUR: {
            isfsSeek = ISFS_SEEK_CUR;
            break;
        }
        case NAND_SEEK_END: {
            isfsSeek = ISFS_SEEK_END;
            break;
        }
        default: {
            (void)0;
            break;
        }
    }

    if (asyncFlag) {
        return ISFS_SeekAsync(fd, offset, isfsSeek, nandCallback, block);
    } else {
        return ISFS_Seek(fd, offset, isfsSeek);
    }
}

s32 NANDSeek(NANDFileInfo* info, s32 offset, s32 whence) {
    ASSERTMSGLINE(513, info, "*info is NULL pointer!");
    ASSERTMSGLINE(514, whence == NAND_SEEK_BEG || whence == NAND_SEEK_CUR || whence == NAND_SEEK_END, "Illegal whence parameter.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandSeek(info->fileDescriptor, offset, whence, NULL, FALSE));
}

s32 NANDSeekAsync(NANDFileInfo* info, s32 offset, s32 whence, NANDAsyncCallback callback, NANDCommandBlock* block) {
    ASSERTMSGLINE(527, info, "*info is NULL pointer!");
    ASSERTMSGLINE(528, whence == NAND_SEEK_BEG || whence == NAND_SEEK_CUR || whence == NAND_SEEK_END, "Illegal whence parameter.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandSeek(info->fileDescriptor, offset, whence, block, TRUE));
}

static s32 nandReadDir(const char* path, char* filesOut, u32* fileCountOut, NANDCommandBlock* block, BOOL asyncFlag, BOOL privateFlag) {
    char absPath[NAND_MAX_PATH] = "";

    ASSERTMSGLINE(547, OSIsAligned32B(filesOut), "Buffer must be 32 bytes aligned.");

    nandGenerateAbsPath(absPath, path);

    if (!privateFlag && nandIsPrivatePath(absPath)) {
        return ISFS_ERROR_ACCESS;
    }

    if (asyncFlag) {
        return ISFS_ReadDirAsync(absPath, filesOut, fileCountOut, nandCallback, block);
    } else {
        return ISFS_ReadDir(absPath, filesOut, fileCountOut);
    }
}

s32 NANDReadDir(const char* path, char* filesOut, u32* fileCountOut) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandReadDir(path, filesOut, fileCountOut, NULL, FALSE, FALSE));
}

s32 NANDPrivateReadDir(const char* path, char* filesOut, u32* fileCountOut) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandReadDir(path, filesOut, fileCountOut, NULL, FALSE, TRUE));
}

s32 NANDReadDirAsync(const char* path, char* filesOut, u32* fileCountOut, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandReadDir(path, filesOut, fileCountOut, block, TRUE, FALSE));
}

s32 NANDPrivateReadDirAsync(const char* path, char* filesOut, u32* fileCountOut, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandReadDir(path, filesOut, fileCountOut, block, TRUE, TRUE));
}

static s32 nandCreateDir(const char* path, u8 perm, u8 attr, NANDCommandBlock* block, BOOL asyncFlag, BOOL privateFlag) {
    char absPath[NAND_MAX_PATH] = "";

#if SDK_VERSION >= 20091211
    if (!nandCheckPathName(path)) {
        return ISFS_ERROR_INVALID;
    }
#endif

    nandGenerateAbsPath(absPath, path);

    if (!privateFlag && nandIsPrivatePath(absPath)) {
        return ISFS_ERROR_ACCESS;
    }

    if (!nandInspectPermission(perm)) {
        return ISFS_ERROR_INVALID;
    }

    {
        u32 ownerAcc = 0, groupAcc = 0, othersAcc = 0;
        nandSplitPerm(perm, &ownerAcc, &groupAcc, &othersAcc);

        if (asyncFlag) {
            return ISFS_CreateDirAsync(absPath, attr, ownerAcc, groupAcc, othersAcc, nandCallback, block);
        } else {
            return ISFS_CreateDir(absPath, attr, ownerAcc, groupAcc, othersAcc);
        }
    }
}

s32 NANDCreateDir(const char* path, u8 perm, u8 attr) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandCreateDir(path, perm, attr, NULL, FALSE, FALSE));
}

s32 NANDPrivateCreateDir(const char* path, u8 perm, u8 attr) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandCreateDir(path, (u8)perm, (u8)attr, NULL, FALSE, TRUE));
}

s32 NANDCreateDirAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandCreateDir(path, perm, attr, block, TRUE, FALSE));
}

s32 NANDPrivateCreateDirAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandCreateDir(path, (u8)perm, (u8)attr, block, TRUE, TRUE));
}

static s32 nandMove(const char* from, const char* to, NANDCommandBlock* block, BOOL asyncFlag, BOOL privateFlag) {
    char absPathFrom[NAND_MAX_PATH] = "";
    char absPathTo[NAND_MAX_PATH] = "";
    char relativeName[ISFS_INODE_NAMELEN + 1] = "";

    ASSERTMSGLINE(708, from, "NULL pointer is detected.\n");
    ASSERTMSGLINE(709, to, "NULL pointer is detected.\n");

    nandGenerateAbsPath(absPathFrom, from);
    nandGetRelativeName(relativeName, absPathFrom);
    nandGenerateAbsPath(absPathTo, to);

    if (strcmp(absPathTo, "/") == 0) {
        sprintf(absPathTo, "/%s", relativeName);
    } else {
        strcat(absPathTo, "/");
        strcat(absPathTo, relativeName);
    }

    if (!privateFlag && (nandIsPrivatePath(absPathFrom) || nandIsPrivatePath(absPathTo))) {
        return ISFS_ERROR_ACCESS;
    }

    if (asyncFlag) {
        return ISFS_RenameAsync(absPathFrom, absPathTo, nandCallback, block);
    } else {
        return ISFS_Rename(absPathFrom, absPathTo);
    }
}

s32 NANDMove(const char* from, const char* to) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandMove(from, to, NULL, FALSE, FALSE));
}

s32 NANDPrivateMove(const char* from, const char* to) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandMove(from, to, NULL, FALSE, TRUE));
}

s32 NANDMoveAsync(const char* from, const char* to, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandMove(from, to, block, TRUE, FALSE));
}

s32 NANDPrivateMoveAsync(const char* from, const char* to, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandMove(from, to, block, TRUE, TRUE));
}

s32 NANDFreeBlocks(u32* bytes, u32* inodes) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;
    FSStats stats ALIGN32;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    isfsErr = ISFS_GetStats(&stats);
    if (isfsErr == ISFS_ERROR_OK) {
        if (bytes) {
            *bytes = NAND_FSBLOCKS_TO_BYTES(stats.freeBlocks);
            ASSERTMSGLINE(807, *bytes <= NAND_FSBLOCKS_TO_BYTES(NAND_MAX_FSBLOCKS), "NAND library internal error:  bytes value must be illegal.");
        }
        if (inodes) {
            *inodes = stats.freeInodes;
            ASSERTMSGLINE(812, *inodes <= NAND_MAX_INODES, "NAND library internal error:  inodes value must be illegal.");
        }
    }

    return nandConvertErrorCode(isfsErr);
}

static void nandFreeBlocksCallback(s32 result, void* arg);

s32 NANDFreeBlocksAsync(u32* bytes, u32* inodes, NANDAsyncCallback callback, NANDCommandBlock* block) {
    FSStats* stats = (FSStats*)OSRoundUp32B((u32)block->absPath);  // Work buffer?
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->bytes = bytes;
    block->inodes = inodes;
    isfsErr = ISFS_GetStatsAsync(stats, nandFreeBlocksCallback, block);

    return nandConvertErrorCode(isfsErr);
}

static void nandFreeBlocksCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;
    FSStats* stats = (FSStats*)OSRoundUp32B((u32)block->absPath);  // Work buffer?

    if (result == ISFS_ERROR_OK) {
        u32* bytes = block->bytes;
        u32* inodes = block->inodes;
        if (bytes) {
            *bytes = NAND_FSBLOCKS_TO_BYTES(stats->freeBlocks);
            ASSERTMSGLINE(848, *bytes <= NAND_FSBLOCKS_TO_BYTES(NAND_MAX_FSBLOCKS), "NAND library internal error:  bytes value must be illegal.");
        }
        if (inodes) {
            *inodes = stats->freeInodes;
            ASSERTMSGLINE(853, *inodes <= NAND_MAX_INODES, "NAND library internal error:  inodes value must be illegal.");
        }
    }

    ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
}

static s32 nandGetFileStatus(s32 fd, u32* lengthOut, u32* posOut) {
    FSFileStats stats ALIGN32;
    s32 isfsErr;

    isfsErr = ISFS_GetFileStats(fd, &stats);
    if (isfsErr == ISFS_ERROR_OK) {
        if (lengthOut) {
            *lengthOut = stats.length;
            ASSERTMSGLINE(873, *lengthOut <= NAND_FSBLOCKS_TO_BYTES(NAND_MAX_FSBLOCKS),
                          "NAND library internal error:  length value must be illegal.");
        }

        if (posOut) {
            *posOut = stats.pos;
        }
    }

    return isfsErr;
}

s32 NANDGetLength(NANDFileInfo* info, u32* lengthOut) {
    ASSERTMSGLINE(886, info, "NULL pointer detected.");
    ASSERTMSGLINE(887, lengthOut, "NULL pointer detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandGetFileStatus(info->fileDescriptor, lengthOut, NULL));
}

s32 NANDTell(NANDFileInfo* info, u32* position) {
    ASSERTMSGLINE(900, info, "NULL pointer detected.");
    ASSERTMSGLINE(901, position, "NULL pointer detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandGetFileStatus(info->fileDescriptor, NULL, position));
}

static s32 nandGetFileStatusAsync(s32 fd, NANDCommandBlock* block) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;
    FSFileStats* stats = (FSFileStats*)OSRoundUp32B((u32)block->absPath);  // Work buffer?

    isfsErr = ISFS_GetFileStatsAsync(fd, stats, nandGetFileStatusAsyncCallback, block);

    return isfsErr;
}

static void nandGetFileStatusAsyncCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;
    FSFileStats* stats = (FSFileStats*)OSRoundUp32B((u32)block->absPath);  // Work buffer?

    if (result == ISFS_ERROR_OK) {
        if (block->length) {
            *block->length = stats->length;
            ASSERTMSGLINE(933, *block->length <= NAND_FSBLOCKS_TO_BYTES(NAND_MAX_FSBLOCKS),
                          "NAND library internal error:  length value must be illegal.");
        }

        if (block->pos) {
            *block->pos = stats->pos;
        }
    }

    ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
}

s32 NANDGetLengthAsync(NANDFileInfo* info, u32* lengthOut, NANDAsyncCallback callback, NANDCommandBlock* block) {
    ASSERTMSGLINE(947, info, "NULL pointer detected.");
    ASSERTMSGLINE(948, lengthOut, "NULL pointer detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->length = lengthOut;
    block->pos = NULL;

    return nandConvertErrorCode(nandGetFileStatusAsync(info->fileDescriptor, block));
}

s32 NANDTellAsync(NANDFileInfo* info, u32* posOut, NANDAsyncCallback callback, NANDCommandBlock* block) {
    ASSERTMSGLINE(964, info, "NULL pointer detected.");
    ASSERTMSGLINE(965, posOut, "NULL pointer detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->length = NULL;
    block->pos = posOut;

    return nandConvertErrorCode(nandGetFileStatusAsync(info->fileDescriptor, block));
}

static void nandComposePerm(u8* out, u32 ownerAcc, u32 groupAcc, u32 othersAcc) {
    u32 perm = 0;

    if (ownerAcc & NAND_ACCESS_READ) {
        perm |= NAND_PERM_USER_READ;
    }

    if (ownerAcc & NAND_ACCESS_WRITE) {
        perm |= NAND_PERM_USER_WRITE;
    }

    if (groupAcc & NAND_ACCESS_READ) {
        perm |= NAND_PERM_GROUP_READ;
    }

    if (groupAcc & NAND_ACCESS_WRITE) {
        perm |= NAND_PERM_GROUP_WRITE;
    }

    if (othersAcc & NAND_ACCESS_READ) {
        perm |= NAND_PERM_BOTH_READ;
    }

    if (othersAcc & NAND_ACCESS_WRITE) {
        perm |= NAND_PERM_BOTH_WRITE;
    }

    ASSERTMSGLINE(1007, !(perm & 0xFFFFFF00), "NAND library internal error.\n");

    *out = perm;
}

static void nandSplitPerm(u8 perm, u32* ownerAcc, u32* groupAcc, u32* othersAcc) {
    *ownerAcc = 0;
    *groupAcc = 0;
    *othersAcc = 0;

    if (perm & NAND_PERM_USER_READ) {
        *ownerAcc |= NAND_ACCESS_READ;
    }

    if (perm & NAND_PERM_USER_WRITE) {
        *ownerAcc |= NAND_ACCESS_WRITE;
    }

    if (perm & NAND_PERM_GROUP_READ) {
        *groupAcc |= NAND_ACCESS_READ;
    }

    if (perm & NAND_PERM_GROUP_WRITE) {
        *groupAcc |= NAND_ACCESS_WRITE;
    }

    if (perm & NAND_PERM_BOTH_READ) {
        *othersAcc |= NAND_ACCESS_READ;
    }

    if (perm & NAND_PERM_BOTH_WRITE) {
        *othersAcc |= NAND_ACCESS_WRITE;
    }
}

static s32 nandGetStatus(const char* path, NANDStatus* status, NANDCommandBlock* block, BOOL asyncFlag, BOOL privateFlag) {
    s32 isfsErr;
    char absPath[NAND_MAX_PATH] = "";

    ASSERTMSGLINE(1052, path, "NULL pointer detected.");
    ASSERTMSGLINE(1053, status, "NULL pointer detected.");

    nandGenerateAbsPath(absPath, path);

    if (!privateFlag && nandIsUnderPrivatePath(absPath)) {
        return ISFS_ERROR_ACCESS;
    }

    if (asyncFlag) {
        return ISFS_GetAttrAsync(absPath, &status->ownerId, &status->groupId, &block->attr, &block->ownerAcc, &block->groupAcc, &block->othersAcc,
                                 nandGetStatusCallback, block);
    } else {
        u32 attr = 0;
        u32 ownerAcc = 0, groupAcc = 0, othersAcc = 0;

        isfsErr = ISFS_GetAttr(absPath, &status->ownerId, &status->groupId, &attr, &ownerAcc, &groupAcc, &othersAcc);

        if (isfsErr == ISFS_ERROR_OK) {
            nandComposePerm(&status->permission, ownerAcc, groupAcc, othersAcc);
            status->attribute = attr;
        }

        return isfsErr;
    }
}

static void nandGetStatusCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result == ISFS_ERROR_OK) {
        NANDStatus* status;
        status = block->status;
        status->attribute = block->attr;
        nandComposePerm(&status->permission, block->ownerAcc, block->groupAcc, block->othersAcc);
    }

    ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
}

s32 NANDGetStatus(const char* path, NANDStatus* status) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandGetStatus(path, status, NULL, FALSE, FALSE));
}

s32 NANDPrivateGetStatus(const char* path, NANDStatus* status) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandGetStatus(path, status, NULL, FALSE, TRUE));
}

s32 NANDGetStatusAsync(const char* path, NANDStatus* status, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->status = status;
    return nandConvertErrorCode(nandGetStatus(path, status, block, TRUE, FALSE));
}

s32 NANDPrivateGetStatusAsync(const char* path, NANDStatus* status, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->status = status;
    return nandConvertErrorCode(nandGetStatus(path, status, block, TRUE, TRUE));
}

static s32 nandSetStatus(const char* path, const NANDStatus* status, NANDCommandBlock* block, BOOL asyncFlag, BOOL privateFlag) {
    char absPath[NAND_MAX_PATH] = "";

    ASSERTMSGLINE(1152, path, "NULL pointer detected.");
    ASSERTMSGLINE(1153, status, "NULL pointer detected.");

    nandGenerateAbsPath(absPath, path);

    if (!privateFlag && nandIsPrivatePath(absPath)) {
        return ISFS_ERROR_ACCESS;
    }

    if (!nandInspectPermission((u8)status->permission)) {
        return ISFS_ERROR_INVALID;
    }

    {
        u32 ownerAcc = 0, groupAcc = 0, othersAcc = 0;
        nandSplitPerm((u8)status->permission, &ownerAcc, &groupAcc, &othersAcc);

        if (asyncFlag) {
            return ISFS_SetAttrAsync(absPath, status->ownerId, (u16)status->groupId, status->attribute, ownerAcc, groupAcc, othersAcc, nandCallback,
                                     block);
        } else {
            return ISFS_SetAttr(absPath, status->ownerId, (u16)status->groupId, status->attribute, ownerAcc, groupAcc, othersAcc);
        }
    }
}

s32 NANDSetStatus(const char* path, NANDStatus* status) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandSetStatus(path, status, NULL, FALSE, FALSE));
}

s32 NANDPrivateSetStatus(const char* path, NANDStatus* status) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandSetStatus(path, status, NULL, FALSE, TRUE));
}

s32 NANDSetStatusAsync(const char* path, NANDStatus* status, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandSetStatus(path, status, block, TRUE, FALSE));
}

s32 NANDPrivateSetStatusAsync(const char* path, NANDStatus* status, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandSetStatus(path, status, block, TRUE, TRUE));
}

void NANDSetUserData(NANDCommandBlock* block, void* data) {
    ASSERTMSGLINE(1233, block, "NULL pointer detected.");
    block->userData = data;
}

void* NANDGetUserData(NANDCommandBlock* block) {
    ASSERTMSGLINE(1240, block, "NULL pointer detected.");
    return block->userData;
}

static BOOL nandInspectPermission(u8 perm) {
    if (perm & NAND_PERM_USER_READ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

#if SDK_VERSION >= 20091211

static BOOL nandCheckCharacter(const char ch);
static BOOL nandCheckPathName(const char* path) {
    int len = -1;
    u32 i;

    ASSERTMSGLINE(1266, path, "null pointer is detected in argument of path\n");

    len = strlen(path);
    if (len == 0) {
        return FALSE;
    }

    if (nandIsRelativePath(path)) {
        for (i = 0; i < len; i++) {
            if (!nandCheckCharacter(path[i])) {
                return FALSE;
            }
        }
    } else {
        char szPath[NAND_MAX_PATH];
        char relativePath[ISFS_INODE_NAMELEN];

        strcpy(szPath, path);
        if (*(szPath + len - 1) == '/' && (len - 1) != 0) {
            *(szPath + len - 1) = 0;
        }

        nandGetRelativeName(relativePath, szPath);

        len = strlen(relativePath);

        for (i = 0; i < len; i++) {
            if (!nandCheckCharacter(relativePath[i])) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

static BOOL nandCheckCharacter(const char ch) {
    // Array of characters that are valid for ISFS path
    const char charset[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
                            0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
                            0x79, 0x7A, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
                            0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x2D, 0x5F, 0x2E, 0x2F};

    int i;
    for (i = 0; i < ARRAY_LENGTH(charset); i++) {
        if (ch == charset[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

#endif

s32 NANDGetTmpDirUsage(u32* blockCountOut, u32* fileCountOut) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(ISFS_GetUsage("/tmp", blockCountOut, fileCountOut));
}

s32 NANDGetTmpDirUsageAsync(u32* blockCountOut, u32* fileCountOut, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(ISFS_GetUsageAsync("/tmp", blockCountOut, fileCountOut, nandCallback, block));
}

s32 NANDGetHomeDirUsage(u32* blockCountOut, u32* fileCountOut) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(ISFS_GetUsage(nandGetHomeDir(), blockCountOut, fileCountOut));
}

s32 NANDGetHomeDirUsageAsync(u32* blockCountOut, u32* fileCountOut, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(ISFS_GetUsageAsync(nandGetHomeDir(), blockCountOut, fileCountOut, nandCallback, block));
}

/* NANDCountNumOpenableFiles */
/* Seemingly debug only and for later SDK revisions */

#if SDK_VERSION >= 20091211

enum {
    COUNTCMP_STATE_WORKING = 0,
    COUNTCMP_STATE_DONE,
    COUNTCMP_STATE_CLEANUP,
};

typedef struct {
    int state;
    int fd[ISFS_FD_ENTRIES];  // 0x04
    u8 count;
} nand_count_tmp;

static s32 nandCancelNumCountOpenableFile(nand_count_tmp* count_tmp, const char* path);
s32 NANDCountNumOpenableFiles(u8* number, BOOL* cleaning) {
#ifdef DEBUG
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    FSStats stats ALIGN32;
    nand_count_tmp count_tmp;

    char path[NAND_MAX_PATH];

    const char* count_tmpdir = "/tmp/count_tmpdir";
    const char* countfile = "countfile";

    u32 free_inodes = 0;

    ASSERTMSGLINE(1398, number, "*number is NULL pointer!");
    ASSERTMSGLINE(1399, cleaning, "*cleaning is NULL pointer!");

    *number = 0;
    *cleaning = TRUE;

    count_tmp.state = COUNTCMP_STATE_WORKING;
    count_tmp.count = 0;

    // Grab NAND stats
    isfsErr = ISFS_GetStats(&stats);
    if (isfsErr == ISFS_ERROR_OK) {
        free_inodes = stats.freeInodes;
        ASSERTMSGLINE(1411, free_inodes <= 6143, "NAND library internal error:  free_inodes value must be illegal.");
    } else {
        return nandConvertErrorCode(isfsErr);
    }

    // Check for free space
    if (free_inodes < ISFS_FD_ENTRIES + 5 /* ? */) {
        return NAND_RESULT_MAXBLOCKS;
    }

    // Create our temporary directory
    isfsErr = ISFS_CreateDir(count_tmpdir, 0, ISFS_ACCESS_RW, 0, 0);
    if (isfsErr != ISFS_ERROR_OK) {
        ISFS_Delete(count_tmpdir);
        return nandConvertErrorCode(isfsErr);
    }

    while (TRUE) {
        // Create first temporary file
        snprintf(path, NAND_MAX_PATH, "%s/%s_%.2d", count_tmpdir, countfile, count_tmp.count);
        isfsErr = ISFS_CreateFile(path, 0, ISFS_ACCESS_READ, 0, 0);
        if (isfsErr != ISFS_ERROR_OK) {
            *cleaning = nandCancelNumCountOpenableFile(&count_tmp, count_tmpdir) == ISFS_ERROR_OK ? TRUE : FALSE;
            return nandConvertErrorCode(isfsErr);
        }

        // Now open the file
        count_tmp.fd[count_tmp.count] = ISFS_Open(path, ISFS_ACCESS_READ);
        if (count_tmp.fd[count_tmp.count] >= 0) {
            count_tmp.count++;

            // Next file!
            continue;
        } else {
            // Error occured? Stop!
            break;
        }
    }

    // If some error occured, something had messed up so abort.
    if (count_tmp.fd[count_tmp.count] != ISFS_ERROR_MAXFD) {
        *cleaning = nandCancelNumCountOpenableFile(&count_tmp, count_tmpdir) == ISFS_ERROR_OK ? TRUE : FALSE;
        return nandConvertErrorCode(count_tmp.fd[count_tmp.count]);
    }

    // We are done! Save values and clean up...

    ASSERTMSGLINE(1453, count_tmp.count < ISFS_FD_ENTRIES, "count_tmp.count must be less than ISFS_FD_ENTRIES.!");
    *number = count_tmp.count;

    count_tmp.state = COUNTCMP_STATE_DONE;
    if (nandCancelNumCountOpenableFile(&count_tmp, count_tmpdir) != ISFS_ERROR_OK) {
        *cleaning = FALSE;
    }

    return NAND_RESULT_OK;
#else
    return NAND_RESULT_FATAL_ERROR;
#endif
}

static s32 nandCancelNumCountOpenableFile(nand_count_tmp* count_tmp, const char* path) {
    s32 result = ISFS_ERROR_OK;
    s32 isfsErr = ISFS_ERROR_UNKNOWN;
    u8 i;

    switch (count_tmp->state) {
        case COUNTCMP_STATE_WORKING:
        case COUNTCMP_STATE_DONE: {
            // Close all opened files
            for (i = 0; i < count_tmp->count; i++) {
                isfsErr = ISFS_Close(count_tmp->fd[i]);
                if (isfsErr != ISFS_ERROR_OK && result != isfsErr) {
                    result = isfsErr;
                }
            }

            // Request to delete out temporary directory
            count_tmp->state = COUNTCMP_STATE_CLEANUP;
            if (result != ISFS_ERROR_OK) {
                nandCancelNumCountOpenableFile(count_tmp, path);
            } else {
                result = nandCancelNumCountOpenableFile(count_tmp, path);
            }
            break;
        }
        case COUNTCMP_STATE_CLEANUP: {
            result = ISFS_Delete(path);
            break;
        }
    }

    return nandConvertErrorCode(result);
}

#endif
