#ifndef PRIVATE_OS_EXEC_H
#define PRIVATE_OS_EXEC_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct {
    BOOL valid;          // 0x00
    u32 restartCode;     // 0x04
    u32 bootDol;         // 0x08
    void* regionStart;   // 0x0C
    void* regionEnd;     // 0x10
    int argsUseDefault;  // 0x14
    void* argsAddr;      // 0x18
} OSExecParams;

void __OSBootDol(u32 doloffset, u32 restartCode, const char** argv);

extern u32 __OSNextPartitionType;

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PRIVATE_OS_EXEC_H
