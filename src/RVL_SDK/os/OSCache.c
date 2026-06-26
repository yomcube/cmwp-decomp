#include <revolution/os.h>

#include <revolution/base.h>
#include <revolution/db.h>

#define HID2 920

// prototypes
void DMAErrorHandler(OSError error, OSContext* context, ...);

// clang-format off
asm void DCFlashInvalidate() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, HID0
    ori r3, r3, 0x400
    mtspr HID0, r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCEnable() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    sync
    mfspr r3, HID0
    ori   r3, r3, 0x4000
    mtspr HID0, r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCDisable() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    sync
    mfspr r3, HID0
    rlwinm r3, r3, 0, 18, 16
    mtspr HID0, r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCFreeze() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    sync
    mfspr r3, HID0
    ori r3, r3, 0x1000
    mtspr HID0, r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCUnfreeze() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, HID0
    rlwinm r3, r3, 0, 20, 18
    mtspr HID0, r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCTouchLoad(register void* addr) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    dcbt r0, addr
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCBlockZero(register void* addr) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    dcbz r0, addr
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCBlockStore(register void* addr) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    dcbst r0, addr
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCBlockFlush(register void* addr) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    dcbf r0, addr
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCBlockInvalidate(register void* addr) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    dcbi r0, addr
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCInvalidateRange(register void* addr, register u32 nBytes) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    cmplwi nBytes, 0
    blelr
    clrlwi r5, addr, 27
    add    nBytes, nBytes, r5
    addi   nBytes, nBytes, 31
    srwi   nBytes, nBytes, 5
    mtctr  nBytes

@1
    dcbi r0, addr
    addi addr, addr, 32
    bdnz @1
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCFlushRange(register void* addr, register u32 nBytes) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    cmplwi nBytes, 0
    blelr
    clrlwi r5, addr, 27
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 31
    srwi nBytes, nBytes, 5
    mtctr nBytes

@1
    dcbf r0, addr
    addi addr, addr, 32
    bdnz @1
    sc
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCStoreRange(register void* addr, register u32 nBytes) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    cmplwi nBytes, 0
    blelr
    clrlwi r5, addr, 27
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 31
    srwi nBytes, nBytes, 5
    mtctr nBytes

@1
    dcbst r0, addr
    addi addr, addr, 32
    bdnz @1
    sc

    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCFlushRangeNoSync(register void* addr, register u32 nBytes) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    cmplwi nBytes, 0
    blelr
    clrlwi r5, addr, 27
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 31
    srwi nBytes, nBytes, 5
    mtctr nBytes

@1
    dcbf r0, addr
    addi addr, addr, 32
    bdnz @1
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCStoreRangeNoSync(register void* addr, register u32 nBytes) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    cmplwi nBytes, 0
    blelr
    clrlwi r5, addr, 27
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 31
    srwi nBytes, nBytes, 5
    mtctr nBytes

@1
    dcbst r0, addr
    addi addr, addr, 32
    bdnz @1

    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCZeroRange(register void* addr, register u32 nBytes) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    cmplwi nBytes, 0
    blelr
    clrlwi r5, addr, 27
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 31
    srwi nBytes, nBytes, 5
    mtctr nBytes

@1
    dcbz r0, addr
    addi addr, addr, 32
    bdnz @1

    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void DCTouchRange(register void* addr, register u32 nBytes) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    cmplwi nBytes, 0
    blelr
    clrlwi r5, addr, 27
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 31
    srwi nBytes, nBytes, 5
    mtctr nBytes

@1
    dcbt r0, addr
    addi addr, addr, 32
    bdnz @1

    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void ICInvalidateRange(register void* addr, register u32 nBytes) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    cmplwi nBytes, 0
    blelr
    clrlwi r5, addr, 27
    add nBytes, nBytes, r5
    addi nBytes, nBytes, 31
    srwi nBytes, nBytes, 5
    mtctr nBytes

@1
    icbi r0, addr
    addi addr, addr, 32
    bdnz @1
    sync
    isync

    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void ICFlashInvalidate() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, HID0
    ori r3, r3, 0x800
    mtspr HID0, r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void ICEnable() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    isync
    mfspr r3, HID0
    ori r3, r3, 0x8000
    mtspr HID0, r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void ICDisable() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    isync
    mfspr r3, HID0
    rlwinm r3, r3, 0, 17, 15
    mtspr HID0, r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void ICFreeze() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    isync
    mfspr r3, HID0
    ori r3, r3, 0x2000
    mtspr HID0, r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void ICUnfreeze() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, HID0
    rlwinm r3, r3, 0, 19, 17
    mtspr HID0, r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void ICBlockInvalidate(register void* addr) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    icbi r0, addr
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void ICSync() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    isync
    blr
    // clang-format on
#endif  // __MWERKS__
}

#define LC_LINES 512
#define CACHE_LINES 1024

static asm void __LCEnable() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfmsr   r5
    ori     r5, r5, 0x1000
    mtmsr   r5

    lis     r3, OS_CACHED_REGION_PREFIX
    li      r4, CACHE_LINES
    mtctr   r4
_touchloop:
    dcbt    0,r3
    dcbst   0,r3
    addi    r3,r3,32
    bdnz    _touchloop
    mfspr   r4, HID2
    oris    r4, r4, 0x100F
    mtspr   HID2, r4

    nop 
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    lis     r3, LC_BASE_PREFIX
    ori     r3, r3, 0x0002
    mtspr   DBAT3L, r3
    ori     r3, r3, 0x01FE
    mtspr   DBAT3U, r3
    isync
    lis     r3, LC_BASE_PREFIX
    li      r6, LC_LINES
    mtctr   r6
    li      r6, 0

_lockloop:
    dcbz_l  r6, r3
    addi    r3, r3, 32
    bdnz+    _lockloop

    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

    blr
    // clang-format on
#endif  // __MWERKS__
}

void LCEnable() {
#ifdef __MWERKS__
    // clang-format off
    BOOL enabled;

    enabled = OSDisableInterrupts();
    __LCEnable();
    OSRestoreInterrupts(enabled);
    // clang-format on
#endif  // __MWERKS__
}

asm void LCDisable() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    lis     r3, LC_BASE_PREFIX
    li      r4, LC_LINES
    mtctr r4
@1
    dcbi r0, r3
    addi r3, r3, 32
    bdnz @1
    mfspr r4, HID2
    rlwinm r4, r4, 0, 4, 2
    mtspr HID2, r4
    blr
    // clang-format on
#endif  // __MWERKS__
}

BOOL LCIsEnable() {
    u32 value = PPCMfhid2();
    BOOL result;
    if (value & HID2_LCE) {
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

asm void LCAllocOneTag(register BOOL invalidate, register void* tag) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    cmpwi invalidate, 0
    beq @1
    dcbi r0, tag
@1
    dcbz_l r0, tag
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void LCAllocTags(register BOOL invalidate, register void* startTag, register u32 numBlocks) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mflr r6
    cmplwi numBlocks, 0
    ble @3
    mtctr numBlocks
    cmpwi invalidate, 0
    beq @2
@1
    dcbi r0, startTag
    dcbz_l r0, startTag
    addi startTag, startTag, 32
    bdnz @1
    b @3
@2
    dcbz_l r0, startTag
    addi startTag, startTag, 32
    bdnz @2
@3
    mtlr r6
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void LCLoadBlocks(register void* destTag, register void* srcAddr, register u32 numBlocks) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    rlwinm  r6, numBlocks, 30, 27, 31
    rlwinm  srcAddr, srcAddr, 0, 3, 31
    or      r6, r6, srcAddr
    mtspr   DMA_U, r6
    rlwinm  r6, numBlocks, 2, 28, 29
    or      r6, r6, destTag
    ori     r6, r6, 0x12
    mtspr   DMA_L, r6
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void LCStoreBlocks(register void* destAddr, register void* srcTag, register u32 numBlocks) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    rlwinm  r6, numBlocks, 30, 27, 31
    rlwinm  destAddr, destAddr, 0, 3, 31
    or      r6, r6, destAddr
    mtspr   DMA_U, r6
    rlwinm  r6, numBlocks, 2, 28, 29
    or      r6, r6, srcTag
    ori     r6, r6, 0x2
    mtspr   DMA_L, r6
    blr
    // clang-format on
#endif  // __MWERKS__
}

void LCAlloc(void* addr, u32 nBytes) {
    u32 numBlocks = nBytes >> 5;
    u32 hid2 = PPCMfhid2();

    ASSERTMSGLINE(1359, !((u32)addr & 31), "LCAlloc(): addr must be 32 byte aligned");
    ASSERTMSGLINE(1361, !((u32)nBytes & 31), "LCAlloc(): nBytes must be 32 byte aligned");

    if ((hid2 & HID2_LCE) == 0) {
        LCEnable();
    }
    LCAllocTags(TRUE, addr, numBlocks);
}

void LCAllocNoInvalidate(void* addr, u32 nBytes) {
    u32 numBlocks = nBytes >> 5;
    u32 hid2 = PPCMfhid2();

    ASSERTMSGLINE(1406, !((u32)addr & 31), "LCAllocNoFlush(): addr must be 32 byte aligned");
    ASSERTMSGLINE(1408, !((u32)nBytes & 31), "LCAllocNoFlush(): nBytes must be 32 byte aligned");

    if ((hid2 & HID2_LCE) == 0) {
        LCEnable();
    }
    LCAllocTags(FALSE, addr, numBlocks);
}

u32 LCLoadData(void* destAddr, void* srcAddr, u32 nBytes) {
    u32 numBlocks = (nBytes + 31) / 32;
    u32 numTransactions = (numBlocks + 128 - 1) / 128;

    ASSERTMSGLINE(1466, !((u32)srcAddr & 31), "LCLoadData(): srcAddr not 32 byte aligned");
    ASSERTMSGLINE(1468, !((u32)destAddr & 31), "LCLoadData(): destAddr not 32 byte aligned");

    while (numBlocks > 0) {
        if (numBlocks < 128) {
            LCLoadBlocks(destAddr, srcAddr, numBlocks);
            numBlocks = 0;
        } else {
            LCLoadBlocks(destAddr, srcAddr, 0);
            numBlocks -= 128;
            destAddr = (void*)((u32)destAddr + 4096);
            srcAddr = (void*)((u32)srcAddr + 4096);
        }
    }

    return numTransactions;
}

u32 LCStoreData(void* destAddr, void* srcAddr, u32 nBytes) {
    u32 numBlocks = (nBytes + 31) / 32;
    u32 numTransactions = (numBlocks + 128 - 1) / 128;

    ASSERTMSGLINE(1534, !((u32)srcAddr & 31), "LCStoreData(): srcAddr not 32 byte aligned");
    ASSERTMSGLINE(1536, !((u32)destAddr & 31), "LCStoreData(): destAddr not 32 byte aligned");

    while (numBlocks > 0) {
        if (numBlocks < 128) {
            LCStoreBlocks(destAddr, srcAddr, numBlocks);
            numBlocks = 0;
        } else {
            LCStoreBlocks(destAddr, srcAddr, 0);
            numBlocks -= 128;
            destAddr = (void*)((u32)destAddr + 4096);
            srcAddr = (void*)((u32)srcAddr + 4096);
        }
    }

    return numTransactions;
}

asm u32 LCQueueLength() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr   r4, HID2
    rlwinm  r3, r4, 8, 28, 31
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void LCQueueWait(register u32 len) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
@1
    mfspr r4, HID2
    rlwinm r4, r4, 8, 28, 31
    cmpw r4, r3
    bgt @1
    blr
    // clang-format on
#endif  // __MWERKS__
}

void LCFlushQueue() {
    union {
        u32 val;
        struct {
            u32 lcAddr : 27;
            u32 dmaLd : 1;
            u32 dmaLenL : 2;
            u32 dmaTrigger : 1;
            u32 dmaFlush : 1;
        } f;
    } dmaL;

    dmaL.val = 0;
    dmaL.f.dmaFlush = 1;
    PPCMtdmaU(0);
    PPCMtdmaL(dmaL.val);
    PPCSync();
}

static void L2Init() {
    u32 oldMSR;
    oldMSR = PPCMfmsr();
    __sync();
    PPCMtmsr(MSR_IR | MSR_DR);
    __sync();
    L2Disable();
    L2GlobalInvalidate();
    PPCMtmsr(oldMSR);
}

void L2Enable() {
    PPCMtl2cr((PPCMfl2cr() | L2CR_L2E) & ~L2CR_L2I);
}

void L2Disable() {
    __sync();
    PPCMtl2cr(PPCMfl2cr() & ~L2CR_L2E);
    __sync();
}

void L2GlobalInvalidate() {
    L2Disable();
    PPCMtl2cr(PPCMfl2cr() | L2CR_L2I);
    while (PPCMfl2cr() & L2CR_L2IP) {
    }

    PPCMtl2cr(PPCMfl2cr() & ~L2CR_L2I);
    while (PPCMfl2cr() & L2CR_L2IP) {
        DB_PRINT(">>> L2 INVALIDATE : SHOULD NEVER HAPPEN\n");
    }
}

void L2SetDataOnly(BOOL dataOnly) {
    if (dataOnly) {
        PPCMtl2cr(PPCMfl2cr() | L2CR_L2DO);
        return;
    }
    PPCMtl2cr(PPCMfl2cr() & ~L2CR_L2DO);
}

void L2SetWriteThrough(BOOL writeThrough) {
    if (writeThrough) {
        PPCMtl2cr(PPCMfl2cr() | L2CR_L2WT);
        return;
    }
    PPCMtl2cr(PPCMfl2cr() & ~L2CR_L2WT);
}

void DMAErrorHandler(OSError error, OSContext* context, ...) {
    u32 hid2 = PPCMfhid2();

    OSReport("Machine check received\n");
    OSReport("HID2 = 0x%x   SRR1 = 0x%x\n", hid2, context->srr1);
    if (!(hid2 & (HID2_DCHERR | HID2_DNCERR | HID2_DCMERR | HID2_DQOERR)) || !(context->srr1 & SRR1_DMA_BIT)) {
        OSReport("Machine check was not DMA/locked cache related\n");
        OSDumpContext(context);
        PPCHalt();
    }

    OSReport("DMAErrorHandler(): An error occurred while processing DMA.\n");
    OSReport("The following errors have been detected and cleared :\n");

    if (hid2 & HID2_DCHERR) {
        OSReport("\t- Requested a locked cache tag that was already in the cache\n");
    }

    if (hid2 & HID2_DNCERR) {
        OSReport("\t- DMA attempted to access normal cache\n");
    }

    if (hid2 & HID2_DCMERR) {
        OSReport("\t- DMA missed in data cache\n");
    }

    if (hid2 & HID2_DQOERR) {
        OSReport("\t- DMA queue overflowed\n");
    }

    // write hid2 back to clear the error bits
    PPCMthid2(hid2);
}

void __OSCacheInit() {
    if (!(PPCMfhid0() & HID0_ICE)) {
        ICEnable();
        DB_PRINT("L1 i-caches initialized\n");
    }

    if (!(PPCMfhid0() & HID0_DCE)) {
        DCEnable();
        DB_PRINT("L1 d-caches initialized\n");
    }

    if (!(PPCMfl2cr() & L2CR_L2E)) {
        L2Init();
        L2Enable();
        DB_PRINT("L2 cache initialized\n");
    }

    OSSetErrorHandler(OS_ERROR_MACHINE_CHECK, DMAErrorHandler);

    DB_PRINT("Locked cache machine check handler installed\n");
}
