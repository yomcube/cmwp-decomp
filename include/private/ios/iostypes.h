#ifndef PRIVATE_IOS_TYPES_H
#define PRIVATE_IOS_TYPES_H

#include <revolution/types.h>

/**
 * https://github.com/iversonjimmy/acer_cloud_wifi_copy/blob/master/sw_x/es_core/esc/core/ioscrypto/iostypes.h
 */

#define IOS_ERROR_OK 0        /* Success! */
#define IOS_ERROR_ACCESS -1   /* Access denied */
#define IOS_ERROR_EXISTS -2   /* Already exists */
#define IOS_ERROR_INTR -3     /* Timeout */
#define IOS_ERROR_INVALID -4  /* Invalid data */
#define IOS_ERROR_MAX -5      /* Overflow */
#define IOS_ERROR_NOEXISTS -6 /* Does not exist */
#define IOS_ERROR_QEMPTY -7   /* Not waiting */
#define IOS_ERROR_QFULL -8    /* Waiting */
#define IOS_ERROR_UNKNOWN -9
#define IOS_ERROR_NOTREADY -10 /* Not ready to do something*/
#define IOS_ERROR_ECC -11      /* ECC error*/
#define IOS_ERROR_ECC_CRIT -12 /* Critical ECC error*/
#define IOS_ERROR_BADBLOCK -13 /* Bad NAND block*/

#define IOS_ERROR_INVALID_OBJTYPE -14
#define IOS_ERROR_INVALID_RNG -15
#define IOS_ERROR_INVALID_FLAG -16
#define IOS_ERROR_INVALID_FORMAT -17
#define IOS_ERROR_INVALID_VERSION -18
#define IOS_ERROR_INVALID_SIGNER -19
#define IOS_ERROR_FAIL_CHECKVALUE -20
#define IOS_ERROR_FAIL_INTERNAL -21
#define IOS_ERROR_FAIL_ALLOC -22
#define IOS_ERROR_INVALID_SIZE -23

#define IOS_ERROR_HW_RESET -24

#define IOS_SEEK_SET 0
#define IOS_SEEK_CURRENT 1
#define IOS_SEEK_END 2

#define IOS_OPEN 1
#define IOS_CLOSE 2
#define IOS_READ 3
#define IOS_WRITE 4
#define IOS_SEEK 5
#define IOS_IOCTL 6
#define IOS_IOCTLV 7
#define IOS_REPLY 8

typedef s32 IOSError;

typedef s32 IOSMessageQueueId;
typedef s32 IOSMessage;
typedef s32 IOSTimerId;
typedef s32 IOSHeapId;

typedef s32 IOSFd;

typedef u32 IOSUid;
typedef u16 IOSGid;

typedef u32 IOSTime;

typedef void (*IOSEntryProc)(u32);

typedef u32 IOSResourceHandle;

typedef struct {
    const u8* path;  // 0x00
    u32 flags;       // 0x04
    IOSUid uid;      // 0x08
    IOSGid gid;      // 0x0C
} IOSResourceOpen;

typedef struct {
    u8* outPtr;  // 0x00
    u32 outLen;  // 0x04
} IOSResourceRead;

typedef struct {
    u8* inPtr;  // 0x00
    u32 inLen;  // 0x04
} IOSResourceWrite;

typedef struct {
    s32 offset;  // 0x00
    u32 whence;  // 0x04
} IOSResourceSeek;

typedef struct {
    u32 cmd;     // 0x00
    u8* inPtr;   // 0x04
    u32 inLen;   // 0x08
    u8* outPtr;  // 0x0C
    u32 outLen;  // 0x10
} IOSResourceIoctl;

typedef struct {
    u8* base;    // 0x00
    u32 length;  // 0x04
} IOSIoVector;

typedef struct {
    u32 cmd;              // 0x00
    u32 readCount;        // 0x04
    u32 writeCount;       // 0x08
    IOSIoVector* vector;  // 0x0C
} IOSResourceIoctlv;

typedef struct {
    u32 cmd;                   // 0x00
    IOSError status;           // 0x04
    IOSResourceHandle handle;  // 0x08
    union {
        IOSResourceOpen open;
        IOSResourceRead read;
        IOSResourceWrite write;
        IOSResourceSeek seek;
        IOSResourceIoctl ioctl;
        IOSResourceIoctlv ioctlv;
    } args;  // 0x0C
} IOSResourceRequest;

#define IOS_SYSTEM_HEAP_ID 0
#define IOS_SYSTEM_FD 0

#endif  // PRIVATE_IOS_TYPES_H
