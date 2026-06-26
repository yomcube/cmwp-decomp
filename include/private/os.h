#ifndef PRIVATE_OS_H
#define PRIVATE_OS_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <revolution/os.h>

#include <private/os/OSAlarm.h>
#include <private/os/OSAudioSystem.h>
#include <private/os/OSBootInfo.h>
#include <private/os/OSCache.h>
#include <private/os/OSException.h>
#include <private/os/OSExec.h>
#include <private/os/OSMutex.h>
#include <private/os/OSReset.h>
#include <private/os/OSRtc.h>
#include <private/os/OSThread.h>
#include <private/os/OSTime.h>

/* Address */

volatile OSContext* __OSCurrentContext AT_ADDRESS(OS_BASE_CACHED | OS_ADDR_CURRENT_CONTEXT);
volatile OSContext* __OSFPUContext AT_ADDRESS(OS_BASE_CACHED | OS_ADDR_CURRENT_FPU_CONTEXT);

OSThreadQueue __OSActiveThreadQueue AT_ADDRESS(OS_BASE_CACHED | OS_ADDR_ACTIVE_THREAD_QUEUE);
OSThread* __OSCurrentThread AT_ADDRESS(OS_BASE_CACHED | OS_ADDR_CURRENT_THREAD);

u32 __OSPhysicalMem1Size AT_ADDRESS(OS_BASE_CACHED | OS_ADDR_PHYSICAL_MEM1_SIZE);
u32 __OSSimulatedMem1Size AT_ADDRESS(OS_BASE_CACHED | OS_ADDR_SIMULATED_MEM1_SIZE);

u32 __OSPhysicalMem2Size AT_ADDRESS(OS_BASE_CACHED | OS_ADDR_PHYSICAL_MEM2_SIZE);
u32 __OSSimulatedMem2Size AT_ADDRESS(OS_BASE_CACHED | OS_ADDR_SIMULATED_MEM2_SIZE);

u32 __OSPartitionType AT_ADDRESS(OS_BASE_CACHED | OS_ADDR_BOOT_PART_TYPE);

/* OS.c */

extern BOOL __OSInIPL;

extern char* __OSExceptionNames[__OS_EXCEPTION_MAX];

extern OSExecParams __OSRebootParams;

void __OSPSInit();
void __OSFPRInit();

/* Misc */

extern u32 __OSFpscrEnableBits;
extern OSErrorHandler __OSErrorTable[__OS_EXCEPTION_MAX];

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PRIVATE_OS_H
