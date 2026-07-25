#ifndef PRIVATE_IPC_H
#define PRIVATE_IPC_H

#include <revolution/os/OSTime.h>
#include <revolution/types.h>

#include <private/ios/iostypes.h>

#if SDK_VERSION >= 20091211
#define IPC_MAX_REQUEST 96
#else
#define IPC_MAX_REQUEST 32
#endif

/* Main */

IOSError IPCCltInit();
IOSError IPCCltReInit();

void IPCInit();
void IPCReInit();

u32 IPCReadReg(u32 addr);
void IPCWriteReg(u32 addr, u32 val);

void* IPCGetBufferHi();
void* IPCGetBufferLo();

void IPCSetBufferHi(void* newHi);
void IPCSetBufferLo(void* newLo);

/* Profile */

int IPCGetNumPendingReqs();
int IPCGetNumUnIssuedReqs();

IOSError IPCGetQueueStatus(IOSResourceHandle queueHandle);

#if SDK_VERSION >= 20091211
IOSResourceRequest* IPCGetQueuedRequests(IOSResourceHandle queueHandle, IOSResourceRequest* request, IOSTime* time);
char* IPCGetPathByHandle(IOSResourceHandle queueHandle);
#endif

void IPCiProfInit();

void IPCiProfQueueReq(IOSResourceRequest* request, IOSFd fd);
void IPCiProfAck();

void IPCiProfReply(IOSResourceRequest* request, IOSFd fd);

#endif  // PRIVATE_IPC_H
