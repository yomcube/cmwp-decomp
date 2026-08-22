#include <revolution/verdefs.h>

#include <stdio.h>

#if defined(SDK_20091112)
#ifdef DEBUG
SDKDefineVersion(EUART, "Dec 11 2009", "15:54:32");
#else
SDKDefineVersion(EUART, "Dec 11 2009", "15:59:11");
#endif
#elif defined(SDK_20090224)
SDKDefineVersion(EUART, "Jul 30 2008", "19:24:23");
#endif

#include <revolution/exi.h>
#include <revolution/os.h>

#define BarnacleEnabled 0
#define Chan EXI_CHAN_0
#define Dev EXI_DEV_INT

static u32 Enabled;

static BOOL __EUARTInitialized = FALSE;
static int __EUARTLastErrorCode = 0;
static BOOL __EUARTSendStop = FALSE;

BOOL EUARTInit() {
    BOOL enabled;
    u8 val;

    if (__EUARTInitialized) {
        return TRUE;
    }

    if ((OSGetConsoleType() & OS_CONSOLE_MASK_DEV) == 0) {
        __EUARTLastErrorCode = EUART_ERR_INVALID;
        return FALSE;
    }

    enabled = OSDisableInterrupts();

    val = 0xF2;
    if (!EXIWriteReg(Chan, Dev, 0xB0000000, &val, sizeof(val))) {
        __EUARTLastErrorCode = EUART_ERR_EXI_ERROR;
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    val = 0xF3;
    if (!EXIWriteReg(Chan, Dev, 0xB0000000, &val, sizeof(val))) {
        __EUARTLastErrorCode = EUART_ERR_EXI_ERROR;
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    OSRestoreInterrupts(enabled);

    __EUARTInitialized = TRUE;
    __EUARTLastErrorCode = EUART_ERR_OK;
    __EUARTSendStop = FALSE;

    return TRUE;
}

void EUARTExit() {
    __EUARTInitialized = FALSE;
}

int EUARTPrintf(const char* msg, ...) {
    va_list marker;
    int num;

    if (!__EUARTInitialized) {
        __EUARTLastErrorCode = EUART_ERR_NOT_INITIALIZED;
        return -1;
    }

    if (__EUARTSendStop) {
        return 0;
    }

    va_start(marker, msg);
    num = vprintf(msg, marker);
    va_end(marker);

    return num;
}

BOOL EUARTSetStatus(int status) {
    if (!__EUARTInitialized) {
        __EUARTLastErrorCode = EUART_ERR_NOT_INITIALIZED;
        return FALSE;
    }

    __EUARTSendStop = status;
    return TRUE;
}

int EUARTGetLastError() {
    return __EUARTLastErrorCode;
}

int InitializeUART() {
    if (BarnacleEnabled == 0xA5FF005A) {
        return EUART_ERR_OK;
    }

    if ((OSGetConsoleType() & OS_CONSOLE_MASK_DEV) == 0) {
        Enabled = 0;
        return EUART_ERR_INVALID;
    }

    Enabled = 0xA5FF005A;
    return EUART_ERR_OK;
}

int ReadUARTN() {
    return EUART_ERR_READ_UNSUPPORTED;
}

static int QueueLength() {
    u32 cmd;
    u32 val;

    if (!EXISelect(Chan, Dev, __EXIFreq)) {
        return -1;
    }

    cmd = 0x30000100;

    EXIImm(Chan, &cmd, sizeof(cmd), EXI_WRITE, NULL);
    EXISync(Chan);
    EXIImm(Chan, &val, 4, EXI_READ, NULL);
    EXISync(Chan);
    EXIDeselect(Chan);

    return 0x20 - (u8)((val >> 0x18) & 0x3F);
}

int WriteUARTN(void* buf, u32 len) {
    u32 cmd;
    int qLen;
    char* ptr;
    char* ptr2;
    int error;

    ptr = (char*)buf;

    if ((Enabled - 0xA5FF0000) != 0x5A) {
        return EUART_ERR_INVALID;
    }

    if (!__EUARTInitialized && !EUARTInit()) {
        return EUART_ERR_INVALID;
    }

    if (!__EUARTInitialized) {
        __EUARTLastErrorCode = EUART_ERR_NOT_INITIALIZED;
        return EUART_ERR_INVALID;
    }

    if (!EXILock(Chan, Dev, NULL)) {
        return EUART_ERR_OK;
    }

    ptr2 = ptr;
    while ((u32)ptr2 - (u32)buf < len) {
        if (*(s8*)ptr2 == 0xA) {
            *ptr2 = 0xD;
        }
        ptr2++;
    }

    error = EUART_ERR_OK;
    cmd = 0xB0000100;

    while (len != 0) {
        qLen = QueueLength();
        if (qLen < 0) {
            error = EUART_ERR_BUSY;
            break;
        }

        if (qLen == 32) {
            if (!EXISelect(Chan, Dev, __EXIFreq)) {
                error = EUART_ERR_BUSY;
                break;
            }

            EXIImm(Chan, &cmd, sizeof(cmd), EXI_WRITE, NULL);
            EXISync(Chan);

            while ((qLen > 0) && (len != 0)) {
                u32 val = (u32)((*ptr & 0xFF) << 0x18);

                EXIImm(Chan, &val, 4, EXI_WRITE, NULL);
                EXISync(Chan);
                ((u8*)ptr)++;
                qLen--;
                len--;
            }
            EXIDeselect(Chan);
        }
    }

    EXIUnlock(Chan);
    return error;
}
