#include <revolution/exi.h>

static BOOL Initialized;

BOOL AD16Init() {
    BOOL err;
    u32 cmd;
    u32 id;

    if (Initialized) {
        return TRUE;
    }

    if (!EXILock(EXI_CHAN_2, 0, NULL)) {
        return FALSE;
    }

    if (!EXISelect(EXI_CHAN_2, 0, EXI_FREQ_1M)) {
        EXIUnlock(EXI_CHAN_2);
        return FALSE;
    }

    cmd = 0;
    err = FALSE;
    err |= !EXIImm(EXI_CHAN_2, &cmd, 2, EXI_WRITE, NULL);
    err |= !EXISync(EXI_CHAN_2);
    err |= !EXIImm(EXI_CHAN_2, &id, 4, EXI_READ, NULL);
    err |= !EXISync(EXI_CHAN_2);
    err |= !EXIDeselect(EXI_CHAN_2);

    EXIUnlock(EXI_CHAN_2);
    if (err || id != EXI_AD16) {
        return FALSE;
    }

    Initialized = TRUE;
    return TRUE;
}

BOOL AD16WriteReg(u32 word) {
    BOOL err;
    u32 cmd;

    if (!Initialized || !EXILock(EXI_CHAN_2, 0, NULL)) {
        return FALSE;
    }

    if (!EXISelect(EXI_CHAN_2, 0, EXI_FREQ_8M)) {
        EXIUnlock(EXI_CHAN_2);
        return FALSE;
    }

    cmd = 0xA0000000;
    err = FALSE;
    err |= !EXIImm(EXI_CHAN_2, &cmd, 1, EXI_WRITE, NULL);
    err |= !EXISync(EXI_CHAN_2);
    err |= !EXIImm(EXI_CHAN_2, &word, 4, EXI_WRITE, NULL);
    err |= !EXISync(EXI_CHAN_2);
    err |= !EXIDeselect(EXI_CHAN_2);

    EXIUnlock(EXI_CHAN_2);

    return err ? FALSE : TRUE;
}

BOOL AD16ReadReg(u32* word) {
    BOOL err;
    u32 cmd;

    if (!Initialized || !EXILock(EXI_CHAN_2, 0, NULL)) {
        return FALSE;
    }

    if (!EXISelect(EXI_CHAN_2, 0, EXI_FREQ_8M)) {
        EXIUnlock(EXI_CHAN_2);
        return FALSE;
    }

    cmd = 0xA2000000;
    err = 0;
    err |= !EXIImm(EXI_CHAN_2, &cmd, 1, EXI_WRITE, NULL);
    err |= !EXISync(EXI_CHAN_2);
    err |= !EXIImm(EXI_CHAN_2, word, 4, EXI_READ, NULL);
    err |= !EXISync(EXI_CHAN_2);
    err |= !EXIDeselect(EXI_CHAN_2);

    EXIUnlock(EXI_CHAN_2);

    return err ? FALSE : TRUE;
}

BOOL AD16Probe() {
    return Initialized;
}
