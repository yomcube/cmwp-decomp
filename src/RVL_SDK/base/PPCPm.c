#include <revolution/base/PPCPm.h>

#include <revolution/base/PPCArch.h>

void PMBegin() {
    PPCMtmmcr0(0);
    PPCMtmmcr1(0);
    PPCMtpmc1(0);
    PPCMtpmc2(0);
    PPCMtpmc3(0);
    PPCMtpmc4(0);
    PPCMtmmcr0(MMCR0_PMC1_CYCLE | MMCR0_PMC2_CYCLE | MMCR0_PMC2_INSTRUCTION | MMCR0_PMC2_DISPATCHED | MMCR0_PMC2_Bx_FALL_TROUGH);
    PPCMtmmcr1(MMCR1_PMC3_L1_MISS_CYCLE | MMCR1_PMC4_INSTRUCTION);
}

void PMEnd() {
    PPCMtmmcr0(0);
    PPCMtmmcr1(0);
}

void PMCycles() {
    PPCMfpmc1();
}

void PML1FetchMisses() {
    PPCMfpmc2();
}

void PML1MissCycles() {
    PPCMfpmc3();
}

void PMInstructions() {
    PPCMfpmc4();
}
