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

#define OSRoundUp32B(x) ROUNDUP(x, 32)

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
#define OSPhysicalToCached(paddr) ((void*)((u32)(OS_BASE_CACHED + (u32)(paddr))))
#define OSPhysicalToUncached(paddr) ((void*)((u32)(OS_BASE_UNCACHED + (u32)(paddr))))
#define OSCachedToPhysical(caddr) ((u32)((u32)(caddr) - OS_BASE_CACHED))
#define OSUncachedToPhysical(ucaddr) ((u32)((u32)(ucaddr) - OS_BASE_UNCACHED))
#define OSCachedToUncached(caddr) ((void*)((u8*)(caddr) + (OS_BASE_UNCACHED - OS_BASE_CACHED)))
#define OSUncachedToCached(ucaddr) ((void*)((u8*)(ucaddr) - (OS_BASE_UNCACHED - OS_BASE_CACHED)))
#endif

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
