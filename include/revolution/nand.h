#ifndef REVOLUTION_NAND_H
#define REVOLUTION_NAND_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NAND_MAX_PATH 64

typedef struct NANDFileInfo {
    s32 fileDescriptor;            // 0x00
    s32 origFd;                    // 0x04
    char origPath[NAND_MAX_PATH];  // 0x08
    char tmpPath[NAND_MAX_PATH];   // 0x48
    u8 accType;                    // 0x88
    u8 stage;                      // 0x89
    u8 mark;                       // 0x8A
} NANDFileInfo;

#define NAND_BANNER_SIGNATURE 'WIBN'

#define NAND_BANNER_COMMENT_MAX 2
#define NAND_BANNER_COMMENT_LENGTH 32

#define NAND_BANNER_TEXTURE_SIZE 0x6000

#define NAND_BANNER_ICON_MAX_FRAME 8
#define NAND_BANNER_ICON_SIZE 0x1200

#define NAND_BANNER_SIZE(ICON_FRAMES) (offsetof(NANDBanner, iconTexture) + (0x1200 * ICON_FRAMES))

#define NAND_BANNER_SET_ICON_SPEED(BANNER, FRAME, SPEED)                                                                                             \
    (BANNER)->iconSpeed = (BANNER)->iconSpeed & ~(0b11 << (FRAME)) | (((SPEED) & 0b11) << (FRAME))

typedef struct NANDBanner {
    u32 signature;                                                      // 0x00
    u32 flags;                                                          // 0x04
    u16 iconSpeed;                                                      // 0x08
    u8 reserved[22];                                                    // 0x0A
    u16 comment[NAND_BANNER_COMMENT_MAX][NAND_BANNER_COMMENT_LENGTH];   // 0x20
    u8 bannerTexture[NAND_BANNER_TEXTURE_SIZE];                         // 0xA0
    u8 iconTexture[NAND_BANNER_ICON_SIZE][NAND_BANNER_ICON_MAX_FRAME];  // 0x60A0
} NANDBanner;

typedef struct NANDStatus {
    u32 ownerId;    // 0x00
    u16 groupId;    // 0x04
    u8 attribute;   // 0x06
    u8 permission;  // 0x07
} NANDStatus;

typedef struct NANDCommandBlock {
    void* userData;               // 0x00
    void* callback;               // 0x04
    void* fileInfo;               // 0x08
    void* bytes;                  // 0x0C
    void* inodes;                 // 0x10
    void* status;                 // 0x14
    u32 ownerId;                  // 0x18
    u16 groupId;                  // 0x1C
    u8 nextStage;                 // 0x1E
    u32 attr;                     // 0x20
    u32 ownerAcc;                 // 0x24
    u32 groupAcc;                 // 0x28
    u32 othersAcc;                // 0x2C
    u32 num;                      // 0x30
    char absPath[NAND_MAX_PATH];  // 0x34
    u32* length;                  // 0x74
    u32* pos;                     // 0x78
    int state;                    // 0x7C
    void* copyBuf;                // 0x80
    u32 bufLength;                // 0x84
    u8* type;                     // 0x88
    u32 uniqNo;                   // 0x8C
    u32 reqBlocks;                // 0x90
    u32 reqInodes;                // 0x94
    u32* answer;                  // 0x98
    u32 homeBlocks;               // 0x9C
    u32 homeInodes;               // 0xA0
    u32 userBlocks;               // 0xA4
    u32 userInodes;               // 0xA8
    u32 workBlocks;               // 0xAC
    u32 workInodes;               // 0xB0
    const char** dir;             // 0xB4
    BOOL simpleFlag;              // 0xB8
} NANDCommandBlock;

typedef void (*NANDAsyncCallback)(s32 result, NANDCommandBlock* block);

// Error codes
#define NAND_RESULT_OK 0
#define NAND_RESULT_ACCESS -1
#define NAND_RESULT_ALLOC_FAILED -2
#define NAND_RESULT_BUSY -3
#define NAND_RESULT_CORRUPT -4
#define NAND_RESULT_ECC_CRIT -5
#define NAND_RESULT_EXISTS -6
#define NAND_RESULT_UNUSED_7 -7 /* Unused by the library, but used by some apps such as the Wii Menu */
#define NAND_RESULT_INVALID -8
#define NAND_RESULT_MAXBLOCKS -9
#define NAND_RESULT_MAXFD -10
#define NAND_RESULT_MAXFILES -11
#define NAND_RESULT_NOEXISTS -12
#define NAND_RESULT_NOTEMPTY -13
#define NAND_RESULT_OPENFD -14
#define NAND_RESULT_AUTHENTICATION -15
#define NAND_RESULT_MAXDEPTH -16
#define NAND_RESULT_UNKNOWN -64
#define NAND_RESULT_FATAL_ERROR -128

// Whence types
#define NAND_SEEK_BEG 0
#define NAND_SEEK_CUR 1
#define NAND_SEEK_END 2

// Access types
#define NAND_ACCESS_NONE 0
#define NAND_ACCESS_READ (1 << 0)
#define NAND_ACCESS_WRITE (1 << 1)
#define NAND_ACCESS_RW (NAND_ACCESS_READ | NAND_ACCESS_WRITE)

// File types
#define NAND_FILE_TYPE_NONE 0
#define NAND_FILE_TYPE_FILE 1
#define NAND_FILE_TYPE_DIR 2

// Read/write by owner
#define NAND_PERM_USER_READ (NAND_ACCESS_READ << 4)
#define NAND_PERM_USER_WRITE (NAND_ACCESS_WRITE << 4)
#define NAND_PERM_USER_RW (NAND_PERM_USER_READ | NAND_PERM_USER_WRITE)

// Read/write by group
#define NAND_PERM_GROUP_READ (NAND_ACCESS_READ << 2)
#define NAND_PERM_GROUP_WRITE (NAND_ACCESS_WRITE << 2)
#define NAND_PERM_GROUP_RW (NAND_PERM_GROUP_READ | NAND_PERM_GROUP_WRITE)

// Read/write by other
#define NAND_PERM_BOTH_READ (NAND_ACCESS_READ << 0)
#define NAND_PERM_BOTH_WRITE (NAND_ACCESS_WRITE << 0)
#define NAND_PERM_BOTH_RW (NAND_PERM_BOTH_READ | NAND_PERM_BOTH_WRITE)

// Read/write by all
#define NAND_PERM_ALL_READ (NAND_PERM_USER_READ | NAND_PERM_GROUP_READ | NAND_PERM_BOTH_READ)
#define NAND_PERM_ALL_WRITE (NAND_PERM_USER_WRITE | NAND_PERM_GROUP_WRITE | NAND_PERM_BOTH_WRITE)
#define NAND_PERM_ALL_RW (NAND_PERM_ALL_READ | NAND_PERM_ALL_WRITE)

/* FS Blocks (AKA Clusters) */

#define NAND_FSPAGE_SIZE 2048                    /* 2 KB */
#define NAND_FSBLOCK_SIZE (NAND_FSPAGE_SIZE * 8) /* 16 KB */

#define NAND_BYTES_TO_FSBLOCKS(x) ((x) / NAND_FSBLOCK_SIZE)
#define NAND_MB_TO_FSBLOCKS(x) (((x) * 1024 * 1024) / NAND_FSBLOCK_SIZE)

#define NAND_FSBLOCKS_TO_BYTES(x) ((x) * NAND_FSBLOCK_SIZE)

// The amount of blocks of the entire area in the System Memory.
#define NAND_MAX_FSBLOCKS_MB 512
#define NAND_MAX_FSBLOCKS NAND_MB_TO_FSBLOCKS(NAND_MAX_FSBLOCKS_MB) /* 32768 blocks / 4096 data management blocks */

// The amount of blocks of the user area in the System Memory.
#define NAND_MAX_USER_FSBLOCKS_MB 272
#define NAND_MAX_USER_FSBLOCKS NAND_MB_TO_FSBLOCKS(NAND_MAX_USER_FSBLOCKS_MB) /* 17408 blocks / 2176 data management blocks */
// The amount of blocks of the systen area in the System Memory.
#define NAND_MAX_SYSTEM_FSBLOCKS_MB 240
#define NAND_MAX_SYSTEM_FSBLOCKS NAND_MB_TO_FSBLOCKS(NAND_MAX_SYSTEM_FSBLOCKS_MB) /* 15360 blocks / 1920 data management blocks */

// The amount of blocks of the game's save data in the System Memory.
#define NAND_MAX_HOME_FSBLOCKS_MB 16
#define NAND_MAX_HOME_FSBLOCKS NAND_MB_TO_FSBLOCKS(NAND_MAX_HOME_FSBLOCKS_MB) /* 1024 blocks / 128 data management blocks */
// The amount of blocks of the FAT area in the System Memory.
#define NAND_MAX_TMP_FSBLOCKS_MB 40
#define NAND_MAX_TMP_FSBLOCKS NAND_MB_TO_FSBLOCKS(NAND_MAX_TMP_FSBLOCKS_MB) /* 2560 blocks / 320 data management blocks */

// The amount of blocks of the boot area in the System Memory.
#define NAND_MAX_BOOT_FSBLOCKS_MB 1
#define NAND_MAX_BOOT_FSBLOCKS NAND_MB_TO_FSBLOCKS(NAND_MAX_BOOT_FSBLOCKS_MB) /* 64 blocks / 8 data management blocks */
// The amount of blocks of the FAT area in the System Memory.
#define NAND_MAX_FAT_FSBLOCKS_MB 4
#define NAND_MAX_FAT_FSBLOCKS NAND_MB_TO_FSBLOCKS(NAND_MAX_FAT_FSBLOCKS_MB) /* 256 blocks / 32 data management blocks */

/* I-nodes */

#define NAND_MAX_INODES 6143

#define NAND_MAX_USER_INODES 4000
#define NAND_MAX_SYSTEM_INODES 2143

#define NAND_MAX_HOME_INODES 32
#define NAND_MAX_TMP_INODES 64

/* Main */

s32 NANDInit();

s32 NANDChangeDir(const char* path);
s32 NANDChangeDirAsync(const char* path, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDGetCurrentDir(char* path);
s32 NANDGetHomeDir(char* path);

s32 NANDGetType(const char* path, u8* type);
s32 NANDGetTypeAsync(const char* path, u8* type, NANDAsyncCallback callback, NANDCommandBlock* block);

void NANDInitBanner(NANDBanner* banner, u32 flags, const u16* comment0, const u16* comment1);

s32 NANDOpen(const char* path, NANDFileInfo* info, u8 accType);
s32 NANDOpenAsync(const char* path, NANDFileInfo* info, u8 accType, NANDAsyncCallback callback, NANDCommandBlock* block);
s32 NANDSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize);
s32 NANDSimpleSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize);
s32 NANDSafeOpenAsync(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, NANDAsyncCallback callback, NANDCommandBlock* block);
s32 NANDSimpleSafeOpenAsync(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, NANDAsyncCallback callback,
                            NANDCommandBlock* block);

s32 NANDClose(NANDFileInfo* info);
s32 NANDCloseAsync(NANDFileInfo* info, NANDAsyncCallback callback, NANDCommandBlock* block);
s32 NANDSafeClose(NANDFileInfo* info);
s32 NANDSimpleSafeClose(NANDFileInfo* info);
s32 NANDSafeCloseAsync(NANDFileInfo* info, NANDAsyncCallback callback, NANDCommandBlock* block);
s32 NANDSimpleSafeCloseAsync(NANDFileInfo* info, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDSafeCancel(NANDFileInfo* info, s32* unk);
s32 NANDSafeCancelAsync(NANDFileInfo* info, s32* unk, NANDAsyncCallback callback, NANDCommandBlock* block);
s32 NANDSimpleSafeCancel(NANDFileInfo* info);
s32 NANDSimpleSafeCancelAsync(NANDFileInfo* info, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDCreate(const char* path, u8 perm, u8 attr);
s32 NANDCreateAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDDelete(const char* path);
s32 NANDDeleteAsync(const char* path, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDRead(NANDFileInfo* info, void* buf, u32 bufLength);
s32 NANDReadAsync(NANDFileInfo* info, void* buf, u32 bufLength, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDWrite(NANDFileInfo* info, const void* buf, u32 bufLength);
s32 NANDWriteAsync(NANDFileInfo* info, const void* buf, u32 bufLength, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDSeek(NANDFileInfo* info, s32 offset, s32 whence);
s32 NANDSeekAsync(NANDFileInfo* info, s32 offset, s32 whence, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDReadDir(const char* path, char* filesOut, u32* fileCountOut);
s32 NANDReadDirAsync(const char* path, char* filesOut, u32* fileCountOut, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDCreateDir(const char* path, u8 perm, u8 attr);
s32 NANDCreateDirAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDMove(const char* from, const char* to);
s32 NANDMoveAsync(const char* from, const char* to, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDFreeBlocks(u32* bytes, u32* inodes);
s32 NANDFreeFsBlocksAsync(u32* bytes, u32* inodes, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDGetLength(NANDFileInfo* info, u32* lengthOut);
s32 NANDTell(NANDFileInfo* info, u32* position);

s32 NANDGetLengthAsync(NANDFileInfo* info, u32* lengthOut, NANDAsyncCallback callback, NANDCommandBlock* block);
s32 NANDTellAsync(NANDFileInfo* info, u32* posOut, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDGetStatus(const char* path, NANDStatus* status);
s32 NANDGetStatusAsync(const char* path, NANDStatus* status, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDSetStatus(const char* path, NANDStatus* status);
s32 NANDSetStatusAsync(const char* path, NANDStatus* status, NANDAsyncCallback callback, NANDCommandBlock* block);

void NANDSetUserData(NANDCommandBlock* block, void* data);
void* NANDGetUserData(NANDCommandBlock* block);

s32 NANDGetTmpDirUsage(u32* blockCountOut, u32* fileCountOut);
s32 NANDGetTmpDirUsageAsync(u32* blockCountOut, u32* fileCountOut, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDGetHomeDirUsage(u32* blockCountOut, u32* fileCountOut);
s32 NANDGetHomeDirUsageAsync(u32* blockCountOut, u32* fileCountOut, NANDAsyncCallback callback, NANDCommandBlock* block);

#if SDK_VERSION >= 20091211
s32 NANDCountNumOpenableFiles(u8* number, BOOL* cleaning);
#endif

/* Private */

s32 NANDPrivateChangeDir(const char* path);
s32 NANDPrivateChangeDirAsync(const char* path, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDPrivateGetType(const char* path, u8* type);
s32 NANDPrivateGetTypeAsync(const char* path, u8* type, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDPrivateOpen(const char* path, NANDFileInfo* info, u8 accType);
s32 NANDPrivateOpenAsync(const char* path, NANDFileInfo* info, u8 accType, NANDAsyncCallback callback, NANDCommandBlock* block);
s32 NANDPrivateSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize);
s32 NANDPrivateSimpleSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize);
s32 NANDPrivateSafeOpenAsync(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, NANDAsyncCallback callback,
                             NANDCommandBlock* block);
s32 NANDPrivateSimpleSafeOpenAsync(const char* path, NANDFileInfo* info, u8 accType, void* buf, u32 bufSize, NANDAsyncCallback callback,
                                   NANDCommandBlock* block);

s32 NANDPrivateCreate(const char* path, u8 perm, u8 attr);
s32 NANDPrivateCreateAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDPrivateDelete(const char* path);
s32 NANDPrivateDeleteAsync(const char* path, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDPrivateReadDir(const char* path, char* filesOut, u32* fileCountOut);
s32 NANDPrivateReadDirAsync(const char* path, char* filesOut, u32* fileCountOut, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDPrivateCreateDir(const char* path, u8 perm, u8 attr);
s32 NANDPrivateCreateDirAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDPrivateMove(const char* from, const char* to);
s32 NANDPrivateMoveAsync(const char* from, const char* to, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDPrivateGetStatus(const char* path, NANDStatus* status);
s32 NANDPrivateGetStatusAsync(const char* path, NANDStatus* status, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDPrivateSetStatus(const char* path, NANDStatus* status);
s32 NANDPrivateSetStatusAsync(const char* path, NANDStatus* status, NANDAsyncCallback callback, NANDCommandBlock* block);

/* Check */

#define NAND_CHECK_SUCCESS 0

#define NAND_CHECK_HOME_INSSPACE (1 << 0)
#define NAND_CHECK_HOME_INSINODE (1 << 1)
#define NAND_CHECK_SYS_INSSPACE (1 << 2)
#define NAND_CHECK_SYS_INSINODE (1 << 3)

s32 NANDCheck(u32 requiredSpace, u32 requiredInodes, u32* answer);
s32 NANDCheckAsync(u32 requiredSpace, u32 requiredInodes, u32* answer, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDGetAvailableArea(u32* freeBlocks, u32* freeInodes);
s32 NANDGetAvailableAreaAsync(u32* freeBlocks, u32* freeInodes, NANDAsyncCallback callback, NANDCommandBlock* block);

/* Secret */

// same as FSStats
typedef struct NANDFileSystemStatus {
    u32 blockSize;  // 0x00

    u32 freeBlocks;  // 0x04
    u32 usedBlocks;  // 0x08

    u32 badBlocks;       // 0x0C
    u32 reservedBlocks;  // 0x10

    u32 freeInodes;  // 0x14
    u32 usedInodes;  // 0x18
} NANDFileSystemStatus;

s32 NANDSecretGetFileSystemStatus(NANDFileSystemStatus* status);

s32 NANDSecretGetUsage(const char* path, u32* usedBlocks, u32* usedInodes);
s32 NANDSecretGetUsageAsync(const char* path, u32* usedBlocks, u32* usedInodes, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDSecretGetUserAvailableArea(u32* freeBlocks, u32* freeInodes);
s32 NANDSecretGetSystemAvailableArea(const NANDFileSystemStatus* status, u32 usedUserBlocks, u32 usedUserInodes, u32* freeSystemBlocks,
                                     u32* freeSystemInodes);

s32 NANDSecretGetUserUsage(u32* usedBlocks, u32* usedInodes);
s32 NANDSecretGetUserUsageAsync(u32* usedBlocks, u32* usedInodes, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDSecretFormat();
s32 NANDSecretShutdown();

/* nanderr.log */

#define NAND_LOG_LINE_COUNT 64
#define NAND_LOG_MESSAGE_LENGTH 256

BOOL NANDLoggingPrepareFile(char* work);

BOOL NANDLoggingAddMessage(const char* format, ...);
#if SDK_VERSION >= 20091211
typedef void (*NANDLoggingCallback)(BOOL done, s32 result);
BOOL NANDLoggingAddMessageAsync(NANDLoggingCallback callback, s32 result, const char* format, ...);
#else
typedef void (*NANDLoggingCallback)(BOOL done);
BOOL NANDLoggingAddMessageAsync(NANDLoggingCallback callback, const char* format, ...);
#endif

#if SDK_VERSION >= 20091211

/* Error Messaging */

BOOL NANDSetAutoErrorMessaging(BOOL flag);

#endif

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_NAND_H
