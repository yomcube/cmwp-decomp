#include <revolution/exi.h>

#include <revolution/os.h>

#include <private/os.h>

const u32 __EXIFreq = EXI_FREQ_16M;

s32 EXIGetConsoleType() {
    return 1;
}

void EXIWait() {
    while (__OSDeviceCode == 0) {
    }
}

static u32 __EXISwap32(u32 val);

s32 EXIReadReg(s32 chan, u32 dev, u32 cmd, u8* buf, s32 len) {
    u32 val;
    BOOL err = FALSE;

    err |= !EXILock(chan, dev, NULL);
    if (err) {
        return FALSE;
    }

    err |= !EXISelect(chan, dev, EXI_FREQ_16M);
    if (err) {
        EXIUnlock(chan);
        return FALSE;
    }

    err |= !EXIImm(chan, &cmd, sizeof(cmd), EXI_WRITE, NULL);
    err |= !EXISync(chan);

    err |= !EXIImm(chan, &val, sizeof(val), EXI_READ, NULL);
    err |= !EXISync(chan);

    err |= !EXIDeselect(chan);
    err |= !EXIUnlock(chan);

    switch (len) {
        case sizeof(u8): {
            *(u8*)buf = val >> 24;
            break;
        }
        case sizeof(u16): {
            *(u16*)buf = val >> 24 | (val >> 8) & 0xFF00;
            break;
        }
        // case sizeof(u32):
        default: {
            *(u32*)buf = __EXISwap32(val);
            break;
        }
    }

    return !err;
}

u32 __EXISwap32(u32 val) {
    return (val >> 24) | ((val >> 8) & 0x0000FF00) | ((val << 8) & 0x00FF0000) | (val << 24);
}

s32 EXIWriteReg(s32 chan, u32 dev, u32 cmd, u8* buf, s32 len) {
    u32 val;
    BOOL err = FALSE;

    switch (len) {
        case sizeof(u8): {
            val = (*(u8*)buf & 0xFF) << 24;
            break;
        }
        case sizeof(u16): {
            val = (*(u16*)buf & 0x00FF) << 24 | (*(u16*)buf & 0xFF00) << 8;
            break;
        }
        // case sizeof(u32):
        default: {
            val = __EXISwap32(*(u32*)buf);
            break;
        }
    }

    err |= !EXILock(chan, dev, NULL);
    if (err) {
        return FALSE;
    }

    err |= !EXISelect(chan, dev, EXI_FREQ_16M);
    if (err) {
        EXIUnlock(chan);
        return FALSE;
    }

    err |= !EXIImm(chan, &cmd, sizeof(cmd), EXI_WRITE, NULL);
    err |= !EXISync(chan);

    err |= !EXIImm(chan, &val, sizeof(val), EXI_WRITE, NULL);
    err |= !EXISync(chan);

    err |= !EXIDeselect(chan);
    err |= !EXIUnlock(chan);

    return !err;
}

s32 EXIReadRam(s32 chan, s32 dev, u32 cmd, void* buf, s32 len, EXICallback callback) {
    BOOL err = FALSE;

    err |= !EXILock(chan, dev, NULL);
    if (err) {
        return FALSE;
    }

    err |= !EXISelect(chan, dev, EXI_FREQ_16M);
    if (err) {
        EXIUnlock(chan);
        return FALSE;
    }

    err |= !EXIImm(chan, &cmd, sizeof(cmd), EXI_WRITE, NULL);
    err |= !EXISync(chan);
    err |= !EXIDma(chan, buf, len, EXI_READ, callback);

    if (callback == NULL) {
        err |= !EXISync(chan);
        err |= !EXIDeselect(chan);
        err |= !EXIUnlock(chan);
    }

    return !err;
}

s32 EXIWriteRam(s32 chan, s32 dev, u32 cmd, void* buf, s32 len, EXICallback callback) {
    BOOL err = FALSE;

    err |= !EXILock(chan, dev, NULL);
    if (err) {
        return FALSE;
    }

    err |= !EXISelect(chan, dev, EXI_FREQ_16M);
    if (err) {
        EXIUnlock(chan);
        return FALSE;
    }

    err |= !EXIImm(chan, &cmd, sizeof(cmd), EXI_WRITE, NULL);
    err |= !EXISync(chan);
    err |= !EXIDma(chan, buf, len, EXI_WRITE, callback);

    if (callback == NULL) {
        err |= !EXISync(chan);
        err |= !EXIDeselect(chan);
        err |= !EXIUnlock(chan);
    }

    return !err;
}

s32 EXIReadRamImm(s32 chan, s32 dev, u32 cmd, void* buf, s32 len) {
    BOOL err = FALSE;

    err |= !EXILock(chan, dev, NULL);
    if (err) {
        return FALSE;
    }

    err |= !EXISelect(chan, dev, EXI_FREQ_16M);
    if (err) {
        EXIUnlock(chan);
        return FALSE;
    }

    err |= !EXIImm(chan, &cmd, sizeof(cmd), EXI_WRITE, NULL);
    err |= !EXISync(chan);
    err |= !EXIImmEx(chan, buf, len, EXI_READ);

    err |= !EXISync(chan);
    err |= !EXIDeselect(chan);
    err |= !EXIUnlock(chan);

    return !err;
}

s32 EXIWriteRamImm(s32 chan, s32 dev, u32 cmd, void* buf, s32 len) {
    BOOL err = FALSE;

    err |= !EXILock(chan, dev, NULL);
    if (err) {
        return FALSE;
    }

    err |= !EXISelect(chan, dev, EXI_FREQ_16M);
    if (err) {
        EXIUnlock(chan);
        return FALSE;
    }

    err |= !EXIImm(chan, &cmd, sizeof(cmd), EXI_WRITE, NULL);
    err |= !EXISync(chan);
    err |= !EXIImmEx(chan, buf, len, EXI_WRITE);

    err |= !EXISync(chan);
    err |= !EXIDeselect(chan);
    err |= !EXIUnlock(chan);

    return !err;
}
