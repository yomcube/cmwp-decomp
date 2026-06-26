
#include <revolution/os.h>

#include <__ppc_eabi_init.h>

#include <private/os.h>

#include <revolution/base.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

__declspec(section ".ctors") extern void (*_ctors[])();
__declspec(section ".dtors") extern void (*_dtors[])();

static void __init_cpp();
static void __fini_cpp();

void _ExitProcess();

#ifdef __cplusplus
}
#endif  // __cplusplus

__declspec(section ".init") asm void __init_hardware() {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    mfmsr r0
    ori  r0,r0,MSR_FP
    mtmsr r0
    mflr    r31
    bl      __OSPSInit
    bl      __OSFPRInit
    bl      __OSCacheInit
    mtlr    r31
    blr
    // clang-format on
#endif  // __MWERKS__
}

__declspec(section ".init") asm void __flush_cache(void* address, unsigned int size) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc
    lis     r5, 0xffff
    ori     r5, r5, 0xfff1
    and     r5, r5, r3
    subf    r3, r5, r3
    add     r4, r4, r3
rept:
    dcbst   0,r5
    sync
    icbi    0,r5
    addic   r5,r5,0x8
    subic.  r4,r4,0x8
    bge     rept
    isync
    blr
    // clang-format on
#endif  // __MWERKS__
}

asm void __init_user() {
#ifdef __MWERKS__
    // clang-format off
    fralloc

    bl __init_cpp

    frfree
    blr
    // clang-format on
#endif  // __MWERKS__
}

static void __init_cpp() {
    void (**constructor)();

    for (constructor = _ctors; *constructor; constructor++) {
        (*constructor)();
    }
}

static void __fini_cpp() {
    void (**destructor)();

    for (destructor = _dtors; *destructor; destructor++) {
        (*destructor)();
    }
}

DECL_WEAK void abort() {
    _ExitProcess();
}

DECL_WEAK void exit(int status) {
    __fini_cpp();
    _ExitProcess();
}

void _ExitProcess() {
    PPCHalt();
}
