#ifndef PRIVATE_HOLLYWOOD_EXI_REGISTERS_H
#define PRIVATE_HOLLYWOOD_EXI_REGISTERS_H

#define EXI_REGISTER_ADDR 0xCD006800

#define EXI_CHAN_PARAM 0x0
#define EXI_MEM_ADDRESS 0x1
#define EXI_LENGTH 0x2
#define EXI_CONTROL 0x3
#define EXI_DATA 0x4

#define EXI_REG_REAL_SIZE 0x0014
#define EXI_REG_SIZE 0x5

#define EXI_0_CHAN_PARAM 0x0
#define EXI_0_MEM_ADDRESS 0x1
#define EXI_0_LENGTH 0x2
#define EXI_0_CONTROL 0x3
#define EXI_0_DATA 0x4

#define EXI_1_CHAN_PARAM 0x5
#define EXI_1_MEM_ADDRESS 0x6
#define EXI_1_LENGTH 0x7
#define EXI_1_CONTROL 0x8
#define EXI_1_DATA 0x9

#define EXI_2_CHAN_PARAM 0xA
#define EXI_2_MEM_ADDRESS 0xB
#define EXI_2_LENGTH 0xC
#define EXI_2_CONTROL 0xD
#define EXI_2_DATA 0xE

#define EXI_CHAN_PARAM_INTMASK (1 << 0)
#define EXI_CHAN_PARAM_INT (1 << 1)
#define EXI_CHAN_PARAM_TCINTMASK (1 << 2)
#define EXI_CHAN_PARAM_TCINT (1 << 3)
#define EXI_CHAN_PARAM_CLK (1 << 4)
#define EXI_CHAN_PARAM_CS (1 << 7)
#define EXI_CHAN_PARAM_EXTINTMASK (1 << 10)
#define EXI_CHAN_PARAM_EXTINT (1 << 11)
#define EXI_CHAN_PARAM_EXT (1 << 12)
#define EXI_CHAN_PARAM_ROMDIS (1 << 13) /* For Gekko, it's the GC's IPL. But for hollywood, this is unknown. (maybe unused?) */

#define EXI_CONTROL_TSTART (1 << 0)
#define EXI_CONTROL_DMA (1 << 1)
#define EXI_CONTROL_RW (1 << 2)
#define EXI_CONTROL_TLEN (1 << 4)

#endif  // PRIVATE_HOLLYWOOD_EXI_REGISTERS_H
