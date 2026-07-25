#ifndef PRIVATE_OS_IPC_H
#define PRIVATE_OS_IPC_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void* __OSGetIPCBufferHi();
void* __OSGetIPCBufferLo();

void __OSInitIPCBuffer();

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PRIVATE_OS_IPC_H
