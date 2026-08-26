#include <private/nand.h>
#include <revolution/nand.h>

#include <revolution/fs.h>
#include <revolution/os.h>

s32 NANDSecretGetUsage(const char* path, u32* usedBlocks, u32* usedInodes) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    {
        char absPath[NAND_MAX_PATH] = "";
        nandGenerateAbsPath(absPath, path);
        return nandConvertErrorCode(ISFS_GetUsage(absPath, usedBlocks, usedInodes));
    }
}

s32 NANDSecretGetUsageAsync(const char* path, u32* usedBlocks, u32* usedInodes, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    {
        char absPath[NAND_MAX_PATH] = "";
        nandGenerateAbsPath(absPath, path);
        block->callback = callback;
        return nandConvertErrorCode(ISFS_GetUsageAsync(absPath, usedBlocks, usedInodes, nandCallback, block));
    }
}

s32 NANDSecretGetFileSystemStatus(NANDFileSystemStatus* status) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    FSStats isfsStats;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    isfsErr = ISFS_GetStats(&isfsStats);
    if (isfsErr == ISFS_ERROR_OK) {
        status->blockSize = isfsStats.blockSize;
        status->freeBlocks = isfsStats.freeBlocks;
        status->usedBlocks = isfsStats.usedBlocks;
        status->badBlocks = isfsStats.badBlocks;
        status->reservedBlocks = isfsStats.reservedBlocks;
        status->freeInodes = isfsStats.freeInodes;
        status->usedInodes = isfsStats.usedInodes;
    }

    return nandConvertErrorCode(isfsErr);
}

s32 NANDSecretFormat() {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(ISFS_Format());
}

s32 NANDSecretShutdown() {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(ISFS_Shutdown());
}

s32 NANDSecretGetSystemAvailableArea(const NANDFileSystemStatus* status, u32 usedUserBlocks, u32 usedUserInodes, u32* freeSystemBlocks,
                                     u32* freeSystemInodes) {
    const u32 BOOT = NAND_MAX_BOOT_FSBLOCKS;
    const u32 FAT = NAND_MAX_FAT_FSBLOCKS;
    const u32 SYSTEM = NAND_MAX_SYSTEM_FSBLOCKS;
    const u32 TMP_MAX_FSBLOCKS = NAND_MAX_TMP_FSBLOCKS;
    const u32 TMP_MAX_INODES = NAND_MAX_TMP_INODES;

    ASSERTMSGLINE(120, status, "NULL pointer is detected.");

    if (usedUserBlocks <= NAND_MAX_USER_FSBLOCKS && usedUserInodes <= NAND_MAX_USER_INODES) {
        s32 isfsErr = NAND_RESULT_UNKNOWN;  // @typo should be ISFS_ERROR_UNKNOWN but eh
        u32 usedTmpBlocks = -1;
        u32 usedTmpInodes = -1;
        isfsErr = ISFS_GetUsage("/tmp", &usedTmpBlocks, &usedTmpInodes);
        if (isfsErr != ISFS_ERROR_OK) {
            return isfsErr;
        }
        if (freeSystemBlocks) {
            u32 unusedBlocks = status->badBlocks + status->reservedBlocks;
            *freeSystemBlocks = (SYSTEM - (unusedBlocks + (BOOT + FAT))) - (status->usedBlocks - usedUserBlocks) - (TMP_MAX_FSBLOCKS - usedTmpBlocks);
        }
        if (freeSystemInodes) {
            *freeSystemInodes = NAND_MAX_SYSTEM_INODES - (status->usedInodes - usedUserInodes) - (TMP_MAX_INODES - usedTmpInodes);
        }
        return NAND_RESULT_OK;
    } else {
        return NAND_RESULT_INVALID;
    }
}
