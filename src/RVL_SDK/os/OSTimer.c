#include <revolution/os.h>

#include <private/os.h>
#include <revolution/base.h>

struct Timer {
    OSTimerCallback callback;
    u32 currval;
    u32 startval;
    u32 mode;
    BOOL stopped;
    BOOL initialized;
};
static struct Timer Timer;

// prototypes
static void DecrementerExceptionHandler(__OSException exception, OSContext* context);

OSTimerCallback OSSetTimerCallback(OSTimerCallback callback) {
    OSTimerCallback prevCallback;

    ASSERTMSGLINE(141, Timer.initialized, "OSSetTimerCallback(): timer is not initialized.");

    Timer.stopped = TRUE;
    prevCallback = Timer.callback;
    Timer.callback = callback;
    return prevCallback;
}

void OSInitTimer(u32 time, u32 mode) {
    ASSERTMSGLINE(165, time < 0x80000000, "OSInitTimer(): time param must be less than 0x80000000.");

    Timer.stopped = TRUE;
    Timer.currval = time;
    Timer.startval = time;
    Timer.mode = mode;

    if (!Timer.initialized) {
        __OSSetExceptionHandler(8, &DecrementerExceptionHandler);
        Timer.initialized = TRUE;
        Timer.callback = NULL;
#if DEBUG
        OSReport("Timer initialized\n");
#endif
    }
}

void OSStartTimer() {
    BOOL enabled;

    ASSERTMSGLINE(198, Timer.initialized, "OSStartTimer(): timer is not initialized.");

    enabled = OSDisableInterrupts();
    PPCMtdec(Timer.currval);
    Timer.stopped = FALSE;
    OSRestoreInterrupts(enabled);
}

void OSStopTimer() {
    BOOL enabled;

    ASSERTMSGLINE(222, Timer.initialized, "OSStopTimer(): timer is not initialized.");

    enabled = OSDisableInterrupts();
    if (!Timer.stopped) {
        Timer.stopped = TRUE;
        Timer.currval = PPCMfdec();
        if (Timer.currval & 0x80000000) {
            Timer.currval = 0;
        }
    }
    OSRestoreInterrupts(enabled);
}

static void DecrementerExceptionCallback(__OSException exception, OSContext* context) {
    OSContext exceptionContext;

    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);

    if (!Timer.stopped) {
        if (Timer.mode == 1) {
            PPCMtdec(Timer.startval);
        }
        if (Timer.mode == 2) {
            Timer.stopped = TRUE;
        }
        if (Timer.callback) {
            Timer.callback();
        }
    }
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(context);
    OSLoadContext(context);
}

static asm void DecrementerExceptionHandler(__OSException exception, register OSContext* context) {
#ifdef __MWERKS__
    // clang-format off
    nofralloc

    stw r0, context->gpr[0]
    stw r1, context->gpr[1]
    stw r2, context->gpr[2]
    stmw r6, context->gpr[6]

    mfspr r0, GQR1
    stw r0, context->gqr[1]
    mfspr r0, GQR2
    stw r0, context->gqr[2]
    mfspr r0, GQR3
    stw r0, context->gqr[3]
    mfspr r0, GQR4
    stw r0, context->gqr[4]
    mfspr r0, GQR5
    stw r0, context->gqr[5]
    mfspr r0, GQR6
    stw r0, context->gqr[6]
    mfspr r0, GQR7
    stw r0, context->gqr[7]

    stwu r1, -0x8(r1)
    b DecrementerExceptionCallback
    // clang-format on
#endif
}
