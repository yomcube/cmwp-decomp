#include <private/nand.h>
#include <revolution/nand.h>

#include <revolution/fs.h>
#include <revolution/ios.h>
#include <revolution/os.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    STAGE_NONE = 0,
    STAGE_BEGIN,
    STAGE_SEEK_TO_BEGINNING,
    STAGE_READ_PREV_MESSAGE,
    STAGE_SEEEK_BACK_TO_BEGIN,
    STAGE_WRITE_NEW_MESSAGE,
    STAGE_SEEK_TO_NEXT_LINE,
    STAGE_WRITE_PREV_MSG_TO_NEW_LINE,
    STAGE_CLOSE,
    STAGE_SUCCESS,
};

static char s_message[NAND_LOG_MESSAGE_LENGTH] ALIGN64;

static s32 s_fd = -255;
#if SDK_VERSION >= 20091211
static s32 s_err = ISFS_ERROR_UNKNOWN;
#endif

static NANDLoggingCallback s_callback = NULL;
static s32 s_stage = STAGE_NONE;

static BOOL open();
static int readHeader();
static BOOL close();
static BOOL write(int line, const char* errMsg);
static BOOL reserveFileDescriptor();
static void callbackRoutine(BOOL flag);
static void asyncRoutine(s32 result, void* arg);
static void prepareLine(char* msg, int line, const char* errMsg);

BOOL NANDLoggingPrepareFile(char* work) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;
    s32 fd = -1;
    u32 dummy = 0;

    isfsErr = ISFS_ReadDir("/shared2/test2/nanderr.log", NULL, &dummy);
    if (isfsErr == ISFS_ERROR_OK || isfsErr == ISFS_ERROR_ACCESS) {
        OSReport("Caution!  Path of logfile is a directory!");
        return FALSE;
    }

    if (isfsErr == ISFS_ERROR_INVALID) {
        return TRUE;
    }

    if (isfsErr != ISFS_ERROR_NOEXISTS) {
        OSReport("Caution!  ISFS_ReadDir() failed.\n");
        return FALSE;
    }

    isfsErr = ISFS_CreateFile("/tmp/nanderr.log", 0, ISFS_ACCESS_RW, ISFS_ACCESS_RW, ISFS_ACCESS_RW);
    if (isfsErr != ISFS_ERROR_OK) {
        return FALSE;
    }

    fd = ISFS_Open("/tmp/nanderr.log", ISFS_ACCESS_WRITE);
    if (fd < ISFS_ERROR_OK) {
        return FALSE;
    }

    {
        int i = 0;

        memset(work, ' ', NAND_LOG_LINE_COUNT * NAND_LOG_MESSAGE_LENGTH);
        prepareLine(work, 0, "Created log file.");

        for (i = 0; i < NAND_LOG_LINE_COUNT; i++) {
            work[(i + 1) * NAND_LOG_MESSAGE_LENGTH - 2] = '\r';
            work[(i + 1) * NAND_LOG_MESSAGE_LENGTH - 1] = '\n';
        }
    }

    isfsErr = ISFS_Write(fd, work, NAND_LOG_LINE_COUNT * NAND_LOG_MESSAGE_LENGTH);
    if (isfsErr != NAND_LOG_LINE_COUNT * NAND_LOG_MESSAGE_LENGTH) {
        OSReport("ISFS_Write() failed.\n");
        ISFS_Close(fd);
        ISFS_Delete("/tmp/nanderr.log");
        return FALSE;
    }

    isfsErr = ISFS_Close(fd);
    if (isfsErr != ISFS_ERROR_OK) {
        OSReport("ISFS_Close() failed.\n");
        return FALSE;
    }

    isfsErr = ISFS_Rename("/tmp/nanderr.log", "/shared2/test2/nanderr.log");
    if (isfsErr != ISFS_ERROR_OK) {
        OSReport("ISFS_Rename() failed.\n");
        return FALSE;
    }

    return TRUE;
}

BOOL NANDLoggingAddMessage(const char* format, ...) {
    va_list list;
    char msg[NAND_LOG_MESSAGE_LENGTH];
    s32 line = 0;

    va_start(list, format);
    vsnprintf(msg, NAND_LOG_MESSAGE_LENGTH, format, list);
    va_end(list);

    if (!open()) {
        return FALSE;
    }

    line = readHeader();
    if (line <= 0) {
        close();
        return FALSE;
    }

    if (!write(line, msg)) {
        close();
        return FALSE;
    }

    if (!close()) {
        return FALSE;
    }

    return TRUE;
}

static BOOL open() {
    if (!reserveFileDescriptor()) {
        return FALSE;
    }

    s_fd = ISFS_Open("/shared2/test2/nanderr.log", ISFS_ACCESS_RW);
    if (s_fd >= 0) {
        return TRUE;
    }

    s_fd = -255;
    return FALSE;
}

static BOOL close() {
    if (s_fd < 0) {
        return FALSE;
    }

    if (ISFS_Close(s_fd) == ISFS_ERROR_OK) {
        s_fd = -255;
        return TRUE;
    }

    return FALSE;
}

static int readHeader() {
    char msg[NAND_LOG_LINE_COUNT] ALIGN64;
    s32 fd = -1;

    if (s_fd < 0) {
        return -1;
    }

    if (ISFS_Seek(s_fd, 0, ISFS_SEEK_BEG) != ISFS_ERROR_OK) {
        return -1;
    }

    if (ISFS_Read(s_fd, msg, NAND_LOG_LINE_COUNT) != NAND_LOG_LINE_COUNT) {
        return -1;
    }

    msg[NAND_LOG_LINE_COUNT - 1] = 0;
    return atoi(msg);
}

static BOOL write(int line, const char* errMsg) {
    char msg[NAND_LOG_MESSAGE_LENGTH] ALIGN64;
    memset(&msg, ' ', NAND_LOG_MESSAGE_LENGTH);

    if (s_fd < 0) {
        return FALSE;
    }
    if (line <= 0 || line >= NAND_LOG_LINE_COUNT) {
        return FALSE;
    }
    if (!errMsg) {
        return FALSE;
    }

    prepareLine(msg, line, errMsg);

    if (ISFS_Seek(s_fd, 0, ISFS_SEEK_BEG)) {
        return FALSE;
    }

    if (ISFS_Write(s_fd, msg, NAND_LOG_MESSAGE_LENGTH) != NAND_LOG_MESSAGE_LENGTH) {
        return FALSE;
    }

    if ((line * NAND_LOG_MESSAGE_LENGTH) != ISFS_Seek(s_fd, line * NAND_LOG_MESSAGE_LENGTH, ISFS_SEEK_BEG)) {
        return FALSE;
    }

    if (ISFS_Write(s_fd, msg, NAND_LOG_MESSAGE_LENGTH) != NAND_LOG_MESSAGE_LENGTH) {
        return FALSE;
    }

    return TRUE;
}

static BOOL reserveFileDescriptor() {
    BOOL enabled = FALSE;
    BOOL result = FALSE;

    enabled = OSDisableInterrupts();

    if (s_fd == -255) {
        s_fd = -254;
        result = FALSE;
    } else if (s_fd == -254) {
        result = TRUE;
    } else if (s_fd >= 0) {
        result = TRUE;
    } else {
        result = TRUE;
    }

    OSRestoreInterrupts(enabled);

    return result ? FALSE : TRUE;
}

#if SDK_VERSION >= 20091211
BOOL NANDLoggingAddMessageAsync(NANDLoggingCallback callback, s32 result, const char* format, ...)
#else
BOOL NANDLoggingAddMessageAsync(NANDLoggingCallback callback, const char* format, ...)
#endif
{
    s32 isfsErr = ISFS_ERROR_UNKNOWN;
    va_list list;

    if (!reserveFileDescriptor()) {
        return FALSE;
    }

    va_start(list, format);
    vsnprintf(s_message, NAND_LOG_MESSAGE_LENGTH, format, list);
    va_end(list);

    s_callback = callback;
    s_stage = STAGE_BEGIN;

#if SDK_VERSION >= 20091211
    if (result == ISFS_ERROR_UNKNOWN || result == IOS_ERROR_UNKNOWN) {
        s_err = result;
    }
#endif

    isfsErr = ISFS_OpenAsync("/shared2/test2/nanderr.log", ISFS_ACCESS_RW, asyncRoutine, NULL);
    if (isfsErr == ISFS_ERROR_OK) {
        return TRUE;
    }

    return FALSE;
}

static void callbackRoutine(BOOL flag) {
    if (s_callback) {
#if SDK_VERSION >= 20091211
        s_callback(flag, s_err);
#else
        s_callback(flag);
#endif
    }
}

static void asyncRoutine(s32 result, void* arg) {
    s32 isfsErr = ISFS_ERROR_UNKNOWN;

    static char s_rBuf[NAND_LOG_MESSAGE_LENGTH] ALIGN64;
    static char s_wBuf[NAND_LOG_MESSAGE_LENGTH] ALIGN64;

    (void)arg;

    s_stage++;

    if (s_stage == STAGE_SEEK_TO_BEGINNING) {
        if (result >= 0) {
            s_fd = result;

            isfsErr = ISFS_SeekAsync(s_fd, 0, ISFS_SEEK_BEG, asyncRoutine, NULL);
            if (isfsErr != ISFS_ERROR_OK) {
                callbackRoutine(FALSE);
            }
        } else {
            callbackRoutine(FALSE);
        }
    } else if (s_stage == STAGE_READ_PREV_MESSAGE) {
        if (result == ISFS_ERROR_OK) {
            isfsErr = ISFS_ReadAsync(s_fd, s_rBuf, NAND_LOG_MESSAGE_LENGTH, asyncRoutine, NULL);
            if (isfsErr != ISFS_ERROR_OK) {
                callbackRoutine(FALSE);
            }
        } else {
            callbackRoutine(FALSE);
        }
    } else if (s_stage == STAGE_SEEEK_BACK_TO_BEGIN) {
        if (result == NAND_LOG_MESSAGE_LENGTH) {
            isfsErr = ISFS_SeekAsync(s_fd, 0, ISFS_SEEK_BEG, asyncRoutine, NULL);
            if (isfsErr != ISFS_ERROR_OK) {
                callbackRoutine(FALSE);
            }
        } else {
            callbackRoutine(FALSE);
        }
    } else if (s_stage == STAGE_WRITE_NEW_MESSAGE) {
        if (result == ISFS_ERROR_OK) {
            s32 line = 0;
            s_rBuf[NAND_LOG_MESSAGE_LENGTH - 1] = 0;
            line = atoi(s_rBuf);

            prepareLine(s_wBuf, line, s_message);

            isfsErr = ISFS_WriteAsync(s_fd, s_wBuf, NAND_LOG_MESSAGE_LENGTH, asyncRoutine, NULL);
            if (isfsErr != ISFS_ERROR_OK) {
                callbackRoutine(FALSE);
            }
        } else {
            callbackRoutine(FALSE);
        }
    } else if (s_stage == STAGE_SEEK_TO_NEXT_LINE) {
        if (result == NAND_LOG_MESSAGE_LENGTH) {
            s32 line = atoi(s_rBuf);

            isfsErr = ISFS_SeekAsync(s_fd, line * NAND_LOG_MESSAGE_LENGTH, ISFS_SEEK_BEG, asyncRoutine, NULL);
            if (isfsErr != ISFS_ERROR_OK) {
                callbackRoutine(FALSE);
            }
        } else {
            callbackRoutine(FALSE);
        }
    } else if (s_stage == STAGE_WRITE_PREV_MSG_TO_NEW_LINE) {
        s32 line = atoi(s_rBuf);
        if (result == (line * NAND_LOG_MESSAGE_LENGTH)) {
            isfsErr = ISFS_WriteAsync(s_fd, s_wBuf, NAND_LOG_MESSAGE_LENGTH, asyncRoutine, NULL);
            if (isfsErr != ISFS_ERROR_OK) {
                callbackRoutine(FALSE);
            }
        } else {
            callbackRoutine(FALSE);
        }
    } else if (s_stage == STAGE_CLOSE) {
        if (result == NAND_LOG_MESSAGE_LENGTH) {
            isfsErr = ISFS_CloseAsync(s_fd, asyncRoutine, NULL);
            if (isfsErr != ISFS_ERROR_OK) {
                callbackRoutine(FALSE);
            }
        } else {
            callbackRoutine(FALSE);
        }
    } else if (s_stage == STAGE_SUCCESS) {
        if (result == ISFS_ERROR_OK) {
            s_fd = -255;
            callbackRoutine(TRUE);
        } else {
            callbackRoutine(FALSE);
        }
    } else {
        (void)0;
    }
}

static void prepareLine(char* msg, int line, const char* errMsg) {
    char titleIdStr[NAND_MAX_PATH];
    OSCalendarTime cal;
    int len = 0;

    memset(msg, ' ', NAND_LOG_MESSAGE_LENGTH - 2);

    OSTicksToCalendarTime(OSGetTime(), &cal);

    // Get title ID (...could they have done a better way of doing this??)
    strncpy(titleIdStr, nandGetHomeDir() + 7, 17);
    titleIdStr[8] = '-';
    titleIdStr[17] = 0;

    len = snprintf(msg, NAND_LOG_MESSAGE_LENGTH, "%d %04d/%02d/%02d %02d:%02d:%02d %s %s", (line % (NAND_LOG_LINE_COUNT - 1)) + 1, cal.year,
                   cal.mon + 1, cal.mday, cal.hour, cal.min, cal.sec, titleIdStr, errMsg);
    if (len < NAND_LOG_MESSAGE_LENGTH) {
        msg[len] = ' ';
    }

    // Add CRLF at the end
    msg[NAND_LOG_MESSAGE_LENGTH - 2] = '\r';
    msg[NAND_LOG_MESSAGE_LENGTH - 1] = '\n';
}
