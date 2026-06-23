#ifndef PRIVATE_AMC_EXI_TYPES_H
#define PRIVATE_AMC_EXI_TYPES_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct OSContext OSContext;
typedef void (*EXICallback)(s32 chan, OSContext* context);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PRIVATE_AMC_EXI_TYPES_H
