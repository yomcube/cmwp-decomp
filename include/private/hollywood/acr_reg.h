#ifndef PRIVATE_HOLLYWOOD_ACR_REGISTERS_H
#define PRIVATE_HOLLYWOOD_ACR_REGISTERS_H

#define ACR_REGISTER_ADDR 0xCD000000
#define ACR_IOP_REGISTER_ADDR (ACR_REGISTER_ADDR | 0x00800000)

/* IPC */
#define HW_IPC_PPCMSG 0x0
#define HW_IPC_PPCCTRL 0x1
#define HW_IPC_ARMMSG 0x2
#define HW_IPC_ARMCTRL 0x3

/* IOP (Starlet) Timer */
#define HW_TIMER 0x4
#define HW_ALARM 0x5

/* Video related */
#define HW_VI1CFG 0x6
#define HW_VIDIM 0x7
#define HW_VISOLID 0x9

/* IRQ Controllers */
#define HW_PPCIRQFLAG 0xC
#define HW_PPCIRQMASK 0xD
#define HW_ARMIRQFLAG 0xE
#define HW_ARMIRQMASK 0xF
#define HW_ARMFIQMASK 0x10
#define HW_IOPINTPPC 0x11
#define HW_WDGINTSTS 0x12
#define HW_WDGCFG 0x13
#define HW_DMAADRINTSTS 0x14
#define HW_CPUADRINTSTS 0x15
#define HW_DBGINTSTS 0x16
#define HW_DBGINTEN 0x17

/* Bus controlling */
#define HW_SRNPROT 0x18
#define HW_AHBPROT 0x19

#define HW_I2CIOPINTEN 0x1A
#define HW_I2CIOPINTSTS 0x1B

#define HW_AIPPROT 0x1C
#define HW_AIPIOCTRL 0x1D

#define HW_VIINTEN 0x1E
#define HW_VIINTSTS 0x1F

/* USB related */
#define HW_USBDBG0 0x20
#define HW_USBDBG1 0x21
#define HW_USBFRCRST 0x22
#define HW_USBIOTEST 0x23

/* "Embedded logic-analyzer" stuff */
#define HW_ELA_REG_ADDR 0x24
#define HW_ELA_REG_DATA 0x25

#define HW_MEMTSTN 0x26
#define HW_MEMTSTP 0x27

/* Hollywood GPIOs */
#define HW_GPIOB_OUT 0x30
#define HW_GPIOB_OUT_SENSOR_BAR (1 << 8)

#define HW_GPIOB_DIR 0x31
#define HW_GPIOB_IN 0x32
#define HW_GPIOB_INTLVL 0x33
#define HW_GPIOB_INTFLAG 0x34
#define HW_GPIOB_INTMASK 0x35
#define HW_GPIOB_STRAPS 0x36
#define HW_GPIO_ENABLE 0x37
#define HW_GPIO_OUT 0x38
#define HW_GPIO_DIR 0x39
#define HW_GPIO_IN 0x3A
#define HW_GPIO_INTLVL 0x3B
#define HW_GPIO_INTFLAG 0x3C
#define HW_GPIO_INTMASK 0x3D
#define HW_GPIO_STRAPS 0x3E
#define HW_GPIO_OWNER 0x3F

#define HW_GPIOPIN_EJECT_DISC (1 << 9)

/* AHB related? */
#define HW_ARB_CFG_M0 0x40
#define HW_ARB_CFG_M1 0x41
#define HW_ARB_CFG_M2 0x42
#define HW_ARB_CFG_M3 0x43
#define HW_ARB_CFG_M4 0x44
#define HW_ARB_CFG_M5 0x45
#define HW_ARB_CFG_M6 0x46
#define HW_ARB_CFG_M7 0x47
#define HW_ARB_CFG_M8 0x48
#define HW_ARB_CFG_M9 0x49
#define HW_ARB_CFG_MA 0x4A /* \ Not documented anywhere but assuming they exist  */
#define HW_ARB_CFG_MB 0x4B /* /                                                  */
#define HW_ARB_CFG_MC 0x4C
#define HW_ARB_CFG_MD 0x4D
#define HW_ARB_CFG_ME 0x4E
#define HW_ARB_CFG_MF 0x4F
#define HW_ARB_CFG_CPU 0x50
#define HW_ARB_CFG_DMA 0x51
#define HW_ARB_PCNTCFG 0x52
#define HW_ARB_PCNTSTS 0x53

#define HW_I2CSCTRL 0x54
#define HW_I2CSSTS 0x55
#define HW_I2CSRDEN 0x56
#define HW_I2CSTRAP 0x58
#define HW_I2CSCTRL_MIRROR 0x59 /* Apparently */
#define HW_I2CSVISETYUV 0x5A
#define HW_I2CSVISETFILT 0x5B

#define HW_SPARE2 0x5C
#define HW_SPARE3 0x5D

/* Some system related stuff */
#define HW_COMPAT 0x60
#define HW_RESET_AHB 0x61
#define HW_SPARE0 0x62
#define HW_BOOT0 0x63
#define HW_CLOCKS 0x64
#define HW_RESETS 0x65
#define HW_IFPOWER 0x66

/* PLL/Clock related(?) */
#define HW_PLLDR 0x67
#define HW_PLLSYSEXT1 0x68
#define HW_PLLSYSEXT2 0x69
#define HW_PLLAIEXT1 0x6A
#define HW_PLLATEXT2 0x6B
#define HW_PLLSYS 0x6C
#define HW_PLLSYSEXT 0x6D
#define HW_PLLDSK 0x6E
#define HW_PLLDDR 0x6F
#define HW_PLLDDREXT 0x70
#define HW_PLLVI 0x71
#define HW_PLLVIEXT 0x72
#define HW_PLLAI 0x73

#define HW_PLLAIEXT 0x74
#define HW_PLLAIEXT_PLL_A (1 << 30)
#define HW_PLLAIEXT_PLL_B (1 << 31)

#define HW_PLLUSB 0x75
#define HW_PLLUSBEXT 0x76

#define HW_IOPWRCTRL 0x77

/* More clock related? */
#define HW_IOSTRCTRL0 0x78
#define HW_IOSTRCTRL1 0x79
#define HW_CLKSTRCTRL 0x7A

/* OTP related */
#define HW_OTPCMD 0x7B
#define HW_OTPDATA 0x7C

/* Debug related */
#define HW_DBGCLK 0x7D
#define HW_OBSCLKOCTRL 0x7E
#define HW_OBSCLKICTRL 0x7F
#define HW_DBGPORT 0x80

/* SI Related */
#define HW_SICLKDIV 0x81
#define HW_SICTRL 0x82
#define HW_SIDATA 0x83
#define HW_SIINT 0x84

#define HW_VERSION 0x85

#define HW_DBGBUSRD 0x86

#endif  // PRIVATE_HOLLYWOOD_ACR_REGISTERS_H
