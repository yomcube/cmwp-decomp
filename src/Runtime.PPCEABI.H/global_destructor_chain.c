#include <NMWException.h>
#include <decomp.h>
#pragma sym on
#define MAX_ATEXIT_FUNCS 0x40

DestructorChain* __global_destructor_chain;

DestructorChain atexit_funcs[MAX_ATEXIT_FUNCS];
long atexit_curr_func = 0;

void __register_global_object(void* object, void* dtor, DestructorChain* chain) {
    chain->next = __global_destructor_chain;
    chain->dtor = dtor;
    chain->object = object;

    __global_destructor_chain = chain;
}

void __destroy_global_chain() {
    DestructorChain* chain;

    while ((chain = __global_destructor_chain) != 0) {
        __global_destructor_chain = chain->next;
        DTORCALL_COMPLETE(chain->dtor, chain->object);
    }
}

long __register_atexit(void* dtor) {
    DestructorChain* atexit_func;
    DestructorChain* old;

    if (atexit_curr_func == MAX_ATEXIT_FUNCS) {
        return -1;
    }

    atexit_func = &atexit_funcs[atexit_curr_func++];
    atexit_func->next = __global_destructor_chain;
    __global_destructor_chain = atexit_func;
    atexit_func->dtor = dtor;
    atexit_func->object = 0;

    return 0;
}

#pragma section ".dtors$10"
__declspec(section ".dtors$10") __declspec(weak) extern void* const __destroy_global_chain_reference = __destroy_global_chain;
