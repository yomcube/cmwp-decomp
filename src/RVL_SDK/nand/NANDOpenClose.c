#include <private/nand.h>
#include <revolution/nand.h>

#include <revolution/fs.h>

#include <revolution/ios.h>
#include <revolution/os.h>

#include <stdio.h>

static u32 nandGetUniqueNumber();

static void nandOpenCallback(s32 result, void* arg);
static s32 nandOpen(const char* path, u8 accType, NANDCommandBlock* block, BOOL asyncFlag, BOOL privateFlag) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;
    s32 isfsAccess;
    char absPath[NAND_MAX_PATH] = "";

    isfsAccess = ISFS_ACCESS_NONE;

    ASSERTMSGLINE(188, path, "NULL pointer is detected.");
    ASSERTMSGLINE(192, accType == NAND_ACCESS_READ || accType == NAND_ACCESS_WRITE || accType == NAND_ACCESS_RW, "Access type is illegal.");
    nandGenerateAbsPath(absPath, path);

    if (!privateFlag && nandIsPrivatePath(absPath)) {
        return ISFS_ERROR_ACCESS;
    }

    switch (accType) {
        case NAND_ACCESS_RW: {
            isfsAccess = ISFS_ACCESS_RW;
            break;
        }
        case NAND_ACCESS_READ: {
            isfsAccess = ISFS_ACCESS_READ;
            break;
        }
        case NAND_ACCESS_WRITE: {
            isfsAccess = ISFS_ACCESS_WRITE;
            break;
        }
        default: {
            (void)0;
            break;
        }
    }

    if (asyncFlag) {
        isfsErr = ISFS_OpenAsync(absPath, isfsAccess, nandOpenCallback, block);
    } else {
        isfsErr = ISFS_Open(absPath, isfsAccess);
    }

    return isfsErr;
}

s32 NANDOpen(const char* path, NANDFileInfo* info, u8 accType) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    ASSERTMSGLINE(234, info, "NULL pointer is detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    isfsErr = nandOpen(path, accType, NULL, FALSE, FALSE);
    if (isfsErr >= ISFS_ERROR_OK) {
        info->fileDescriptor = isfsErr;
        info->mark = NAND_MARK_OPENED;
        return NAND_RESULT_OK;
    }

    return nandConvertErrorCode(isfsErr);
}

s32 NANDPrivateOpen(const char* path, NANDFileInfo* info, u8 accType) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    ASSERTMSGLINE(259, info, "NULL pointer is detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    isfsErr = nandOpen(path, accType, NULL, FALSE, TRUE);
    if (isfsErr >= ISFS_ERROR_OK) {
        info->fileDescriptor = isfsErr;
        info->mark = NAND_MARK_OPENED;
        return NAND_RESULT_OK;
    }

    return nandConvertErrorCode(isfsErr);
}

s32 NANDOpenAsync(const char* path, NANDFileInfo* info, u8 accType, NANDAsyncCallback callback, NANDCommandBlock* block) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    ASSERTMSGLINE(284, info, "NULL pointer is detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->fileInfo = info;

    isfsErr = nandOpen(path, accType, block, TRUE, FALSE);
    return nandConvertErrorCode(isfsErr);
}

s32 NANDPrivateOpenAsync(const char* path, NANDFileInfo* info, u8 accType, NANDAsyncCallback callback, NANDCommandBlock* block) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    ASSERTMSGLINE(301, info, "NULL pointer is detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->fileInfo = info;

    isfsErr = nandOpen(path, accType, block, TRUE, TRUE);
    return nandConvertErrorCode(isfsErr);
}

static void nandOpenCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result >= ISFS_ERROR_OK) {
        ((NANDFileInfo*)block->fileInfo)->fileDescriptor = result;
        ((NANDFileInfo*)block->fileInfo)->stage = NAND_STAGE_2;
        ((NANDFileInfo*)block->fileInfo)->mark = NAND_MARK_OPENED;
        ((NANDAsyncCallback)block->callback)(NAND_RESULT_OK, block);
    } else {
        ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
    }
}

s32 NANDClose(NANDFileInfo* info) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    ASSERTMSGLINE(341, info, "NULL pointer is detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (info->mark != NAND_MARK_OPENED) {
        return NAND_RESULT_INVALID;
    }

    isfsErr = ISFS_Close(info->fileDescriptor);
    if (isfsErr == ISFS_ERROR_OK) {
        info->mark = NAND_MARK_CLOSED;
    }

    return nandConvertErrorCode(isfsErr);
}

static void nandCloseCallback(s32 result, void* arg);
s32 NANDCloseAsync(NANDFileInfo* info, NANDAsyncCallback callback, NANDCommandBlock* block) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    ASSERTMSGLINE(364, info, "NULL pointer is detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (info->mark != NAND_MARK_OPENED) {
        return NAND_RESULT_INVALID;
    }

    block->callback = callback;
    block->fileInfo = info;

    isfsErr = ISFS_CloseAsync(info->fileDescriptor, nandCloseCallback, block);
    return nandConvertErrorCode(isfsErr);
}

static s32 nandSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, BOOL privateFlag, BOOL simpleFlag);
s32 NANDSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize) {
    return nandSafeOpen(path, info, accType, buf, bufSize, FALSE, FALSE);
}

s32 NANDPrivateSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize) {
    return nandSafeOpen(path, info, accType, buf, bufSize, TRUE, FALSE);
}

s32 NANDSimpleSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize) {
    return nandSafeOpen(path, info, accType, buf, bufSize, FALSE, TRUE);
}

s32 NANDPrivateSimpleSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize) {
    return nandSafeOpen(path, info, accType, buf, bufSize, TRUE, TRUE);
}

static s32 nandCopy(s32 to, s32 from, void* buf, u32 bufSize);
static s32 nandSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, BOOL privateFlag, BOOL simpleFlag) {
    ASSERTMSGLINE(411, path, "NULL pointer detected.\n");
    ASSERTMSGLINE(412, info, "NULL pointer detected.\n");
    ASSERTMSGLINE(413, accType == NAND_ACCESS_READ || accType == NAND_ACCESS_WRITE || accType == NAND_ACCESS_RW, "Illegal access type.\n");
    ASSERTMSGLINE(414, buf, "NULL pointer detected.\n");
    ASSERTMSGLINE(415, OSIsAligned32B(buf), "32byte alignment is required.\n");
    ASSERTMSGLINE(416, OSIsAligned32B(bufSize), "Buffer size must be multiples of 32.\n");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (simpleFlag && (bufSize & 0x3FFF) != 0) {
        return NAND_RESULT_INVALID;
    }

    info->accType = (u8)accType;
    info->stage = NAND_STAGE_0;

    nandGenerateAbsPath(info->origPath, path);

    if (!privateFlag && nandIsPrivatePath(info->origPath)) {
        return NAND_RESULT_ACCESS;
    }

    if (accType == NAND_ACCESS_READ) {
        s32 isfsErr = ISFS_Open(info->origPath, ISFS_ACCESS_READ);
        if (isfsErr >= ISFS_ERROR_OK) {
            info->fileDescriptor = isfsErr;
            info->stage = NAND_STAGE_2;
            if (!simpleFlag) {
                info->mark = NAND_MARK_SAFE_OPENED;
            } else {
                info->mark = NAND_MARK_SIMPLE_SAFE_OPENED;
            }
            return NAND_RESULT_OK;
        } else {
            return nandConvertErrorCode(isfsErr);
        }
    } else if (accType == NAND_ACCESS_WRITE || accType == NAND_ACCESS_RW) {
        u32 ownerId;
        u16 groupId;
        u32 attr;

        u32 ownerAcc, groupAcc, othersAcc;

        char dirPath[NAND_MAX_PATH];
        char relativePath[ISFS_INODE_NAMELEN + 1] = "";

        s32 isfsErr = ISFS_ERROR_UNKNOWN;
        s32 uniqNo = -1;

        isfsErr = ISFS_CreateDir("/tmp/sys", 0, ISFS_ACCESS_RW, ISFS_ACCESS_RW, ISFS_ACCESS_RW);
        if (isfsErr != ISFS_ERROR_OK && isfsErr != ISFS_ERROR_EXISTS) {
            return nandConvertErrorCode(isfsErr);
        }

        info->stage = NAND_STAGE_1;

        isfsErr = ISFS_GetAttr(info->origPath, &ownerId, &groupId, &attr, &ownerAcc, &groupAcc, &othersAcc);
        if (isfsErr != ISFS_ERROR_OK) {
            return nandConvertErrorCode(isfsErr);
        }

        info->origFd = ISFS_Open(info->origPath, ISFS_ACCESS_READ);
        if (info->origFd < ISFS_ERROR_OK) {
            return nandConvertErrorCode(info->origFd);
        }

        info->stage = NAND_STAGE_2;

        if (!simpleFlag) {
            uniqNo = nandGetUniqueNumber();

            sprintf(dirPath, "%s/%08x", "/tmp/sys", uniqNo);

            isfsErr = ISFS_CreateDir(dirPath, 0, ISFS_ACCESS_RW, 0, 0);
            if (isfsErr != ISFS_ERROR_OK) {
                return nandConvertErrorCode(isfsErr);
            }

            info->stage = NAND_STAGE_3;
        }

        nandGetRelativeName(relativePath, info->origPath);

        if (!simpleFlag) {
            sprintf(info->tmpPath, "%s/%08x/%s", "/tmp/sys", uniqNo, relativePath);
        } else {
            sprintf(info->tmpPath, "%s/%s", "/tmp/sys", relativePath);
        }

        isfsErr = ISFS_CreateFile(info->tmpPath, attr, ownerAcc, groupAcc, othersAcc);
        if (isfsErr != ISFS_ERROR_OK) {
            return nandConvertErrorCode(isfsErr);
        }

        info->stage = NAND_STAGE_4;

        if (accType == NAND_ACCESS_WRITE) {
            info->fileDescriptor = ISFS_Open(info->tmpPath, ISFS_ACCESS_WRITE);
        } else if (accType == NAND_ACCESS_RW) {
            info->fileDescriptor = ISFS_Open(info->tmpPath, ISFS_ACCESS_RW);
        }

        if (info->fileDescriptor < 0) {
            return nandConvertErrorCode(info->fileDescriptor);
        }

        info->stage = NAND_STAGE_5;

        isfsErr = nandCopy(info->fileDescriptor, info->origFd, buf, bufSize);
        if (isfsErr != ISFS_ERROR_OK) {
            return nandConvertErrorCode(isfsErr);
        }

        isfsErr = ISFS_Seek(info->fileDescriptor, 0, 0);
        if (isfsErr == ISFS_ERROR_OK) {
            isfsErr = ISFS_ERROR_OK;
        } else {
            return nandConvertErrorCode(isfsErr);
        }

        if (isfsErr == ISFS_ERROR_OK) {
            if (simpleFlag) {
                info->mark = NAND_MARK_SIMPLE_SAFE_OPENED;
            } else {
                info->mark = NAND_MARK_SAFE_OPENED;
            }
        }
        return nandConvertErrorCode(isfsErr);

    } else {
        return NAND_RESULT_INVALID;
    }
}

static s32 nandSafeClose(NANDFileInfo* info, BOOL simpleFlag);
s32 NANDSafeClose(NANDFileInfo* info) {
    return nandSafeClose(info, FALSE);
}

s32 NANDSimpleSafeClose(NANDFileInfo* info) {
    return nandSafeClose(info, TRUE);
}

static s32 nandSafeClose(NANDFileInfo* info, BOOL simpleFlag) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;
    char parentDir[NAND_MAX_PATH] = "";

    ASSERTMSGLINE(612, info, "NULL pointer detected.\n");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if ((info->mark != NAND_MARK_SAFE_OPENED || simpleFlag) && (info->mark != NAND_MARK_SIMPLE_SAFE_OPENED || !simpleFlag)) {
        return NAND_RESULT_INVALID;
    }

    if (info->accType == NAND_ACCESS_READ) {
        isfsErr = ISFS_Close(info->fileDescriptor);
        if (isfsErr == ISFS_ERROR_OK) {
            info->stage = NAND_STAGE_7;
            if (!simpleFlag) {
                info->mark = NAND_MARK_SAFE_CLOSED;
            } else {
                info->mark = NAND_MARK_SIMPLE_SAFE_CLOSED;
            }
        }
        return nandConvertErrorCode(isfsErr);
    } else if (info->accType == NAND_ACCESS_WRITE || info->accType == NAND_ACCESS_RW) {
        isfsErr = ISFS_Close(info->fileDescriptor);
        if (isfsErr != ISFS_ERROR_OK) {
            return nandConvertErrorCode(isfsErr);
        }

        info->stage = NAND_STAGE_6;

        isfsErr = ISFS_Close(info->origFd);
        if (isfsErr != ISFS_ERROR_OK) {
            return nandConvertErrorCode(isfsErr);
        }

        info->stage = NAND_STAGE_7;

        isfsErr = ISFS_Rename(info->tmpPath, info->origPath);
        if (isfsErr != ISFS_ERROR_OK) {
            return nandConvertErrorCode(isfsErr);
        }

        info->stage = NAND_STAGE_8;

        if (!simpleFlag) {
            nandGetParentDirectory(parentDir, info->tmpPath);
            isfsErr = ISFS_Delete(parentDir);
            if (isfsErr == ISFS_ERROR_OK) {
                info->stage = NAND_STAGE_9;
                info->mark = NAND_MARK_SAFE_CLOSED;
            }
        } else {
            info->mark = NAND_MARK_SIMPLE_SAFE_CLOSED;
        }
        return nandConvertErrorCode(isfsErr);

    } else {
        OSReport("Illegal NANDFileInfo.\n");
        return NAND_RESULT_INVALID;
    }

    return NAND_RESULT_INVALID;
}

static s32 nandSafeOpenAsync(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, NANDAsyncCallback callback,
                             NANDCommandBlock* block, BOOL privateFlag, BOOL simpleFlag);
s32 NANDSafeOpenAsync(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, NANDAsyncCallback callback, NANDCommandBlock* block) {
    return nandSafeOpenAsync(path, info, accType, buf, bufSize, callback, block, FALSE, FALSE);
}

s32 NANDPrivateSafeOpenAsync(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, NANDAsyncCallback callback,
                             NANDCommandBlock* block) {
    return nandSafeOpenAsync(path, info, accType, buf, bufSize, callback, block, TRUE, FALSE);
}

s32 NANDSimpleSafeOpenAsync(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, NANDAsyncCallback callback,
                            NANDCommandBlock* block) {
    return nandSafeOpenAsync(path, info, accType, buf, bufSize, callback, block, FALSE, TRUE);
}

s32 NANDPrivateSimpleSafeOpenAsync(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, NANDAsyncCallback callback,
                                   NANDCommandBlock* block) {
    return nandSafeOpenAsync(path, info, accType, buf, bufSize, callback, block, TRUE, TRUE);
}

static void nandSafeOpenCallback(s32 result, void* arg);
static void nandReadOpenCallback(s32 result, void* arg);
static s32 nandSafeOpenAsync(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, NANDAsyncCallback callback,
                             NANDCommandBlock* block, BOOL privateFlag, BOOL simpleFlag) {
    u32 ownerId;
    u16 groupId;
    u32 attr;

    ASSERTMSGLINE(745, path, "NULL pointer detected.\n");
    ASSERTMSGLINE(746, info, "NULL pointer detected.\n");
    ASSERTMSGLINE(747, accType == NAND_ACCESS_READ || accType == NAND_ACCESS_WRITE || accType == NAND_ACCESS_RW, "Illegal access type.\n");
    ASSERTMSGLINE(748, buf, "NULL pointer detected.\n");
    ASSERTMSGLINE(749, OSIsAligned32B(buf), "32byte alignment is required.\n");
    ASSERTMSGLINE(750, OSIsAligned32B(bufSize), "Buffer size must be multiples of 32.\n");
    ASSERTMSGLINE(751, block, "NULL pointer detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (simpleFlag && (bufSize & 0x3FFF) != 0) {
        return NAND_RESULT_INVALID;
    }

    info->accType = (u8)accType;
    info->stage = NAND_STAGE_0;
    block->simpleFlag = simpleFlag;

    nandGenerateAbsPath(info->origPath, path);

    if (!privateFlag && nandIsPrivatePath(info->origPath)) {
        return NAND_RESULT_ACCESS;
    }

    if (accType == NAND_ACCESS_READ) {
        s32 isfsErr = NAND_RESULT_ACCESS;  // ?
        block->fileInfo = info;
        block->callback = callback;
        isfsErr = ISFS_OpenAsync(info->origPath, ISFS_ACCESS_READ, nandReadOpenCallback, block);
        if (isfsErr == ISFS_ERROR_OK) {
            return NAND_RESULT_OK;
        } else {
            return nandConvertErrorCode(isfsErr);
        }
    } else if (accType == NAND_ACCESS_WRITE || accType == NAND_ACCESS_RW) {
        s32 isfsErr = ISFS_ERROR_UNKNOWN;
        block->fileInfo = info;
        block->callback = callback;
        block->state = NAND_STATE_0;
        block->copyBuf = buf;
        block->bufLength = bufSize;
        isfsErr = ISFS_CreateDirAsync("/tmp/sys", 0, ISFS_ACCESS_RW, ISFS_ACCESS_RW, ISFS_ACCESS_RW, nandSafeOpenCallback, block);
        if (isfsErr == ISFS_ERROR_OK) {
            return NAND_RESULT_OK;
        } else {
            return nandConvertErrorCode(isfsErr);
        }
    } else {
        return NAND_RESULT_INVALID;
    }
}

static void nandSafeOpenCallback(s32 result, void* arg) {
    s32 isfsErr;
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result >= ISFS_ERROR_OK || (result == ISFS_ERROR_EXISTS && block->state == NAND_STATE_0)) {
        NANDFileInfo* info = block->fileInfo;
        isfsErr = ISFS_ERROR_UNKNOWN;

        if (block->state == NAND_STATE_0) {
            info->stage = NAND_STAGE_1;
        }

        if (block->state == NAND_STATE_2) {
            info->origFd = result;
            info->stage = NAND_STAGE_2;
        }

        if (block->state == NAND_STATE_2 && block->simpleFlag) {
            block->state += 2;
        } else {
            block->state++;
        }

        if (block->state == NAND_STATE_1) {
            isfsErr = ISFS_GetAttrAsync(info->origPath, &block->ownerId, &block->groupId, &block->attr, &block->ownerAcc, &block->groupAcc,
                                        &block->othersAcc, nandSafeOpenCallback, arg);
        } else if (block->state == NAND_STATE_2) {
            isfsErr = ISFS_OpenAsync(info->origPath, ISFS_ACCESS_READ, nandSafeOpenCallback, arg);
        } else if (block->state == NAND_STATE_3) {
            char tmpPath[NAND_MAX_PATH];

            block->uniqNo = nandGetUniqueNumber();

            sprintf(tmpPath, "%s/%08x", "/tmp/sys", block->uniqNo);
            isfsErr = ISFS_CreateDirAsync(tmpPath, 0, ISFS_ACCESS_RW, ISFS_ACCESS_NONE, ISFS_ACCESS_NONE, nandSafeOpenCallback, arg);
        } else if (block->state == NAND_STATE_4) {
            char relativeName[ISFS_INODE_NAMELEN];
            nandGetRelativeName(relativeName, info->origPath);

            if (!block->simpleFlag) {
                info->stage = NAND_STAGE_3;
                sprintf(info->tmpPath, "%s/%08x/%s", "/tmp/sys", block->uniqNo, relativeName);
            } else {
                sprintf(info->tmpPath, "%s/%s", "/tmp/sys", relativeName);
            }
            isfsErr = ISFS_CreateFileAsync(info->tmpPath, block->attr, block->ownerAcc, block->groupAcc, block->othersAcc, nandSafeOpenCallback, arg);
        } else if (block->state == NAND_STATE_5) {
            info->stage = NAND_STAGE_4;

            if (info->accType == NAND_ACCESS_WRITE) {
                isfsErr = ISFS_OpenAsync(info->tmpPath, ISFS_ACCESS_WRITE, nandSafeOpenCallback, arg);
            } else if (info->accType == NAND_ACCESS_RW) {
                isfsErr = ISFS_OpenAsync(info->tmpPath, ISFS_ACCESS_RW, nandSafeOpenCallback, arg);
            } else {
                isfsErr = ISFS_ERROR_UNKNOWN;
            }
        } else if (block->state == NAND_STATE_6) {
            info->fileDescriptor = result;

            info->stage = NAND_STAGE_5;
            block->state = NAND_STATE_7;

            isfsErr = ISFS_ReadAsync(info->origFd, block->copyBuf, block->bufLength, nandSafeOpenCallback, arg);
        } else if (block->state == NAND_STATE_7) {
            isfsErr = ISFS_ReadAsync(info->origFd, block->copyBuf, block->bufLength, nandSafeOpenCallback, arg);
        } else if (block->state == NAND_STATE_8) {
            if (result > ISFS_ERROR_OK) {
                block->state = NAND_STATE_6;

                isfsErr = ISFS_WriteAsync(info->fileDescriptor, block->copyBuf, result, nandSafeOpenCallback, arg);
            } else if (result == ISFS_ERROR_OK) {
                isfsErr = ISFS_SeekAsync(info->fileDescriptor, 0, ISFS_SEEK_BEG, nandSafeOpenCallback, arg);
            }
        } else if (block->state == NAND_STATE_9) {
            if (result == ISFS_ERROR_OK) {
                if (!block->simpleFlag) {
                    info->mark = NAND_MARK_SAFE_OPENED;
                } else {
                    info->mark = NAND_MARK_SIMPLE_SAFE_OPENED;
                }

                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(ISFS_ERROR_OK), block);
            } else {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
            }
            return;
        }

        if (isfsErr != ISFS_ERROR_OK) {
            ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
        }
    } else {
        ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
    }
}

static void nandReadOpenCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result >= ISFS_ERROR_OK) {
        ((NANDFileInfo*)block->fileInfo)->fileDescriptor = result;
        ((NANDFileInfo*)block->fileInfo)->stage = NAND_STAGE_2;
        if (!block->simpleFlag) {
            ((NANDFileInfo*)block->fileInfo)->mark = NAND_MARK_SAFE_OPENED;
        } else {
            ((NANDFileInfo*)block->fileInfo)->mark = NAND_MARK_SIMPLE_SAFE_OPENED;
        }
        ((NANDAsyncCallback)block->callback)(NAND_RESULT_OK, block);
    } else {
        ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
    }
}

static s32 nandSafeCloseAsync(NANDFileInfo* info, NANDAsyncCallback callback, NANDCommandBlock* block, BOOL simpleFlag);
s32 NANDSafeCloseAsync(NANDFileInfo* info, NANDAsyncCallback callback, NANDCommandBlock* block) {
    return nandSafeCloseAsync(info, callback, block, FALSE);
}

s32 NANDSimpleSafeCloseAsync(NANDFileInfo* info, NANDAsyncCallback callback, NANDCommandBlock* block) {
    return nandSafeCloseAsync(info, callback, block, TRUE);
}

static void nandSafeCloseCallback(s32 result, void* arg);
static void nandReadCloseCallback(s32 result, void* arg);
s32 nandSafeCloseAsync(NANDFileInfo* info, NANDAsyncCallback callback, NANDCommandBlock* block, BOOL simpleFlag) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    ASSERTMSGLINE(993, info, "NULL pointer detected.\n");
    ASSERTMSGLINE(994, block, "NULL pointer detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if ((info->mark != NAND_MARK_SAFE_OPENED || simpleFlag) && (info->mark != NAND_MARK_SIMPLE_SAFE_OPENED || !simpleFlag)) {
        return NAND_RESULT_INVALID;
    }

    block->simpleFlag = simpleFlag;

    if (info->accType == NAND_ACCESS_READ) {
        block->fileInfo = info;
        block->callback = callback;
        isfsErr = ISFS_CloseAsync(info->fileDescriptor, nandReadCloseCallback, block);
    } else if (info->accType == NAND_ACCESS_WRITE || info->accType == NAND_ACCESS_RW) {
        block->fileInfo = info;
        block->callback = callback;
        block->state = NAND_STATE_10;
        isfsErr = ISFS_CloseAsync(info->fileDescriptor, nandSafeCloseCallback, block);
    } else {
        isfsErr = ISFS_ERROR_INVALID;
    }

    return nandConvertErrorCode(isfsErr);
}

static void nandSafeCloseCallback(s32 result, void* arg) {
    s32 isfsErr;
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result == ISFS_ERROR_OK) {
        NANDFileInfo* info = block->fileInfo;
        isfsErr = ISFS_ERROR_UNKNOWN;

        if (block->state == NAND_STATE_12) {
            info->stage = NAND_STAGE_8;
        }

        if (block->state == NAND_STATE_12 && block->simpleFlag) {
            block->state += 2;
        } else {
            block->state++;
        }

        if (block->state == NAND_STATE_11) {
            info->stage = NAND_STAGE_6;
            isfsErr = ISFS_CloseAsync(info->origFd, nandSafeCloseCallback, arg);
        } else if (block->state == NAND_STATE_12) {
            info->stage = NAND_STAGE_7;
            isfsErr = ISFS_RenameAsync(info->tmpPath, info->origPath, nandSafeCloseCallback, arg);
        } else if (block->state == NAND_STATE_13) {
            char parentDir[NAND_MAX_PATH] = "";

            nandGetParentDirectory(parentDir, info->tmpPath);
            isfsErr = ISFS_DeleteAsync(parentDir, nandSafeCloseCallback, arg);
        } else if (block->state == NAND_STATE_14) {
            if (!block->simpleFlag) {
                info->stage = NAND_STAGE_9;
            }
            info->mark = NAND_MARK_SAFE_CLOSED;
            ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
            return;
        }

        if (isfsErr != ISFS_ERROR_OK) {
            ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
        }
        return;
    }

    ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
}

static void nandReadCloseCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result == ISFS_ERROR_OK) {
        ((NANDFileInfo*)block->fileInfo)->stage = NAND_STAGE_7;
        ((NANDFileInfo*)block->fileInfo)->mark = NAND_MARK_SAFE_CLOSED;
    }

    ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
}

static void nandCloseCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result == ISFS_ERROR_OK) {
        ((NANDFileInfo*)block->fileInfo)->stage = NAND_STAGE_7;
        ((NANDFileInfo*)block->fileInfo)->mark = NAND_MARK_CLOSED;
    }

    ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
}

static s32 nandCopy(s32 to, s32 from, void* buf, u32 bufSize) {
    while (TRUE) {
        s32 read = ISFS_Read(from, buf, bufSize);
        s32 isfsErr;

        if (read == 0) {
            return 0;
        }

        if (read < ISFS_ERROR_OK) {
            return read;
        }

        isfsErr = ISFS_Write(to, buf, read);
        if (isfsErr < ISFS_ERROR_OK) {
            return isfsErr;
        }

        if (isfsErr != read) {
            // left over condition for debug
        }
    }
}

static u32 nandGetUniqueNumber() {
    static u32 s_counter = 0;
    u32 num;

    BOOL enabled = OSDisableInterrupts();
    num = s_counter++;
    OSRestoreInterrupts(enabled);

    return num;
}

static BOOL nandIsOrigFileOpened(u8 stage) {
    if (stage == NAND_STAGE_2 || stage == NAND_STAGE_3 || stage == NAND_STAGE_4 || stage == NAND_STAGE_5 || stage == NAND_STAGE_6) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static BOOL nandIsTmpFileOpened(u8 stage) {
    if (stage == NAND_STAGE_5) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static BOOL nandIsTmpDirExisted(u8 stage) {
    if (stage == NAND_STAGE_3 || stage == NAND_STAGE_4 || stage == NAND_STAGE_5 || stage == NAND_STAGE_6 || stage == NAND_STAGE_7 ||
        stage == NAND_STAGE_8) {
        return TRUE;
    } else {
        return FALSE;
    }
}

s32 NANDSimpleSafeCancel(NANDFileInfo* info) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    ASSERTMSGLINE(1211, info, "NULL pointer detected.\n");
    ASSERTMSGLINE(1212, info->accType == NAND_ACCESS_READ || info->accType == NAND_ACCESS_WRITE || info->accType == NAND_ACCESS_RW,
                  "Illegal access type.\n");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (info->accType == NAND_ACCESS_READ) {
        if (info->stage == NAND_STAGE_0) {
            return NAND_RESULT_OK;
        }

        if (info->stage == NAND_STAGE_2) {
            isfsErr = ISFS_Close(info->fileDescriptor);
            return nandConvertErrorCode(isfsErr);
        }

        return NAND_RESULT_INVALID;
    } else if (info->accType == NAND_ACCESS_WRITE || info->accType == NAND_ACCESS_RW) {
        if (info->stage == NAND_STAGE_8) {
            return NAND_RESULT_INVALID;
        }

        if (info->stage == NAND_STAGE_7) {
            isfsErr = ISFS_Delete(info->tmpPath);
            if (isfsErr == ISFS_ERROR_OK) {
                info->stage = NAND_STAGE_0;
            } else {
                return nandConvertErrorCode(isfsErr);
            }
        }

        if (info->stage == NAND_STAGE_6) {
            isfsErr = ISFS_Delete(info->tmpPath);
            if (isfsErr == ISFS_ERROR_OK) {
                info->stage = NAND_STAGE_2;
            } else {
                return nandConvertErrorCode(isfsErr);
            }
        }

        if (info->stage == NAND_STAGE_5) {
            isfsErr = ISFS_Close(info->fileDescriptor);
            if (isfsErr == ISFS_ERROR_OK) {
                info->stage = NAND_STAGE_4;
            } else {
                return nandConvertErrorCode(isfsErr);
            }
        }

        if (info->stage == NAND_STAGE_4) {
            isfsErr = ISFS_Delete(info->tmpPath);
            if (isfsErr == ISFS_ERROR_OK) {
                info->stage = NAND_STAGE_2;
            } else {
                return nandConvertErrorCode(isfsErr);
            }
        }

        if (info->stage == NAND_STAGE_2) {
            isfsErr = ISFS_Close(info->origFd);
            if (isfsErr == ISFS_ERROR_OK) {
                info->stage = NAND_STAGE_0;
            } else {
                return nandConvertErrorCode(isfsErr);
            }
        }

        if (info->stage == NAND_STAGE_0 || info->stage == NAND_STAGE_1) {
            return NAND_RESULT_OK;
        }

        return NAND_RESULT_UNKNOWN;
    }

    return NAND_RESULT_INVALID;
}

static void nandSimpleSafeCancelCallback(s32 result, void* arg);
s32 NANDSimpleSafeCancelAsync(NANDFileInfo* info, NANDAsyncCallback callback, NANDCommandBlock* block) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;
    static u32 s_dmy;

    ASSERTMSGLINE(1343, info, "NULL pointer detected.\n");
    ASSERTMSGLINE(1344, info->accType == NAND_ACCESS_READ || info->accType == NAND_ACCESS_WRITE || info->accType == NAND_ACCESS_RW,
                  "Illegal access type.\n");
    ASSERTMSGLINE(1345, block, "NULL pointer detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->fileInfo = info;
    block->callback = callback;
    block->nextStage = NAND_STAGE_64;

    isfsErr = ISFS_ReadDirAsync("/", NULL, &s_dmy, nandSimpleSafeCancelCallback, block);
    return nandConvertErrorCode(isfsErr);
}

static void nandSimpleSafeCancelCallback(s32 result, void* arg) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;
    NANDCommandBlock* block = (NANDCommandBlock*)arg;
    NANDFileInfo* info = (NANDFileInfo*)block->fileInfo;

    if (result < ISFS_ERROR_OK) {
        ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
    }

    if (block->nextStage != NAND_STAGE_64) {
        info->stage = block->nextStage;
    }

    if (info->accType == NAND_ACCESS_READ) {
        if (info->stage == NAND_STAGE_0) {
            ((NANDAsyncCallback)block->callback)(NAND_RESULT_OK, block);
        } else if (info->stage == NAND_STAGE_2) {
            block->nextStage = NAND_STAGE_0;
            isfsErr = ISFS_CloseAsync(info->fileDescriptor, nandSimpleSafeCancelCallback, arg);
            if (isfsErr != ISFS_ERROR_OK) {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
            }
        } else {
            ((NANDAsyncCallback)block->callback)(NAND_RESULT_INVALID, block);
        }
    } else if (info->accType == NAND_ACCESS_WRITE || info->accType == NAND_ACCESS_RW) {
        if (info->stage == NAND_STAGE_8) {
            ((NANDAsyncCallback)block->callback)(NAND_RESULT_INVALID, block);
        }

        if (info->stage == NAND_STAGE_7) {
            block->nextStage = NAND_STAGE_0;
            isfsErr = ISFS_DeleteAsync(info->tmpPath, nandSimpleSafeCancelCallback, arg);
            if (isfsErr != ISFS_ERROR_OK) {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
            }
        }

        if (info->stage == NAND_STAGE_6) {
            block->nextStage = NAND_STAGE_2;
            isfsErr = ISFS_DeleteAsync(info->tmpPath, nandSimpleSafeCancelCallback, arg);
            if (isfsErr != ISFS_ERROR_OK) {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
            }
        }

        if (info->stage == NAND_STAGE_5) {
            block->nextStage = NAND_STAGE_4;
            isfsErr = ISFS_CloseAsync(info->fileDescriptor, nandSimpleSafeCancelCallback, arg);
            if (isfsErr != ISFS_ERROR_OK) {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
            }
        }

        if (info->stage == NAND_STAGE_4) {
            block->nextStage = NAND_STAGE_2;
            isfsErr = ISFS_DeleteAsync(info->tmpPath, nandSimpleSafeCancelCallback, arg);
            if (isfsErr != ISFS_ERROR_OK) {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
            }
        }

        if (info->stage == NAND_STAGE_2) {
            block->nextStage = NAND_STAGE_0;
            isfsErr = ISFS_CloseAsync(info->origFd, nandSimpleSafeCancelCallback, arg);
            if (isfsErr != ISFS_ERROR_OK) {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
            }
        }

        if (info->stage == NAND_STAGE_0 || info->stage == NAND_STAGE_1) {
            ((NANDAsyncCallback)block->callback)(NAND_RESULT_OK, block);
        }
    } else {
        (void)0;
    }
}

s32 NANDSafeCancel(NANDFileInfo* info, s32* unk) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    ASSERTMSGLINE(1519, info, "NULL pointer detected.\n");
    ASSERTMSGLINE(1520, info->accType == NAND_ACCESS_READ || info->accType == NAND_ACCESS_WRITE || info->accType == NAND_ACCESS_RW,
                  "Illegal access type.\n");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (info->accType == NAND_ACCESS_READ) {
        if (info->stage == NAND_STAGE_2) {
            isfsErr = ISFS_Close(info->fileDescriptor);
            return nandConvertErrorCode(isfsErr);
        }

        return NAND_RESULT_INVALID;
    } else if (info->accType == NAND_ACCESS_WRITE || info->accType == NAND_ACCESS_RW) {
        if (unk) {
            if (info->stage == NAND_STAGE_8 || info->stage == NAND_STAGE_9) {
                *unk = 2;
            } else {
                *unk = 1;
            }
        }

        if (info->stage == NAND_STAGE_9 || info->stage == NAND_STAGE_0 || info->stage == NAND_STAGE_1) {
            return NAND_RESULT_INVALID;
        }

        if (nandIsOrigFileOpened((u8)info->stage)) {
            isfsErr = ISFS_Close(info->origFd);
            if (isfsErr == ISFS_ERROR_CORRUPT || isfsErr == ISFS_ERROR_UNKNOWN) {
                return nandConvertErrorCode(isfsErr);
            }
        }

        if (nandIsTmpFileOpened((u8)info->stage)) {
            isfsErr = ISFS_Close(info->fileDescriptor);
            if (isfsErr == ISFS_ERROR_CORRUPT || isfsErr == ISFS_ERROR_UNKNOWN) {
                return nandConvertErrorCode(isfsErr);
            }
        }

        if (nandIsTmpDirExisted((u8)info->stage)) {
            char parentDir[NAND_MAX_PATH] = "";
            nandGetParentDirectory(parentDir, info->tmpPath);
            isfsErr = ISFS_Delete(parentDir);
            if (isfsErr == ISFS_ERROR_CORRUPT || isfsErr == ISFS_ERROR_UNKNOWN) {
                return nandConvertErrorCode(isfsErr);
            }
        }

        return NAND_RESULT_OK;
    }

    return NAND_RESULT_INVALID;
}

static void nandSafeCancelCallback1(s32 result, void* arg);
static void nandSafeCancelCallback2(s32 result, void* arg);
static void nandSafeCancelCallback3(s32 result, void* arg);

s32 NANDSafeCancelAsync(NANDFileInfo* info, s32* unk, NANDAsyncCallback callback, NANDCommandBlock* block) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    ASSERTMSGLINE(1601, info, "NULL pointer detected.\n");
    ASSERTMSGLINE(1602, info->accType == NAND_ACCESS_READ || info->accType == NAND_ACCESS_WRITE || info->accType == NAND_ACCESS_RW,
                  "Illegal access type.\n");
    ASSERTMSGLINE(1603, block, "NULL pointer detected.");

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (info->accType == NAND_ACCESS_READ) {
        if (info->stage == NAND_STAGE_2) {
            block->fileInfo = info;
            block->callback = callback;
            isfsErr = ISFS_CloseAsync(info->fileDescriptor, nandReadCloseCallback, block);
            return nandConvertErrorCode(isfsErr);
        }

        return NAND_RESULT_INVALID;
    } else if (info->accType == NAND_ACCESS_WRITE || info->accType == NAND_ACCESS_RW) {
        block->fileInfo = info;
        block->callback = callback;

        if (unk) {
            if (info->stage == NAND_STAGE_8 || info->stage == NAND_STAGE_9) {
                *unk = 2;
            } else {
                *unk = 1;
            }
        }

        if (info->stage == NAND_STAGE_9 || info->stage == NAND_STAGE_0 || info->stage == NAND_STAGE_1) {
            return NAND_RESULT_INVALID;
        }

        if (nandIsOrigFileOpened((u8)info->stage)) {
            isfsErr = ISFS_CloseAsync(info->origFd, nandSafeCancelCallback1, block);
            return nandConvertErrorCode(isfsErr);
        }

        if (nandIsTmpFileOpened((u8)info->stage)) {
            isfsErr = ISFS_CloseAsync(info->fileDescriptor, nandSafeCancelCallback2, block);
            return nandConvertErrorCode(isfsErr);
        }

        if (nandIsTmpDirExisted((u8)info->stage)) {
            char parentDir[NAND_MAX_PATH] = "";
            nandGetParentDirectory(parentDir, info->tmpPath);
            isfsErr = ISFS_DeleteAsync(parentDir, nandSafeCancelCallback3, block);
            return nandConvertErrorCode(isfsErr);
        }

        return NAND_RESULT_INVALID;
    }

    isfsErr = ISFS_ERROR_INVALID;
    return nandConvertErrorCode(isfsErr);
}

static void nandSafeCancelCallback1(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;
    NANDFileInfo* info = (NANDFileInfo*)block->fileInfo;

    if (result == ISFS_ERROR_OK || result == ISFS_ERROR_INVALID) {
        s32 isfsErr = ISFS_ERROR_UNKNOWN;
        if (nandIsTmpFileOpened((u8)info->stage)) {
            isfsErr = ISFS_CloseAsync(info->fileDescriptor, nandSafeCancelCallback2, arg);
            if (isfsErr != ISFS_ERROR_OK) {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
            }
        } else if (nandIsTmpDirExisted((u8)info->stage)) {
            char parentDir[NAND_MAX_PATH] = "";
            nandGetParentDirectory(parentDir, info->tmpPath);

            isfsErr = ISFS_DeleteAsync(parentDir, nandSafeCancelCallback3, arg);
            if (isfsErr != ISFS_ERROR_OK) {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
            }
        } else {
            ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
        }
    } else {
        ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
    }
}

static void nandSafeCancelCallback2(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;
    NANDFileInfo* info = (NANDFileInfo*)block->fileInfo;

    if (result == ISFS_ERROR_OK || result == ISFS_ERROR_INVALID) {
        if (nandIsTmpDirExisted((u8)info->stage)) {
            s32 isfsErr = ISFS_ERROR_UNKNOWN;

            char parentDir[NAND_MAX_PATH] = "";
            nandGetParentDirectory(parentDir, info->tmpPath);

            isfsErr = ISFS_DeleteAsync(parentDir, nandSafeCancelCallback3, arg);
            if (isfsErr != ISFS_ERROR_OK) {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
            }
        } else {
            ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
        }
    } else {
        ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
    }
}

static void nandSafeCancelCallback3(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;
    NANDFileInfo* info = (NANDFileInfo*)block->fileInfo;
    ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
}
