#pragma once
#include <revolution/types.h>
#include <wchar.h>

enum KPR_MODE {
    KPR_MODE_DEADKEY = 2,
    KPR_MODE_JP_ROMAJI_HIRAGANA = 4,
    KPR_MODE_JP_ROMAJI_KATAKANA = 8,
};

typedef struct _KPRQueue {
    u16 unk_00;
    u16 unk_02;
    u32 unk_04;
    u32 unk_08;
    u32 mode;
    u8 empty;
    u8 iCount;
    u16 unk_12;
    u32 unk_14;
} KPRQueue;

void KPRInitQueue(KPRQueue* queue);
void KPRClearQueue(KPRQueue* queue);
void KPRSetMode(KPRQueue* queue, int mode);
int KPRGetMode(KPRQueue* queue);


static void KPRProcessDeadKeys();
static void KPRProcessRomaji();
static int isConsonant(wchar_t c);
