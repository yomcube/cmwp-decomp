#ifndef PRIVATE_HOLLYWOOD_FLIPPER_H
#define PRIVATE_HOLLYWOOD_FLIPPER_H

#include <revolution/types.h>

#include <private/hollywood/acr_reg.h>
#include <private/hollywood/ai_reg.h>
#include <private/hollywood/di_reg.h>
#include <private/hollywood/dsp_reg.h>
#include <private/hollywood/exi_reg.h>
#include <private/hollywood/mem_reg.h>
#include <private/hollywood/pi_reg.h>
#include <private/hollywood/si_reg.h>
#include <private/hollywood/vi_reg.h>

vu16 __VIRegs[59] AT_ADDRESS(VI_REGISTER_ADDR);
vu32 __PIRegs[12] AT_ADDRESS(PI_REGISTER_ADDR);
vu16 __MEMRegs[64] AT_ADDRESS(MEM_REGISTER_ADDR);
vu16 __DSPRegs[32] AT_ADDRESS(DSP_REGISTER_ADDR);
vu32 __DIRegs[16] AT_ADDRESS(DI_REGISTER_ADDR);
vu32 __SIRegs[256] AT_ADDRESS(SI_REGISTER_ADDR);
vu32 __EXIRegs[64] AT_ADDRESS(EXI_REGISTER_ADDR);
vu32 __AIRegs[8] AT_ADDRESS(AI_REGISTER_ADDR);
vu32 __ACRRegs[256] AT_ADDRESS(ACR_REGISTER_ADDR);
vu32 __ACRIOPRegs[256] AT_ADDRESS(ACR_IOP_REGISTER_ADDR);

#endif  // PRIVATE_HOLLYWOOD_FLIPPER_H
