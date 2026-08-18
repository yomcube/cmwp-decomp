#include <revolution/esp.h>

#include <revolution/ios.h>
#include <revolution/os.h>

#include <stddef.h>

/**
 * https://wiibrew.org/wiki//dev/es
 * https://github.com/iversonjimmy/acer_cloud_wifi_copy/blob/master/sw_x/es_core/esc/core/base/include/es.h
 */

IOSFd __esFd = IOS_INVALID_FD;

#define ES_WORK_AREA_SIZE 256
#define ES_WORK u8 __esWork[ES_WORK_AREA_SIZE] ALIGN32
#define ES_WORK_AT(x) ((u8*)__esWork + x)

#define ES_VECTOR_ADDR ((IOSIoVector*)ES_WORK_AT(0xD0))

enum {
    ES_IOCTL_IMPORT_TICKET = 1,
    ES_IOCTL_IMPORT_TITLE_INIT = 2,
    ES_IOCTL_IMPORT_CONTENT_BEGIN = 3,
    ES_IOCTL_IMPORT_CONTENT_DATA = 4,
    ES_IOCTL_IMPORT_CONTENT_END = 5,
    ES_IOCTL_IMPORT_TITLE_DONE = 6,
    ES_IOCTL_GET_DEVICE_ID = 7,
    ES_IOCTL_LAUNCH_TITLE = 8,
    ES_IOCTL_OPEN_CONTENT = 9,
    ES_IOCTL_READ_CONTENT = 10,
    ES_IOCTL_CLOSE_CONTENT = 11,
    ES_IOCTL_LIST_OWNED_TITLES = 12,
    ES_IOCTL_LIST_OWNED_TITLES_WITH_COUNT = 13,
    ES_IOCTL_LIST_TITLES_ON_CARD = 14,
    ES_IOCTL_LIST_TITLES_ON_CARD_WITH_COUNT = 15,
    ES_IOCTL_LIST_TITLE_CONTENTS_ON_CARD = 16,
    ES_IOCTL_LIST_TITLE_CONTENTS_ON_CARD_WITH_COUNT = 17,
    ES_IOCTL_GET_TICKET_VIEWS = 18,
    ES_IOCTL_GET_TICKET_VIEWS_WITH_COUNT = 19,
    ES_IOCTL_GET_TMDVIEW = 20,
    ES_IOCTL_GET_TMDVIEW_WITH_SIZE = 21,
    ES_IOCTL_GET_CONSUMPTION = 22,
    ES_IOCTL_DELETE_TICKET = 24,
    ES_IOCTL_GET_DATA_DIR = 29,
    ES_IOCTL_GET_DVD_TICKET_VIEW = 27,
    ES_IOCTL_GET_DEVICE_CERT = 30,
    ES_IOCTL_GET_TITLE_ID = 32,
    ES_IOCTL_DELETE_TITLE_CONTENT = 34,
    ES_IOCTL_SEEK_CONTENT = 35,
    ES_IOCTL_OPEN_TITLE_CONTENT_FILE = 36,
    ES_IOCTL_EXPORT_TITLE_INIT = 38,
    ES_IOCTL_EXPORT_CONTENT_BEGIN = 39,
    ES_IOCTL_EXPORT_CONTENT_DATA = 40,
    ES_IOCTL_EXPORT_CONTENT_END = 41,
    ES_IOCTL_EXPORT_TITLE_DONE = 42,
    ES_IOCTL_IMPORT_TITLE_INIT_ALT = 43,
    ES_IOCTL_ENCRYPT = 44,
    ES_IOCTL_DECRYPT = 45,
    ES_IOCTL_IMPORT_TITLE_CANCEL = 47,
    ES_IOCTL_SIGN = 48,
    ES_IOCTL_VERIFY_SIGN = 49,
    ES_IOCTL_LIST_TMD_CONTENTS_ON_CARD = 50,
    ES_IOCTL_LIST_TMD_CONTENTS_ON_CARD_WITH_COUNT = 51,
    ES_IOCTL_GET_TMD = 52,
    ES_IOCTL_GET_TMD_WITH_SIZE = 53,
    ES_IOCTL_GET_DVD_TMD = 57,
    ES_IOCTL_GET_DVD_TMD_WITH_SIZE = 58,
    ES_IOCTL_DELETE_CONTENT = 62,
    ES_IOCTL_GET_TICKET = 64,
};

ESError ESP_InitLib() {
    IOSFd err;

    err = ES_ERR_OK;
    if (__esFd >= ES_ERR_OK) {
        goto exit;
    }

    __esFd = IOS_Open("/dev/es", 0);
    if (__esFd < ES_ERR_OK) {
        err = __esFd;
    }

exit:
    return err;
}

ESError ESP_CloseLib() {
    IOSFd err;

    err = ES_ERR_OK;
    if (__esFd < ES_ERR_OK) {
        goto exit;
    }

    err = IOS_Close(__esFd);
    if (err != ES_ERR_OK) {
        goto exit;
    }

    __esFd = IOS_INVALID_FD;

exit:
    return err;
}

ESError ESP_ImportTicket(const ESTicket* ticket, void* certs, u32 nCerts, void* crls, u32 nCrls, BOOL unk) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    if (ticket == NULL || certs == NULL || nCerts == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(ticket) || !OSIsAligned32B(certs) || !OSIsAligned32B(crls)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    vec[0].base = (u8*)ticket;
    vec[0].length = sizeof(ESTicket);
    vec[1].base = (u8*)certs;
    vec[1].length = nCerts;
    vec[2].base = (u8*)crls;
    vec[2].length = nCrls;

    if (unk == FALSE) {
        err = IOS_Ioctlv(__esFd, ES_IOCTL_IMPORT_TICKET, 3, 0, vec);
        goto exit;
    }

    err = ES_ERR_INVALID_UNKNOWN;

exit:
    return err;
}

ESError ESP_ImportTitleInit(ESTitleMeta* tmd, u32 tmdSize, void* certs, u32 nCerts, void* crls, u32 nCrls, int unknown0, int unknown1) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    int* pUnknown = (int*)ES_WORK_AT(0x00);

    u32 gotTmdSize;

    if (!OSIsAligned32B(tmd) || !OSIsAligned32B(certs) || !OSIsAligned32B(crls)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    switch (unknown0) {
        case 0: {
            if (tmd == NULL || tmdSize == 0 || certs == NULL || nCerts == 0) {
                err = ES_ERR_INVALID;
                goto exit;
            }
            *pUnknown = unknown1;

            vec[0].base = (u8*)tmd;
            vec[0].length = tmdSize;
            vec[1].base = (u8*)certs;
            vec[1].length = nCerts;
            vec[2].base = (u8*)crls;
            vec[2].length = nCrls;
            vec[3].base = (u8*)pUnknown;
            vec[3].length = sizeof(*pUnknown);

            err = IOS_Ioctlv(__esFd, ES_IOCTL_IMPORT_TITLE_INIT, 4, 0, vec);
            break;
        }
        case 2: {
            if (tmd == NULL) {
                err = ES_ERR_INVALID;
                goto exit;
            }

            ESP_GetTmdSize(tmd, &gotTmdSize);
            if (tmdSize != gotTmdSize) {
                err = ES_ERR_INVALID;
                goto exit;
            }

            vec[0].base = (u8*)tmd;
            vec[0].length = tmdSize;

            err = IOS_Ioctlv(__esFd, ES_IOCTL_IMPORT_TITLE_INIT_ALT, 1, 0, vec);
            break;
        }
        case 1:
        default: {
            err = ES_ERR_INVALID_UNKNOWN;
            break;
        }
    }

exit:
    return err;
}

ESFd ESP_ImportContentBegin(ESTitleId titleId, ESContentId contentId) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);
    ESContentId* pContentId = (ESContentId*)ES_WORK_AT(0x20);

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTitleId = titleId;
    *pContentId = contentId;

    vec[0].base = (u8*)pTitleId;
    vec[0].length = sizeof(*pTitleId);
    vec[1].base = (u8*)pContentId;
    vec[1].length = sizeof(*pContentId);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_IMPORT_CONTENT_BEGIN, 2, 0, vec);

exit:
    return err;
}

ESError ESP_ImportContentData(ESFd fd, void* buf, u32 bufSize) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESFd* pFd = (ESFd*)ES_WORK_AT(0x00);

    if (fd < 0 || buf == NULL || bufSize == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(buf)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pFd = fd;

    vec[0].base = (u8*)pFd;
    vec[0].length = sizeof(*pFd);
    vec[1].base = (u8*)buf;
    vec[1].length = bufSize;

    err = IOS_Ioctlv(__esFd, ES_IOCTL_IMPORT_CONTENT_DATA, 2, 0, vec);

exit:
    return err;
}

ESError ESP_ImportContentEnd(ESFd fd) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESFd* pFd = (ESFd*)ES_WORK_AT(0x00);

    if (fd < 0 || __esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pFd = fd;

    vec[0].base = (u8*)pFd;
    vec[0].length = sizeof(*pFd);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_IMPORT_CONTENT_END, 1, 0, vec);

exit:
    return err;
}

ESError ESP_ImportTitleDone() {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    err = IOS_Ioctlv(__esFd, ES_IOCTL_IMPORT_TITLE_DONE, 0, 0, vec);

exit:
    return err;
}

ESError ESP_ImportTitleCancel() {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    err = IOS_Ioctlv(__esFd, ES_IOCTL_IMPORT_TITLE_CANCEL, 0, 0, vec);

exit:
    return err;
}

ESError ESP_LaunchTitle(ESTitleId titleId, ESTicketView* ticket) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(ticket)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTitleId = titleId;

    vec[0].base = (u8*)pTitleId;
    vec[0].length = sizeof(*pTitleId);
    vec[1].base = (u8*)ticket;
    vec[1].length = sizeof(*ticket);

    err = IOS_IoctlvReboot(__esFd, ES_IOCTL_LAUNCH_TITLE, 2, 0, vec);

    __esFd = -1;

exit:
    return err;
}

ESFd ESP_OpenContentFile(ESContentId contentId) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESContentId* pContentId = (ESContentId*)ES_WORK_AT(0x00);

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pContentId = contentId;

    vec[0].base = (u8*)pContentId;
    vec[0].length = sizeof(*pContentId);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_OPEN_CONTENT, 1, 0, vec);

exit:
    return err;
}

ESFd ESP_OpenTitleContentFile(ESTitleId titleId, ESTicketView* ticketView, ESContentId contentId) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);
    ESContentId* pContentId = (ESContentId*)ES_WORK_AT(0x20);

    if (__esFd < 0 || !OSIsAligned32B(ticketView)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTitleId = titleId;
    *pContentId = contentId;

    vec[0].base = (u8*)pTitleId;
    vec[0].length = sizeof(*pTitleId);
    vec[1].base = (u8*)ticketView;
    vec[1].length = sizeof(*ticketView);
    vec[2].base = (u8*)pContentId;
    vec[2].length = sizeof(*pContentId);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_OPEN_TITLE_CONTENT_FILE, 3, 0, vec);

exit:
    return err;
}

ESError ESP_ReadContentFile(ESFd fd, void* buf, u32 bufSize) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESFd* pFd = (ESFd*)ES_WORK_AT(0x00);

    if (__esFd < 0 || fd < 0 || buf == NULL || bufSize == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(buf)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pFd = fd;

    vec[0].base = (u8*)pFd;
    vec[0].length = sizeof(*pFd);
    vec[1].base = (u8*)buf;
    vec[1].length = bufSize;

    err = IOS_Ioctlv(__esFd, ES_IOCTL_READ_CONTENT, 1, 1, vec);

exit:
    return err;
}

ESError ESP_SeekContentFile(ESFd fd, s32 offset, u32 whence) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    s32* pFd = (s32*)ES_WORK_AT(0x00);
    s32* pOffset = (s32*)ES_WORK_AT(0x20);
    u32* pWhence = (u32*)ES_WORK_AT(0x40);

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pFd = fd;
    *pOffset = offset;
    *pWhence = whence;

    vec[0].base = (u8*)pFd;
    vec[0].length = sizeof(*pFd);
    vec[1].base = (u8*)pOffset;
    vec[1].length = sizeof(*pOffset);
    vec[2].base = (u8*)pWhence;
    vec[2].length = sizeof(*pWhence);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_SEEK_CONTENT, 3, 0, vec);

exit:
    return err;
}

ESError ESP_CloseContentFile(ESFd fd) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    s32* pFd = (s32*)ES_WORK_AT(0x00);

    if (__esFd < 0 || fd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pFd = fd;

    vec[0].base = (u8*)pFd;
    vec[0].length = sizeof(*pFd);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_CLOSE_CONTENT, 1, 0, vec);

exit:
    return err;
}

ESError ESP_ListOwnedTitles(ESTitleId* titleIds, u32* numTitles) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    u32* pNumTitles = (u32*)ES_WORK_AT(0x00);

    if (__esFd < 0 || numTitles == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(titleIds)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (titleIds == NULL) {
        vec[0].base = (u8*)pNumTitles;
        vec[0].length = sizeof(*pNumTitles);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_LIST_OWNED_TITLES, 0, 1, vec);
        if (err == IOS_ERROR_OK) {
            *numTitles = *pNumTitles;
        }
        goto exit;
    } else if (*numTitles == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    } else {
        *pNumTitles = *numTitles;

        vec[0].base = (u8*)pNumTitles;
        vec[0].length = sizeof(*pNumTitles);
        vec[1].base = (u8*)titleIds;
        vec[1].length = *numTitles * sizeof(*titleIds);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_LIST_OWNED_TITLES_WITH_COUNT, 1, 1, vec);
        goto exit;
    }

exit:
    return err;
}

ESError ESP_ListTitlesOnCard(ESTitleId* titleIds, u32* numTitles) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    u32* pNumTitles = (u32*)ES_WORK_AT(0x00);

    if (__esFd < 0 || numTitles == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(titleIds)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (titleIds == NULL) {
        vec[0].base = (u8*)pNumTitles;
        vec[0].length = sizeof(*pNumTitles);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_LIST_TITLES_ON_CARD, 0, 1, vec);
        if (err == IOS_ERROR_OK) {
            *numTitles = *pNumTitles;
        }
        goto exit;
    } else if (*numTitles == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    } else {
        *pNumTitles = *numTitles;

        vec[0].base = (u8*)pNumTitles;
        vec[0].length = sizeof(*pNumTitles);
        vec[1].base = (u8*)titleIds;
        vec[1].length = *numTitles * sizeof(*titleIds);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_LIST_TITLES_ON_CARD_WITH_COUNT, 1, 1, vec);
        goto exit;
    }

exit:
    return err;
}

ESError ESP_ListTitleContentsOnCard(ESTitleId titleId, ESContentId* contentIds, u32* numContents) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);
    u32* pNumContents = (u32*)ES_WORK_AT(0x20);

    if (__esFd < 0 || numContents == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(contentIds)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTitleId = titleId;

    if (contentIds == NULL) {
        vec[0].base = (u8*)pTitleId;
        vec[0].length = sizeof(*pTitleId);
        vec[1].base = (u8*)pNumContents;
        vec[1].length = sizeof(*pNumContents);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_LIST_TITLE_CONTENTS_ON_CARD, 1, 1, vec);
        if (err == IOS_ERROR_OK) {
            *numContents = *pNumContents;
        }
        goto exit;
    } else if (*numContents == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    } else {
        *pNumContents = *numContents;

        vec[0].base = (u8*)pTitleId;
        vec[0].length = sizeof(*pTitleId);
        vec[1].base = (u8*)pNumContents;
        vec[1].length = sizeof(*pNumContents);
        vec[2].base = (u8*)contentIds;
        vec[2].length = *numContents * sizeof(*contentIds);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_LIST_TITLE_CONTENTS_ON_CARD_WITH_COUNT, 2, 1, vec);
        goto exit;
    }

exit:
    return err;
}

ESError ESP_ListTmdContentsOnCard(ESTitleMeta* tmd, u32 tmdSize, ESContentId* contentIds, u32* numContents) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    u32* pNumContents = (u32*)ES_WORK_AT(0x00);

    if (__esFd < 0 || tmd == NULL || tmdSize == 0 || tmdSize > sizeof(ESTitleMeta) || numContents == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(tmd) || !OSIsAligned32B(contentIds)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (contentIds == NULL) {
        vec[0].base = (u8*)tmd;
        vec[0].length = tmdSize;
        vec[1].base = (u8*)pNumContents;
        vec[1].length = sizeof(*pNumContents);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_LIST_TMD_CONTENTS_ON_CARD, 1, 1, vec);
        if (err == IOS_ERROR_OK) {
            *numContents = *pNumContents;
        }
        goto exit;
    } else if (*numContents == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    } else {
        *pNumContents = *numContents;

        vec[0].base = (u8*)tmd;
        vec[0].length = tmdSize;
        vec[1].base = (u8*)pNumContents;
        vec[1].length = sizeof(*pNumContents);
        vec[2].base = (u8*)contentIds;
        vec[2].length = *numContents * sizeof(*contentIds);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_LIST_TMD_CONTENTS_ON_CARD_WITH_COUNT, 2, 1, vec);
        goto exit;
    }

exit:
    return err;
}

ESError ESP_GetTicketViews(ESTitleId titleId, ESTicketView* ticketViews, u32* numTicketViews) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);
    u32* pNumTicketViews = (u32*)ES_WORK_AT(0x20);

    if (__esFd < 0 || numTicketViews == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(ticketViews)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTitleId = titleId;

    if (ticketViews == NULL) {
        vec[0].base = (u8*)pTitleId;
        vec[0].length = sizeof(*pTitleId);
        vec[1].base = (u8*)pNumTicketViews;
        vec[1].length = sizeof(*pNumTicketViews);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_TICKET_VIEWS, 1, 1, vec);
        if (err == IOS_ERROR_OK) {
            *numTicketViews = *pNumTicketViews;
        }
        goto exit;
    } else if (*numTicketViews == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    } else {
        *pNumTicketViews = *numTicketViews;

        vec[0].base = (u8*)pTitleId;
        vec[0].length = sizeof(*pTitleId);
        vec[1].base = (u8*)pNumTicketViews;
        vec[1].length = sizeof(*pNumTicketViews);
        vec[2].base = (u8*)ticketViews;
        vec[2].length = *numTicketViews * sizeof(*ticketViews);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_TICKET_VIEWS_WITH_COUNT, 2, 1, vec);
        goto exit;
    }

exit:
    return err;
}

ESError ESP_DiGetTicketView(void* ticket, ESTicketView* ticketView) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    if (__esFd < 0 || ticketView == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(ticket) || !OSIsAligned32B(ticketView)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    vec[0].base = (u8*)ticket;
    if (ticket == NULL) {
        vec[0].length = 0;
    } else {
        vec[0].length = sizeof(ESTicket);
    }

    vec[1].base = (u8*)ticketView;
    vec[1].length = sizeof(*ticketView);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_DVD_TICKET_VIEW, 1, 1, vec);

exit:
    return err;
}

ESError ESP_DiGetTmd(ESTitleMeta* tmd, u32* tmdSize) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    u32* pTmdSize = (u32*)ES_WORK_AT(0x00);

    if (__esFd < 0 || tmdSize == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(tmd)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (tmd == NULL) {
        vec[0].base = (u8*)pTmdSize;
        vec[0].length = sizeof(*pTmdSize);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_DVD_TMD, 0, 1, vec);
        if (err == IOS_ERROR_OK) {
            *tmdSize = *pTmdSize;
        }
        goto exit;
    } else if (*tmdSize == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    } else {
        *pTmdSize = *tmdSize;

        vec[0].base = (u8*)pTmdSize;
        vec[0].length = sizeof(*pTmdSize);
        vec[1].base = (u8*)tmd;
        vec[1].length = *tmdSize;

        err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_DVD_TMD_WITH_SIZE, 1, 1, vec);
        goto exit;
    }

exit:
    return err;
}

ESError ESP_GetTmd(ESTitleId titleId, ESTitleMeta* tmd, u32* tmdSize) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);
    u32* pTmdSize = (u32*)ES_WORK_AT(0x20);

    if (__esFd < 0 || tmdSize == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(tmd)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTitleId = titleId;

    if (tmd == NULL) {
        vec[0].base = (u8*)pTitleId;
        vec[0].length = sizeof(*pTitleId);
        vec[1].base = (u8*)pTmdSize;
        vec[1].length = sizeof(*pTmdSize);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_TMD, 1, 1, vec);
        if (err == IOS_ERROR_OK) {
            *tmdSize = *pTmdSize;
        }
        goto exit;
    } else if (*tmdSize == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    } else {
        *pTmdSize = *tmdSize;

        vec[0].base = (u8*)pTitleId;
        vec[0].length = sizeof(*pTitleId);
        vec[1].base = (u8*)pTmdSize;
        vec[1].length = sizeof(*pTmdSize);
        vec[2].base = (u8*)tmd;
        vec[2].length = *tmdSize;

        err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_TMD_WITH_SIZE, 2, 1, vec);
        goto exit;
    }

exit:
    return err;
}

ESError ESP_GetTmdSize(ESTitleMeta* tmd, u32* tmdSize) {
    ESError err = ES_ERR_OK;

    if (tmd == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *tmdSize = (tmd->head.numContents * 0x24) + (sizeof(IOSCSigRsa2048) + sizeof(ESTitleMetaHeader)) /* todo: figure these out. */;

exit:
    return err;
}

ESError ESP_GetTmdView(ESTitleId titleId, ESTmdView* tmdView, u32* tmdSize) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);
    u32* pTmdSize = (u32*)ES_WORK_AT(0x20);

    if (__esFd < 0 || tmdSize == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(tmdView)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTitleId = titleId;
    if (tmdView == NULL) {
        vec[0].base = (u8*)pTitleId;
        vec[0].length = sizeof(*pTitleId);

        vec[1].base = (u8*)pTmdSize;
        vec[1].length = sizeof(*pTmdSize);

        err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_TMDVIEW, 1, 1, vec);
        if (err == IOS_ERROR_OK) {
            *tmdSize = *pTmdSize;
        }
        goto exit;
    } else if (*tmdSize == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    } else {
        *pTmdSize = *tmdSize;

        vec[0].base = (u8*)pTitleId;
        vec[0].length = sizeof(*pTitleId);

        vec[1].base = (u8*)pTmdSize;
        vec[1].length = sizeof(*pTmdSize);

        vec[2].base = (u8*)tmdView;
        vec[2].length = *tmdSize;

        err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_TMDVIEW_WITH_SIZE, 2, 1, vec);
        goto exit;
    }

exit:
    return err;
}

ESError ESP_GetDataDir(ESTitleId titleId, char* dataDir) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);

    if (__esFd < 0 || dataDir == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(dataDir)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTitleId = titleId;
    vec[0].base = (u8*)pTitleId;
    vec[0].length = sizeof(*pTitleId);

    vec[1].base = (u8*)dataDir;
    vec[1].length = sizeof(*dataDir) * 30;

    err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_DATA_DIR, 1, 1, vec);

exit:
    return err;
}

ESError ESP_GetTitleId(ESTitleId* titleId) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    if (__esFd < 0 || titleId == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    vec[0].base = ES_WORK_AT(0x00);
    vec[0].length = sizeof(ESTitleId);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_TITLE_ID, 0, 1, vec);
    if (err == IOS_ERROR_OK) {
        *titleId = *(ESTitleId*)ES_WORK_AT(0x00);
    }

exit:
    return err;
}

ESError ESP_GetDeviceId(ESDeviceId* deviceId) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    if (__esFd < 0 || deviceId == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    vec[0].base = ES_WORK_AT(0x00);
    vec[0].length = sizeof(ESDeviceId);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_DEVICE_ID, 0, 1, vec);
    if (err == IOS_ERROR_OK) {
        *deviceId = *(ESDeviceId*)ES_WORK_AT(0x00);
    }

exit:
    return err;
}

ESError ESP_GetConsumption(ESTicketId ticketId, ESLpEntry* limitEntries, u32* numEntries) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTicketId = (ESTitleId*)ES_WORK_AT(0x00);
    u32* pNumEntries = (u32*)ES_WORK_AT(0x20);

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(limitEntries)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTicketId = ticketId;
    vec[0].base = (u8*)pTicketId;
    vec[0].length = sizeof(ticketId);

    if (limitEntries == NULL) {
        vec[1].base = NULL;
        vec[1].length = 0;
    } else {
        vec[1].base = (u8*)limitEntries;
        vec[1].length = *numEntries * sizeof(*limitEntries);
        *pNumEntries = *numEntries;
    }

    vec[2].base = (u8*)pNumEntries;
    vec[2].length = sizeof(*pNumEntries);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_CONSUMPTION, 1, 2, vec);
    *numEntries = *pNumEntries;

exit:
    return err;
}

ESError ESP_GetDeviceCert(IOSCSigRsa2048* deviceCert) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    if (__esFd < 0 || deviceCert == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!IS_ALIGNED(deviceCert, 64)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    vec[0].base = (u8*)deviceCert;
    vec[0].length = sizeof(IOSCSigRsa2048);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_DEVICE_CERT, 0, 1, vec);

exit:
    return err;
}

ESError ESP_Encrypt(u32 keyNum, u8* iv, u8* input, u32 size, u8* output) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    u32* pKeyNum = (u32*)ES_WORK_AT(0x00);
    u32* pSize = (u32*)ES_WORK_AT(0x20);

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pKeyNum = keyNum;
    *pSize = size;
    vec[0].base = (u8*)pKeyNum;
    vec[0].length = sizeof(*pKeyNum);

    // IV read
    vec[1].base = (u8*)iv;
    vec[1].length = 16;

    vec[2].base = (u8*)input;
    vec[2].length = size;

    // IV write
    vec[3].base = (u8*)iv;
    vec[3].length = 16;

    vec[4].base = (u8*)output;
    vec[4].length = size;

    err = IOS_Ioctlv(__esFd, ES_IOCTL_ENCRYPT, 3, 2, vec);

exit:
    return err;
}

ESError ESP_Decrypt(u32 keyNum, u8* iv, u8* input, u32 size, u8* output) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    u32* pKeyNum = (u32*)ES_WORK_AT(0x00);
    u32* pSize = (u32*)ES_WORK_AT(0x20);

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pKeyNum = keyNum;
    *pSize = size;
    vec[0].base = (u8*)pKeyNum;
    vec[0].length = sizeof(*pKeyNum);

    // IV read
    vec[1].base = (u8*)iv;
    vec[1].length = 16;

    vec[2].base = (u8*)input;
    vec[2].length = size;

    // IV write
    vec[3].base = (u8*)iv;
    vec[3].length = 16;

    vec[4].base = (u8*)output;
    vec[4].length = size;

    err = IOS_Ioctlv(__esFd, ES_IOCTL_DECRYPT, 3, 2, vec);

exit:
    return err;
}

ESError ESP_Sign(void* data, u32 dataSize, IOSCSigDummy sig, IOSCSigRsa2048* sigSize) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    if (__esFd < 0 || data == NULL || sig == NULL || sigSize == NULL) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(data) || !OSIsAligned32B(sig) || !OSIsAligned32B(sigSize)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    vec[0].base = (u8*)data;
    vec[0].length = dataSize;
    vec[1].base = (u8*)sig;
    vec[1].length = sizeof(IOSCSigDummy);
    vec[2].base = (u8*)sigSize;
    vec[2].length = sizeof(*sigSize);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_SIGN, 1, 2, vec);

exit:
    return err;
}

ESError ESP_VerifySign(void* data, u32 dataSize, IOSCSigDummy sig, void* certs, u32 nCerts) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    if (__esFd < 0 || data == NULL || sig == NULL || certs == NULL || nCerts == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!IS_ALIGNED(data, 64) || !OSIsAligned32B(sig) || !OSIsAligned32B(certs)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    vec[0].base = (u8*)data;
    vec[0].length = dataSize;

    vec[1].base = (u8*)sig;
    vec[1].length = sizeof(IOSCSigDummy);

    vec[2].base = (u8*)certs;
    vec[2].length = nCerts;

    err = IOS_Ioctlv(__esFd, ES_IOCTL_VERIFY_SIGN, 3, 0, vec);

exit:
    return err;
}

ESError ESP_DeleteTitleContent(ESTitleId titleId) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTitleId = titleId;

    vec[0].base = (u8*)pTitleId;
    vec[0].length = sizeof(*pTitleId);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_DELETE_TITLE_CONTENT, 1, 0, vec);

exit:
    return err;
}

ESError ESP_DeleteContent(ESTitleId titleId, ESContentId contentId) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);
    ESContentId* pContentId = (ESContentId*)ES_WORK_AT(0x20);

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTitleId = titleId;
    *pContentId = contentId;

    vec[0].base = (u8*)pTitleId;
    vec[0].length = sizeof(*pTitleId);
    vec[1].base = (u8*)pContentId;
    vec[1].length = sizeof(*pContentId);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_DELETE_CONTENT, 2, 0, vec);

exit:
    return err;
}

ESError ESP_ExportTitleInit(ESTitleId titleId, ESDeviceId deviceId, ESTicketId ticketId, void* certs, u32 nCerts, void* crls, u32 nCrls, int unknown,
                            void* ticket, void* tmd, u32 tmdSize) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);

    if (!OSIsAligned32B(tmd) || !OSIsAligned32B(ticket) || !OSIsAligned32B(certs) || !OSIsAligned32B(crls)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    switch (unknown) {
        case 2: {
            // @bug: Value is not returned.
            if (tmd == NULL || tmdSize == 0) {
                err = ES_ERR_INVALID;
            }

            *pTitleId = titleId;

            vec[0].base = (u8*)pTitleId;
            vec[0].length = sizeof(*pTitleId);
            vec[1].base = (u8*)tmd;
            vec[1].length = tmdSize;

            err = IOS_Ioctlv(__esFd, ES_IOCTL_EXPORT_TITLE_INIT, 1, 1, vec);
            goto exit;
        }
        default: {
            err = ES_ERR_INVALID_UNKNOWN;
            goto exit;
        }
    }

exit:
    return err;
}

ESError ESP_ExportContentBegin(ESTitleId titleId, ESContentId contentId) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESTitleId* pTitleId = (ESTitleId*)ES_WORK_AT(0x00);
    ESContentId* pContentId = (ESContentId*)ES_WORK_AT(0x20);

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pTitleId = titleId;
    *pContentId = contentId;

    vec[0].base = (u8*)pTitleId;
    vec[0].length = sizeof(*pTitleId);
    vec[1].base = (u8*)pContentId;
    vec[1].length = sizeof(*pContentId);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_EXPORT_CONTENT_BEGIN, 2, 0, vec);

exit:
    return err;
}

ESError ESP_ExportContentData(ESFd fd, void* data, u32 dataSize) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESFd* pFd = (ESFd*)ES_WORK_AT(0x00);

    if (fd < 0 || data == NULL || dataSize == 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(data)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pFd = fd;

    vec[0].base = (u8*)pFd;
    vec[0].length = sizeof(*pFd);
    vec[1].base = (u8*)data;
    vec[1].length = dataSize;

    err = IOS_Ioctlv(__esFd, ES_IOCTL_EXPORT_CONTENT_DATA, 1, 1, vec);

exit:
    return err;
}

ESError ESP_ExportContentEnd(ESFd fd) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    ESFd* pFd = (ESFd*)ES_WORK_AT(0x00);

    if (fd < 0 || __esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    *pFd = fd;

    vec[0].base = (u8*)pFd;
    vec[0].length = sizeof(*pFd);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_EXPORT_CONTENT_END, 1, 0, vec);

exit:
    return err;
}

ESError ESP_ExportTitleDone() {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    err = IOS_Ioctlv(__esFd, ES_IOCTL_EXPORT_TITLE_DONE, 0, 0, vec);

exit:
    return err;
}

ESError ESP_GetTicket(ESTicketView* ticketView, ESTicket* ticket) {
    ES_WORK;

    ESError err = ES_ERR_OK;

    IOSIoVector* vec = ES_VECTOR_ADDR;

    if (__esFd < 0) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    if (!OSIsAligned32B(ticketView) || !OSIsAligned32B(ticket)) {
        err = ES_ERR_INVALID;
        goto exit;
    }

    vec[0].base = (u8*)ticketView;
    vec[0].length = sizeof(*ticketView);
    vec[1].base = (u8*)ticket;
    vec[1].length = sizeof(*ticket);

    err = IOS_Ioctlv(__esFd, ES_IOCTL_GET_TICKET, 1, 1, vec);

exit:
    return err;
}
