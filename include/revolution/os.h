#ifndef REVOLUTION_OS_H
#define REVOLUTION_OS_H

#include <decomp.h>

#include <revolution/types.h>

#include <revolution/os/OSMemMap.h>
#include <revolution/os/OSTime.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**************************/
/* UNCACHED/CACHED MEMORY */
/**************************/

#define OSRoundUp32B(x) ROUNDUP(x, DEFAULT_ALIGN)
#define OSRoundDown32B(x) ROUNDDOWN(x, DEFAULT_ALIGN)
#define OSIsAligned32B(x) IS_ALIGNED(x, DEFAULT_ALIGN)

#define OS_CACHED_REGION_PREFIX 0x8000
#define OS_UNCACHED_REGION_PREFIX 0xC000
#define OS_PHYSICAL_MASK 0x3FFF

#define OS_BASE_CACHED (OS_CACHED_REGION_PREFIX << 16)
#define OS_BASE_UNCACHED (OS_UNCACHED_REGION_PREFIX << 16)

void* OSPhysicalToCached(u32 paddr);
void* OSPhysicalToUncached(u32 paddr);
u32 OSCachedToPhysical(void* caddr);
u32 OSUncachedToPhysical(void* ucaddr);
void* OSCachedToUncached(void* caddr);
void* OSUncachedToCached(void* ucaddr);

#ifndef DEBUG
#define OSPhysicalToCached(paddr) ((void*)((u32)(paddr) + OS_BASE_CACHED))
#define OSPhysicalToUncached(paddr) ((void*)((u32)(paddr) + OS_BASE_UNCACHED))
#define OSCachedToPhysical(caddr) ((u32)((u8*)(caddr) - OS_BASE_CACHED))
#define OSUncachedToPhysical(ucaddr) ((u32)((u8*)(ucaddr) - OS_BASE_UNCACHED))
#define OSCachedToUncached(caddr) ((void*)((u8*)(caddr) + (OS_BASE_UNCACHED - OS_BASE_CACHED)))
#define OSUncachedToCached(ucaddr) ((void*)((u8*)(ucaddr) - (OS_BASE_UNCACHED - OS_BASE_CACHED)))
#endif

/****************/
/* CONSOLE TYPE */
/****************/

#define OS_CONSOLE_MASK (0xF000 << 16)
#define OS_CONSOLE_MASK_RVL (0x0000 << 16)
#define OS_CONSOLE_MASK_DEV (0x1000 << 16)
#define OS_CONSOLE_MASK_TDEV (0x2000 << 16)

#define OS_CONSOLE_RVL_PP_0 (OS_CONSOLE_MASK_RVL + 0x0010)
#define OS_CONSOLE_RVL_PP_1 (OS_CONSOLE_MASK_RVL + 0x0011)
#define OS_CONSOLE_RVL_PP_2_1 (OS_CONSOLE_MASK_RVL + 0x0012)
#define OS_CONSOLE_RVL_PP_2_2 (OS_CONSOLE_MASK_RVL + 0x0020)
#define OS_CONSOLE_RETAIL (OS_CONSOLE_MASK_RVL + 0x0021)
#define OS_CONSOLE_RETAIL_RVA (OS_CONSOLE_MASK_RVL + 0x0100)

#define OS_CONSOLE_UNK (OS_CONSOLE_MASK_DEV + 0x0002)
#define OS_CONSOLE_RVL_EMU (OS_CONSOLE_MASK_DEV + 0x0008)
#define OS_CONSOLE_NDEV_1_0 (OS_CONSOLE_MASK_DEV + 0x0010)
#define OS_CONSOLE_NDEV_1_1 (OS_CONSOLE_MASK_DEV + 0x0011)
#define OS_CONSOLE_NDEV_1_2 (OS_CONSOLE_MASK_DEV + 0x0012)
#define OS_CONSOLE_NDEV_2_0 (OS_CONSOLE_MASK_DEV + 0x0020)
#define OS_CONSOLE_NDEV_2_1 (OS_CONSOLE_MASK_DEV + 0x0021)

#define OS_CONSOLE_TDEV_EMU (OS_CONSOLE_MASK_TDEV + 0x0000)

u32 OSGetConsoleType();

/********************/
/* TICKS CONVERSION */
/********************/

u32 __OSBusClock AT_ADDRESS(OS_BASE_CACHED | OS_ADDR_BUS_CLOCK_SPEED);
u32 __OSCoreClock AT_ADDRESS(OS_BASE_CACHED | OS_ADDR_CPU_CLOCK_SPEED);

#define OS_BUS_CLOCK __OSBusClock
#define OS_CORE_CLOCK __OSCoreClock
#define OS_TIMER_CLOCK (OS_BUS_CLOCK / 4)

// Time base frequency = 1 / 4 bus clock
#define OS_TIME_SPEED (OS_BUS_CLOCK / 4)

// OS time -> Real time
#define OS_TICKS_TO_SEC(x) ((x) / (OS_TIME_SPEED))
#define OS_TICKS_TO_MSEC(x) ((x) / (OS_TIME_SPEED / 1000))
#define OS_TICKS_TO_USEC(x) (((x) * 8) / (OS_TIME_SPEED / 125000))
#define OS_TICKS_TO_NSEC(x) (((x) * 8000) / (OS_TIME_SPEED / 125000))

#define OSTicksToSeconds(ticks) ((ticks) / (OS_TIMER_CLOCK))
#define OSTicksToMilliseconds(ticks) ((ticks) / (OS_TIMER_CLOCK / 1000))
#define OSTicksToMicroseconds(ticks) ((ticks) * 8 / (OS_TIMER_CLOCK / 125000))
#define OSTicksToNanoseconds(ticks) ((ticks) * 8000 / (OS_TIMER_CLOCK / 125000))

// Real time -> OS time
#define OS_SEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED))
#define OS_MSEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED / 1000))
#define OS_USEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED / 125000) / 8)
#define OS_NSEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED / 125000) / 8000)

#define OSSecondsToTicks(sec) ((sec) * (OS_TIMER_CLOCK))
#define OSMillisecondsToTicks(msec) ((msec) * (OS_TIMER_CLOCK / 1000))
#define OSNanosecondsToTicks(nsec) (((nsec) * (OS_TIMER_CLOCK / 125000)) / 8000)
#define OSMicrosecondsToTicks(usec) (((usec) * (OS_TIMER_CLOCK / 125000)) / 8)

/*****************/
/* DEBUG ASSERTS */
/*****************/

#ifdef DEBUG
#define ASSERTLINE(line, cond) ((cond) || (OSPanic(__FILE__, line, "Failed assertion " #cond), 0))
#define ASSERTMSGLINE(line, cond, ...) ((cond) || (OSPanic(__FILE__, line, __VA_ARGS__), 0))
#else
#define ASSERTLINE(line, cond) (void)0
#define ASSERTMSGLINE(line, cond, ...) (void)0
#endif

/***************/
/* OTHER STUFF */
/***************/

void OSInit();
void OSRegisterVersion(const char* id);

#include <revolution/os/OSContext.h>

#include <revolution/os/OSAlarm.h>
#include <revolution/os/OSAlloc.h>
#include <revolution/os/OSArena.h>
#include <revolution/os/OSCache.h>
#include <revolution/os/OSDC.h>
#include <revolution/os/OSException.h>
#include <revolution/os/OSFont.h>
#include <revolution/os/OSIC.h>
#include <revolution/os/OSInterrupt.h>
#include <revolution/os/OSL2.h>
#include <revolution/os/OSLC.h>
#include <revolution/os/OSMemory.h>
#include <revolution/os/OSMessage.h>
#include <revolution/os/OSMutex.h>
#include <revolution/os/OSReboot.h>
#include <revolution/os/OSReset.h>
#include <revolution/os/OSRtc.h>
#include <revolution/os/OSSemaphore.h>
#include <revolution/os/OSStopwatch.h>
#include <revolution/os/OSThread.h>
#include <revolution/os/OSTimer.h>
#include <revolution/os/OSUtf.h>

#include <revolution/os/OSError.h>
#include <revolution/os/OSModule.h>

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // REVOLUTION_OS_H
