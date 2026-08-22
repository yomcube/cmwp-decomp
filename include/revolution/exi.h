#ifndef REVOLUTION_EXI_H
#define REVOLUTION_EXI_H

#include <revolution/types.h>

#include <revolution/euart.h>

#include <revolution/os/OSContext.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const u32 __EXIFreq;

#define EXI_CHAN_0 0
#define EXI_CHAN_1 1
#define EXI_CHAN_2 2
#define EXI_CHAN_MAX 3

#define EXI_FREQ_2M 1
#define EXI_FREQ_4M 2
#define EXI_FREQ_8M 3
#define EXI_FREQ_16M 4
#define EXI_FREQ_32M 5

#define EXI_MEMORY_CARD_59 0x00000004
#define EXI_MEMORY_CARD_123 0x00000008
#define EXI_MEMORY_CARD_251 0x00000010
#define EXI_MEMORY_CARD_507 0x00000020

#define EXI_MEMORY_CARD_1019 0x00000040
#define EXI_MEMORY_CARD_2043 0x00000080

#define EXI_MEMORY_CARD_1019A 0x00000140
#define EXI_MEMORY_CARD_1019B 0x00000240
#define EXI_MEMORY_CARD_1019C 0x00000340
#define EXI_MEMORY_CARD_1019D 0x00000440
#define EXI_MEMORY_CARD_1019E 0x00000540
#define EXI_MEMORY_CARD_1019F 0x00000640
#define EXI_MEMORY_CARD_1019G 0x00000740

#define EXI_MEMORY_CARD_2043A 0x00000180
#define EXI_MEMORY_CARD_2043B 0x00000280
#define EXI_MEMORY_CARD_2043C 0x00000380
#define EXI_MEMORY_CARD_2043D 0x00000480
#define EXI_MEMORY_CARD_2043E 0x00000580
#define EXI_MEMORY_CARD_2043F 0x00000680
#define EXI_MEMORY_CARD_2043G 0x00000780

#define EXI_USB_ADAPTER 0x01010000
#define EXI_NPDP_GDEV 0x01020000

#define EXI_MODEM 0x02020000
#define EXI_ETHER 0x04020200
#define EXI_MIC 0x04060000
#define EXI_AD16 0x04120000
#define EXI_RS232C 0x04040404
#define EXI_ETHER_VIEWER 0x04220001
#define EXI_STREAM_HANGER 0x04130000

#define EXI_MARLIN 0x03010000

#define EXI_IS_VIEWER 0x05070000

#define EXI_READ 0
#define EXI_WRITE 1

#define EXI_DEV_EXT 0
#define EXI_DEV_INT 1
#define EXI_DEV_NET 2

#define EXI_FREQ_1M 0
#define EXI_FREQ_2M 1
#define EXI_FREQ_4M 2
#define EXI_FREQ_8M 3
#define EXI_FREQ_16M 4
#define EXI_FREQ_32M 5

typedef void (*EXICallback)(s32 chan, OSContext* context);

/* EXI */

BOOL EXIImm(s32 chan, void* buf, s32 len, u32 type, EXICallback callback);
BOOL EXIImmEx(s32 chan, void* buf, s32 len, u32 mode);

BOOL EXIDma(s32 chan, void* buf, s32 len, u32 type, EXICallback callback);

BOOL EXISync(s32 chan);

u32 EXIClearInterrupts(s32 chan, BOOL exi, BOOL tc, BOOL ext);

EXICallback EXISetExiCallback(s32 chan, EXICallback exiCallback);

void EXIProbeReset();

BOOL EXIProbe(s32 chan);
s32 EXIProbeEx(s32 chan);

BOOL EXIAttach(s32 chan, EXICallback extCallback);

BOOL EXIDetach(s32 chan);

BOOL EXISelectSD(s32 chan, u32 dev, u32 freq);

BOOL EXISelect(s32 chan, u32 dev, u32 freq);
BOOL EXISelectEx(s32 chan, u32 dev, u32 freq);

BOOL EXIDeselect(s32 chan);

void EXIInit();

BOOL EXILock(s32 chan, u32 dev, EXICallback unlockedCallback);
BOOL EXIUnlock(s32 chan);

u32 EXIGetState(s32 chan);

s32 EXIGetID(s32 chan, u32 dev, u32* id);
s32 EXIGetIDEx(s32 chan, u32 dev, u32* id);

s32 EXIGetType(s32 chan, u32 dev, u32* type);
char* EXIGetTypeString(u32 type);

s32 EXIGetConsoleType();
void EXIWait();

s32 EXIReadReg(s32 chan, u32 dev, u32 cmd, u8* buf, s32 len);
s32 EXIWriteReg(s32 chan, u32 dev, u32 cmd, u8* buf, s32 len);

s32 EXIReadRam(s32 chan, s32 dev, u32 cmd, void* buf, s32 len, EXICallback callback);
s32 EXIWriteRam(s32 chan, s32 dev, u32 cmd, void* buf, s32 len, EXICallback callback);

s32 EXIReadRamImm(s32 chan, s32 dev, u32 cmd, void* buf, s32 len);
s32 EXIWriteRamImm(s32 chan, s32 dev, u32 cmd, void* buf, s32 len);

/* AD16 */

BOOL AD16Init();

BOOL AD16WriteReg(u32 word);
BOOL AD16ReadReg(u32* word);

BOOL AD16Probe();

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_EXI_H
