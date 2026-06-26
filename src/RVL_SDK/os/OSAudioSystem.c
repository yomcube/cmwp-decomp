#include <revolution/os.h>

#include <private/os.h>

#include <private/hollywood.h>

#include <string.h>

static u8 DSPInitCode[128] = {0x02, 0x9F, 0x00, 0x10, 0x02, 0x9F, 0x00, 0x33, 0x02, 0x9F, 0x00, 0x34, 0x02, 0x9F, 0x00, 0x35, 0x02, 0x9F, 0x00,
                              0x36, 0x02, 0x9F, 0x00, 0x37, 0x02, 0x9F, 0x00, 0x38, 0x02, 0x9F, 0x00, 0x39, 0x12, 0x06, 0x12, 0x03, 0x12, 0x04,
                              0x12, 0x05, 0x00, 0x80, 0x80, 0x00, 0x00, 0x88, 0xFF, 0xFF, 0x00, 0x84, 0x10, 0x00, 0x00, 0x64, 0x00, 0x1D, 0x02,
                              0x18, 0x00, 0x00, 0x81, 0x00, 0x1C, 0x1E, 0x00, 0x44, 0x1B, 0x1E, 0x00, 0x84, 0x08, 0x00, 0x00, 0x64, 0x00, 0x27,
                              0x19, 0x1E, 0x00, 0x00, 0x00, 0xDE, 0xFF, 0xFC, 0x02, 0xA0, 0x80, 0x00, 0x02, 0x9C, 0x00, 0x28, 0x16, 0xFC, 0x00,
                              0x54, 0x16, 0xFD, 0x43, 0x48, 0x00, 0x21, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF,
                              0x02, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define __DSPWorkBuffer (void*)0x81000000

static void waitMicroSec(u32 microSec) {
    OSTick start = OSGetTick();
    while (OSTicksToMicroseconds(OSGetTick() - start) < microSec) {
    }
}

void __AIClockInit(BOOL arg0) {
    u32 reg;

    reg = __ACRIOPRegs[HW_COMPAT];
    reg &= ~0x100;
    reg |= arg0 << 8;
    reg &= ~0x80;

    __ACRIOPRegs[HW_COMPAT] = reg;
    reg = __ACRIOPRegs[HW_PLLAIEXT];
    reg &= ~HW_PLLAIEXT_PLL_B;
    reg &= ~HW_PLLAIEXT_PLL_A;
    __ACRIOPRegs[HW_PLLAIEXT] = reg;
    waitMicroSec(100);

    if (!arg0) {
        reg = __ACRIOPRegs[HW_PLLAI];
        reg &= ~0x3FFC0;
        reg |= 0xFC0;
        reg &= ~0x3F;
        reg &= ~0x7FC0000;
        reg |= 0x04640000;
        __ACRIOPRegs[HW_PLLAI] = reg;
    } else {
        reg = __ACRIOPRegs[HW_PLLAI];
        reg &= ~0x3FFC0;
        reg |= 0xFFC0;
        reg &= ~0x3F;
        reg |= 0xE;
        reg &= ~0x7FC0000;
        reg |= 0x04B00000;
        __ACRIOPRegs[HW_PLLAI] = reg;
    }
    waitMicroSec(100);

    reg = __ACRIOPRegs[HW_PLLAIEXT];
    reg &= ~0x10000000;
    __ACRIOPRegs[HW_PLLAIEXT] = reg;
    waitMicroSec(1000);

    reg = __ACRIOPRegs[HW_PLLAIEXT];
    reg &= ~HW_PLLAIEXT_PLL_A;
    reg |= HW_PLLAIEXT_PLL_A;
    __ACRIOPRegs[HW_PLLAIEXT] = reg;
    waitMicroSec(1000);

    reg = __ACRIOPRegs[HW_PLLAIEXT];
    reg &= ~HW_PLLAIEXT_PLL_B;
    reg |= HW_PLLAIEXT_PLL_B;
    __ACRIOPRegs[HW_PLLAIEXT] = reg;
    waitMicroSec(1000);
}

void __OSInitAudioSystem() {
    u8 errFlag;
    u16 reg16;
    u32 start_tick;

    if (!__OSInIPL) {
        __AIClockInit(TRUE);
    }

    memcpy((void*)((u32)OSGetArenaHi() - 0x80), __DSPWorkBuffer, sizeof(DSPInitCode));
    memcpy(__DSPWorkBuffer, (void*)DSPInitCode, sizeof(DSPInitCode));
    DCFlushRange(__DSPWorkBuffer, sizeof(DSPInitCode));

    __DSPRegs[DSP_AR_SIZE] = 0x43;
    ASSERTMSGLINE(253, !(__DSPRegs[DSP_CONTROL_STATUS] & DSP_CONTROL_STATUS_ARDMASTAT), "__OSInitAudioSystem(): ARAM DMA already in progress");
    ASSERTMSGLINE(257, !(__DSPRegs[DSP_CONTROL_STATUS] & DSP_CONTROL_STATUS_DMASTAT), "__OSInitAudioSystem(): DSP DMA already in progress");
    ASSERTMSGLINE(261, (__DSPRegs[DSP_CONTROL_STATUS] & DSP_CONTROL_STATUS_HALT), "__OSInitAudioSystem(): DSP already working");

    __DSPRegs[DSP_CONTROL_STATUS] =
        DSP_CONTROL_STATUS_RESET | DSP_CONTROL_STATUS_DSPINT | DSP_CONTROL_STATUS_ARINT | DSP_CONTROL_STATUS_AIDINT | DSP_CONTROL_STATUS_HALT;
    __DSPRegs[DSP_CONTROL_STATUS] |= DSP_CONTROL_STATUS_RES;

    while (__DSPRegs[DSP_CONTROL_STATUS] & DSP_CONTROL_STATUS_RES) {
    }
    __DSPRegs[DSP_MAILBOX] = 0;

    while (((__DSPRegs[DSP_CPUMAILBOX_H] << 16) | __DSPRegs[DSP_CPUMAILBOX_L]) & 0x80000000) {
    }

    *(u32*)&__DSPRegs[DSP_AR_DMA_MMADDR] = 0x1000000;
    *(u32*)&__DSPRegs[DSP_AR_DMA_ARADDR] = 0;
    *(u32*)&__DSPRegs[DSP_AR_DMA_SIZE] = 0x20;

    reg16 = __DSPRegs[DSP_CONTROL_STATUS];
    while (!(reg16 & DSP_CONTROL_STATUS_ARINT))
        reg16 = __DSPRegs[DSP_CONTROL_STATUS];

    __DSPRegs[DSP_CONTROL_STATUS] = reg16;

    start_tick = OSGetTick();
    while ((s32)(OSGetTick() - start_tick) < 2194) {
    }

    *(u32*)&__DSPRegs[DSP_AR_DMA_MMADDR] = 0x1000000;
    *(u32*)&__DSPRegs[DSP_AR_DMA_ARADDR] = 0;
    *(u32*)&__DSPRegs[DSP_AR_DMA_SIZE] = 0x20;

    reg16 = __DSPRegs[DSP_CONTROL_STATUS];
    while (!(reg16 & DSP_CONTROL_STATUS_ARINT)) {
        reg16 = __DSPRegs[DSP_CONTROL_STATUS];
    }
    __DSPRegs[DSP_CONTROL_STATUS] = reg16;

    __DSPRegs[DSP_CONTROL_STATUS] &= ~0x800;
    while ((__DSPRegs[DSP_CONTROL_STATUS]) & 0x400) {
    }

    __DSPRegs[DSP_CONTROL_STATUS] &= ~4;
    errFlag = 0;

    reg16 = __DSPRegs[DSP_CPUMAILBOX_H];

    while (!(reg16 & 0x8000)) {
        reg16 = __DSPRegs[DSP_CPUMAILBOX_H];
    }

    if (((u32)((reg16 << 16) | __DSPRegs[DSP_CPUMAILBOX_L]) + 0x7FAC0000U) != 0x4348) {
        ASSERTMSGLINE(333, 0, "__OSInitAudioSystem(): DSP returns invalid message");
    }

    // reg16 != 0x81800;  // fake but fixes reg alloc on retail
    __DSPRegs[DSP_CONTROL_STATUS] |= DSP_CONTROL_STATUS_HALT;
    __DSPRegs[DSP_CONTROL_STATUS] =
        DSP_CONTROL_STATUS_RESET | DSP_CONTROL_STATUS_DSPINT | DSP_CONTROL_STATUS_ARINT | DSP_CONTROL_STATUS_AIDINT | DSP_CONTROL_STATUS_HALT;
    __DSPRegs[DSP_CONTROL_STATUS] |= DSP_CONTROL_STATUS_RES;
    while (__DSPRegs[DSP_CONTROL_STATUS] & DSP_CONTROL_STATUS_RES) {
    }

    memcpy(__DSPWorkBuffer, (void*)((u32)OSGetArenaHi() - 0x80), sizeof(DSPInitCode));
}

void __OSStopAudioSystem() {
    u16 reg16;
    u32 start_tick;

#define waitUntil(load, mask)                                                                                                                        \
    reg16 = (load);                                                                                                                                  \
    while (reg16 & (mask)) {                                                                                                                         \
        reg16 = (load);                                                                                                                              \
    }

    __DSPRegs[DSP_CONTROL_STATUS] = DSP_CONTROL_STATUS_RESET | DSP_CONTROL_STATUS_HALT;
    reg16 = __DSPRegs[DSP_DMA_CONTROL_LENGTH];
    __DSPRegs[DSP_DMA_CONTROL_LENGTH] = reg16 & ~DSP_DMA_CONTROL_LENGTH_CTRL;
    waitUntil(__DSPRegs[DSP_CONTROL_STATUS], DSP_CONTROL_STATUS_DMASTAT);
    waitUntil(__DSPRegs[DSP_CONTROL_STATUS], DSP_CONTROL_STATUS_ARDMASTAT);
    __DSPRegs[DSP_CONTROL_STATUS] =
        DSP_CONTROL_STATUS_RESET | DSP_CONTROL_STATUS_DSPINT | DSP_CONTROL_STATUS_ARINT | DSP_CONTROL_STATUS_AIDINT | DSP_CONTROL_STATUS_HALT;
    __DSPRegs[DSP_MAILBOX] = 0;

    while (((__DSPRegs[DSP_CPUMAILBOX_H] << 16) | __DSPRegs[DSP_CPUMAILBOX_L]) & 0x80000000) {
    }

    start_tick = OSGetTick();
    while ((s32)(OSGetTick() - start_tick) < 44) {
    }

    reg16 = __DSPRegs[DSP_CONTROL_STATUS];
    __DSPRegs[DSP_CONTROL_STATUS] = reg16 | DSP_CONTROL_STATUS_RES;
    waitUntil(__DSPRegs[DSP_CONTROL_STATUS], DSP_CONTROL_STATUS_RES);

#undef waitUntil
}
