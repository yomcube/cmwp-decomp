#include <private/nand.h>
#include <revolution/nand.h>

#include <revolution/fs.h>
#include <revolution/ios.h>

#include <revolution/gx.h>
#include <revolution/os.h>
#include <revolution/sc.h>

// TODO: shift-jis/utf-8 conflicting going on.

// clang-format off

static const char* const __NANDMaxBlocksErrorMessageDefault[] = {
    "\n\n\nWii本体保存メモリの空き容量が異常です。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\n\nThere is no more available space in\n"
    "Wii system memory. Refer to the Wii\n"
    "Operations Manual for details.",

    "\n\n\nDer Speicher der Wii-Konsole ist belegt.\n"
    "Bitte lies die Wii-Bedienungsanleitung,\n"
    "um weitere Informationen zu erhalten.",

    "\n\n\nIl n'y a pas assez d'espace libre\n"
    "dans la m駑oire de la console Wii.\n"
    "Veuillez vous r馭駻er au Mode d'emploi\n"
    "de la Wii pour plus de d騁ails.",

    "\n\n\nNo queda espacio libre en la memoria\n"
    "de la consola Wii. Consulta el manual\n"
    "de operaciones de la consola Wii para\n"
    "obtener m疽 informaci\xF3n.",

    "\n\n\nNon c'\xE8 pi\xF9 spazio libero nella memoria\n"
    "della console Wii. Per maggiori\n"
    "informazioni, consulta il manuale di\n"
    "istruzioni della console Wii.",

    "\n\n\nEr is geen vrije ruimte meer in het\n"
    "interne geheugen van het Wii-systeem.\n"
    "Lees de handleiding voor meer informatie.",
};

static const char* const __NANDMaxBlocksErrorMessageEurope[] = {
    "\n\n\nWii本体保存メモリの空き容量が異常です。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\n\nThere is no more available space in\n"
    "the Wii System Memory. Please refer to\n"
    "the Wii Operations Manual for details.",

    "\n\n\nDer Speicher der Wii-Konsole ist belegt.\n"
    "Bitte lies die Wii-Bedienungsanleitung,\n"
    "um weitere Informationen zu erhalten.",

    "\n\n\nIl n'y a pas assez d'espace libre dans\n"
    "la m駑oire de la console Wii. Veuillez\n"
    "vous r馭駻er au mode d'emploi Wii pour\n"
    "plus de d騁ails.",

    "\n\n\nNo queda espacio libre en la memoria de\n"
    "la consola Wii. Consulta el manual de\n"
    "instrucciones de la consola Wii para\n"
    "obtener m疽 informaci\xF3n.",

    "\n\n\nNon c'\xE8 pi\xF9 spazio libero nella memoria\n"
    "della console Wii. Per maggiori\n"
    "informazioni, consulta il manuale di\n"
    "istruzioni della console Wii.",

    "\n\n\nEr is geen vrije ruimte meer in het\n"
    "interne geheugen van het Wii-systeem.\n"
    "Lees de handleiding voor meer informatie.",
};

static const char* const __NANDMaxBlocksErrorMessageChinaKorea[] = {
    "\n\nエラーコード４０５。\n"
    "\n"
    "Wii本体保存メモリの空き容量が異常です。\n"
    "くわしくはWii本体の取扱説明書をお読み\nください。",

    "\n\nError #405,\n"
    "\n"
    "There is no more available space in\n"
    "Wii system memory. Refer to the Wii\n"
    "Operations Manual for details.",
};

static const char* const __NANDMaxFilesErrorMessageDefault[] = {
    "\n\n\nWii本体保存メモリの空きファイル数が異常です。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\n\nThere is no more available space in\n"
    "Wii system memory. Refer to the Wii\n"
    "Operations Manual for details.",

    "\n\n\nDer Speicher der Wii-Konsole ist belegt.\n"
    "Bitte lies die Wii-Bedienungsanleitung,\n"
    "um weitere Informationen zu erhalten.",

    "\n\n\nIl n'y a pas assez d'espace libre dans\n"
    "la m駑oire de la console Wii. Veuillez\n"
    "vous r馭駻er au Mode d'emploi de la Wii\n"
    "pour plus de d騁ails.",

    "\n\n\nNo queda espacio libre en la memoria de\n"
    "la consola Wii. Consulta el manual de\n"
    "operaciones de la consola Wii para\n"
    "obtener m疽 informaci\xF3n.",

    "\n\n\nImpossibile salvare altri dati nella\n"
    "memoria della console Wii. Per maggiori\n"
    "informazioni, consulta il manuale di\n"
    "istruzioni della console Wii.",

    "\n\n\nEr is geen ruimte meer beschikbaar\n"
    "in het interne geheugen van het\n"
    "Wii-systeem. Lees de handleiding voor\n"
    "meer informatie.",
};

static const char* const __NANDMaxFilesErrorMessageEurope[] = {
    "\n\n\nWii本体保存メモリの空きファイル数が異常です。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\n\nThere is no more available space in\n"
    "the Wii System Memory. Please refer to\n"
    "the Wii Operations Manual for details.",

    "\n\n\nDer Speicher der Wii-Konsole ist belegt.\n"
    "Bitte lies die Wii-Bedienungsanleitung,\n"
    "um weitere Informationen zu erhalten.",

    "\n\n\nIl n'y a pas assez d'espace libre dans\n"
    "la m駑oire de la console Wii. Veuillez\n"
    "vous r馭駻er au mode d'emploi Wii pour\n"
    "plus de d騁ails.",

    "\n\n\nNo queda espacio libre en la memoria de\n"
    "la consola Wii. Consulta el manual de\n"
    "instrucciones de la consola Wii para\n"
    "obtener m疽 informaci\xF3n.",

    "\n\n\nImpossibile salvare altri dati nella\n"
    "memoria della console Wii. Per maggiori\n"
    "informazioni, consulta il manuale di\n"
    "istruzioni della console Wii.",

    "\n\n\nEr is geen ruimte meer beschikbaar\n"
    "in het interne geheugen van het\n"
    "Wii-systeem. Lees de handleiding voor\n"
    "meer informatie.",
};

static const char* const __NANDMaxFilesErrorMessageChinaKorea[] = {
    "\n\nエラーコード４０６。\n"
    "\n"
    "Wii本体保存メモリの空きファイル数が異常です。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\nError #406,\n"
    "\n"
    "There is no more available space in\n"
    "Wii system memory. Refer to the Wii\n"
    "Operations Manual for details.",
};

static const char* const __NANDCorruptErrorMessageDefault[] = {
    "\n\n\nWii本体保存メモリが壊れました。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\n\nThe Wii system memory has been damaged.\n"
    "Refer to the Wii Operations Manual for\n"
    "details.",

    "\n\n\nDer Speicher der Wii-Konsole\n"
    "ist besch臈igt. Bitte lies die\n"
    "Wii-Bedienungsanleitung, um weitere\n"
    "Informationen zu erhalten.",

    "\n\n\nLa m駑oire de la console Wii a 騁\xE9 \n"
    "endommag馥. Veuillez vous r馭駻er au\n"
    "Mode d'emploi de la Wii pour plus de\n"
    "d騁ails.",

    "\n\n\nLa memoria de la consola Wii\n"
    "est\xE1 da""\xF1\x61""da. Consulta el manual de\n"
    "operaciones de la consola Wii para\n"
    "obtener m疽 informaci\xF3n.",

    "\n\n\nLa memoria della console Wii e\n"
    "danneggiata. Per maggiori informazioni,\n"
    "consulta il manuale di istruzioni della\n"
    "console Wii.",

    "\n\n\nHet interne geheugen van het\n"
    "Wii-systeem is beschadigd. Lees de\n"
    "Wii-handleiding voor meer informatie.",
};

static const char* const __NANDCorruptErrorMessageEurope[] = {
    "\n\n\nWii本体保存メモリが壊れました。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\n\nThe Wii System Memory has been damaged.\n"
    "Please refer to the Wii Operations Manual\n"
    "for details.",

    "\n\n\nDer Speicher der Wii-Konsole\n"
    "ist beschadigt. Bitte lies die\n"
    "Wii-Bedienungsanleitung, um weitere\n"
    "Informationen zu erhalten.",

    "\n\n\nLa m駑oire de la console Wii est\n"
    "endommag馥. Veuillez vous r馭駻er au\n"
    "mode d'emploi Wii pour plus de d騁ails.\n",

    "\n\n\nLa memoria de la consola Wii est\xE1 da""\xF1\x61""da.\n"
    "Consulta el manual de instrucciones de la\n"
    "consola Wii para obtener m疽 informaci\xF3n.",

    "\n\n\nLa memoria della console Wii e\n"
    "danneggiata. Per maggiori informazioni,\n"
    "consulta il manuale di istruzioni della\n"
    "console Wii.",

    "\n\n\nHet interne geheugen van het\n"
    "Wii-systeem is beschadigd. Lees de\n"
    "Wii-handleiding voor meer informatie.",
};

static const char* const __NANDCorruptErrorMessageChinaKorea[] = {
    "\n\nエラーコード４０８。\n"
    "\n"
    "Wii本体保存メモリが壊れました。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\nError #408,\n"
    "\n"
    "The Wii system memory has been damaged.\n"
    "Refer to the Wii Operations Manual for\n"
    "details.",
};

static const char* const __NANDBusyErrorMessageDefault[] = {
    "\n\n\nWii本体保存メモリの書き込み/読み出しが\n"
    "できませんでした。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\n\nCould not access Wii system memory.\n"
    "Refer to the Wii Operations Manual for\n"
    "details.",

    "\n\n\nAuf den Speicher der Wii-Konsole konnte\n"
    "nicht zugegriffen werden. Bitte lies die\n"
    "Wii-Bedienungsanleitung, um weitere\n"
    "Informationen zu erhalten.",

    "\n\n\nImpossible d'acc馘er \xE0 la m駑oire de\n"
    "la console Wii. Veuillez vous r馭駻er\n"
    "au Mode d'emploi de la Wii pour plus\n"
    "de d騁ails.",

    "\n\n\nNo se ha podido acceder a la memoria de\n"
    "la consola Wii. Consulta el manual de\n"
    "operaciones de la consola Wii para\n"
    "obtener m疽 informaci\xF3n.",

    "\n\n\nImpossibile accedere alla memoria della\n"
    "console Wii. Per maggiori informazioni,\n"
    "consulta il manuale di istruzioni della\n"
    "console Wii.",

    "\n\n\nHet interne geheugen van het Wii-systeem\n"
    "kan niet worden gelezen of beschreven.\n"
    "Lees de Wii-handleiding voor meer\n"
    "informatie.",
};

static const char* const __NANDBusyErrorMessageEurope[] = {
    "\n\n\nWii本体保存メモリの書き込み/読み出しが\n"
    "できませんでした。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\n\nCould not access the Wii System Memory." // how could they forget the \n...
    "Please refer to the Wii Operations Manual\n"
    "for details.",

    "\n\n\nAuf den Speicher der Wii-Konsole konnte\n"
    "nicht zugegriffen werden. Bitte lies die\n"
    "Wii-Bedienungsanleitung, um weitere\n"
    "Informationen zu erhalten.",

    "\n\n\nImpossible d'acc馘er \xE0 la m駑oire de la\n"
    "console Wii. Veuillez vous r馭駻er au\n"
    "mode d'emploi Wii pour plus de d騁ails.",

    "\n\n\nNo se ha podido acceder a la memoria de\n"
    "la consola Wii. Consulta el manual de\n"
    "instrucciones de la consola Wii para\n"
    "obtener m疽 informaci\xF3n.",

    "\n\n\nImpossibile accedere alla memoria della\n"
    "console Wii. Per maggiori informazioni,\n"
    "consulta il manuale di istruzioni della\n"
    "console Wii.",

    "\n\n\nHet interne geheugen van het Wii-systeem\n"
    "kan niet worden gelezen of beschreven.\n"
    "Lees de Wii-handleiding voor meer\n"
    "informatie.",
};

static const char* const __NANDBusyErrorMessageChinaKorea[] = {
    "\n\nエラーコード４１１。\n"
    "\n"
    "Wii本体保存メモリの書き込み/読み出しが\n"
    "できませんでした。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\nError #411,\n"
    "\n"
    "Could not access Wii system memory.\n"
    "Refer to the Wii Operations Manual for\n"
    "details.",
};

static const char* const __NANDUnknownErrorMessageDefault[] = {
    "\n\n\nWii本体保存メモリの書き込み/読み出し中に\n"
    "エラーが発生しました。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\n\nAn error occurred while accessing Wii\n"
    "system memory. Refer to the Wii\n"
    "Operations Manual for details.",

    "\n\n\nBeim Zugriff auf den Speicher der\n"
    "Wii-Konsole ist ein Fehler aufgetreten.\n"
    "Bitte lies die Wii-Bedienungsanleitung,\n"
    "um weitere Informationen zu erhalten.",

    "\n\n\nUne erreur est survenue pendant le\n"
    "processus de lecture ou d'馗riture\n"
    "dans la m駑oire de la console Wii.\n"
    "Veuillez vous r馭駻er au Mode d'emploi\n"
    "de la Wii pour plus de d騁ails.",

    "\n\n\nSe ha producido un error al intentar\n"
    "acceder a la memoria de la consola Wii.\n"
    "Consulta el manual de operaciones\n"
    "de la consola Wii para obtener m疽\n"
    "informaci\xF3n.",

    "\n\n\nSi \xE8 verificato un errore durante la\n"
    "lettura o la modifica dei dati\n"
    "all'interno della memoria della\n"
    "console Wii. Per maggiori informazioni,\n"
    "consulta il manuale di istruzioni della\n"
    "console Wii.",

    "\n\n\nEr is een fout opgetreden tijdens het\n"
    "lezen of beschrijven van het interne\n"
    "geheugen van het Wii-systeem. Lees de\n"
    "Wii-handleiding voor meer informatie.",
};

static const char* const __NANDUnknownErrorMessageEurope[] = {
    "\n\n\nWii本体保存メモリの書き込み/読み出し中に\n"
    "エラーが発生しました。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\n\nAn error occurred during the process of\n"
    "reading from or writing to the Wii System\n"
    "Memory. Please refer to the Wii Operations\n"
    "Manual for details.",

    "\n\n\nBeim Zugriff auf den Speicher der\n"
    "Wii-Konsole ist ein Fehler aufgetreten.\n"
    "Bitte lies die Wii-Bedienungsanleitung,\n"
    "um weitere Informationen zu erhalten.",

    "\n\n\nUne erreur est survenue avec la m駑oire\n"
    "de la console Wii pendant le processus\n"
    "de lecture ou d'馗riture. Veuillez vous\n"
    "r馭駻er au mode d'emploi Wii pour plus\n"
    "de d鑼ails.",

    "\n\n\nSe ha producido un error durante la\n"
    "lectura o escritura de la memoria de\n"
    "la consola Wii. Consulta el manual de\n"
    "instrucciones de la consola Wii para\n"
    "obtener m疽 informaci\xF3n.",

    "\n\n\nSi \xE8 verificato un errore durante la\n"
    "lettura o la modifica dei dati\n"
    "all'interno della memoria della\n"
    "console Wii. Per maggiori informazioni,\n"
    "consulta il manuale di istruzioni della\n"
    "console Wii.",

    "\n\n\nEr is een fout opgetreden tijdens het\n"
    "lezen of beschrijven van het interne\n"
    "geheugen van het Wii-systeem. Lees de\n"
    "Wii-handleiding voor meer informatie.",
};

static const char* const __NANDUnknownErrorMessageChinaKorea[] = {
    "\n\nエラーコード４１２。\n"
    "\n"
    "Wii本体保存メモリの書き込み/読み出し中に\n"
    "エラーが発生しました。\n"
    "くわしくはWii本体の取扱説明書をお読み\n"
    "ください。",

    "\n\nError #412,\n"
    "\n"
    "An error occurred while accessing Wii\n"
    "system memory. Refer to the Wii\n"
    "Operations Manual for details.",
};

// clang-format on

typedef void (*NANDShowErrorFunc)(s32 result);

NANDShowErrorFunc NANDErrorFunc = NULL;

// @typo should be __NANDGetErrorMessageList
static const char** __NANDSetErrorMessageList(s32 result, int region) {
    u8 i = 0;

    struct {
        s32 result;
        int region;
        const char** msgs;
    } list[] = {
        // clang-format off
        {ISFS_ERROR_MAXBLOCKS, 0, (const char**)__NANDMaxBlocksErrorMessageDefault},
        {ISFS_ERROR_MAXFILES, 0, (const char**)__NANDMaxFilesErrorMessageDefault},
        {ISFS_ERROR_CORRUPT, 0, (const char**)__NANDCorruptErrorMessageDefault},
        {ISFS_ERROR_BUSY, 0, (const char**)__NANDBusyErrorMessageDefault},
        {IOS_ERROR_FAIL_ALLOC, 0, (const char**)__NANDBusyErrorMessageDefault},
        {ISFS_ERROR_UNKNOWN, 0, (const char**)__NANDUnknownErrorMessageDefault},
        {NAND_RESULT_MAXBLOCKS, 0, (const char**)__NANDUnknownErrorMessageDefault}, // @bug? should be __NANDMaxBlocksErrorMessageDefault

        {ISFS_ERROR_MAXBLOCKS, 1, (const char**)__NANDMaxBlocksErrorMessageEurope},
        {ISFS_ERROR_MAXFILES, 1, (const char**)__NANDMaxFilesErrorMessageEurope},
        {ISFS_ERROR_CORRUPT, 1, (const char**)__NANDCorruptErrorMessageEurope},
        {ISFS_ERROR_BUSY, 1, (const char**)__NANDBusyErrorMessageEurope},
        {IOS_ERROR_FAIL_ALLOC, 1, (const char**)__NANDBusyErrorMessageEurope},
        {ISFS_ERROR_UNKNOWN, 1, (const char**)__NANDUnknownErrorMessageEurope},
        {NAND_RESULT_MAXBLOCKS, 1, (const char**)__NANDUnknownErrorMessageEurope}, // @bug? should be __NANDMaxBlocksErrorMessageEurope
        
        {ISFS_ERROR_MAXBLOCKS, 2, (const char**)__NANDMaxBlocksErrorMessageChinaKorea},
        {ISFS_ERROR_MAXFILES, 2, (const char**)__NANDMaxFilesErrorMessageChinaKorea},
        {ISFS_ERROR_CORRUPT, 2, (const char**)__NANDCorruptErrorMessageChinaKorea},
        {ISFS_ERROR_BUSY, 2, (const char**)__NANDBusyErrorMessageChinaKorea},
        {IOS_ERROR_FAIL_ALLOC, 2, (const char**)__NANDBusyErrorMessageChinaKorea},
        {ISFS_ERROR_UNKNOWN, 2, (const char**)__NANDUnknownErrorMessageChinaKorea},
        {NAND_RESULT_MAXBLOCKS, 2, (const char**)__NANDUnknownErrorMessageChinaKorea}, // @bug? should be __NANDMaxBlocksErrorMessageChinaKorea
        // clang-format on
    };

    while (i < ARRAY_LENGTH(list)) {
        if (region == list[i].region) {
            if (result == list[i].result) {
                return list[i].msgs;
            } else {
                i++;
            }
        } else {
            i += (u8)7;  // sure
        }
    }

    return (const char**)__NANDUnknownErrorMessageDefault;
}

static void __NANDShowErrorMessage(s32 result) {
    GXColor bg = {0, 0, 0, 0};
    GXColor fg = {255, 255, 255, 0};

    const char** gotMsg;
    const char* msg;

    if (SCGetLanguage() == SC_LANG_JAPANESE) {
        OSSetFontEncode(OS_FONT_ENCODE_SJIS);
    } else {
        OSSetFontEncode(OS_FONT_ENCODE_ANSI);
    }

    switch (SCGetProductGameRegion()) {
        case SC_PRODUCT_GAME_REGION_JP:
        case SC_PRODUCT_GAME_REGION_US:
        default: {
            gotMsg = __NANDSetErrorMessageList(result, 0);
            break;
        }
        case SC_PRODUCT_GAME_REGION_EU: {
            gotMsg = __NANDSetErrorMessageList(result, 1);
            break;
        }
        case SC_PRODUCT_GAME_REGION_KR:
        case SC_PRODUCT_GAME_REGION_CN: {
            gotMsg = __NANDSetErrorMessageList(result, 2);
            break;
        }
    }

    if (SCGetLanguage() > SC_LANG_DUTCH) {
        msg = gotMsg[SC_LANG_ENGLISH];
    } else {
        msg = gotMsg[SCGetLanguage()];
    }

    OSFatal(fg, bg, msg);
}

BOOL NANDSetAutoErrorMessaging(BOOL flag) {
    BOOL enabled = OSDisableInterrupts();

    BOOL prev = NANDErrorFunc ? TRUE : FALSE;
    NANDErrorFunc = flag ? __NANDShowErrorMessage : NULL;

    OSRestoreInterrupts(enabled);

    return prev;
}

static BOOL __NANDGetAutoErrorMessaging() {
    return NANDErrorFunc ? TRUE : FALSE;
}

void __NANDPrintErrorMessage(s32 result) {
    if (NANDErrorFunc) {
        NANDErrorFunc(result);
    }
}
