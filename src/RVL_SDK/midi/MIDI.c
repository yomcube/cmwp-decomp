#include <revolution/verdefs.h>
#if defined(SDK_20091112)
#ifdef DEBUG
SDKDefineVersion(MIDI, "Dec 11 2009", "15:53:52");
#else
SDKDefineVersion(MIDI, "Dec 11 2009", "15:58:29");
#endif
#elif defined(SDK_20090224)
SDKDefineVersion(MIDI, "Jul 30 2008", "19:23:09");
#endif

#include <revolution/midi.h>

#include <private/midi.h>

#include <revolution/os.h>

static u32 SecParams[MIDI_MAX_CHAN][0x10];
static BOOL Initialized = FALSE;

__MIDIControl __MIDI[MIDI_MAX_CHAN];
BOOL __MIDIReset = FALSE;

static BOOL OnShutdown(BOOL, u32);

OSShutdownFunctionInfo ShutdownFunctionInfo = {OnShutdown, 126, NULL, NULL};

void ShortCommandProc(s32 chan) {
    __MIDIControl* midi = &__MIDI[chan];

    if (midi->result == MIDI_RESULT_OK) {
        if (midi->transferInput.unk_0x00 != 0 || midi->transferInput.unk_0x01 != 4) {
            midi->result = MIDI_RESULT_1;
        } else {
            *midi->outStatus = (u8)(midi->transferInput.unk_0x02 & 0x3A);
        }
    }
}

#pragma push
#pragma ppc_iro_level 0  // uh
void MIDIInit() {
    __MIDIControl* midi;
    s32 chan;

    if (!Initialized) {
        Initialized = TRUE;
        OSRegisterVersion(GetVersion(MIDI));

        for (chan = 0; chan < MIDI_MAX_CHAN; chan++) {
            midi = &__MIDI[chan];

            // Default SI delay time
            midi->delayTime = OSMicrosecondsToTicks(60);

            OSInitThreadQueue(&midi->threadQueue);

            // Setup params (unused)
            midi->param = SecParams[chan];
            // clang-format off
            ASSERTLINE(77, (u32) midi->param % 32 == 0);
            // clang-format on
        }
        __MIDIReset = FALSE;
        OSRegisterShutdownFunction(&ShutdownFunctionInfo);
    }
}
#pragma pop

s32 MIDIGetStatusAsync(s32 chan, u8* status, MIDICallback callback) {
    __MIDIControl* midi = &__MIDI[chan];

    if (midi->callback) {
        return MIDI_RESULT_2;
    }
    midi->transferOutput.unk_0x00 = 0;
    midi->outStatus = (u8*)status;
    midi->callback = callback;
    return __MIDITransfer(chan, 1, 3, ShortCommandProc);
}

s32 MIDIGetStatus(s32 chan, u8* status) {
    __MIDIControl* midi = &__MIDI[chan];

    s32 result = MIDIGetStatusAsync(chan, status, __MIDISyncCallback);

    if (result != MIDI_RESULT_OK) {
        return result;
    }
    return __MIDISync(chan);
}

static BOOL OnShutdown(BOOL arg0, u32 arg1) {
    __MIDIReset = TRUE;
    return TRUE;
}
