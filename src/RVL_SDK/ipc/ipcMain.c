#include <private/ipc.h>

#include <private/hollywood.h>
#include <private/os.h>

#include <stdbool.h>

static bool Initialized = false;

static void* IPCBufferHi;
static void* IPCBufferLo;

static void* IPCCurrentBufferHi;
static void* IPCCurrentBufferLo;

void IPCInit() {
    if (Initialized) {
        return;
    }

    IPCBufferHi = __OSGetIPCBufferHi();
    IPCBufferLo = __OSGetIPCBufferLo();

    IPCCurrentBufferHi = IPCBufferHi;
    IPCCurrentBufferLo = IPCBufferLo;

    Initialized = true;
}

void IPCReInit() {
    Initialized = false;
    IPCInit();
}

u32 IPCReadReg(u32 addr) {
    u32 read = __ACRRegs[addr];
    return read;
}

void IPCWriteReg(u32 addr, u32 val) {
    __ACRRegs[addr] = val;
}

void* IPCGetBufferHi() {
    return IPCCurrentBufferHi;
}

void* IPCGetBufferLo() {
    return IPCCurrentBufferLo;
}

void IPCSetBufferHi(void* newHi) {
    ASSERTLINE(281, newHi <= IPCBufferHi);
    IPCCurrentBufferHi = newHi;
}

void IPCSetBufferLo(void* newLo) {
    ASSERTLINE(296, IPCBufferLo <= newLo);
    IPCCurrentBufferLo = newLo;
}
