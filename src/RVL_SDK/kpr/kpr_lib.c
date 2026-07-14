#include <revolution/kpr.h>
#include <revolution/os.h>
#include <wchar.h>

#ifdef DEBUG
static const char* __KPRVersion = "<< RVL_SDK - KPR 	debug build: Dec 11 2009 15:54:57 (0x4302_145) >>";
#else
static const char* __KPRVersion = "<< RVL_SDK - KPR 	release build: Dec 11 2009 15:59:36 (0x4302_145) >>";
#endif

void* kprProcDeadKeysFP;
void* kprProcRomajiFP;

void KPRInitRegionUS() {
    kprProcDeadKeysFP = KPRProcessDeadKeys;
}
void KPRInitRegionJP() {
    kprProcRomajiFP = KPRProcessRomaji;
}
void KPRInitRegionEU() {
    kprProcDeadKeysFP = KPRProcessDeadKeys;
}

void KPRInitQueue(KPRQueue *queue) {
    static u8 once = 0;
    ASSERTLINE(111, queue);
    if (!once) {
        OSRegisterVersion(__KPRVersion);
        once = TRUE;
    }
    KPRSetMode(queue, 1);
}

void KPRClearQueue(KPRQueue *queue) {
    ASSERTLINE(133, queue);
    
    queue->empty = 0;
    queue->iCount = 0;
    queue->unk_14 = 0;
}

int KPRGetMode(KPRQueue *queue) {
    ASSERTLINE(185, queue);
    return queue->mode;
}

void KPRSetMode(KPRQueue *queue, int mode) {
    ASSERTLINE(153, queue);


    ASSERTLINE(156, (mode & (KPR_MODE_DEADKEY | KPR_MODE_JP_ROMAJI_HIRAGANA)) !=
        (KPR_MODE_DEADKEY | KPR_MODE_JP_ROMAJI_HIRAGANA));
    ASSERTLINE(158, (mode & (KPR_MODE_DEADKEY | KPR_MODE_JP_ROMAJI_KATAKANA)) !=
        (KPR_MODE_DEADKEY | KPR_MODE_JP_ROMAJI_KATAKANA));
    ASSERTLINE(160, (mode & (KPR_MODE_JP_ROMAJI_HIRAGANA | KPR_MODE_JP_ROMAJI_KATAKANA)) !=
        (KPR_MODE_JP_ROMAJI_HIRAGANA | KPR_MODE_JP_ROMAJI_KATAKANA));

    ASSERTMSGLINE(163, (mode & KPR_MODE_DEADKEY) && (kprProcDeadKeysFP == NULL),
        "KPRSetMode: KPR_MODE_DEADKEY selected, but US/EU KPR region not set.");
    ASSERTMSGLINE(165, (mode & KPR_MODE_JP_ROMAJI_HIRAGANA) && (kprProcRomajiFP == NULL),
        "KPRSetMode: KPR_MODE_JP_ROMAJI_HIRAGANA selected, but JP region not set.");
    ASSERTMSGLINE(167, (mode & KPR_MODE_JP_ROMAJI_KATAKANA) && (kprProcRomajiFP == NULL),
        "KPRSetMode: KPR_MODE_JP_ROMAJI_KATAKANA selected, but JP region not set.");
    
    queue->mode = mode;
    KPRClearQueue(queue);
}


void KPRProcessDeadKeys();
void KPRProcessRomaji();

int isConsonant(wchar_t c) {
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
        return FALSE;
    }
    return TRUE;
}
