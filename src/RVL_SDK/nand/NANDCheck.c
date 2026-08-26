#include <private/nand.h>
#include <revolution/nand.h>

#include <revolution/fs.h>
#include <revolution/os.h>

// clang-format off
static const char* USER_DIR_LIST[] = {
    "/meta",
    "/ticket",
    "/title/00010000",
    "/title/00010001",
    "/title/00010003",
    "/title/00010004",
    "/title/00010005",
    "/title/00010006",
    "/title/00010007",
    "/shared2/title",
    NULL
};
// clang-format on

static u32 nandSubtract(u32 num0, u32 num1);

static s32 nandCalcUsage(u32* blockCountOut, u32* fileCountOut, const char** paths);
static s32 nandCalcUserUsage(u32* blockCountOut, u32* fileCountOut) {
    return nandCalcUsage(blockCountOut, fileCountOut, USER_DIR_LIST);
}

static s32 nandCalcUsage(u32* blockCountOut, u32* fileCountOut, const char** paths) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    *blockCountOut = 0;
    *fileCountOut = 0;

    while (*paths) {
        u32 blockCount = 0;
        u32 fileCount = 0;

        isfsErr = ISFS_GetUsage(*paths, &blockCount, &fileCount);
        if (isfsErr == ISFS_ERROR_OK) {
            *blockCountOut += blockCount;
            *fileCountOut += fileCount;
        } else if (isfsErr == ISFS_ERROR_NOEXISTS) {
            isfsErr = ISFS_ERROR_OK;
        } else {
            break;
        }

        paths++;
    }

    return isfsErr;
}

static u32 nandCheck(u32 requiredSpace, u32 requiredInodes, u32 usedHomeSpace, u32 usedHomeInodes, u32 usedUserSpace, u32 usedUserInodes) {
    static const u32 HOME_SPACE_LIMIT = NAND_MAX_HOME_FSBLOCKS_MB;
    static const u32 HOME_INODE_LIMIT = NAND_MAX_HOME_INODES;

    static const u32 USER_SPACE_LIMIT = NAND_MAX_USER_FSBLOCKS_MB;
    static const u32 USER_INODE_LIMIT = NAND_MAX_USER_INODES;

    u32 answer = NAND_CHECK_SUCCESS;

    if ((usedHomeSpace + requiredSpace) > NAND_MB_TO_FSBLOCKS(HOME_SPACE_LIMIT)) {
        answer |= NAND_CHECK_HOME_INSSPACE;
    }
    if ((usedHomeInodes + requiredInodes) > (HOME_INODE_LIMIT + 1)) {
        answer |= NAND_CHECK_HOME_INSINODE;
    }
    if ((usedUserSpace + requiredSpace) > NAND_MB_TO_FSBLOCKS(USER_SPACE_LIMIT)) {
        answer |= NAND_CHECK_SYS_INSSPACE;
    }
    if ((usedUserInodes + requiredInodes) > USER_INODE_LIMIT) {
        answer |= NAND_CHECK_SYS_INSINODE;
    }

    return answer;
}

s32 NANDCheck(u32 requiredSpace, u32 requiredInodes, u32* answer) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    u32 usedHomeSpace = -1, usedHomeInodes = -1, usedUserSpace = -1, usedUserInodes = -1;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    isfsErr = ISFS_GetUsage(nandGetHomeDir(), &usedHomeSpace, &usedHomeInodes);
    if (isfsErr != ISFS_ERROR_OK) {
        return nandConvertErrorCode(isfsErr);
    }

    isfsErr = nandCalcUserUsage(&usedUserSpace, &usedUserInodes);
    if (isfsErr != ISFS_ERROR_OK) {
        return nandConvertErrorCode(isfsErr);
    }

    *answer = nandCheck(requiredSpace, requiredInodes, usedHomeSpace, usedHomeInodes, usedUserSpace, usedUserInodes);
    return NAND_RESULT_OK;
}

static void nandUserAreaCallback(s32 result, void* arg);
s32 NANDCheckAsync(u32 requiredSpace, u32 requiredInodes, u32* answer, NANDAsyncCallback callback, NANDCommandBlock* block) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->reqBlocks = requiredSpace;
    block->reqInodes = requiredInodes;
    block->answer = answer;
    block->userBlocks = 0;
    block->userInodes = 0;
    block->workBlocks = 0;
    block->workInodes = 0;
    block->dir = USER_DIR_LIST;
    block->state = NAND_STATE_20;

    isfsErr = ISFS_GetUsageAsync(nandGetHomeDir(), &block->homeBlocks, &block->homeInodes, nandUserAreaCallback, block);
    return nandConvertErrorCode(isfsErr);
}

static void nandUserAreaCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result == ISFS_ERROR_OK || result == ISFS_ERROR_NOEXISTS) {
        const char* dir = *block->dir;
        if (result == ISFS_ERROR_OK) {
            block->userBlocks += block->workBlocks;
            block->userInodes += block->workInodes;
        }

        if (dir) {
            s32 isfsErr = ISFS_ERROR_UNKNOWN;

            block->dir++;

            isfsErr = ISFS_GetUsageAsync(dir, &block->workBlocks, &block->workInodes, nandUserAreaCallback, arg);
            if (isfsErr != ISFS_ERROR_OK) {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
            }
        } else if (block->state == NAND_STATE_20) {
            *block->answer =
                nandCheck(block->reqBlocks, block->reqInodes, block->homeBlocks, block->homeInodes, block->userBlocks, block->userInodes);

            ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(ISFS_ERROR_OK), block);
        } else if (block->state == NAND_STATE_21) {
            u32 usedUserSpace = nandSubtract(NAND_MAX_USER_FSBLOCKS, block->userBlocks);
            u32 freeUserInodes = nandSubtract(NAND_MAX_USER_INODES, block->userInodes);
            u32 usedHomeSpace = nandSubtract(NAND_MAX_HOME_FSBLOCKS, block->homeBlocks);
            u32 usedHomeInodes = nandSubtract(NAND_MAX_HOME_INODES + 1, block->homeInodes);

            *(u32*)block->bytes = usedUserSpace < usedHomeSpace ? usedUserSpace : usedHomeSpace;
            *(u32*)block->inodes = freeUserInodes < usedHomeInodes ? freeUserInodes : usedHomeInodes;

            ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(ISFS_ERROR_OK), block);
        } else {
            OSReport("Illegal status is detected at %s()", __FUNCTION__);

            ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(ISFS_ERROR_UNKNOWN), block);
        }
    } else {
        ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
    }
}

static u32 nandSubtract(u32 num0, u32 num1) {
    if (num0 < num1) {
        return 0;
    } else {
        return num0 - num1;
    }
}

s32 NANDGetAvailableArea(u32* freeBlocks, u32* freeInodes) {
    u32 usedUserSpace = -1, usedUserInodes = -1, usedHomeSpace = -1, usedHomeInodes = -1;
    u32 freeUserSpace = -1, freeUserInodes = -1, freeHomeSpace = -1, freeHomeInodes = -1;

    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    isfsErr = ISFS_GetUsage(nandGetHomeDir(), &usedHomeSpace, &usedHomeInodes);
    if (isfsErr != ISFS_ERROR_OK) {
        return nandConvertErrorCode(isfsErr);
    }
    usedHomeInodes--;

    isfsErr = nandCalcUserUsage(&usedUserSpace, &usedUserInodes);
    if (isfsErr != ISFS_ERROR_OK) {
        return nandConvertErrorCode(isfsErr);
    }

    freeUserSpace = nandSubtract(NAND_MAX_USER_FSBLOCKS, usedUserSpace);
    freeUserInodes = nandSubtract(NAND_MAX_USER_INODES, usedUserInodes);
    freeHomeSpace = nandSubtract(NAND_MAX_HOME_FSBLOCKS, usedHomeSpace);
    freeHomeInodes = nandSubtract(NAND_MAX_HOME_INODES, usedHomeInodes);

    *freeBlocks = freeUserSpace < freeHomeSpace ? freeUserSpace : freeHomeSpace;
    *freeInodes = freeUserInodes < freeHomeInodes ? freeUserInodes : freeHomeInodes;

    return NAND_RESULT_OK;
}

s32 NANDGetAvailableAreaAsync(u32* freeBlocks, u32* freeInodes, NANDAsyncCallback callback, NANDCommandBlock* block) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    block->callback = callback;
    block->bytes = freeBlocks;
    block->inodes = freeInodes;
    block->userBlocks = 0;
    block->userInodes = 0;
    block->workBlocks = 0;
    block->workInodes = 0;
    block->dir = USER_DIR_LIST;
    block->state = NAND_STATE_21;

    isfsErr = ISFS_GetUsageAsync(nandGetHomeDir(), &block->homeBlocks, &block->homeInodes, nandUserAreaCallback, block);
    return nandConvertErrorCode(isfsErr);
}

s32 NANDSecretGetUserAvailableArea(u32* freeBlocks, u32* freeInodes) {
    u32 usedUserSpace = -1;
    u32 usedUserInodes = -1;

    u32 freeUserSpace = -1;
    u32 freeUserInodes = -1;

    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    isfsErr = nandCalcUserUsage(&usedUserSpace, &usedUserInodes);
    if (isfsErr != ISFS_ERROR_OK) {
        return nandConvertErrorCode(isfsErr);
    }

    freeUserSpace = nandSubtract(NAND_MAX_USER_FSBLOCKS, usedUserSpace);
    freeUserInodes = nandSubtract(NAND_MAX_USER_INODES, usedUserInodes);

    *freeBlocks = freeUserSpace;
    *freeInodes = freeUserInodes;

    return NAND_RESULT_OK;
}

s32 NANDSecretGetUserUsage(u32* usedBlocks, u32* usedInodes) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandCalcUserUsage(usedBlocks, usedInodes));
}

static void nandSecretGetUserUsageCallback(s32 result, void* arg);
s32 NANDSecretGetUserUsageAsync(u32* usedBlocks, u32* usedInodes, NANDAsyncCallback callback, NANDCommandBlock* block) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->inodes = usedInodes;
    block->bytes = usedBlocks;
    block->userBlocks = 0;
    block->userInodes = 0;
    block->workBlocks = 0;
    block->workInodes = 0;
    block->dir = &USER_DIR_LIST[1];

    isfsErr = ISFS_GetUsageAsync(*USER_DIR_LIST, &block->workBlocks, &block->workInodes, nandSecretGetUserUsageCallback, block);
    return nandConvertErrorCode(isfsErr);
}

static void nandSecretGetUserUsageCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result == ISFS_ERROR_OK || result == ISFS_ERROR_NOEXISTS) {
        const char* dir = *block->dir;
        if (result == ISFS_ERROR_OK) {
            block->userBlocks += block->workBlocks;
            block->userInodes += block->workInodes;
        }

        if (dir) {
            s32 isfsErr = ISFS_ERROR_UNKNOWN;

            block->dir++;

            isfsErr = ISFS_GetUsageAsync(dir, &block->workBlocks, &block->workInodes, nandSecretGetUserUsageCallback, arg);
            if (isfsErr != ISFS_ERROR_OK) {
                ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(isfsErr), block);
            }
        } else {
            *(u32*)block->inodes = block->userInodes;
            *(u32*)block->bytes = block->userBlocks;

            ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(ISFS_ERROR_OK), block);
        }
    } else {
        ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
    }
}
