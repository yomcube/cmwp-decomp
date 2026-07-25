#include <private/ipc.h>

#include <revolution/os.h>

#include <string.h>

#define MAX_PATH_LENGTH 48

#if SDK_VERSION >= 20091211
static char IpcOpenPathBuf[IPC_MAX_REQUEST][MAX_PATH_LENGTH];
static char IpcHandlePathBuf[IPC_MAX_REQUEST + 32][MAX_PATH_LENGTH];

static OSTime IpcStartTimeArray[IPC_MAX_REQUEST];

static IOSResourceRequest IpcReqArray[IPC_MAX_REQUEST];
#endif

static IOSResourceRequest* IpcReqPtrArray[IPC_MAX_REQUEST];

#if SDK_VERSION < 20091211
static IOSFd IpcFdArray[IPC_MAX_REQUEST];
#endif

static int IpcNumUnIssuedReqs;
static int IpcNumPendingReqs;

#if SDK_VERSION >= 20091211
static void AddReqInfo(IOSResourceRequest* request);
static void DelReqInfo(IOSResourceRequest* request);
#else
static void AddReqInfo(IOSResourceRequest* request, IOSFd fd);
static void DelReqInfo(IOSResourceRequest* request, IOSFd fd);
#endif

int IPCGetNumPendingReqs() {
    return IpcNumPendingReqs;
}

int IPCGetNumUnIssuedReqs() {
    return IpcNumUnIssuedReqs;
}

IOSError IPCGetQueueStatus(IOSResourceHandle queueHandle) {
    IOSError status;

    if (queueHandle > IPC_MAX_REQUEST) {
        return IOS_ERROR_ACCESS;
    }
#if SDK_VERSION >= 20091211
    if ((IOSResourceRequest*)IpcReqPtrArray[queueHandle]) {
        status = IpcReqArray[queueHandle].handle;
    } else {
        status = IOS_ERROR_ACCESS;
    }
#else
    status = IpcFdArray[queueHandle];
#endif
    return status;
}

#if SDK_VERSION >= 20091211
IOSResourceRequest* IPCGetQueuedRequests(IOSResourceHandle queueHandle, IOSResourceRequest* request, IOSTime* time) {
    BOOL enabled = OSDisableInterrupts();

    IOSResourceRequest* reqPtr = IpcReqPtrArray[queueHandle];
    *request = IpcReqArray[queueHandle];

    OSRestoreInterrupts(enabled);

    if (reqPtr) {
        *time = OSTicksToMilliseconds(OSGetTime() - IpcStartTimeArray[queueHandle]);
    } else {
        *time = 0;
    }

    return reqPtr;
}

char* IPCGetPathByHandle(IOSResourceHandle queueHandle) {
    return IpcHandlePathBuf[queueHandle];
}
#endif

void IPCiProfInit() {
    IOSResourceHandle i;

    IpcNumPendingReqs = 0;
    IpcNumUnIssuedReqs = 0;

    for (i = 0; i < IPC_MAX_REQUEST; i++) {
        IpcReqPtrArray[i] = NULL;
#if SDK_VERSION < 20091211
        IpcFdArray[i] = -1;
#else
        IpcStartTimeArray[i] = 0;
#endif
    }

#if SDK_VERSION >= 20091211
    memset(IpcHandlePathBuf, 0, sizeof(IpcHandlePathBuf));
    memset(IpcOpenPathBuf, 0, sizeof(IpcOpenPathBuf));
    memset(IpcReqArray, 0, sizeof(IpcReqArray));
#endif
}

void IPCiProfQueueReq(IOSResourceRequest* request, IOSFd fd) {
    IpcNumPendingReqs++;
    IpcNumUnIssuedReqs++;

#if SDK_VERSION >= 20091211
    AddReqInfo(request);
#else
    AddReqInfo(request, fd);
#endif
}

void IPCiProfAck() {
    IpcNumUnIssuedReqs--;
}

void IPCiProfReply(IOSResourceRequest* request, IOSFd fd) {
    IpcNumPendingReqs--;
#if SDK_VERSION >= 20091211
    DelReqInfo(request);
#else
    DelReqInfo(request, fd);
#endif
}

#if SDK_VERSION >= 20091211
static void AddReqInfo(IOSResourceRequest* request)
#else
static void AddReqInfo(IOSResourceRequest* request, IOSFd fd)
#endif
{
    IOSResourceHandle i;

    for (i = 0; i < IPC_MAX_REQUEST; i++) {
#if SDK_VERSION >= 20091211
        if (!IpcReqPtrArray[i]) {
            BOOL enabled = OSDisableInterrupts();

            IpcReqPtrArray[i] = request;
            IpcReqArray[i] = *request;

            IpcStartTimeArray[i] = OSGetTime();

            if (IpcReqArray[i].cmd == IOS_OPEN) {
                char* p = (char*)OSPhysicalToCached((u32)IpcReqArray[i].args.open.path);

                strncpy(IpcOpenPathBuf[i], p, MAX_PATH_LENGTH - 1);
                IpcOpenPathBuf[i][MAX_PATH_LENGTH - 1] = 0;

                IpcReqArray[i].args.open.path = (const u8*)IpcOpenPathBuf[i];
            }

            OSRestoreInterrupts(enabled);
            break;
        }
#else
        if (IpcReqPtrArray[i] == NULL && IpcFdArray[i] == -1) {
            IpcReqPtrArray[i] = request;
            IpcFdArray[i] = fd;
            break;
        }
#endif
    }
}

#if SDK_VERSION >= 20091211
static void DelReqInfo(IOSResourceRequest* request)
#else
static void DelReqInfo(IOSResourceRequest* request, IOSFd fd)
#endif
{
    IOSResourceHandle i;

    for (i = 0; i < IPC_MAX_REQUEST; i++) {
#if SDK_VERSION >= 20091211
        if (request == IpcReqPtrArray[i] && request->handle == IpcReqArray[i].cmd) {
            BOOL enabled = OSDisableInterrupts();

            if (IpcReqArray[i].cmd == IOS_OPEN && request->status >= IOS_ERROR_OK) {
                strncpy(IpcHandlePathBuf[request->status], (char*)IpcReqArray[i].args.open.path, MAX_PATH_LENGTH - 1);
                IpcHandlePathBuf[request->status][MAX_PATH_LENGTH - 1] = 0;

                memset(&IpcOpenPathBuf[i], 0, MAX_PATH_LENGTH);
            }

            if (IpcReqArray[i].cmd == IOS_CLOSE) {
                memset(&IpcHandlePathBuf[IpcReqArray[i].handle], 0, MAX_PATH_LENGTH);
            }

            IpcReqPtrArray[i] = NULL;
            memset(&IpcReqArray[i], 0, sizeof(IOSResourceRequest));
            IpcStartTimeArray[i] = 0;

            OSRestoreInterrupts(enabled);

            break;
        }
#else
        if (request == IpcReqPtrArray[i] && IpcFdArray[i] == fd) {
            IpcReqPtrArray[i] = NULL;
            IpcFdArray[i] = -1;
            return;
        }
#endif
    }
}
