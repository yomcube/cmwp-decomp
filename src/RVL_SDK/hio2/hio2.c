#include <revolution/verdefs.h>

#ifdef DEBUG
static const char* __HIO2Version = "<< RVL_SDK - HIO2 	debug build: Jul 30 2008 19:02:00 (0x4199_60831) >>";
#else
static const char* __HIO2Version = "<< RVL_SDK - HIO2 	release build: Jul 30 2008 19:24:26 (0x4199_60831) >>";
#endif

#include <revolution/hio2.h>

#include <revolution/exi.h>
#include <revolution/os.h>

/* HIO2Handle is pretty much the same as HIO2DeviceType, which is also the same as EXI channels. */

HIO2Error __HIO2LastErrorCode = HIO2_ERR_OK;
BOOL __HIO2Initialized = FALSE;

s32 __HIO2ConsoleType = 0;

typedef struct HIO2ChanInfo {
    HIO2DeviceType type;                   // 0x00
    HIO2Handle handle;                     // 0x04
    s32 dev;                               // 0x08
    HIOHandleCallback receiveCallback;     // 0x0C
    HIOHandleCallback writeCallback;       // 0x10
    HIOHandleCallback readCallback;        // 0x14
    HIOHandleCallback disconnectCallback;  // 0x18
} HIO2ChanInfo;

#define IS_BAD_HANDLE(handle) (!__HIO2IsInitialized() || !__HIO2IsValidHandle(handle))

HIO2ChanInfo __HIO2Control[HIO2_MAX_HANDLE_VALUE] = {
    {HIO2_DEVICE_INVALID, HIO2_INVALID_HANDLE_VALUE, 0, NULL, NULL, NULL, NULL},
    {HIO2_DEVICE_INVALID, HIO2_INVALID_HANDLE_VALUE, 0, NULL, NULL, NULL, NULL},
};

static BOOL __HIO2IsInitialized();
static BOOL __HIO2IsValidHandle(HIO2Handle handle);

static void __HIO2ClearChanInfo(HIO2Handle handle) {
    __HIO2Control[handle].type = HIO2_DEVICE_INVALID;
    __HIO2Control[handle].handle = HIO2_INVALID_HANDLE_VALUE;
    __HIO2Control[handle].dev = 0;
    __HIO2Control[handle].receiveCallback = __HIO2Control[handle].writeCallback = __HIO2Control[handle].readCallback =
        __HIO2Control[handle].disconnectCallback = NULL;
}

static void __HIO2ExtHandler(HIO2Handle handle, OSContext* context) {
    if (__HIO2Control[handle].disconnectCallback != NULL) {
        __HIO2Control[handle].disconnectCallback(handle);
    }
    __HIO2ClearChanInfo(handle);
    EXISetExiCallback(handle, NULL);
}

static void __HIO2ExiHandler(HIO2DeviceType handle, OSContext* context) {
    if (handle == HIO2_DEVICE_2) {
        handle = HIO2_DEVICE_0;
    }
    if (__HIO2Control[handle].receiveCallback != NULL) {
        __HIO2Control[handle].receiveCallback(handle);
    }
}

static void __HIO2TxHandler(HIO2Handle handle, OSContext* context) {
    EXIDeselect(handle);
    EXIUnlock(handle);
    if (__HIO2Control[handle].writeCallback != NULL) {
        __HIO2Control[handle].writeCallback(handle);
    }
}

static void __HIO2RxHandler(HIO2Handle handle, OSContext* context) {
    EXIDeselect(handle);
    EXIUnlock(handle);
    if (__HIO2Control[handle].readCallback != NULL) {
        __HIO2Control[handle].readCallback(handle);
    }
}

BOOL HIO2Init() {
    if (__HIO2Initialized) {
        HIO2Exit();
    }
    EXIWait();
    __HIO2ConsoleType = EXIGetConsoleType();
    if ((__HIO2ConsoleType & 7) == 0) {
        __HIO2LastErrorCode = HIO2_ERR_NOT_AVAILABLE;
        return FALSE;
    } else {
        __HIO2LastErrorCode = HIO2_ERR_OK;
        OSRegisterVersion(__HIO2Version);
        return __HIO2Initialized = TRUE;
    }
}

BOOL HIO2EnumDevices(HIODeviceTypeCallback callback) {
    u32 id;
    HIO2DeviceType type;

    ASSERTLINE(275, callback != NULL);

    if (!__HIO2IsInitialized()) {
        return FALSE;
    }
    if (__HIO2Control[HIO2_HANDLE_0].handle != HIO2_INVALID_HANDLE_VALUE && __HIO2Control[HIO2_HANDLE_1].handle != HIO2_INVALID_HANDLE_VALUE) {
        __HIO2LastErrorCode = HIO2_ERR_CANNOT_ENUM_DEVICES;
        return FALSE;
    }
    for (type = 0; type < HIO2_MAX_HANDLE_VALUE; type++) {
        while (!EXIProbeEx(type)) {
        }
        if (EXIGetID(type, 0, &id) && id == 0 && !callback(type)) {
            return TRUE;
        }
    }
    if ((__HIO2ConsoleType & 1) != 0) {
        callback(HIO2_DEVICE_2);
    }
    return TRUE;
}

static BOOL __HIO2IsInitialized() {
    if (!__HIO2Initialized) {
        __HIO2LastErrorCode = HIO2_ERR_NOT_INITIALIZED;
    }
    return __HIO2Initialized;
}

HIO2Handle HIO2Open(HIO2DeviceType type, HIOHandleCallback receiveCallback, HIOHandleCallback disconnectCallback) {
    HIO2Handle handle;
    u32 dev;

    if (!__HIO2IsInitialized()) {
        return HIO2_INVALID_HANDLE_VALUE;
    }
    switch (type) {
        case HIO2_DEVICE_0:
        case HIO2_DEVICE_1: {
            handle = (HIO2Handle)type;
            dev = 0;
            break;
        }
        case HIO2_DEVICE_2: {
            if ((__HIO2ConsoleType & 1) != 0) {
                handle = HIO2_HANDLE_0;
                dev = 1;
            } else {
                __HIO2LastErrorCode = HIO2_ERR_NOT_AVAILABLE;
                return HIO2_INVALID_HANDLE_VALUE;
            }
            break;
        }
        default: {
            __HIO2LastErrorCode = HIO2_ERR_INVALID_EXI_CHAN;
            return HIO2_INVALID_HANDLE_VALUE;
        }
    }
    if (__HIO2Control[handle].handle != HIO2_INVALID_HANDLE_VALUE) {
        __HIO2LastErrorCode = HIO2_ERR_ALREADY_OPEN;
        return HIO2_INVALID_HANDLE_VALUE;
    } else {
        u8 reg;
        BOOL enabled;
        if (dev == 0) {
            u32 id = 0;
            while (!EXIProbeEx(handle)) {
            }
            if (!EXIAttach(handle, __HIO2ExtHandler) || !EXIGetID(handle, 0, &id) || id != 0) {
                __HIO2LastErrorCode = HIO2_ERR_MISSING;
                return HIO2_INVALID_HANDLE_VALUE;
            }
        }
        reg = 0xD8;
        enabled = OSDisableInterrupts();
        if (!EXIWriteReg(handle, dev, 0xB4000000, (u32*)&reg, 1)) {
            if (dev == 0) {
                EXIDetach(handle);
            }
            __HIO2LastErrorCode = HIO2_ERR_EXI;
            OSRestoreInterrupts(enabled);
            return HIO2_INVALID_HANDLE_VALUE;
        }
        OSRestoreInterrupts(enabled);
        __HIO2Control[handle].type = type;
        __HIO2Control[handle].dev = dev;
        __HIO2Control[handle].handle = handle;
        __HIO2Control[handle].receiveCallback = receiveCallback;
        __HIO2Control[handle].writeCallback = NULL;
        __HIO2Control[handle].readCallback = NULL;
        __HIO2Control[handle].disconnectCallback = disconnectCallback;
        if (receiveCallback != NULL) {
            if (type == HIO2_DEVICE_2) {
                EXISetExiCallback(HIO2_DEVICE_2, __HIO2ExiHandler);
            } else {
                EXISetExiCallback(handle, __HIO2ExiHandler);
            }
        }
    }

    return handle;
}

s32 HIO2GetDeviceType(HIO2Handle handle) {
    if (IS_BAD_HANDLE(handle)) {
        return HIO2_DEVICE_INVALID;
    } else {
        return __HIO2Control[handle].dev == 0 ? __HIO2Control[handle].handle : HIO2_DEVICE_2;
    }
}

static BOOL __HIO2IsValidHandle(HIO2Handle handle) {
    if ((handle == HIO2_HANDLE_0 || handle == HIO2_HANDLE_1) && __HIO2Control[handle].handle != HIO2_INVALID_HANDLE_VALUE) {
        return TRUE;
    } else {
        __HIO2LastErrorCode = HIO2_ERR_INVALID_CHAN_HANDLE;
        return FALSE;
    }
}

BOOL HIO2Close(HIO2Handle handle) {
    if (IS_BAD_HANDLE(handle)) {
        return FALSE;
    } else {
        EXISetExiCallback(__HIO2Control[handle].handle, NULL);
        if (__HIO2Control[handle].dev == 0) {
            EXIDetach(__HIO2Control[handle].handle);
        }
        __HIO2ClearChanInfo(handle);
    }
    return TRUE;
}

BOOL HIO2ReadMailbox(HIO2Handle handle, u32* mail) {
    BOOL result;

    if (IS_BAD_HANDLE(handle)) {
        return FALSE;
    } else {
        ASSERTLINE(509, mail != NULL);
        result = EXIReadReg(handle, __HIO2Control[handle].dev, 0x34000200, mail, 4);
        if (!result) {
            __HIO2LastErrorCode = HIO2_ERR_EXI;
        }
    }
    return result;
}

BOOL HIO2WriteMailbox(HIO2Handle handle, u32 mail) {
    BOOL result;

    if (IS_BAD_HANDLE(handle)) {
        return FALSE;
    } else {
        result = EXIWriteReg(handle, __HIO2Control[handle].dev, 0xB4000100, &mail, 4);
        if (!result) {
            __HIO2LastErrorCode = HIO2_ERR_EXI;
        }
    }
    return result;
}

BOOL HIO2Read(HIO2Handle handle, u32 addr, void* buf, s32 size) {
    BOOL result;

    if (IS_BAD_HANDLE(handle)) {
        return FALSE;
    } else {
        ASSERTLINE(564, (addr & 3) == 0);
        result = EXIReadRam(handle, __HIO2Control[handle].dev, ((addr + 0xD10000) << 6) & 0x3FFFFF00, buf, size, NULL);
        if (!result) {
            __HIO2LastErrorCode = HIO2_ERR_EXI;
        }
    }
    return result;
}

BOOL HIO2Write(HIO2Handle handle, u32 addr, void* buf, s32 size) {
    BOOL result;

    if (IS_BAD_HANDLE(handle)) {
        return FALSE;
    } else {
        ASSERTLINE(593, (addr & 3) == 0);
        result = EXIWriteRam(handle, __HIO2Control[handle].dev, (((addr + 0xD10000) << 6) & 0x3FFFFF00) | 0x80000000, buf, size, NULL);
        if (!result) {
            __HIO2LastErrorCode = HIO2_ERR_EXI;
        }
    }
    return result;
}

BOOL HIO2ReadAsync(HIO2Handle handle, u32 addr, void* buf, s32 size, HIOHandleCallback callback) {
    BOOL result;

    if (IS_BAD_HANDLE(handle)) {
        return FALSE;
    } else {
        ASSERTLINE(624, (addr & 3) == 0);
        __HIO2Control[handle].readCallback = callback;
        result = EXIReadRam(handle, __HIO2Control[handle].dev, ((addr + 0xD10000) << 6) & 0x3FFFFF00, buf, size, __HIO2RxHandler);
        if (!result) {
            __HIO2LastErrorCode = HIO2_ERR_EXI;
        }
    }
    return result;
}

BOOL HIO2WriteAsync(HIO2Handle handle, u32 addr, void* buf, s32 size, HIOHandleCallback callback) {
    BOOL result;

    if (IS_BAD_HANDLE(handle)) {
        return FALSE;
    } else {
        ASSERTLINE(658, (addr & 3) == 0);
        __HIO2Control[handle].writeCallback = callback;
        result = EXIWriteRam(handle, __HIO2Control[handle].dev, (((addr + 0xD10000) << 6) & 0x3FFFFF00) | 0x80000000, buf, size, __HIO2TxHandler);
        if (!result) {
            __HIO2LastErrorCode = HIO2_ERR_EXI;
        }
    }
    return result;
}

BOOL HIO2ReadStatus(HIO2Handle handle, s32* status) {
    BOOL result;

    if (IS_BAD_HANDLE(handle)) {
        return FALSE;
    } else {
        u8 reg;
        ASSERTLINE(688, status != NULL);
        result = EXIReadReg(handle, __HIO2Control[handle].dev, 0x34000000, (u32*)&reg, 1);
        *status = !(reg & 8) ? 1 : 0;
        *status |= !(reg & 4) ? 0 : 2;
        *status &= 3;
        if (!result) {
            __HIO2LastErrorCode = HIO2_ERR_EXI;
        }
    }
    return result;
}

void HIO2Exit() {
    HIO2Handle handle;

    for (handle = 0; handle < HIO2_MAX_HANDLE_VALUE; handle++) {
        if (__HIO2Control[handle].handle != HIO2_INVALID_HANDLE_VALUE) {
            HIO2Close(handle);
        }
    }
    __HIO2Initialized = FALSE;
    __HIO2ConsoleType = 0;
}

HIO2Error HIO2GetLastError() {
    return __HIO2LastErrorCode;
}

HIOHandleCallback HIO2GetReceiveCallback(HIO2Handle handle) {
    return (IS_BAD_HANDLE(handle)) ? NULL : __HIO2Control[handle].receiveCallback;
}

BOOL HIO2SetReceiveCallback(HIO2Handle handle, HIOHandleCallback receiveCallback) {
    BOOL enabled;

    if (IS_BAD_HANDLE(handle)) {
        return FALSE;
    } else {
        enabled = OSDisableInterrupts();
        __HIO2Control[handle].receiveCallback = receiveCallback;
        OSRestoreInterrupts(enabled);
    }
    return TRUE;
}
