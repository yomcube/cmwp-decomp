#include <revolution/exi.h>

#include <private/os.h>
#include <revolution/os.h>

#include <string.h>

#include <revolution/verdefs.h>

#include <private/hollywood.h>

#pragma sym on

int __gUnknown800030C0[2] AT_ADDRESS(OS_BASE_CACHED | 0x30C0);

#define OFFSET(addr, align) (((u32)(addr) & ((align) - 1)))

typedef struct EXIControl {
    EXICallback exiCallback;
    EXICallback tcCallback;
    EXICallback extCallback;
    volatile u32 state;
    int immLen;
    u8* immBuf;
    u32 dev;
    u32 id;
    s32 idTime;
    int items;
    struct {
        u32 dev;
        EXICallback callback;
    } queue[3];
} EXIControl;

#define REG_MAX 5
#define REG(chan, idx) (__EXIRegs[((chan) * REG_MAX) + (idx)])

#define STATE_IDLE 0
#define STATE_DMA 1
#define STATE_IMM 2
#define STATE_BUSY 3
#define STATE_SELECTED 4
#define STATE_ATTACHED 8
#define STATE_LOCKED 16

#define MAX_CHAN 3

#define MAX_IMM 4
#define MAX_TYPE 3
#define MAX_DEV 3
#define MAX_FREQ 6

#define EXI_0LENGTH_EXILENGTH_MASK 0xFFFFFFE0

#if defined(SDK_20091112)
#ifdef DEBUG
SDKDefineVersion(EXI, "Dec 11 2009", "15:51:28");
#else
SDKDefineVersion(EXI, "Dec 11 2009", "15:55:59");
#endif
#elif defined(SDK_20090224)
SDKDefineVersion(EXI, "Jul 30 2008", "19:19:33");
#endif

static EXIControl Ecb[MAX_CHAN];
static u32 IDSerialPort1;

extern void __OSEnableBarnacle(s32 chan, u32 dev);

// prototypes
static BOOL __EXIProbe(s32 chan);

static void SetExiInterruptMask(s32 chan, EXIControl* exi) {
    EXIControl* exi2;
    exi2 = &Ecb[EXI_CHAN_2];

    switch (chan) {
        case EXI_CHAN_0: {
            if ((!exi->exiCallback && !exi2->exiCallback) || (exi->state & STATE_LOCKED)) {
                __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_0_EXI | OS_INTERRUPTMASK_EXI_2_EXI);
            } else {
                __OSUnmaskInterrupts(OS_INTERRUPTMASK_EXI_0_EXI | OS_INTERRUPTMASK_EXI_2_EXI);
            }
            break;
        }
        case EXI_CHAN_1: {
            if (!exi->exiCallback || (exi->state & STATE_LOCKED)) {
                __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_1_EXI);
            } else {
                __OSUnmaskInterrupts(OS_INTERRUPTMASK_EXI_1_EXI);
            }
            break;
        }
        case EXI_CHAN_2: {
            if (!__OSGetInterruptHandler(__OS_INTERRUPT_PI_DEBUG) || (exi->state & STATE_LOCKED)) {
                __OSMaskInterrupts(OS_INTERRUPTMASK_PI_DEBUG);
            } else {
                __OSUnmaskInterrupts(OS_INTERRUPTMASK_PI_DEBUG);
            }
            break;
        }
    }
}

static void CompleteTransfer(s32 chan) {
    EXIControl* exi;
    u8* buf;
    u32 data;
    int i;
    int len;

    exi = &Ecb[chan];
    ASSERTLINE(395, 0 <= chan && chan < MAX_CHAN);

    if (exi->state & STATE_BUSY) {
        if (exi->state & STATE_IMM) {
            if (exi->immLen) {
                len = exi->immLen;
                buf = exi->immBuf;
                data = REG(chan, 4);
                for (i = 0; i < len; i++) {
                    *buf++ = data >> ((MAX_CHAN - i) * 8);
                }
            }
        }
        exi->state &= ~STATE_BUSY;
    }
}

BOOL EXIImm(s32 chan, void* buf, s32 len, u32 type, EXICallback callback) {
    EXIControl* exi;
    BOOL enabled;

    exi = &Ecb[chan];
    ASSERTLINE(434, exi->state & STATE_SELECTED);
    ASSERTLINE(435, 0 <= chan && chan < MAX_CHAN);
    ASSERTLINE(436, 0 < len && len <= MAX_IMM);
    ASSERTLINE(437, type < MAX_TYPE);
    enabled = OSDisableInterrupts();

    if ((exi->state & STATE_BUSY) || !(exi->state & STATE_SELECTED)) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    exi->tcCallback = callback;
    if (exi->tcCallback) {
        EXIClearInterrupts(chan, FALSE, TRUE, FALSE);
        __OSUnmaskInterrupts(OS_INTERRUPTMASK_EXI_0_TC >> (chan * MAX_CHAN));
    }

    exi->state |= STATE_IMM;
    if (type) {
        u32 data = 0;
        int i;
        for (i = 0; i < len; i++) {
            data |= ((u8*)buf)[i] << ((MAX_CHAN - i) * 8);
        }
        REG(chan, 4) = data;
    }

    exi->immBuf = buf;
    exi->immLen = (type != 1) ? len : 0;

    REG(chan, 3) = (type << 2) | 1 | ((len - 1) << 4);

    OSRestoreInterrupts(enabled);

    return TRUE;
}

BOOL EXIImmEx(s32 chan, void* buf, s32 len, u32 mode) {
    s32 xLen;

    while (len) {
        xLen = (len < 4) ? len : 4;
        if (!EXIImm(chan, buf, xLen, mode, NULL)) {
            return FALSE;
        }
        if (!EXISync(chan)) {
            return FALSE;
        }
        ((u8*)buf) += xLen;
        len -= xLen;
    }

    return TRUE;
}

BOOL EXIDma(s32 chan, void* buf, s32 len, u32 type, EXICallback callback) {
    EXIControl* exi;
    BOOL enabled;

    exi = &Ecb[chan];

    ASSERTLINE(539, exi->state & STATE_SELECTED);
    ASSERTLINE(540, OFFSET(buf, 32) == 0);
    ASSERTLINE(541, 0 < len && OFFSET(len, 32) == 0);
    // clang-format off
    ASSERTLINE(543, ((u32) len & ~EXI_0LENGTH_EXILENGTH_MASK) == 0);
    // clang-format on
    ASSERTLINE(545, type == EXI_READ || type == EXI_WRITE);

    enabled = OSDisableInterrupts();
    if ((exi->state & STATE_BUSY) || !(exi->state & STATE_SELECTED)) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    exi->tcCallback = callback;
    if (exi->tcCallback) {
        EXIClearInterrupts(chan, FALSE, TRUE, FALSE);
        __OSUnmaskInterrupts(OS_INTERRUPTMASK_EXI_0_TC >> (chan * MAX_CHAN));
    }

    exi->state |= STATE_DMA;
    REG(chan, 1) = (u32)buf & EXI_0LENGTH_EXILENGTH_MASK;
    REG(chan, 2) = len;
    REG(chan, 3) = (type * 4) | 3;

    OSRestoreInterrupts(enabled);

    return TRUE;
}

BOOL EXISync(s32 chan) {
    EXIControl* exi;
    BOOL rc;
    BOOL enabled;

    exi = &Ecb[chan];
    rc = FALSE;
    ASSERTLINE(595, 0 <= chan && chan < MAX_CHAN);

    while ((exi->state & STATE_SELECTED)) {
        if (!(REG(chan, 3) & 1)) {
            enabled = OSDisableInterrupts();
            if (exi->state & STATE_SELECTED) {
                CompleteTransfer(chan);
                if (__OSGetDIConfig() != 0xFF || (OSGetConsoleType() & OS_CONSOLE_MASK) == OS_CONSOLE_TDEV_EMU || exi->immLen != 4 ||
                    (REG(chan, 0) & 0x70) || (REG(chan, 4) != 0x01010000 && REG(chan, 4) != 0x05070000 && REG(chan, 4) != 0x04220001) ||
                    __OSDeviceCode == 0x8200) {
                    rc = TRUE;
                }
            }
            OSRestoreInterrupts(enabled);
            break;
        }
    }

    ASSERTLINE(623, !(exi->state & STATE_BUSY));
    return rc;
}

u32 EXIClearInterrupts(s32 chan, BOOL exi, BOOL tc, BOOL ext) {
    u32 cpr;
    u32 prev;

    ASSERTLINE(644, 0 <= chan && chan < MAX_CHAN);

    cpr = prev = REG(chan, 0);
    prev &= 0x7F5;

    if (exi) {
        prev |= 2;
    }

    if (tc) {
        prev |= 8;
    }

    if (ext) {
        prev |= 0x800;
    }

    REG(chan, 0) = prev;

    return cpr;
}

EXICallback EXISetExiCallback(s32 chan, EXICallback exiCallback) {
    EXIControl* exi;
    EXICallback prev;
    BOOL enabled;

    exi = &Ecb[chan];
    ASSERTLINE(678, 0 <= chan && chan < MAX_CHAN);
    enabled = OSDisableInterrupts();

    prev = exi->exiCallback;
    exi->exiCallback = exiCallback;
    if (chan != EXI_CHAN_2) {
        SetExiInterruptMask(chan, exi);
    } else {
        SetExiInterruptMask(EXI_CHAN_0, &Ecb[EXI_CHAN_0]);
    }

    OSRestoreInterrupts(enabled);
    return prev;
}

void EXIProbeReset() {
    __gUnknown800030C0[EXI_CHAN_0] = __gUnknown800030C0[EXI_CHAN_1] = 0;
    Ecb[EXI_CHAN_0].idTime = Ecb[EXI_CHAN_1].idTime = 0;
    __EXIProbe(EXI_CHAN_0);
    __EXIProbe(EXI_CHAN_1);
}

static BOOL __EXIProbe(s32 chan) {
    EXIControl* exi;
    BOOL enabled;
    BOOL rc;
    u32 cpr;
    s32 t;

    exi = &Ecb[chan];
    ASSERTLINE(733, 0 <= chan && chan < MAX_CHAN);
    if (chan == EXI_CHAN_2) {
        return TRUE;
    }

    rc = TRUE;
    enabled = OSDisableInterrupts();
    cpr = REG(chan, 0);

    if (!(exi->state & STATE_ATTACHED)) {
        if (cpr & 0x800) {
            EXIClearInterrupts(chan, FALSE, FALSE, TRUE);
            __gUnknown800030C0[chan] = exi->idTime = 0;
        }

        if (cpr & 0x1000) {
            t = ((s32)(OSTicksToMilliseconds(OSGetTime()) / 100) + 1);

            if (__gUnknown800030C0[chan] == 0) {
                __gUnknown800030C0[chan] = t;
            }

            if (t - (s32)__gUnknown800030C0[chan] < 3) {
                rc = FALSE;
            }
        } else {
            __gUnknown800030C0[chan] = exi->idTime = 0;
            rc = FALSE;
        }
    } else if (!(cpr & 0x1000) || (cpr & 0x800)) {
        __gUnknown800030C0[chan] = exi->idTime = 0;
        rc = FALSE;
    }

    OSRestoreInterrupts(enabled);
    return rc;
}

BOOL EXIProbe(s32 chan) {
    EXIControl* exi = &Ecb[chan];
    BOOL rc;
    u32 id;

    rc = __EXIProbe(chan);
    if (rc && !exi->idTime) {
        rc = EXIGetID(chan, 0, &id) ? TRUE : FALSE;
    }

    return rc;
}

s32 EXIProbeEx(s32 chan) {
    if (EXIProbe(chan)) {
        return 1;
    }

    if (__gUnknown800030C0[chan] != 0) {
        return 0;
    }

    return -1;
}

static BOOL __EXIAttach(s32 chan, EXICallback extCallback) {
    EXIControl* exi;
    BOOL enabled;

    exi = &Ecb[chan];
    ASSERTLINE(838, 0 <= chan && chan < 2);
    enabled = OSDisableInterrupts();

    if ((exi->state & STATE_ATTACHED) || !__EXIProbe(chan)) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    EXIClearInterrupts(chan, TRUE, FALSE, FALSE);
    exi->extCallback = extCallback;
    __OSUnmaskInterrupts(OS_INTERRUPTMASK_EXI_0_EXT >> (chan * MAX_CHAN));
    exi->state |= STATE_ATTACHED;

    OSRestoreInterrupts(enabled);

    return TRUE;
}

BOOL EXIAttach(s32 chan, EXICallback extCallback) {
    EXIControl* exi;
    BOOL enabled;
    BOOL rc;

    exi = &Ecb[chan];
    ASSERTLINE(864, 0 <= chan && chan < 2);

    EXIProbe(chan);
    enabled = OSDisableInterrupts();
    if (exi->idTime == 0) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    rc = __EXIAttach(chan, extCallback);
    OSRestoreInterrupts(enabled);
    return rc;
}

BOOL EXIDetach(s32 chan) {
    EXIControl* exi;
    BOOL enabled;

    exi = &Ecb[chan];
    ASSERTLINE(898, 0 <= chan && chan < 2);
    enabled = OSDisableInterrupts();

    if (!(exi->state & STATE_ATTACHED)) {
        OSRestoreInterrupts(enabled);
        return TRUE;
    }

    if ((exi->state & STATE_LOCKED) && exi->dev == 0) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    exi->state &= ~STATE_ATTACHED;
    __OSMaskInterrupts((OS_INTERRUPTMASK_EXI_0_EXT | OS_INTERRUPTMASK_EXI_0_EXI) >> (chan * MAX_CHAN));

    OSRestoreInterrupts(enabled);

    return TRUE;
}

BOOL EXISelectSD(s32 chan, u32 dev, u32 freq) {
    EXIControl* exi;
    u32 cpr;
    BOOL enabled;

    exi = &Ecb[chan];

    ASSERTLINE(938, 0 <= chan && chan < MAX_CHAN);
    ASSERTLINE(939, chan == 0 && dev < MAX_DEV || dev == 0);
    ASSERTLINE(940, freq < MAX_FREQ);
    ASSERTLINE(941, !(exi->state & STATE_SELECTED));

    enabled = OSDisableInterrupts();
    if ((exi->state & STATE_SELECTED) || (chan != EXI_CHAN_2 && ((dev == 0 && !(exi->state & STATE_ATTACHED) && !EXIProbe(chan)) ||
                                                                 !(exi->state & STATE_LOCKED) || exi->dev != dev))) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    exi->state |= STATE_SELECTED;
    cpr = REG(chan, 0);
    cpr &= 0x405;
    cpr |= freq * 0x10;
    REG(chan, 0) = cpr;

    if (exi->state & STATE_ATTACHED) {
        switch (chan) {
            case EXI_CHAN_0: {
                __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_0_EXT);
                break;
            }
            case EXI_CHAN_1: {
                __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_1_EXT);
                break;
            }
        }
    }

    OSRestoreInterrupts(enabled);

    return TRUE;
}

BOOL EXISelect(s32 chan, u32 dev, u32 freq) {
    EXIControl* exi;
    u32 cpr;
    BOOL enabled;

    exi = &Ecb[chan];

    ASSERTLINE(996, 0 <= chan && chan < MAX_CHAN);
    ASSERTLINE(997, chan == 0 && dev < MAX_DEV || dev == 0);
    ASSERTLINE(998, freq < MAX_FREQ);
    ASSERTLINE(999, !(exi->state & STATE_SELECTED));

    enabled = OSDisableInterrupts();
    if ((exi->state & STATE_SELECTED) || (chan != EXI_CHAN_2 && ((dev == 0 && !(exi->state & STATE_ATTACHED) && !__EXIProbe(chan)) ||
                                                                 !(exi->state & STATE_LOCKED) || exi->dev != dev))) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    exi->state |= STATE_SELECTED;
    cpr = REG(chan, 0);
    cpr &= 0x405;
    cpr |= (((1 << dev) << 7) | (freq * 0x10));
    REG(chan, 0) = cpr;

    if (exi->state & STATE_ATTACHED) {
        switch (chan) {
            case EXI_CHAN_0: {
                __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_0_EXT);
                break;
            }
            case EXI_CHAN_1: {
                __OSMaskInterrupts(OS_INTERRUPTMASK_EXI_1_EXT);
                break;
            }
        }
    }

    OSRestoreInterrupts(enabled);

    return TRUE;
}

BOOL EXISelectEx(s32 chan, u32 dev, u32 freq) {
    EXIControl* exi;
    u32 cpr;

    exi = &Ecb[chan];

    exi->state |= STATE_SELECTED;
    cpr = REG(chan, 0);
    cpr &= 0x405;
    cpr |= (((1 << dev) << 7) | (freq * 0x10));
    REG(chan, 0) = cpr;

    return TRUE;
}

BOOL EXIDeselect(s32 chan) {
    EXIControl* exi;
    u32 cpr;
    BOOL enabled;

    exi = &Ecb[chan];
    ASSERTLINE(1077, 0 <= chan && chan < MAX_CHAN);
    enabled = OSDisableInterrupts();

    if (!(exi->state & STATE_SELECTED)) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    exi->state &= ~STATE_SELECTED;
    cpr = REG(chan, 0);
    REG(chan, 0) = cpr & 0x405;

    if (exi->state & STATE_ATTACHED) {
        switch (chan) {
            case EXI_CHAN_0: {
                __OSUnmaskInterrupts(OS_INTERRUPTMASK_EXI_0_EXT);
                break;
            }
            case EXI_CHAN_1: {
                __OSUnmaskInterrupts(OS_INTERRUPTMASK_EXI_1_EXT);
                break;
            }
        }
    }

    OSRestoreInterrupts(enabled);

    if (chan != EXI_CHAN_2 && (cpr & 0x80)) {
        return __EXIProbe(chan) ? TRUE : FALSE;
    }

    return TRUE;
}

static BOOL EXIDeselectEx(s32 chan) {
    EXIControl* exi;
    u32 cpr;

    exi = &Ecb[chan];

    exi->state &= ~STATE_SELECTED;
    cpr = REG(chan, 0);
    REG(chan, 0) = cpr & 0x405;

    return TRUE;
}

static void EXIIntrruptHandler(__OSInterrupt interrupt, OSContext* context) {
    s32 chan;
    EXIControl* exi;
    EXICallback callback;

    chan = (interrupt - 9) / 3;

    ASSERTLINE(1150, 0 <= chan && chan < MAX_CHAN);
    exi = &Ecb[chan];
    EXIClearInterrupts(chan, TRUE, FALSE, FALSE);

    callback = exi->exiCallback;
    if (callback) {
        OSContext exceptionContext;

        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);
        callback(chan, context);
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
    }
}

static void TCIntrruptHandler(__OSInterrupt interrupt, OSContext* context) {
    s32 chan;
    EXIControl* exi;
    EXICallback callback;

    chan = (interrupt - 10) / 3;

    ASSERTLINE(1186, 0 <= chan && chan < MAX_CHAN);
    exi = &Ecb[chan];
    __OSMaskInterrupts(OS_INTERRUPTMASK(interrupt));
    EXIClearInterrupts(chan, FALSE, TRUE, FALSE);

    callback = exi->tcCallback;
    if (callback) {
        OSContext exceptionContext;

        exi->tcCallback = NULL;
        CompleteTransfer(chan);

        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);
        callback(chan, context);
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
    }
}

static void EXTIntrruptHandler(__OSInterrupt interrupt, OSContext* context) {
    s32 chan;
    EXIControl* exi;
    EXICallback callback;

    chan = (interrupt - 11) / 3;

    ASSERTLINE(1226, 0 <= chan && chan < 2);
    __OSMaskInterrupts((OS_INTERRUPTMASK_EXI_0_EXI | OS_INTERRUPTMASK_EXI_0_EXT) >> (chan * MAX_CHAN));
    exi = &Ecb[chan];
    callback = exi->extCallback;
    exi->state &= ~STATE_ATTACHED;

    if (callback) {
        OSContext exceptionContext;

        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);
        exi->extCallback = NULL;
        callback(chan, context);
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
    }
}

void EXIInit() {
    u32 id;

    while ((REG(0, 3) & 1) == 1 || (REG(1, 3) & 1) == 1 || (REG(2, 3) & 1) == 1) {
    }

    __OSMaskInterrupts(OS_INTERRUPTMASK_EXI);
    __EXIRegs[0] = 0;
    __EXIRegs[5] = 0;
    __EXIRegs[10] = 0;
    __EXIRegs[0] = 0x2000;
    __OSSetInterruptHandler(__OS_INTERRUPT_EXI_0_EXI, EXIIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_EXI_0_TC, TCIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_EXI_0_EXT, EXTIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_EXI_1_EXI, EXIIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_EXI_1_TC, TCIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_EXI_1_EXT, EXTIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_EXI_2_EXI, EXIIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_EXI_2_TC, TCIntrruptHandler);

    EXIGetID(EXI_CHAN_0, 2, &IDSerialPort1);

    if (__OSInIPL) {
        EXIProbeReset();
    } else if (EXIGetID(EXI_CHAN_0, 0, &id) && id == 0x7010000) {
        __OSEnableBarnacle(EXI_CHAN_1, 0);
    } else if (EXIGetID(EXI_CHAN_1, 0, &id) && id == 0x7010000) {
        __OSEnableBarnacle(EXI_CHAN_0, 2);
    }

    OSRegisterVersion(__EXIVersion);
}

BOOL EXILock(s32 chan, u32 dev, EXICallback unlockedCallback) {
    EXIControl* exi;
    BOOL enabled;
    int i;

    exi = &Ecb[chan];
    ASSERTLINE(1338, 0 <= chan && chan < MAX_CHAN);
    ASSERTLINE(1339, chan == 0 && dev < MAX_DEV || dev == 0);
    enabled = OSDisableInterrupts();

    if (exi->state & STATE_LOCKED) {
        if (unlockedCallback) {
            ASSERTLINE(1345, chan == 0 && exi->items < (MAX_DEV - 1) || exi->items == 0);
            for (i = 0; i < exi->items; i++) {
                if (exi->queue[i].dev == dev) {
                    OSRestoreInterrupts(enabled);
                    return FALSE;
                }
            }
            exi->queue[exi->items].callback = unlockedCallback;
            exi->queue[exi->items].dev = dev;
            exi->items++;
        }
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    ASSERTLINE(1361, exi->items == 0);
    exi->state |= STATE_LOCKED;
    exi->dev = dev;
    SetExiInterruptMask(chan, exi);
    OSRestoreInterrupts(enabled);
    return TRUE;
}

BOOL EXIUnlock(s32 chan) {
    EXIControl* exi;
    BOOL enabled;
    EXICallback unlockedCallback;

    exi = &Ecb[chan];
    ASSERTLINE(1385, 0 <= chan && chan < MAX_CHAN);
    enabled = OSDisableInterrupts();

    if (!(exi->state & STATE_LOCKED)) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    exi->state &= ~STATE_LOCKED;
    SetExiInterruptMask(chan, exi);
    if (exi->items > 0) {
        unlockedCallback = exi->queue[0].callback;
        if (--exi->items > 0) {
            memmove(&exi->queue[0], &exi->queue[1], exi->items * 8);
        }
        unlockedCallback(chan, NULL);
    }

    OSRestoreInterrupts(enabled);
    return TRUE;
}

u32 EXIGetState(s32 chan) {
    EXIControl* exi;

    exi = &Ecb[chan];
    ASSERTLINE(1422, 0 <= chan && chan < MAX_CHAN);
    return exi->state;
}

static void UnlockedHandler(s32 chan, OSContext* context) {
    u32 id;
    EXIGetID(chan, 0, &id);
}

s32 EXIGetID(s32 chan, u32 dev, u32* id) {
    EXIControl* exi = &Ecb[chan];
    BOOL err;
    u32 cmd;
    s32 startTime;
    BOOL enabled;

    ASSERTLINE(1459, 0 <= chan && chan < MAX_CHAN);
    if (chan == 0 && dev == 2 && IDSerialPort1) {
        *id = IDSerialPort1;
        return 1;
    }

    if (chan < EXI_CHAN_2 && dev == 0) {
        if (!__EXIProbe(chan)) {
            return 0;
        }

        if (exi->idTime == __gUnknown800030C0[chan]) {
            *id = exi->id;
            return exi->idTime;
        }

        if (!__EXIAttach(chan, NULL)) {
            return 0;
        }

        startTime = __gUnknown800030C0[chan];
    }

    enabled = OSDisableInterrupts();

    err = !EXILock(chan, dev, (chan < 2 && dev == 0) ? &UnlockedHandler : 0);
    if (!err) {
        err = !EXISelect(chan, dev, 0);
        if (!err) {
            cmd = 0;
            err |= !EXIImm(chan, &cmd, 2, EXI_WRITE, NULL);
            err |= !EXISync(chan);
            err |= !EXIImm(chan, id, 4, EXI_READ, NULL);
            err |= !EXISync(chan);
            err |= !EXIDeselect(chan);
        }

        EXIUnlock(chan);
    }

    OSRestoreInterrupts(enabled);

    if (chan < EXI_CHAN_2 && dev == 0) {
        EXIDetach(chan);
        enabled = OSDisableInterrupts();
        err |= __gUnknown800030C0[chan] != startTime;

        if (!err) {
            exi->id = *id;
            exi->idTime = startTime;
        }

        OSRestoreInterrupts(enabled);

        return err ? 0 : exi->idTime;
    }

    return err ? 0 : 1;
}

s32 EXIGetIDEx(s32 chan, u32 dev, u32* id) {
    EXIControl* exi = &Ecb[chan];
    BOOL err;
    u32 cmd;

    err = !EXISelectEx(chan, dev, 0);
    if (!err) {
        cmd = 0;
        err |= !EXIImm(chan, &cmd, 2, EXI_WRITE, NULL);
        err |= !EXISync(chan);
        err |= !EXIImm(chan, id, 4, EXI_READ, NULL);
        err |= !EXISync(chan);
        err |= !EXIDeselectEx(chan);
    }

    if (!err) {
        exi->id = *id;
        exi->idTime = __gUnknown800030C0[chan];
    }

    return err ? 0 : 1;
}

s32 EXIGetType(s32 chan, u32 dev, u32* type) {
    u32 _type;
    s32 probe;

    probe = EXIGetID(chan, dev, &_type);

    if (!probe) {
        return probe;
    }

    switch (_type & ~0xFF) {
        case 0x04020100:
        case EXI_ETHER:
        case 0x04020300:
        case EXI_MIC: {
            *type = (_type & ~0xFF);
            return probe;
        }
        default: {
            switch (_type & ~0xFFFF) {
                case 0x00000000: {
                    if (!(_type & 0x3803)) {
                        switch (_type & 0xFC) {
                            case EXI_MEMORY_CARD_59:
                            case EXI_MEMORY_CARD_123:
                            case EXI_MEMORY_CARD_251:
                            case EXI_MEMORY_CARD_507:
                            case EXI_MEMORY_CARD_1019:
                            case EXI_MEMORY_CARD_2043: {
                                *type = _type & 0xFC;
                                return probe;
                            }
                        }
                    }
                    break;
                }
                case EXI_IS_VIEWER: {
                    *type = EXI_IS_VIEWER;
                    return probe;
                }
            }

            *type = _type;
            return probe;
        }
    }
}

char* EXIGetTypeString(u32 type) {
    switch (type) {
        case EXI_MEMORY_CARD_59: {
            return "Memory Card 59";
        }
        case EXI_MEMORY_CARD_123: {
            return "Memory Card 123";
        }
        case EXI_MEMORY_CARD_251: {
            return "Memory Card 251";
        }
        case EXI_MEMORY_CARD_507: {
            return "Memory Card 507";
        }
        case EXI_MEMORY_CARD_1019: {
            return "Memory Card 1019";
        }
        case EXI_MEMORY_CARD_2043: {
            return "Memory Card 2043";
        }
        case EXI_USB_ADAPTER: {
            return "USB Adapter";
        }
        case EXI_NPDP_GDEV: {
            return "GDEV";
        }
        case EXI_MODEM: {
            return "Modem";
        }
        case EXI_MARLIN: {
            return "Marlin";
        }
        case EXI_AD16: {
            return "AD16";
        }
        case EXI_RS232C: {
            return "RS232C";
        }
        case 0x80000020:
        case 0x80000080:
        case 0x80000040:
        case 0x80000008:
        case 0x80000010:
        case 0x80000004: {
            return "Net Card";
        }
        case EXI_ETHER_VIEWER: {
            return "Artist Ether";
        }
        case 0x4020100:
        case 0x4020300:
        case EXI_ETHER:
        case 0x4220000: {
            return "Broadband Adapter";
        }
        case EXI_MIC: {
            return "Mic";
        }
        case EXI_STREAM_HANGER: {
            return "Stream Hanger";
        }
        case EXI_IS_VIEWER: {
            return "IS-DOL-VIEWER";
        }
        default: {
            return "Unknown";
        }
    }
}
