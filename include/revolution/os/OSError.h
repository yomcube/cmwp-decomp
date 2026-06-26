#ifndef REVOLUTION_OS_ERROR_H
#define REVOLUTION_OS_ERROR_H

#include <revolution/types.h>

#include <revolution/os/OSContext.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

DECL_WEAK void OSReport(const char* msg, ...);
DECL_WEAK void OSVReport(const char* msg, va_list list);

DECL_WEAK void OSPanic(const char* file, int line, const char* msg, ...);

#define OSHaltLine(line, ...) OSPanic(__FILE__, line, __VA_ARGS__)
#define OSHalt(...) OSHaltLine(__LINE__, __VA_ARGS__)

typedef u16 OSError;
typedef void (*OSErrorHandler)(OSError error, OSContext* context, ...);

#define OS_ERROR_SYSTEM_RESET 0
#define OS_ERROR_MACHINE_CHECK 1
#define OS_ERROR_DSI 2
#define OS_ERROR_ISI 3
#define OS_ERROR_EXTERNAL_INTERRUPT 4
#define OS_ERROR_ALIGNMENT 5
#define OS_ERROR_PROGRAM 6
#define OS_ERROR_FLOATING_POINT 7
#define OS_ERROR_DECREMENTER 8
#define OS_ERROR_SYSTEM_CALL 9
#define OS_ERROR_TRACE 10
#define OS_ERROR_PERFORMACE_MONITOR 11
#define OS_ERROR_BREAKPOINT 12
#define OS_ERROR_SYSTEM_INTERRUPT 13
#define OS_ERROR_THERMAL_INTERRUPT 14
#define OS_ERROR_MAX (OS_ERROR_THERMAL_INTERRUPT + 1)

OSErrorHandler OSSetErrorHandler(OSError error, OSErrorHandler handler);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // REVOLUTION_OS_ERROR_H
