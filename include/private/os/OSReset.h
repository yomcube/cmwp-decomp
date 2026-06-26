#ifndef PRIVATE_OS_RESET_H
#define PRIVATE_OS_RESET_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

BOOL __OSCallShutdownFunctions(BOOL, u32 event);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PRIVATE_OS_RESET_H
