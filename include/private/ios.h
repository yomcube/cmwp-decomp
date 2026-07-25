#ifndef PRIVATE_IOS_H
#define PRIVATE_IOS_H

#include <private/ios/iostypes.h>

#define IOS_MAX_PATH 64

/* Heap */

IOSHeapId iosCreateHeap(void* start, u32 size);
IOSError iosDestroyHeap(IOSHeapId id);

void* iosAlloc(IOSHeapId id, u32 size);
void* iosAllocAligned(IOSHeapId id, u32 size, u32 align);

IOSError iosFree(IOSHeapId id, void* block);

/* Main */

typedef IOSError (*IOSCallback)(IOSError result, void* arg);

IOSFd IOS_OpenAsync(char* path, u32 flags, IOSCallback callback, void* callbackArg);
IOSFd IOS_Open(char* path, u32 flags);
#if SDK_VERSION >= 20091211
IOSFd IOS_OpenWithTimeout(char* path, u32 flags);
#endif

IOSError IOS_CloseAsync(IOSFd fd, IOSCallback callback, void* callbackArg);
IOSFd IOS_Close(IOSFd fd);
#if SDK_VERSION >= 20091211
IOSFd IOS_CloseWithTimeout(IOSFd fd);
#endif

IOSError IOS_ReadAsync(IOSFd fd, u8* outPtr, u32 outLen, IOSCallback callback, void* callbackArg);
IOSError IOS_Read(IOSFd fd, u8* outPtr, u32 outLen);

IOSError IOS_WriteAsync(IOSFd fd, u8* inPtr, u32 inLen, IOSCallback callback, void* callbackArg);
IOSError IOS_Write(IOSFd fd, u8* inPtr, u32 inLen);

IOSError IOS_SeekAsync(IOSFd fd, s32 offset, u32 whence, IOSCallback callback, void* callbackArg);
IOSError IOS_Seek(IOSFd fd, s32 offset, u32 whence);

IOSError IOS_IoctlAsync(IOSFd fd, u32 cmd, u8* inPtr, u32 inLen, u8* outPtr, u32 outLen, IOSCallback callback, void* callbackArg);
IOSError IOS_Ioctl(IOSFd fd, u32 cmd, u8* inPtr, u32 inLen, u8* outPtr, u32 outLen);

IOSError IOS_IoctlvAsync(IOSFd fd, u32 cmd, u32 readCount, u32 writeCount, IOSIoVector* vector, IOSCallback callback, void* callbackArg);
IOSError IOS_Ioctlv(IOSFd fd, u32 cmd, u32 readCount, u32 writeCount, IOSIoVector* vector);
IOSError IOS_IoctlvReboot(IOSFd fd, u32 cmd, u32 readCount, u32 writeCount, IOSIoVector* vector);

#endif  // PRIVATE_IOS_H
