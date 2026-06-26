#include <NMWException.h>

#include <__ppc_eabi_init.h>
#include <__ppc_eabi_linker.h>

static int fragmentID = -2;

#ifdef __cplusplus
extern "C" {
#endif

extern void __init_cpp_exceptions();
extern void __fini_cpp_exceptions();

#ifdef __cplusplus
}
#endif

void __exception_info_constants(void** info, char** R2) {
    register char* temp;

#ifdef __MWERKS__
    asm { mr temp, r2; }
#endif
    *R2 = temp;

    *info = _eti_init_info;
}

long __find_exception_addresses(void* myInfo, char* code_addr, void** start, void** end) {
    __eti_init_info* info = (__eti_init_info*)myInfo;
    while (TRUE) {
        if (!(info->code_size != 0)) {
            break;
        }
        if (code_addr >= info->code_start && code_addr < (char*)info->code_start + info->code_size) {
            *start = info->eti_start;
            *end = info->eti_end;

            return 1;
        }
        info++;
    }

    return 0;
}

void __init_cpp_exceptions() {
    if (fragmentID == -2) {
        char* R2;
        __eti_init_info* info;

        __exception_info_constants((void**)&info, &R2);
        fragmentID = __register_fragment(_eti_init_info, R2);
    }
}

void __fini_cpp_exceptions() {
    if (fragmentID != -2) {
        __unregister_fragment(fragmentID);
        fragmentID = -2;
    }
}

#pragma force_active on
#pragma section ".ctors$10"
__declspec(section ".ctors$10") extern void* const __init_cpp_exceptions_reference = __init_cpp_exceptions;
#pragma section ".dtors$10"
__declspec(section ".dtors$10") extern void* const __destroy_global_chain_reference = __destroy_global_chain;
#pragma section ".dtors$15"
__declspec(section ".dtors$15") extern void* const __fini_cpp_exceptions_reference = __fini_cpp_exceptions;
