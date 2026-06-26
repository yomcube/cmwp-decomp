#include <revolution/os.h>

#include <private/os.h>

/* sure */

OSTime __get_clock() {
#ifdef DEBUG
    return OSGetTick();
#else
    return __OSGetSystemTime();
#endif
}

u32 __get_time() {
#ifdef DEBUG
    return (u32)OSTicksToSeconds(OSGetTime()) - 0x43E83E00;
#else
    return OSTicksToSeconds(OSGetTime()) - 0x43E83E00;
#endif
}

int __to_gm_time() {
    return 0;
}

#ifdef DEBUG
int __isdst() {
    return 0;
}
#endif
