#ifndef PPC_EABI_INIT_H
#define PPC_EABI_INIT_H

#include <decomp.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void __init_user();

void __init_hardware();
void __flush_cache(void* address, unsigned int size);

DECL_WEAK void abort();
DECL_WEAK void exit(int status);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PPC_EABI_INIT_H
