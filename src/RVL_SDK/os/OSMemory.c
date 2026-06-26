#include <revolution/os.h>

#include <private/os.h>

#include <private/hollywood.h>

static BOOL OnShutdown(BOOL final, u32);

static OSShutdownFunctionInfo ShutdownFunctionInfo = {OnShutdown, 0x7F, NULL, NULL};

u32 OSGetPhysicalMem1Size() {
    u32 size = *(u32*)OSPhysicalToCached(OS_ADDR_PHYSICAL_MEM1_SIZE);
    return size;
}

u32 OSGetPhysicalMem2Size() {
    u32 size = *(u32*)OSPhysicalToCached(OS_ADDR_PHYSICAL_MEM2_SIZE);
    return size;
}

u32 OSGetConsoleSimulatedMem1Size() {
    u32 size = *(u32*)OSPhysicalToCached(OS_ADDR_SIMULATED_MEM1_SIZE);
    return size;
}

u32 OSGetConsoleSimulatedMem2Size() {
    u32 size = *(u32*)OSPhysicalToCached(OS_ADDR_SIMULATED_MEM2_SIZE);
    return size;
}

static BOOL OnShutdown(BOOL final, u32 event) {
    if (final != FALSE) {
        __MEMRegs[MEM_MARR_CONTROL] = 0xFF;
        __OSMaskInterrupts(OS_INTERRUPTMASK_MEM_RESET);
    }
    return TRUE;
}

static void MEMIntrruptHandler(__OSInterrupt interrupt, OSContext* context) {
    u32 addr;
    u32 cause;

    cause = __MEMRegs[MEM_INT_STAT];
    addr = (((u32)__MEMRegs[MEM_INT_ADDRH] & 0x3ff) << 16) | __MEMRegs[MEM_INT_ADDRL];
    __MEMRegs[MEM_INT_CLR] = 0;

    if (__OSErrorTable[__OS_EXCEPTION_MEMORY_PROTECTION]) {
        __OSErrorTable[__OS_EXCEPTION_MEMORY_PROTECTION](__OS_EXCEPTION_MEMORY_PROTECTION, context, cause, addr);
        return;
    }

    __OSUnhandledException(__OS_EXCEPTION_MEMORY_PROTECTION, context, cause, addr);
}

void OSProtectRange(u32 chan, void* addr, u32 nBytes, u32 control) {
    BOOL enabled;
    u32 start;
    u32 end;
    u16 reg;

    ASSERTLINE(388, chan < 4);
    ASSERTLINE(389, (control & ~(OS_PROTECT_CONTROL_RDWR)) == 0);

    if (4 <= chan) {
        return;
    }

    control &= 3;

    end = (u32)addr + nBytes;
    start = ROUNDDOWN((u32)addr, 1u << 10);
    end = ROUNDUP((u32)end, 1u << 10);

    DCFlushRange((void*)start, end - start);

    enabled = OSDisableInterrupts();

    __OSMaskInterrupts(OS_INTERRUPTMASK(__OS_INTERRUPT_MEM_0 + chan));

    __MEMRegs[MEM_MARR_START + (MEM_MARR_SIZE * chan)] = (u16)(start >> 10);
    __MEMRegs[MEM_MARR_END + (MEM_MARR_SIZE * chan)] = (u16)(end >> 10);

    reg = __MEMRegs[MEM_MARR_CONTROL];
    reg &= ~(3 << 2 * chan);
    reg |= control << 2 * chan;
    __MEMRegs[MEM_MARR_CONTROL] = reg;

    if (control != 3) {
        __OSUnmaskInterrupts(OS_INTERRUPTMASK(__OS_INTERRUPT_MEM_0 + chan));
    }

    OSRestoreInterrupts(enabled);
}

static asm void ConfigMEM1_24MB() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x00000002@ha
    addi r4, r4, 0x00000002@l
    lis r3, 0x800001FF@ha
    addi r3, r3, 0x800001FF@l
    lis r6, 0x01000002@ha
    addi r6, r6, 0x01000002@l
    lis r5, 0x810000FF@ha
    addi r5, r5, 0x810000FF@l
    isync
    mtdbatu 0, r7
    mtdbatl 0, r4
    mtdbatu 0, r3
    isync
    mtibatu 0, r7
    mtibatl 0, r4
    mtibatu 0, r3
    isync
    mtdbatu 2, r7
    mtdbatl 2, r6
    mtdbatu 2, r5
    isync
    mtibatu 2, r7
    mtibatl 2, r6
    mtibatu 2, r5
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}

#if SDK_VERSION < 20090224
static asm void ConfigMEM1_48MB() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x00000002@ha
    addi r4, r4, 0x00000002@l
    lis r3, 0x800003FF@ha
    addi r3, r3, 0x800003FF@l
    lis r6, 0x02000002@ha
    addi r6, r6, 0x02000002@l
    lis r5, 0x820001FF@ha
    addi r5, r5, 0x820001FF@l
    isync
    mtdbatu 0, r7
    mtdbatl 0, r4
    mtdbatu 0, r3
    isync
    mtibatu 0, r7
    mtibatl 0, r4
    mtibatu 0, r3
    isync
    mtdbatu 2, r7
    mtdbatl 2, r6
    mtdbatu 2, r5
    isync
    mtibatu 2, r7
    mtibatl 2, r6
    mtibatu 2, r5
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}
#endif

static asm void ConfigMEM2_52MB() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x10000002@ha
    addi r4, r4, 0x10000002@l
    lis r3, 0x900003FF@ha
    addi r3, r3, 0x900003FF@l
    lis r6, 0x1000002A@ha
    addi r6, r6, 0x1000002A@l
    lis r5, 0xD00007FF@ha
    addi r5, r5, 0xD00007FF@l
    isync
    mtspr 0x238, r7
    mtspr 0x239, r4
    mtspr 0x238, r3
    isync
    mtspr 0x230, r7
    mtspr 0x231, r7
    isync
    mtspr 0x23a, r7
    mtspr 0x23b, r6
    mtspr 0x23a, r5
    isync
    mtspr 0x232, r7
    mtspr 0x233, r7
    isync
    lis r4, 0x12000002@ha
    addi r4, r4, 0x12000002@l
    lis r3, 0x920001FF@ha
    addi r3, r3, 0x920001FF@l
    lis r6, 0x13000002@ha
    addi r6, r6, 0x13000002@l
    lis r5, 0x9300007F@ha
    addi r5, r5, 0x9300007F@l
    isync
    mtspr 0x23c, r7
    mtspr 0x23d, r4
    mtspr 0x23c, r3
    isync
    mtspr 0x234, r7
    mtspr 0x235, r7
    isync
    mtspr 0x23e, r7
    mtspr 0x23f, r6
    mtspr 0x23e, r5
    isync
    mtspr 0x236, r7
    mtspr 0x237, r7
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}

static asm void ConfigMEM2_56MB() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x10000002@ha
    addi r4, r4, 0x10000002@l
    lis r3, 0x900003FF@ha
    addi r3, r3, 0x900003FF@l
    lis r6, 0x1000002A@ha
    addi r6, r6, 0x1000002A@l
    lis r5, 0xD00007FF@ha
    addi r5, r5, 0xD00007FF@l
    isync
    mtspr 0x238, r7
    mtspr 0x239, r4
    mtspr 0x238, r3
    isync
    mtspr 0x230, r7
    mtspr 0x231, r7
    isync
    mtspr 0x23a, r7
    mtspr 0x23b, r6
    mtspr 0x23a, r5
    isync
    mtspr 0x232, r7
    mtspr 0x233, r7
    isync
    lis r4, 0x12000002@ha
    addi r4, r4, 0x12000002@l
    lis r3, 0x920001FF@ha
    addi r3, r3, 0x920001FF@l
    lis r6, 0x13000002@ha
    addi r6, r6, 0x13000002@l
    lis r5, 0x930000FF@ha
    addi r5, r5, 0x930000FF@l
    isync
    mtspr 0x23c, r7
    mtspr 0x23d, r4
    mtspr 0x23c, r3
    isync
    mtspr 0x234, r7
    mtspr 0x235, r7
    isync
    mtspr 0x23e, r7
    mtspr 0x23f, r6
    mtspr 0x23e, r5
    isync
    mtspr 0x236, r7
    mtspr 0x237, r7
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}

static asm void ConfigMEM2_64MB() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x10000002@ha
    addi r4, r4, 0x10000002@l
    lis r3, 0x900007FF@ha
    addi r3, r3, 0x900007FF@l
    lis r6, 0x1000002A@ha
    addi r6, r6, 0x1000002A@l
    lis r5, 0xD00007FF@ha
    addi r5, r5, 0xD00007FF@l
    isync
    mtspr 0x238, r7
    mtspr 0x239, r4
    mtspr 0x238, r3
    isync
    mtspr 0x230, r7
    mtspr 0x231, r7
    isync
    mtspr 0x23a, r7
    mtspr 0x23b, r6
    mtspr 0x23a, r5
    isync
    mtspr 0x232, r7
    mtspr 0x233, r7
    isync
    mtspr 0x234, r7
    mtspr 0x235, r7
    isync
    mtspr 0x236, r7
    mtspr 0x237, r7
    isync
    mtspr 0x23c, r7
    mtspr 0x23d, r7
    isync
    mtspr 0x23e, r7
    mtspr 0x23f, r7
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}

static asm void ConfigMEM2_112MB() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x10000002@ha
    addi r4, r4, 0x10000002@l
    lis r3, 0x900007FF@ha
    addi r3, r3, 0x900007FF@l
    lis r6, 0x1000002A@ha
    addi r6, r6, 0x1000002A@l
    lis r5, 0xD0000FFF@ha
    addi r5, r5, 0xD0000FFF@l
    isync
    mtspr 0x238, r7
    mtspr 0x239, r4
    mtspr 0x238, r3
    isync
    mtspr 0x230, r7
    mtspr 0x231, r7
    isync
    mtspr 0x23a, r7
    mtspr 0x23b, r6
    mtspr 0x23a, r5
    isync
    mtspr 0x232, r7
    mtspr 0x233, r7
    isync
    lis r4, 0x14000002@ha
    addi r4, r4, 0x14000002@l
    lis r3, 0x940003FF@ha
    addi r3, r3, 0x940003FF@l
    lis r6, 0x16000002@ha
    addi r6, r6, 0x16000002@l
    lis r5, 0x960001FF@ha
    addi r5, r5, 0x960001FF@l
    isync
    mtspr 0x23c, r7
    mtspr 0x23d, r4
    mtspr 0x23c, r3
    isync
    mtspr 0x234, r7
    mtspr 0x235, r7
    isync
    mtspr 0x23e, r7
    mtspr 0x23f, r6
    mtspr 0x23e, r5
    isync
    mtspr 0x236, r7
    mtspr 0x237, r7
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}

static asm void ConfigMEM2_128MB() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x10000002@ha
    addi r4, r4, 0x10000002@l
    lis r3, 0x90000FFF@ha
    addi r3, r3, 0x90000FFF@l
    lis r6, 0x1000002A@ha
    addi r6, r6, 0x1000002A@l
    lis r5, 0xD0000FFF@ha
    addi r5, r5, 0xD0000FFF@l
    isync
    mtspr 0x238, r7
    mtspr 0x239, r4
    mtspr 0x238, r3
    isync
    mtspr 0x230, r7
    mtspr 0x231, r7
    isync
    mtspr 0x23a, r7
    mtspr 0x23b, r6
    mtspr 0x23a, r5
    isync
    mtspr 0x232, r7
    mtspr 0x233, r7
    isync
    mtspr 0x234, r7
    mtspr 0x235, r7
    isync
    mtspr 0x236, r7
    mtspr 0x237, r7
    isync
    mtspr 0x23c, r7
    mtspr 0x23d, r7
    isync
    mtspr 0x23e, r7
    mtspr 0x23f, r7
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}

#if SDK_VERSION < 20090224
static asm void ConfigMEM_ES1_0() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc

    li r7, 0
    lis r4, 0x00000002@ha
    addi r4, r4, 0x00000002@l
    lis r3, 0x80000FFF@ha
    addi r3, r3, 0x80000FFF@l
    isync
    mtdbatu 0, r7
    mtdbatl 0, r4
    mtdbatu 0, r3
    isync
    mtibatu 0, r7
    mtibatl 0, r4
    mtibatu 0, r3
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    mflr r3
    mtspr 0x1a, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}
#endif

static asm void DisableInstsOnMEM1Hi8MB() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    li r4, 0
    isync
    mtibatu 2, r4
    mtibatl 2, r4
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr SRR1, r3
    mflr r3
    mtspr SRR0, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}

static asm void DisableInstsOnMEM1Hi16MB() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc

    li r4, 0
    lis r5, 0x00000002@ha
    addi r5, r5, 0x00000002@l
    lis r6, 0x800000FF@ha
    addi r6, r6, 0x800000FF@l
    isync
    mtibatu 2, r4
    mtibatl 2, r4
    isync
    mtibatu 0, r4
    mtibatl 0, r5
    mtibatu 0, r6
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr SRR1, r3
    mflr r3
    mtspr SRR0, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}

static asm void EnableInstsOnMEM2Lo8MB() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    li r4, 0
    lis r5, 0x10000002@ha
    addi r5, r5, 0x10000002@l
    lis r6, 0x900000FF@ha
    addi r6, r6, 0x900000FF@l
    isync
    mtspr 560, r4
    mtspr 561, r5
    mtspr 560, r6
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr SRR1, r3
    mflr r3
    mtspr SRR0, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}

static asm void EnableInstsOnMEM2Lo16MB() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    li r4, 0
    lis r5, 0x10000002@ha
    addi r5, r5, 0x10000002@l
    lis r6, 0x900001FF@ha
    addi r6, r6, 0x900001FF@l
    isync
    mtspr 560, r4
    mtspr 561, r5
    mtspr 560, r6
    isync
    mfmsr r3
    ori r3, r3, 0x30
    mtspr SRR1, r3
    mflr r3
    mtspr SRR0, r3
    rfi
    // clang-format on
#endif  // __MWERKS__
}

static asm void RealMode(register u32 addr) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    clrlwi addr, addr, 2
    mtsrr0 addr
    mfmsr addr
    rlwinm addr, addr, 0, 28, 25
    mtsrr1 addr
    rfi
    // clang-format on
#endif  // __MWERKS__
}

void BATConfig(u32 arg0) {
    u32 mem1Size;
    u32 mem2Size;
    u32 mem2End;

    mem1Size = OSGetConsoleSimulatedMem1Size();
    if ((mem1Size < OSGetPhysicalMem1Size()) && ((u32)(mem1Size) == 0x01800000U)) {
        DCInvalidateRange((void*)0x81800000, 0x01800000);
        __MEMRegs[MEM_CONFIG] = 2;
    }
    if (mem1Size <= 0x01800000U) {
        RealMode((u32)ConfigMEM1_24MB);
    }
    mem2Size = OSGetConsoleSimulatedMem2Size();
    mem2End = *(u32*)OSPhysicalToCached(OS_ADDR_ACCESSIBLE_MEM2);
    if (mem2Size <= 0x04000000U) {
        if (mem2End <= 0x93400000U) {
            RealMode((u32)ConfigMEM2_52MB);
        } else if (mem2End <= 0x93800000U) {
            RealMode((u32)ConfigMEM2_56MB);
        } else {
            RealMode((u32)ConfigMEM2_64MB);
        }
    } else if (mem2Size <= 0x08000000U) {
        if (mem2End <= 0x97000000U) {
            RealMode((u32)ConfigMEM2_112MB);
        } else {
            RealMode((u32)ConfigMEM2_128MB);
        }
    }
    do {
    } while (arg0 != 0xBA2CF);
}

void OSDisableCodeExecOnMEM1Hi8MB() {
#if SDK_VERSION >= 20091211
    BOOL enabled;

    enabled = OSDisableInterrupts();
#endif
    RealMode((s32)DisableInstsOnMEM1Hi8MB);
#if SDK_VERSION >= 20091211
    OSRestoreInterrupts(enabled);
#endif
}

void OSDisableCodeExecOnMEM1Hi16MB() {
#if SDK_VERSION >= 20091211
    BOOL enabled;

    enabled = OSDisableInterrupts();
#endif
    RealMode((s32)DisableInstsOnMEM1Hi16MB);
#if SDK_VERSION >= 20091211
    OSRestoreInterrupts(enabled);
#endif
}

void __OSRestoreCodeExecOnMEM1(s32 arg0) {
    RealMode((s32)ConfigMEM1_24MB);
    while (arg0 != 0xBA2CF) {
    }
}

void OSEnableCodeExecOnMEM2Lo16MB() {
#if SDK_VERSION >= 20091211
    BOOL enabled;

    enabled = OSDisableInterrupts();
#endif
    RealMode((s32)EnableInstsOnMEM2Lo16MB);
#if SDK_VERSION >= 20091211
    OSRestoreInterrupts(enabled);
#endif
}

void OSEnableCodeExecOnMEM2Lo8MB() {
#if SDK_VERSION >= 20091211
    BOOL enabled;

    enabled = OSDisableInterrupts();
#endif
    RealMode((u32)EnableInstsOnMEM2Lo8MB);
#if SDK_VERSION >= 20091211
    OSRestoreInterrupts(enabled);
#endif
}

void __OSInitMemoryProtection() {
    static BOOL initialized;
    BOOL enabled;

    if (initialized) {
        return;
    }

    enabled = OSDisableInterrupts();

    __MEMRegs[MEM_INT_CLR] = 0;
    __MEMRegs[MEM_MARR_CONTROL] = 0xFF;

    __OSMaskInterrupts(OS_INTERRUPTMASK_MEM_0 | OS_INTERRUPTMASK_MEM_1 | OS_INTERRUPTMASK_MEM_2 | OS_INTERRUPTMASK_MEM_3);
    __OSSetInterruptHandler(__OS_INTERRUPT_MEM_0, MEMIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_MEM_1, MEMIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_MEM_2, MEMIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_MEM_3, MEMIntrruptHandler);
    __OSSetInterruptHandler(__OS_INTERRUPT_MEM_ADDRESS, MEMIntrruptHandler);
    OSRegisterShutdownFunction(&ShutdownFunctionInfo);
    BATConfig(0xBA2CF);
    __OSUnmaskInterrupts(OS_INTERRUPTMASK_MEM_ADDRESS);
    initialized = TRUE;
    OSRestoreInterrupts(enabled);
}
