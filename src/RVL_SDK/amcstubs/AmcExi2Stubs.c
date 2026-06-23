#include <private/amcexi/amcexi2.h>

void EXI2_Init(volatile u8** inputPendingPtrRef, EXICallback monitorCallback) {
}

void EXI2_EnableInterrupts() {
}

int EXI2_Poll() {
    return 0;
}

AmcExiError EXI2_ReadN(void* bytes, u32 length) {
    return AMC_EXI_NO_ERROR;
}

AmcExiError EXI2_WriteN(const void* bytes, u32 length) {
    return AMC_EXI_NO_ERROR;
}

void EXI2_Reserve() {
}

void EXI2_Unreserve() {
}

BOOL AMC_IsStub() {
    return TRUE;
}
