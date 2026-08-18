#ifndef REVOLUTION_HIO2_H
#define REVOLUTION_HIO2_H

#include <revolution/types.h>

enum {
    HIO2_INVALID_HANDLE_VALUE = -1,
    HIO2_HANDLE_0,
    HIO2_HANDLE_1,
    HIO2_MAX_HANDLE_VALUE
};
typedef s32 HIO2Handle;

typedef enum HIO2DeviceType {
    HIO2_DEVICE_INVALID = -1,

    HIO2_DEVICE_EXI2USB_0,
    HIO2_DEVICE_EXI2USB_1,
    HIO2_DEVICE_MrEXI
} HIO2DeviceType;

enum {
    HIO2_ERR_OK = 0,
    HIO2_ERR_NOT_INITIALIZED,
    HIO2_ERR_INVALID_CHAN_HANDLE,
    HIO2_ERR_INVALID_EXI_CHAN,
    HIO2_ERR_ALREADY_OPEN,
    HIO2_ERR_CANNOT_ENUM_DEVICES,
    HIO2_ERR_NOT_AVAILABLE,
    HIO2_ERR_MISSING,
    HIO2_ERR_EXI,
};
typedef s32 HIO2Error;

typedef BOOL (*HIOHandleCallback)(HIO2Handle chan);
typedef BOOL (*HIODeviceTypeCallback)(HIO2DeviceType chan);

BOOL HIO2Init();

BOOL HIO2EnumDevices(HIODeviceTypeCallback callback);

HIO2Handle HIO2Open(HIO2DeviceType type, HIOHandleCallback receiveCallback, HIOHandleCallback disconnectCallback);

s32 HIO2GetDeviceType(HIO2Handle chan);

BOOL HIO2Close(HIO2Handle chan);

BOOL HIO2ReadMailbox(HIO2Handle chan, u32* mail);
BOOL HIO2WriteMailbox(HIO2Handle chan, u32 mail);

BOOL HIO2Read(HIO2Handle chan, u32 addr, void* buf, s32 size);
BOOL HIO2Write(HIO2Handle chan, u32 addr, void* buf, s32 size);

BOOL HIO2ReadAsync(HIO2Handle chan, u32 addr, void* buf, s32 size, HIOHandleCallback callback);
BOOL HIO2WriteAsync(HIO2Handle chan, u32 addr, void* buf, s32 size, HIOHandleCallback callback);

BOOL HIO2ReadStatus(HIO2Handle chan, s32* status);

void HIO2Exit();

HIO2Error HIO2GetLastError();

HIOHandleCallback HIO2GetReceiveCallback(HIO2Handle chan);
BOOL HIO2SetReceiveCallback(HIO2Handle chan, HIOHandleCallback receiveCallback);

#endif  // REVOLUTION_HIO2_H
