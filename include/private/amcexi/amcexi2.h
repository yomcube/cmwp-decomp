#ifndef PRIVATE_AMC_EXI2_H
#define PRIVATE_AMC_EXI2_H

#include <private/amcexi/amctypes.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef enum {
    AMC_EXI_NO_ERROR = 0,
    AMC_EXI_UNSELECTED,
} AmcExiError;

void EXI2_Init(volatile u8** inputPendingPtrRef, EXICallback monitorCallback);

void EXI2_EnableInterrupts();

int EXI2_Poll();

AmcExiError EXI2_ReadN(void* bytes, u32 length);
AmcExiError EXI2_WriteN(const void* bytes, u32 length);

void EXI2_Reserve();
void EXI2_Unreserve();

AmcExiError EXI2_GetStatusReg(u16* pu16StatusReg);

BOOL AMC_IsStub();

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PRIVATE_AMC_EXI2_H
