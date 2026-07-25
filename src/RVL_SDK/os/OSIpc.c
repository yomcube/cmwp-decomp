#include <revolution/os.h>

static void* IpcBufferHi = NULL;
static void* IpcBufferLo = (void*)-1;

void* __OSGetIPCBufferHi() {
    return IpcBufferHi;
}

void* __OSGetIPCBufferLo() {
    return IpcBufferLo;
}

void __OSInitIPCBuffer() {
    IpcBufferLo = (void*)*(u32*)OSPhysicalToCached(OS_ADDR_IPC_BUFFER_START);
    IpcBufferHi = (void*)*(u32*)OSPhysicalToCached(OS_ADDR_IPC_BUFFER_END);
}
