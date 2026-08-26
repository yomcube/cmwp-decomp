#include <private/nand.h>
#include <revolution/nand.h>

#include <revolution/os.h>

#include <revolution/esp.h>
#include <revolution/fs.h>
#include <revolution/ios.h>

#include <revolution/sdkconfig.h>

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <revolution/verdefs.h>

#if defined(SDK_20091211)
#ifdef DEBUG
SDKDefineVersion(NAND, "Dec 11 2009", "15:54:29");
#else
SDKDefineVersion(NAND, "Dec 11 2009", "15:59:08");
#endif
#elif defined(SDK_20090224)
SDKDefineVersion(NAND, "Sep 11 2008", "17:34:00");
#endif

char s_currentDir[NAND_MAX_PATH] ALIGN32 = "/";
char s_homeDir[NAND_MAX_PATH] ALIGN32;

static BOOL nandOnShutdown(BOOL final, u32 event);
OSShutdownFunctionInfo s_shutdownFuncInfo = {nandOnShutdown, 0xFF};

enum {
    STATE_NOT_INITIALIZED = 0,
    STATE_INITIALIZING,
    STATE_INITIALIZED
};

s32 s_libState = STATE_NOT_INITIALIZED;

void nandRemoveTailToken(char* newpath, const char* oldpath) {
    ASSERTMSGLINE(189, newpath, "null pointer is detected in argument of newpath\n");
    ASSERTMSGLINE(190, oldpath, "null pointer is detected in argument of oldpath\n");
    ASSERTMSGLINE(191, *oldpath == '/', "Head of path must be '/' .\n");

    if (oldpath[0] == '/' && oldpath[1] == '\0') {
        newpath[0] = '/';
        newpath[1] = '\0';
    } else {
        int i = 0;
        for (i = strlen(oldpath) - 1; i >= 0; i--) {
            if (oldpath[i] == '/') {
                if (i != 0) {
                    strncpy(newpath, oldpath, i);
                    newpath[i] = '\0';
                    break;
                } else {
                    newpath[0] = '/';
                    newpath[1] = '\0';
                    break;
                }
            }
        }
    }
}

void nandGetHeadToken(char* head, char* rest, const char* path) {
    u32 i = 0;

    ASSERTMSGLINE(224, head && rest && path, "Null pointer detected!\n");
    ASSERTMSGLINE(225, strlen(path), "Null string was detected!\n");
    ASSERTMSGLINE(226, *path != '/', "Head of relative path must not be '/' .\n");

    for (i = 0; i <= strlen(path); i++) {
        if (path[i] == '/') {
            strncpy(head, path, i);
            head[i] = '\0';

            if (path[(int)i /*?*/ + 1] == '\0') {
                *rest = '\0';
            } else {
                strcpy(rest, path + i + 1);
            }
            break;
        } else if (path[i] == '\0') {
            strncpy(head, path, i);
            head[i] = '\0';
            *rest = '\0';
            break;
        }
    }
}

void nandGetRelativeName(char* name, const char* path) {
    if (strcmp("/", path) == 0) {
        strcpy(name, "");
    } else {
        int i = 0;

        ASSERTMSGLINE(260, nandIsAbsolutePath(path), "path must be absolute path.");
        ASSERTMSGLINE(261, path[strlen(path) - 1] != '/', "path must not be terminated with '/'.");

        for (i = strlen(path) - 1; i >= 0; i--) {
            if (path[i] == '/') {
                break;
            }
        }

        ASSERTMSGLINE(270, strlen(path + i + 1) <= ISFS_INODE_NAMELEN, "path must be smaller than or equal to ISFS_INODE_NAMELEN.");
        strcpy(name, path + i + 1);
    }
}

void nandConvertPath(char* abs, const char* wd, const char* rel) {
    char head[128];
    char rest[128];

    ASSERTMSGLINE(283, abs && wd && rel, "Null pointer detected!\n");
    ASSERTMSGLINE(284, *wd == '/', "Head of 'wd' must be '/'.\n");

    if (strlen(rel) == 0) {
        strcpy(abs, wd);
        return;
    }

    nandGetHeadToken(head, rest, rel);

    if (strcmp(head, ".") == 0) {
        nandConvertPath(abs, wd, rest);
    } else if (strcmp(head, "..") == 0) {
        char notail[128];
        nandRemoveTailToken(notail, wd);
        nandConvertPath(abs, notail, rest);
    } else if (head[0] != '\0') {
        char newdir[128];
        if (strcmp(wd, "/") == 0) {
            sprintf(newdir, "/%s", head);
        } else {
            sprintf(newdir, "%s/%s", wd, head);
        }
        nandConvertPath(abs, newdir, rest);
    } else {
        strcpy(abs, wd);
    }
}

static BOOL removeSingleDot(char* dst, const char* src) {
    BOOL foundDot = FALSE;
    int i = 0;

    for (i = 0; i < strlen(src) - 2; i++) {
        if (src[i] == '/' && src[i + 1] == '.' && src[i + 2] == '/') {
            strncpy(dst, src, i);
            dst[i] = 0;

            strcat(dst, src + i + 2);
            foundDot = TRUE;
            break;
        }
    }

    if (foundDot) {
        removeSingleDot(dst, dst);
    } else {
        strcpy(dst, src);
    }

    return foundDot;
}

static BOOL removeDoubleDot(char* dst, const char* src) {
    BOOL foundDot = FALSE;
    int i = 0;

    for (i = 0; i < strlen(src) - 3; i++) {
        if (src[i] == '/' && src[i + 1] == '.' && src[i + 2] == '.') {
            if (i == 0) {
                strcpy(dst, &src[3]);
            } else {
                int j = 0;

                for (j = i - 1; j >= 0; j--) {
                    if (src[j] == '/') {
                        strncpy(dst, src, j);
                        dst[j] = 0;
                        strcat(dst, &src[i] + 3);
                        break;
                    }
                }
            }
            foundDot = TRUE;
            break;
        }
    }
    if (foundDot) {
        removeDoubleDot(dst, dst);
    } else {
        strcpy(dst, src);
    }
    return foundDot;
}

BOOL nandIsRelativePath(const char* path) {
    if (*path == '/') {
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL nandIsAbsolutePath(const char* path) {
    if (nandIsRelativePath(path)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL nandIsPrivatePath(const char* path) {
    if (strncmp(path, "/shared2", sizeof("/shared2") - 1) == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL nandIsUnderPrivatePath(const char* path) {
    if (strncmp(path, "/shared2/", sizeof("/shared2/") - 1) == 0 && path[sizeof("/shared2/") - 1] != '\0') {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL nandIsJustBelowShared2(const char* path) {
    if (strncmp(path, "/shared2/", sizeof("/shared2/") - 1) == 0) {
        int i = sizeof("/shared2/") - 1;
        u32 len = strlen(path);
        for (; i < len; i++) {
            if (path[i] == '/') {
                return FALSE;
            }
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL nandIsAddressMEM2(const void* buf) {
    const u32 CACHED = 0x90000000;
    const u32 UNCACHED = 0xD0000000;

    u32 mem2Size = OSGetConsoleSimulatedMem2Size();
    u32 p = (u32)buf;

    if ((p >= CACHED && p < mem2Size + CACHED) || (p >= UNCACHED && p < mem2Size + UNCACHED)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL nandIsInitialized() {
    if (s_libState == STATE_INITIALIZED) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void nandReportErrorCode(s32 result) {
#define REPORT_ERROR_CODE(err)                                                                                                                       \
    case err: {                                                                                                                                      \
        /* nah, it stubbed out  */                                                                                                                   \
        break;                                                                                                                                       \
    }                                                                                                                                                \
        (void)0 /* swallow semicolon */

    switch (result) {
        // ISFS
        REPORT_ERROR_CODE(ISFS_ERROR_OK);
        REPORT_ERROR_CODE(ISFS_ERROR_INVALID);
        REPORT_ERROR_CODE(ISFS_ERROR_ACCESS);
        REPORT_ERROR_CODE(ISFS_ERROR_CORRUPT);
        REPORT_ERROR_CODE(ISFS_ERROR_NEEDFORMAT);
        REPORT_ERROR_CODE(ISFS_ERROR_EXISTS);
        REPORT_ERROR_CODE(ISFS_ERROR_NOEXISTS);
        REPORT_ERROR_CODE(ISFS_ERROR_MAXFILES);
        REPORT_ERROR_CODE(ISFS_ERROR_MAXBLOCKS);
        REPORT_ERROR_CODE(ISFS_ERROR_MAXFD);
        REPORT_ERROR_CODE(ISFS_ERROR_MAXDEPTH);
        REPORT_ERROR_CODE(ISFS_ERROR_OPENFD);
        REPORT_ERROR_CODE(ISFS_ERROR_BADBLOCK);
        REPORT_ERROR_CODE(ISFS_ERROR_ECC);
        REPORT_ERROR_CODE(ISFS_ERROR_ECC_CRIT);
        REPORT_ERROR_CODE(ISFS_ERROR_NOTEMPTY);
        REPORT_ERROR_CODE(ISFS_ERROR_AUTHENTICATION);
        REPORT_ERROR_CODE(ISFS_ERROR_UNKNOWN);
        REPORT_ERROR_CODE(ISFS_ERROR_BUSY);

        // IOS
        REPORT_ERROR_CODE(IOS_ERROR_ACCESS);
        REPORT_ERROR_CODE(IOS_ERROR_EXISTS);
        REPORT_ERROR_CODE(IOS_ERROR_INTR);
        REPORT_ERROR_CODE(IOS_ERROR_INVALID);
        REPORT_ERROR_CODE(IOS_ERROR_MAX);
        REPORT_ERROR_CODE(IOS_ERROR_NOEXISTS);
        REPORT_ERROR_CODE(IOS_ERROR_QEMPTY);
        REPORT_ERROR_CODE(IOS_ERROR_QFULL);
        REPORT_ERROR_CODE(IOS_ERROR_UNKNOWN);
        REPORT_ERROR_CODE(IOS_ERROR_NOTREADY);
        REPORT_ERROR_CODE(IOS_ERROR_ECC);
        REPORT_ERROR_CODE(IOS_ERROR_ECC_CRIT);
        REPORT_ERROR_CODE(IOS_ERROR_BADBLOCK);
        REPORT_ERROR_CODE(IOS_ERROR_INVALID_OBJTYPE);
        REPORT_ERROR_CODE(IOS_ERROR_INVALID_RNG);
        REPORT_ERROR_CODE(IOS_ERROR_INVALID_FLAG);
        REPORT_ERROR_CODE(IOS_ERROR_INVALID_FORMAT);
        REPORT_ERROR_CODE(IOS_ERROR_INVALID_VERSION);
        REPORT_ERROR_CODE(IOS_ERROR_INVALID_SIGNER);
        REPORT_ERROR_CODE(IOS_ERROR_FAIL_CHECKVALUE);
        REPORT_ERROR_CODE(IOS_ERROR_FAIL_INTERNAL);
        REPORT_ERROR_CODE(IOS_ERROR_FAIL_ALLOC);
        REPORT_ERROR_CODE(IOS_ERROR_INVALID_SIZE);
    }

#undef REPORT_ERROR_CODE
}

#if SDK_VERSION >= 20091211
static void nandLoggingCallback(BOOL done, s32 result) {
    // The error is unknown! What do we do?! Let's halt the system.
    if (result == ISFS_ERROR_UNKNOWN || result == IOS_ERROR_UNKNOWN) {
        __NANDPrintErrorMessage(result);
    }
}
#endif

s32 nandConvertErrorCode(s32 result) {
    int i;

    // clang-format off
    const s32 errorMap[] = {
        // ISFS
        ISFS_ERROR_OK,              NAND_RESULT_OK,
        ISFS_ERROR_ACCESS,          NAND_RESULT_ACCESS,
        ISFS_ERROR_CORRUPT,         NAND_RESULT_CORRUPT,
        ISFS_ERROR_ECC_CRIT,        NAND_RESULT_ECC_CRIT,
        ISFS_ERROR_EXISTS,          NAND_RESULT_EXISTS,
        ISFS_ERROR_AUTHENTICATION,  NAND_RESULT_AUTHENTICATION,
        ISFS_ERROR_INVALID,         NAND_RESULT_INVALID,
        ISFS_ERROR_MAXBLOCKS,       NAND_RESULT_MAXBLOCKS,
        ISFS_ERROR_MAXFD,           NAND_RESULT_MAXFD,
        ISFS_ERROR_MAXFILES,        NAND_RESULT_MAXFILES,
        ISFS_ERROR_MAXDEPTH,        NAND_RESULT_MAXDEPTH,
        ISFS_ERROR_NOEXISTS,        NAND_RESULT_NOEXISTS,
        ISFS_ERROR_NOTEMPTY,        NAND_RESULT_NOTEMPTY,
        ISFS_ERROR_NEEDFORMAT,      NAND_RESULT_UNKNOWN,
        ISFS_ERROR_OPENFD,          NAND_RESULT_OPENFD,
        ISFS_ERROR_UNKNOWN,         NAND_RESULT_UNKNOWN,
        ISFS_ERROR_BUSY,            NAND_RESULT_BUSY,
        ISFS_ERROR_FATAL_ERROR,     NAND_RESULT_FATAL_ERROR,

        // IOS
        IOS_ERROR_ACCESS,           NAND_RESULT_ACCESS,
        IOS_ERROR_EXISTS,           NAND_RESULT_EXISTS,
        IOS_ERROR_INTR,             NAND_RESULT_UNKNOWN,
        IOS_ERROR_INVALID,          NAND_RESULT_INVALID,
        IOS_ERROR_MAX,              NAND_RESULT_UNKNOWN,
        IOS_ERROR_NOEXISTS,         NAND_RESULT_NOEXISTS,
        IOS_ERROR_QEMPTY,           NAND_RESULT_UNKNOWN,
        IOS_ERROR_QFULL,            NAND_RESULT_BUSY,
        IOS_ERROR_UNKNOWN,          NAND_RESULT_UNKNOWN,
        IOS_ERROR_NOTREADY,         NAND_RESULT_UNKNOWN,
        IOS_ERROR_ECC,              NAND_RESULT_UNKNOWN,
        IOS_ERROR_ECC_CRIT,         NAND_RESULT_ECC_CRIT,
        IOS_ERROR_BADBLOCK,         NAND_RESULT_UNKNOWN,
        IOS_ERROR_INVALID_OBJTYPE,  NAND_RESULT_UNKNOWN,
        IOS_ERROR_INVALID_RNG,      NAND_RESULT_UNKNOWN,
        IOS_ERROR_INVALID_FLAG,     NAND_RESULT_UNKNOWN,
        IOS_ERROR_INVALID_FORMAT,   NAND_RESULT_UNKNOWN,
        IOS_ERROR_INVALID_VERSION,  NAND_RESULT_UNKNOWN,
        IOS_ERROR_INVALID_SIGNER,   NAND_RESULT_UNKNOWN,
        IOS_ERROR_FAIL_CHECKVALUE,  NAND_RESULT_UNKNOWN,
        IOS_ERROR_FAIL_INTERNAL,    NAND_RESULT_UNKNOWN,
        IOS_ERROR_FAIL_ALLOC,       NAND_RESULT_ALLOC_FAILED,
        IOS_ERROR_INVALID_SIZE,     NAND_RESULT_UNKNOWN
    };
    // clang-format on

    i = 0;

    if (result >= ISFS_ERROR_OK) {
        return result;
    }

    for (; i < ARRAY_LENGTH(errorMap); i += 2) {
        if (result == errorMap[i]) {
            if (result == ISFS_ERROR_ECC_CRIT || result == ISFS_ERROR_AUTHENTICATION || result == ISFS_ERROR_UNKNOWN || result == IOS_ERROR_UNKNOWN ||
                result == IOS_ERROR_ECC_CRIT) {
                // Log code to shared2/test/nanderr.log
                char msg[128] ALIGN64;
                sprintf(msg, "ISFS error code: %d", result);

#if SDK_VERSION >= 20091211
                NANDLoggingAddMessageAsync(nandLoggingCallback, result, msg);
#else
                NANDLoggingAddMessageAsync(NULL, msg);
#endif
            }
            nandReportErrorCode(result);

#if SDK_VERSION >= 20091211
            if (result == ISFS_ERROR_MAXBLOCKS || result == ISFS_ERROR_MAXFILES || result == ISFS_ERROR_CORRUPT || result == ISFS_ERROR_BUSY ||
                result == IOS_ERROR_QFULL || result == IOS_ERROR_FAIL_ALLOC) {
                __NANDPrintErrorMessage(result);
            }
#endif

            return errorMap[i + 1];
        }
    }

    OSReport("CAUTION!  Unexpected error code [%d] was found.\n", result);

    // Log code to shared2/test/nanderr.log
    {
        char msg[128] ALIGN64;
        sprintf(msg, "ISFS unexpected error code: %d", result);
#if SDK_VERSION >= 20091211
        NANDLoggingAddMessageAsync(nandLoggingCallback, result, msg);
#else
        NANDLoggingAddMessageAsync(NULL, msg);
#endif
        nandReportErrorCode(result);
    }

    return NAND_RESULT_UNKNOWN;
}

void nandGenerateAbsPath(char* abs, const char* rel) {
    if (strlen(rel) == 0) {
        strcpy(abs, "");
    } else if (nandIsRelativePath(rel)) {
        nandConvertPath(abs, s_currentDir, rel);
    } else {
        s32 len = -1;
        strcpy(abs, rel);
        len = strlen(abs);
        if (len != 0 && abs[len - 1] == '/' && len - 1 != 0) {
            abs[len - 1] = '\0';
        }
    }
}

void nandGetParentDirectory(char* dir, const char* path) {
    int i = 0;

    for (i = strlen(path); i >= 0; i--) {
        if (path[i] == '/') {
            break;
        }
    }

    if (i == 0) {
        strcpy(dir, "/");
    } else {
        strncpy(dir, path, i);
        dir[i] = '\0';
    }
}

s32 NANDInit() {
    s32 isfsErr;
    BOOL enabled;

    enabled = OSDisableInterrupts();

    if (s_libState == STATE_INITIALIZING) {
        OSRestoreInterrupts(enabled);
        return NAND_RESULT_BUSY;
    }

    if (s_libState == STATE_INITIALIZED) {
        OSRestoreInterrupts(enabled);
        return NAND_RESULT_OK;
    }

    isfsErr = ISFS_ERROR_UNKNOWN;

    s_libState = STATE_INITIALIZING;
    OSRestoreInterrupts(enabled);

    isfsErr = ISFS_OpenLib();

    if (isfsErr == ISFS_ERROR_OK) {
        ESError esErr;
        ESTitleId titleId;

        esErr = ESP_InitLib();

        if (esErr == ES_ERR_OK) {
            esErr = ESP_GetTitleId(&titleId);
        }

        if (esErr == ES_ERR_OK) {
            esErr = ESP_GetDataDir(titleId, s_homeDir);
        }

        if (esErr == ES_ERR_OK) {
            strcpy(s_currentDir, s_homeDir);
        }

        ESP_CloseLib();

        if (esErr != ES_ERR_OK) {
            OSReport("Failed to set home directory.\n");
        }

        OSRegisterShutdownFunction(&s_shutdownFuncInfo);

        enabled = OSDisableInterrupts();
        s_libState = STATE_INITIALIZED;
        OSRestoreInterrupts(enabled);

#if SDK_VERSION >= 20091211
        NANDSetAutoErrorMessaging(TRUE);
#endif

        OSRegisterVersion(GetVersion(NAND));

        return NAND_RESULT_OK;
    } else {
        enabled = OSDisableInterrupts();
        s_libState = STATE_NOT_INITIALIZED;
        OSRestoreInterrupts(enabled);
        return nandConvertErrorCode(isfsErr);
    }
}

static void nandShutdownCallback(s32 result, void* arg);
static BOOL nandOnShutdown(BOOL final, u32 event) {
    if (!final) {
        if (event == OS_SHUTDOWN_2) {
            volatile BOOL shutdown = FALSE;
            s64 start = OSGetTime();
            ISFS_ShutdownAsync(nandShutdownCallback, (void*)&shutdown);

            while (OSTicksToMilliseconds(OSGetTime() - start) < 500) {
                if (shutdown) {
                    break;
                }
            }
        }

        return TRUE;
    }

    return TRUE;
}

static void nandShutdownCallback(s32 result, void* arg) {
#pragma unused(result)
    *(BOOL*)arg = TRUE;
}

static void nandChangeDirCallback(s32 result, void* arg);
static s32 nandChangeDir(const char* path, NANDCommandBlock* block, BOOL asyncFlag, BOOL privateFlag) {
    s32 isfsErr;

    if (asyncFlag) {
        nandGenerateAbsPath(block->absPath, path);

        if (!privateFlag && nandIsPrivatePath(block->absPath)) {
            return ISFS_ERROR_ACCESS;
        }

        return ISFS_ReadDirAsync(block->absPath, NULL, &block->num, nandChangeDirCallback, block);
    } else {
        u32 numFiles = 0;
        char absPath[NAND_MAX_PATH] = "";
        nandGenerateAbsPath(absPath, path);

        if (!privateFlag && nandIsPrivatePath(absPath)) {
            return ISFS_ERROR_ACCESS;
        }

        isfsErr = ISFS_ReadDir(absPath, NULL, &numFiles);

        if (isfsErr == ISFS_ERROR_OK) {
            BOOL enabled = OSDisableInterrupts();
            strcpy(s_currentDir, absPath);
            OSRestoreInterrupts(enabled);
        }

        return isfsErr;
    }
}

s32 NANDChangeDir(const char* path) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandChangeDir(path, NULL, FALSE, FALSE));
}

s32 NANDChangeDirAsync(const char* path, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandChangeDir(path, block, TRUE, FALSE));
}

s32 NANDPrivateChangeDir(const char* path) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandChangeDir(path, NULL, FALSE, TRUE));
}

s32 NANDPrivateChangeDirAsync(const char* path, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandChangeDir(path, block, TRUE, TRUE));
}

static void nandChangeDirCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result == ISFS_ERROR_OK) {
        BOOL enabled = OSDisableInterrupts();
        strcpy(s_currentDir, block->absPath);
        OSRestoreInterrupts(enabled);
    }

    ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
}

s32 NANDGetCurrentDir(char* path) {
    BOOL enabled = FALSE;

    ASSERTMSGLINE(1089, path, "NULL pointer is detected: %s()", __FUNCTION__);

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    enabled = OSDisableInterrupts();
    strcpy(path, s_currentDir);
    OSRestoreInterrupts(enabled);
    return NAND_RESULT_OK;
}

s32 NANDGetHomeDir(char* path) {
    ASSERTMSGLINE(1109, path, "NULL pointer is detected: %s()", __FUNCTION__);

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    strcpy(path, s_homeDir);
    return NAND_RESULT_OK;
}

void nandCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;
    ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
}

static void nandGetTypeCallback(s32 result, void* arg);
static s32 nandGetType(const char* path, u8* type, NANDCommandBlock* block, BOOL asyncFlag, BOOL privateFlag) {
    u32 numFiles;
    s32 isfsErr;

    ASSERTMSGLINE(1133, path, "NULL pointer is detected: %s()", __FUNCTION__);
    ASSERTMSGLINE(1134, type, "NULL pointer is detected: %s()", __FUNCTION__);

    if (strlen(path) == 0) {
        return ISFS_ERROR_INVALID;
    }

    if (asyncFlag) {
        nandGenerateAbsPath(block->absPath, path);

        if (!privateFlag && nandIsUnderPrivatePath(block->absPath)) {
            return ISFS_ERROR_ACCESS;
        }

        block->type = type;
        return ISFS_ReadDirAsync(block->absPath, NULL, &block->num, nandGetTypeCallback, block);
    } else {
        char absPath[NAND_MAX_PATH] = "";
        nandGenerateAbsPath(absPath, path);

        if (!privateFlag && nandIsUnderPrivatePath(absPath)) {
            return ISFS_ERROR_ACCESS;
        }

        numFiles = 0;
        isfsErr = ISFS_ReadDir(absPath, NULL, &numFiles);

        if (isfsErr == ISFS_ERROR_OK || isfsErr == ISFS_ERROR_ACCESS) {
            *type = NAND_FILE_TYPE_DIR;
            isfsErr = ISFS_ERROR_OK;
        } else if (isfsErr == ISFS_ERROR_INVALID) {
            *type = NAND_FILE_TYPE_FILE;
            isfsErr = ISFS_ERROR_OK;
        }

        return isfsErr;
    }
}

s32 NANDGetType(const char* path, u8* type) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandGetType(path, type, NULL, FALSE, FALSE));
}

s32 NANDGetTypeAsync(const char* path, u8* type, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandGetType(path, type, block, TRUE, FALSE));
}

s32 NANDPrivateGetType(const char* path, u8* type) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandGetType(path, type, NULL, FALSE, TRUE));
}

s32 NANDPrivateGetTypeAsync(const char* path, u8* type, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandGetType(path, type, block, TRUE, TRUE));
}

static void nandGetTypeCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result == ISFS_ERROR_OK || result == ISFS_ERROR_ACCESS) {
        *block->type = NAND_FILE_TYPE_DIR;
        result = ISFS_ERROR_OK;
    } else if (result == ISFS_ERROR_INVALID) {
        *block->type = NAND_FILE_TYPE_FILE;
        result = ISFS_ERROR_OK;
    }

    ((NANDAsyncCallback)block->callback)(nandConvertErrorCode(result), block);
}

const char* nandGetHomeDir() {
    return s_homeDir;
}

void NANDInitBanner(NANDBanner* banner, u32 flags, const u16* comment0, const u16* comment1) {
    ASSERTMSGLINE(1253, banner, "Null pointer is detected at %s()", __FUNCTION__);
    ASSERTMSGLINE(1254, comment0, "Null pointer is detected at %s()", __FUNCTION__);
    ASSERTMSGLINE(1255, comment1, "Null pointer is detected at %s()", __FUNCTION__);

    memset(banner, 0, sizeof(NANDBanner));

    banner->signature = NAND_BANNER_SIGNATURE;
    banner->flags = flags;

    if (wcscmp(comment0, L"") == 0) {
        wcsncpy(banner->comment[0], L" ", NAND_BANNER_COMMENT_LENGTH);
    } else {
        wcsncpy(banner->comment[0], comment0, NAND_BANNER_COMMENT_LENGTH);
    }

    if (wcscmp(comment1, L"") == 0) {
        wcsncpy(banner->comment[1], L" ", NAND_BANNER_COMMENT_LENGTH);
    } else {
        wcsncpy(banner->comment[1], comment1, NAND_BANNER_COMMENT_LENGTH);
    }
}
