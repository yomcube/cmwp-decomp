#ifndef PRIVATE_NAND_H
#define PRIVATE_NAND_H

#include <revolution/nand.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO
enum {
    NAND_STATE_0 = 0,
    NAND_STATE_1,
    NAND_STATE_2,
    NAND_STATE_3,
    NAND_STATE_4,
    NAND_STATE_5,
    NAND_STATE_6,
    NAND_STATE_7,
    NAND_STATE_8,
    NAND_STATE_9,
    NAND_STATE_10,
    NAND_STATE_11,
    NAND_STATE_12,
    NAND_STATE_13,
    NAND_STATE_14,
    NAND_STATE_20 = 20,
    NAND_STATE_21,
};

// TODO
enum {
    NAND_STAGE_0 = 0,
    NAND_STAGE_1,
    NAND_STAGE_2,
    NAND_STAGE_3,
    NAND_STAGE_4,
    NAND_STAGE_5,
    NAND_STAGE_6,
    NAND_STAGE_7,
    NAND_STAGE_8,
    NAND_STAGE_9,
    NAND_STAGE_64 = 64,
};

enum {
    NAND_MARK_NONE = 0,
    NAND_MARK_OPENED,
    NAND_MARK_CLOSED,
    NAND_MARK_SAFE_OPENED,
    NAND_MARK_SAFE_CLOSED,
    NAND_MARK_SIMPLE_SAFE_OPENED,
    NAND_MARK_SIMPLE_SAFE_CLOSED,
};

/* Internal functions only for use in the library */

void nandRemoveTailToken(char* newpath, const char* oldpath);
void nandGetHeadToken(char* head, char* rest, const char* path);
void nandGetRelativeName(char* name, const char* path);
void nandConvertPath(char* abs, const char* wd, const char* rel);

BOOL nandIsRelativePath(const char* path);
BOOL nandIsAbsolutePath(const char* path);
BOOL nandIsPrivatePath(const char* path);
BOOL nandIsUnderPrivatePath(const char* path);
BOOL nandIsJustBelowShared2(const char* path);

BOOL nandIsAddressMEM2(const void* buf);
BOOL nandIsInitialized();

void nandReportErrorCode(s32 result);
s32 nandConvertErrorCode(s32 result);

void nandGenerateAbsPath(char* abs, const char* rel);
void nandGetParentDirectory(char* dir, const char* path);

void nandCallback(s32 result, void* arg);

const char* nandGetHomeDir();

/* Other stuff */

void __NANDPrintErrorMessage(s32 result);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_NAND_H
