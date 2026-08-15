#include <private/ios.h>
#include <private/ipc.h>

#include <private/hollywood.h>

#include <revolution/os.h>

#include <string.h>

#if SDK_VERSION >= 20091211 || defined(SDK_IPL)
#define QUEUE_CAPACITY 48
#else
#define QUEUE_CAPACITY 16
#endif

typedef struct {
    IOSResourceRequest request;    // 0x00
    IOSCallback callback ALIGN32;  // 0x20
    void* callbackArg;             // 0x24
    BOOL reboot;                   // 0x28
    OSThreadQueue threadQueue;     // 0x2C
    u32 unk_0x34;
    u32 unk_0x38;
    u32 unk_0x3C;
} __ios_rpc;

typedef struct {
    u32 sent;                                   // 0x00
    u32 queued;                                 // 0x04
    u32 front;                                  // 0x08
    u32 back;                                   // 0x0C
    IOSResourceRequest* queue[QUEUE_CAPACITY];  // 0x10
} __ios_Response;

static s32 __mailboxAck = 1;
static IOSHeapId hid = IOS_INVALID_HEAP;

static BOOL __relnchFl = FALSE;
static __ios_rpc* __relnchRpc = NULL;
static __ios_rpc* __relnchRpcSave = NULL;
static __ios_rpc __rpcBuf;

static __ios_Response __responses;

#if SDK_VERSION >= 20091211
static OSAlarm __timeout_alarm;
#endif

size_t strnlen(const char* str, size_t length) {
    u8* p;
    for (p = (u8*)str; *p && length--; p++) {
    }
    return (size_t)((char*)p - str);
}

/************/
/* REQUESTS */
/************/

static __ios_rpc* ipcAllocReq() {
    __ios_rpc* p = NULL;
    p = iosAllocAligned(hid, sizeof(__ios_rpc), DEFAULT_ALIGN);
    return p;
}

static IOSError ipcFree(void* p) {
    IOSError result = IOS_ERROR_OK;
    iosFree(hid, p);
    return result;
}

static IOSError __ipcQueueRequest(IOSResourceRequest* request) {
    IOSError result = IOS_ERROR_OK;

    // Difference of count
    if ((__responses.queued < __responses.sent ? (0xFFFFFFFF - __responses.sent + 1 /* hack to subtract by 0 */ + __responses.queued) :
                                                 ((__responses.queued - __responses.sent) >= QUEUE_CAPACITY))) /* Is queue full?*/ {
        result = IOS_ERROR_QFULL;
        goto exit;
    }

    __responses.queue[__responses.back] = request;
    __responses.back = (__responses.back + 1) % QUEUE_CAPACITY;
    __responses.queued++;

    IPCiProfQueueReq(request, (IOSFd)request->handle);

exit:
    return result;
}

static void __ipcSendRequest() {
    __ios_rpc* response;

    // Difference of count
    if ((__responses.queued < __responses.sent ? (0xFFFFFFFF - __responses.sent + 1 /* hack to subtract by 0 */ + __responses.queued) :
                                                 ((__responses.queued - __responses.sent) == 0))) /* Is queue full?*/ {
        goto exit;
    }

    response = (__ios_rpc*)__responses.queue[__responses.front];

    if (!response) {
        goto exit;
    }

    if (response->reboot) {
        __mailboxAck--;
    }

    IPCWriteReg(HW_IPC_PPCMSG, OSCachedToPhysical(response));

    __responses.front = (__responses.front + 1) % QUEUE_CAPACITY;
    __responses.sent++;

    __mailboxAck--;

    IPCWriteReg(HW_IPC_PPCCTRL, (IPCReadReg(HW_IPC_PPCCTRL) & 0x30) | 1);

exit:
    return;
}

/**************/
/* MAIN STUFF */
/**************/

static void IpcReplyHandler(__OSInterrupt interrupt, OSContext* context) {
    IOSMessage recvMsg;
    IOSResourceRequest* request;
    __ios_rpc* rpc;

    recvMsg = IPCReadReg(HW_IPC_ARMMSG);
    if (!recvMsg) {
        return;
    }

    rpc = (__ios_rpc*)OSPhysicalToCached(recvMsg);

    IPCWriteReg(HW_IPC_PPCCTRL, (IPCReadReg(HW_IPC_PPCCTRL) & 0x30) | 4);
    ACRWriteReg(HW_PPCIRQFLAG << 2, 0x40000000);

    request = &rpc->request;

    DCInvalidateRange(request, sizeof(*request));

    // Not cmd??
    switch (request->handle) {
        case IOS_READ: {
            request->args.read.outPtr = request->args.open.path ? OSPhysicalToCached((u32)request->args.read.outPtr) : NULL;
            if (request->status > 0) {
                DCInvalidateRange(request->args.read.outPtr, request->status);
            }
            break;
        }
        case IOS_IOCTL: {
            request->args.ioctl.outPtr = request->args.ioctl.outPtr ? OSPhysicalToCached((u32)request->args.ioctl.outPtr) : NULL;
            DCInvalidateRange(request->args.ioctl.inPtr, request->args.ioctl.inLen);
            DCInvalidateRange(request->args.ioctl.outPtr, request->args.ioctl.outLen);
            break;
        }
        case IOS_IOCTLV: {
            IOSResourceIoctlv* ioctlv = &request->args.ioctlv;
            int i;

            request->args.ioctlv.vector = request->args.ioctlv.vector ? OSPhysicalToCached((u32)request->args.ioctlv.vector) : NULL;
            DCInvalidateRange(ioctlv->vector, (request->args.ioctlv.readCount + request->args.ioctlv.writeCount) * sizeof(IOSIoVector));

            for (i = 0; i < (request->args.ioctlv.readCount + request->args.ioctlv.writeCount); i++) {
                ioctlv->vector[i].base = ioctlv->vector[i].base ? OSPhysicalToCached((u32)ioctlv->vector[i].base) : NULL;
                DCInvalidateRange(ioctlv->vector[i].base, ioctlv->vector[i].length);
            }

            if (__relnchFl && __relnchRpcSave == rpc) {
                __relnchFl = FALSE;
                if (__mailboxAck < 1) {
                    __mailboxAck++;
                }
            }
            break;
        }
    }

    if (rpc->callback) {
        OSContext tmpContext;
        OSClearContext(&tmpContext);
        OSSetCurrentContext(&tmpContext);

        rpc->callback(request->status, rpc->callbackArg);

        OSClearContext(&tmpContext);
        OSSetCurrentContext(context);

        ipcFree(rpc);
    } else {
        OSWakeupThread(&rpc->threadQueue);
    }

    IPCWriteReg(HW_IPC_PPCCTRL, (IPCReadReg(HW_IPC_PPCCTRL) & 0x30) | 8);
    IPCiProfReply(request, (IOSFd)request->handle);
}

static void IpcAckHandler(__OSInterrupt interrupt, void* context) {
    IPCWriteReg(HW_IPC_PPCCTRL, (IPCReadReg(HW_IPC_PPCCTRL) & 0x30) | 2);
    ACRWriteReg(HW_PPCIRQFLAG << 2, 0x40000000);

    if (__mailboxAck < 1) {
        __mailboxAck++;
        IPCiProfAck();
    }

    if (__mailboxAck <= 0) {
        return;
    }

    if (__relnchFl) {
        __ios_rpc* rpc = __relnchRpc;
        rpc->request.status = IOS_ERROR_OK;
        __relnchFl = FALSE;
        OSWakeupThread(&__relnchRpc->threadQueue);
        IPCWriteReg(HW_IPC_PPCCTRL, (IPCReadReg(HW_IPC_PPCCTRL) & 0x30) | 8);
    }
    __ipcSendRequest();
}

static void IPCInterruptHandler(__OSInterrupt interrupt, OSContext* context) {
    if ((IPCReadReg(HW_IPC_PPCCTRL) & 0x14) == 0x14) {
        IpcReplyHandler(interrupt, context);
    }
    if ((IPCReadReg(HW_IPC_PPCCTRL) & 0x22) == 0x22) {
        IpcAckHandler(interrupt, context);
    }
}

IOSError IPCCltInit() {
    IOSError result = IOS_ERROR_OK;

    u32 heapSize;
    u8* heapStart;

    static BOOL initialized;

    if (initialized) {
        goto exit;
    }

    initialized = TRUE;
    IPCInit();

#if SDK_VERSION >= 20091211 || defined(SDK_IPL)
    heapSize = 0x2000;
#else
    heapSize = 0x1000;
#endif
    heapStart = IPCGetBufferLo();

    if (((u8*)heapStart + heapSize) > (u8*)IPCGetBufferHi()) {
        result = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    hid = iosCreateHeap(heapStart, heapSize);

    IPCSetBufferLo(((u8*)heapStart + heapSize));

    __OSSetInterruptHandler(__OS_INTERRUPT_PI_ACR, IPCInterruptHandler);
    __OSUnmaskInterrupts(OS_INTERRUPTMASK_PI_ACR);

    IPCWriteReg(HW_IPC_PPCCTRL, 0x38);

    IPCiProfInit();

#if SDK_VERSION >= 20091211
    OSCreateAlarm(&__timeout_alarm);
#endif

exit:
    return result;
}

IOSError IPCCltReInit() {
    IOSError result = IOS_ERROR_OK;

    u32 heapSize;
    u8* heapStart;
#if SDK_VERSION >= 20091211 || defined(SDK_IPL)
    heapSize = 0x1000;
#else
    heapSize = 0x800;
#endif
    heapStart = IPCGetBufferLo();

    if (((u8*)heapStart + heapSize) > (u8*)IPCGetBufferHi()) {
        result = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    hid = iosCreateHeap(heapStart, heapSize);

    IPCSetBufferLo(((u8*)heapStart + heapSize));

exit:
    return result;
}

static IOSError __ios_Ipc1(IOSFd fd, s32 type, IOSCallback callback, void* callbackArg, __ios_rpc** out) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    if (out == 0) {
        result = IOS_ERROR_INVALID;
        goto exit;
    }

    *out = ipcAllocReq();
    if (*out == 0) {
        result = IOS_ERROR_FAIL_ALLOC;
        goto exit;
    }

    rpc = *out;

    (*out)->callback = callback;
    (*out)->callbackArg = callbackArg;
    (*out)->reboot = FALSE;

    rpc->request.cmd = type;
    rpc->request.handle = (IOSResourceHandle)fd;

exit:
    return result;
}

static IOSError __ios_Ipc2(__ios_rpc* rpc, IOSCallback callback) {
    IOSResourceRequest* request;
    IOSError result = IOS_ERROR_OK;
    BOOL enabled;

    if (!rpc) {
        result = IOS_ERROR_INVALID;
        goto exit;
    }

    request = &rpc->request;
    if (!callback) {
        OSInitThreadQueue(&rpc->threadQueue);
    }

    DCFlushRange(request, sizeof(*request));

    enabled = OSDisableInterrupts();

    result = __ipcQueueRequest(request);

    if (result != IOS_ERROR_OK) {
        OSRestoreInterrupts(enabled);
        if (callback) {
            ipcFree(rpc);
        }
        goto exit;
    }

    if (__mailboxAck > 0) {
        __ipcSendRequest();
    }

    if (!callback) {
        OSSleepThread(&rpc->threadQueue);
    }

    OSRestoreInterrupts(enabled);

    if (!callback) {
        result = request->status;
    }

exit:
    if (rpc && !callback) {
        ipcFree(rpc);
    }

    return result;
}

/************/
/* TIME OUT */
/************/

#if SDK_VERSION >= 20091211
static __ios_rpc* __timeout_rpc = NULL;
static BOOL __timeout_flag = FALSE;

static void __timeout_cb(OSAlarm* alarm, OSContext* context);

static IOSError __ios_Ipc2WithTimeout(__ios_rpc* rpc) {
    IOSResourceRequest* request;
    IOSError result = IOS_ERROR_OK;
    BOOL enabled;

    if (!rpc) {
        result = IOS_ERROR_INVALID;
        goto exit;
    }

    request = &rpc->request;
    OSInitThreadQueue(&rpc->threadQueue);

    DCFlushRange(request, sizeof(*request));

    enabled = OSDisableInterrupts();

    result = __ipcQueueRequest(request);

    if (result != IOS_ERROR_OK) {
        OSRestoreInterrupts(enabled);
        ipcFree(rpc);
        goto exit;
    }

    if (__mailboxAck > 0) {
        __ipcSendRequest();
    }

    __timeout_flag = FALSE;
    __timeout_rpc = rpc;

    OSSetAlarm(&__timeout_alarm, OSMillisecondsToTicks(2000), __timeout_cb);
    OSSleepThread(&rpc->threadQueue);

    OSRestoreInterrupts(enabled);
    if (!__timeout_flag) {
        OSCancelAlarm(&__timeout_alarm);
        result = request->status;
        ipcFree(rpc);
    } else {
        result = IOS_ERROR_INTR;
    }

exit:

    return result;
}

static void __timeout_cb(OSAlarm* alarm, OSContext* context) {
    __timeout_flag = TRUE;
    OSWakeupThread(&__timeout_rpc->threadQueue);
}
#endif

/********/
/* OPEN */
/********/

static IOSError __ios_Open(__ios_rpc* rpc, char* path, u32 flags) {
    IOSResourceRequest* request;
    IOSError result = IOS_ERROR_OK;

    if (!rpc) {
        result = IOS_ERROR_INVALID;
        goto exit;
    }

    request = &rpc->request;
    DCFlushRange(path, strnlen(path, IOS_MAX_PATH) + 1);
    request->args.open.path = (const u8*)OSCachedToPhysical(path);
    request->args.open.flags = flags;

exit:
    return result;
}

IOSFd IOS_OpenAsync(char* path, u32 flags, IOSCallback callback, void* callbackArg) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(0, IOS_OPEN, callback, callbackArg, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Open(rpc, path, flags);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, callback);

exit:
    return result;
}

IOSFd IOS_Open(char* path, u32 flags) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(0, IOS_OPEN, NULL, NULL, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Open(rpc, path, flags);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, NULL);

exit:
    return result;
}

#if SDK_VERSION >= 20091211
IOSFd IOS_OpenWithTimeout(char* path, u32 flags) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(0, IOS_OPEN, NULL, NULL, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Open(rpc, path, flags);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2WithTimeout(rpc);

exit:
    return result;
}
#endif

/*********/
/* CLOSE */
/*********/

IOSError IOS_CloseAsync(IOSFd fd, IOSCallback callback, void* callbackArg) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_CLOSE, callback, callbackArg, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, callback);

exit:
    return result;
}

IOSFd IOS_Close(IOSFd fd) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_CLOSE, NULL, NULL, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, NULL);

exit:
    return result;
}

#if SDK_VERSION >= 20091211
IOSFd IOS_CloseWithTimeout(IOSFd fd) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_CLOSE, NULL, NULL, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2WithTimeout(rpc);

exit:
    return result;
}
#endif

/********/
/* READ */
/********/

static IOSError __ios_Read(__ios_rpc* rpc, u8* outPtr, u32 outLen) {
    IOSResourceRequest* request;
    s32 result = IOS_ERROR_OK;

    if (rpc == 0) {
        result = IOS_ERROR_INVALID;
        goto exit;
    }

    request = &rpc->request;
    DCInvalidateRange(outPtr, outLen);
    request->args.read.outPtr = outPtr ? (u8*)OSCachedToPhysical(outPtr) : NULL;
    request->args.read.outLen = outLen;

exit:
    return result;
}

IOSError IOS_ReadAsync(IOSFd fd, u8* outPtr, u32 outLen, IOSCallback callback, void* callbackArg) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_READ, callback, callbackArg, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Read(rpc, outPtr, outLen);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, callback);

exit:
    return result;
}

IOSError IOS_Read(IOSFd fd, u8* outPtr, u32 outLen) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_READ, NULL, NULL, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Read(rpc, outPtr, outLen);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, NULL);

exit:
    return result;
}

/*********/
/* WRITE */
/*********/

static IOSError __ios_Write(__ios_rpc* rpc, u8* inPtr, u32 inLen) {
    IOSResourceRequest* request;
    s32 result = IOS_ERROR_OK;

    if (rpc == 0) {
        result = IOS_ERROR_INVALID;
        goto exit;
    }

    request = &rpc->request;
    request->args.write.inPtr = inPtr ? (u8*)OSCachedToPhysical(inPtr) : NULL;
    request->args.write.inLen = inLen;
    DCFlushRange(inPtr, inLen);

exit:
    return result;
}

IOSError IOS_WriteAsync(IOSFd fd, u8* inPtr, u32 inLen, IOSCallback callback, void* callbackArg) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_WRITE, callback, callbackArg, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Write(rpc, inPtr, inLen);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, callback);

exit:
    return result;
}

IOSError IOS_Write(IOSFd fd, u8* inPtr, u32 inLen) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_WRITE, NULL, NULL, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Write(rpc, inPtr, inLen);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, NULL);

exit:
    return result;
}

/********/
/* SEEK */
/********/

static IOSError __ios_Seek(__ios_rpc* rpc, s32 offset, u32 whence) {
    IOSResourceRequest* request;
    s32 result = IOS_ERROR_OK;

    if (rpc == 0) {
        result = IOS_ERROR_INVALID;
        goto exit;
    }

    request = &rpc->request;

    request->args.seek.offset = offset;
    request->args.seek.whence = whence;

exit:
    return result;
}

IOSError IOS_SeekAsync(IOSFd fd, s32 offset, u32 whence, IOSCallback callback, void* callbackArg) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_SEEK, callback, callbackArg, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Seek(rpc, offset, whence);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, callback);

exit:
    return result;
}

IOSError IOS_Seek(IOSFd fd, s32 offset, u32 whence) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_SEEK, NULL, NULL, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Seek(rpc, offset, whence);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, NULL);

exit:
    return result;
}

static IOSError __ios_Ioctl(__ios_rpc* rpc, u32 cmd, u8* inPtr, u32 inLen, u8* outPtr, u32 outLen) {
    IOSResourceRequest* request;
    s32 result = IOS_ERROR_OK;

    if (rpc == 0) {
        result = IOS_ERROR_INVALID;
        goto exit;
    }

    request = &rpc->request;

    request->args.ioctl.cmd = cmd;
    request->args.ioctl.outPtr = outPtr ? (u8*)OSCachedToPhysical(outPtr) : NULL;
    request->args.ioctl.outLen = outLen;
    request->args.ioctl.inPtr = inPtr ? (u8*)OSCachedToPhysical(inPtr) : NULL;
    request->args.ioctl.inLen = inLen;

    DCFlushRange(inPtr, inLen);
    DCFlushRange(outPtr, outLen);

exit:
    return result;
}

IOSError IOS_IoctlAsync(IOSFd fd, u32 cmd, u8* inPtr, u32 inLen, u8* outPtr, u32 outLen, IOSCallback callback, void* callbackArg) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_IOCTL, callback, callbackArg, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ioctl(rpc, cmd, inPtr, inLen, outPtr, outLen);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, callback);

exit:
    return result;
}

IOSError IOS_Ioctl(IOSFd fd, u32 cmd, u8* inPtr, u32 inLen, u8* outPtr, u32 outLen) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_IOCTL, NULL, NULL, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ioctl(rpc, cmd, inPtr, inLen, outPtr, outLen);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, NULL);

exit:
    return result;
}

static IOSError __ios_Ioctlv(__ios_rpc* rpc, u32 cmd, u32 readCount, u32 writeCount, IOSIoVector* vector) {
    IOSResourceRequest* request;
    s32 result = IOS_ERROR_OK;
    IOSResourceIoctlv* iosIoctlv;
    int i;
    s32 readCount_;  // why

    if (rpc == 0) {
        result = IOS_ERROR_INVALID;
        goto exit;
    }

    request = &rpc->request;
    request->args.ioctlv.cmd = cmd;
    request->args.ioctlv.readCount = readCount;
    request->args.ioctlv.writeCount = writeCount;
    request->args.ioctlv.vector = vector;

    iosIoctlv = &request->args.ioctlv;

    for (i = 0, readCount_ = iosIoctlv->readCount; i < request->args.ioctlv.writeCount; i++) {
        DCFlushRange(iosIoctlv->vector[readCount_ + i].base, iosIoctlv->vector[readCount_ + i].length);

        iosIoctlv->vector[readCount_ + i].base =
            iosIoctlv->vector[readCount_ + i].base ? (u8*)OSCachedToPhysical(iosIoctlv->vector[readCount_ + i].base) : NULL;
    }

    for (i = 0; i < request->args.ioctlv.readCount; i++) {
        DCFlushRange(iosIoctlv->vector[i].base, iosIoctlv->vector[i].length);

        iosIoctlv->vector[i].base = iosIoctlv->vector[i].base ? (u8*)OSCachedToPhysical(iosIoctlv->vector[i].base) : NULL;
    }

    DCFlushRange(iosIoctlv->vector, (iosIoctlv->readCount + iosIoctlv->writeCount) * sizeof(IOSIoVector));

    request->args.ioctlv.vector = vector ? (IOSIoVector*)OSCachedToPhysical(vector) : NULL;

exit:
    return result;
}

IOSError IOS_IoctlvAsync(IOSFd fd, u32 cmd, u32 readCount, u32 writeCount, IOSIoVector* vector, IOSCallback callback, void* callbackArg) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_IOCTLV, callback, callbackArg, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ioctlv(rpc, cmd, readCount, writeCount, vector);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, callback);

exit:
    return result;
}

IOSError IOS_Ioctlv(IOSFd fd, u32 cmd, u32 readCount, u32 writeCount, IOSIoVector* vector) {
    __ios_rpc* rpc;
    IOSError result = IOS_ERROR_OK;

    result = __ios_Ipc1(fd, IOS_IOCTLV, NULL, NULL, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ioctlv(rpc, cmd, readCount, writeCount, vector);
    if (result != IOS_ERROR_OK) {
        goto exit;
    }

    result = __ios_Ipc2(rpc, NULL);

exit:
    return result;
}

IOSError IOS_IoctlvReboot(IOSFd fd, u32 cmd, u32 readCount, u32 writeCount, IOSIoVector* vector) {
    IOSError result = IOS_ERROR_OK;
    BOOL enabled = OSDisableInterrupts();
    __ios_rpc* rpc;
    IOSResourceRequest* request;

    if (__relnchFl) {
        OSRestoreInterrupts(enabled);
        result = IOS_ERROR_NOTREADY;
        goto exit;
    }

    __relnchFl = TRUE;

    OSRestoreInterrupts(enabled);

    result = __ios_Ipc1(fd, IOS_IOCTLV, NULL, NULL, &rpc);
    if (result != IOS_ERROR_OK) {
        goto exit2;
    }

    __relnchRpcSave = &*rpc;
    rpc->reboot = TRUE;

    result = __ios_Ioctlv(rpc, cmd, readCount, writeCount, vector);
    if (result != IOS_ERROR_OK) {
        goto exit2;
    }

    memcpy(&__rpcBuf, rpc, sizeof(__ios_rpc));
    __relnchRpc = &__rpcBuf;

    request = &rpc->request;

    OSInitThreadQueue(&__relnchRpc->threadQueue);
    DCFlushRange(request, sizeof(IOSResourceRequest));

    enabled = OSDisableInterrupts();

    result = __ipcQueueRequest(request);
    if (result != IOS_ERROR_OK) {
        OSRestoreInterrupts(enabled);
    } else {
        if (__mailboxAck > 0) {
            __ipcSendRequest();
        }
        OSSleepThread(&__relnchRpc->threadQueue);
        OSRestoreInterrupts(enabled);
        result = __relnchRpc->request.status;
    }

exit2:
    __relnchFl = FALSE;
    __relnchRpcSave = NULL;

    if (rpc && result != IOS_ERROR_OK) {
        ipcFree(rpc);
    }

exit:
    return result;
}
