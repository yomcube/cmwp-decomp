#include <revolution/base/PPCArch.h>

#include <revolution/os.h>

asm u32 PPCMfmsr() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfmsr r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtmsr(register u32 newMSR) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtmsr newMSR
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCOrMsr(register u32 value) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfmsr r4
    or value, r4, value
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCAndMsr(register u32 value) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfmsr r4
    and value, r4, value
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCAndCMsr(register u32 value) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfmsr r4
    andc value, r4, value
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfhid0() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, HID0
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMthid0(register u32 newHID0) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr HID0, newHID0
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfhid1() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, HID1
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfl2cr() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, L2CR
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtl2cr(register u32 newL2cr) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr L2CR, newL2cr
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtdec(register u32 newDec) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtdec newDec
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfdec() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfdec r3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCSync() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    sc
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCEieio() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfmsr r5
    rlwinm r6, r5, 0, 17, 15
    mtmsr r6
    mfspr r3, HID0
    ori r4, r3, 0x8
    mtspr HID0, r4
    isync
    eieio
    isync
    mtspr HID0, r3
    mtmsr r5
    isync
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCHalt() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    sync
loop:
    nop
    li r3, 0
    nop
    b loop
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfmmcr0() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, MMCR0
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtmmcr0(register u32 newMmcr0) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr MMCR0, newMmcr0
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfmmcr1() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, MMCR1
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtmmcr1(register u32 newMmcr1) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr MMCR1, newMmcr1
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfpmc1() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, PMC1
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtpmc1(register u32 newPmc1) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr PMC1, newPmc1
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfpmc2() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, PMC2
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtpmc2(register u32 newPmc2) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr PMC2, newPmc2
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfpmc3() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, PMC3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtpmc3(register u32 newPmc3) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr PMC3, newPmc3
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfpmc4() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, PMC4
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtpmc4(register u32 newPmc4) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr PMC4, newPmc4
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfsia() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, SIA
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtsia(register u32 newSia){
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr SIA, newSia
    blr
// clang-format on
#endif  // __MWERKS__
}

u32 PPCMffpscr() {
    union FpscrUnion m;

    asm {
#ifdef __MWERKS__
        // clang-format off
        mffs fp31
        stfd fp31, m.f;
    // clang-format on
#endif  // __MWERKS__
    }

    return m.u.fpscr;
}

void PPCMtfpscr(register u32 newFPSCR) {
    union FpscrUnion m;

    asm {
#ifdef __MWERKS__
        // clang-format off
        li    r4, 0
        stw   r4, m.u.fpscr_pad;
        stw   newFPSCR, m.u.fpscr
        lfd   fp31, m.f
        mtfsf 0xff, fp31
    // clang-format on
#endif  // __MWERKS__
    }
}

asm u32 PPCMfhid2() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, HID2
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMthid2(register u32 newhid2) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr HID2, newhid2
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfwpar() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    sync
    mfspr r3, WPAR
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtwpar(register u32 newwpar) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr WPAR, newwpar
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfdmaU() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, DMA_U
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfdmaL() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, DMA_L
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtdmaU(register u32 newdmau) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr DMA_U, newdmau
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCMtdmaL(register u32 newdmal) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mtspr DMA_L, newdmal
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfpvr() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, PVR
    blr
    // clang-format on
#endif  // __MWERKS__
}

void PPCEnableSpeculation() {
    PPCMthid0(PPCMfhid0() & ~HID0_SPD);
}

void PPCDisableSpeculation() {
    PPCMthid0(PPCMfhid0() | HID0_SPD);
}

asm void PPCSetFpIEEEMode() {
#ifdef __MWERKS__
    // clang-format off
	nofralloc
	mtfsb0 29
	blr
    // clang-format on
#endif  // __MWERKS__
}

asm void PPCSetFpNonIEEEMode() {
#ifdef __MWERKS__
    // clang-format off
	nofralloc
	mtfsb1 29
	blr
    // clang-format on
#endif  // __MWERKS__
}

asm u32 PPCMfhid4() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfspr r3, HID4
    blr
    // clang-format on
#endif  // __MWERKS__
}

#ifdef __MWERKS__
// clang-format off
#define PPCMthid4_(newHID4) \
    asm {                   \
        mtspr HID4, newHID4 \
    }
#else
#define PPCMthid4_(newHID4)
#endif

void PPCMthid4(register u32 newHID4) {
    if (newHID4 & HID4_H4A) {
        PPCMthid4_(newHID4)
    } else {
        OSReport("H4A should not be cleared because of Broadway errata.\n");
        newHID4 |= HID4_H4A;
        PPCMthid4_(newHID4)
    }
}
