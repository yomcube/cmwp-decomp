#ifndef REVOLUTION_ES_TYPES_H
#define REVOLUTION_ES_TYPES_H

#include <revolution/types.h>

/**
 * https://wiibrew.org/wiki/Title_metadata
 * https://wiibrew.org/wiki/Ticket
 * https://github.com/devkitPro/libogc/blob/master/gc/ogc/es.h
 * https://github.com/iversonjimmy/acer_cloud_wifi_copy/blob/master/sw_x/es_core/esc/core/base/include/estypes.h
 * https://github.com/iversonjimmy/acer_cloud_wifi_copy/blob/master/sw_x/es_core/esc/core/base/include/esitypes.h
 */

#define ES_ERR_OK 0
#define ES_ERR_DONT_EXISTS -106
#define ES_ERR_INVALID_PUB_KEY_TYPE -1005
#define ES_ERR_FILE_READ_FAILED -1009
#define ES_ERR_FILE_WRITE_FAILED -1010
#define ES_ERR_INVALID_SIGNATURE -1012
#define ES_ERR_TMD_MAXFD -1016
#define ES_ERR_INVALID -1017
#define ES_ERR_INVALID_UNKNOWN -1019
#define ES_ERR_INVALID_DEVICE_ID -1020
#define ES_ERR_INVALID_CONTENT_HASH -1022
#define ES_ERR_NO_MEMORY -1024
#define ES_ERR_NO_TMD_FILE -1025
#define ES_ERR_NO_RIGHT -1026
#define ES_ERR_ISSUER_NOT_FOUND -1027
#define ES_ERR_TICKET_NOT_FOUND -1028
#define ES_ERR_INVALID_TICKET -1029
#define ES_ERR_INVALID_BOOT2 -1031
#define ES_ERR_UNKNOWN_FATAL -1032
#define ES_ERR_NO_COMMON_2 -1034
#define ES_ERR_INVALID_TITLE_VER -1035
#define ES_ERR_BAD_SYSMENU_TICKET -1036
#define ES_ERR_BAD_SYSMENU_CONTENTS -1037
#define ES_ERR_NO_DISC_NAND_TMD -1039

// ES signature types
#define ES_SIG_TYPE_ECC_SHA1 1
#define ES_SIG_TYPE_ECC_SHA256 2

// ES license types
#define ES_LICENSE_MASK 0x0F
#define ES_LICENSE_PERMANENT 0
#define ES_LICENSE_DEMO 1
#define ES_LICENSE_TRIAL 2
#define ES_LICENSE_RENTAL 3
#define ES_LICENSE_SUBSCRIPTION 4
#define ES_LICENSE_SERVICE 5

// ES title-level limit codes
#define ES_MAX_LIMIT_TYPE 8

#define ES_LC_DURATION_TIME 1
#define ES_LC_ABSOLUTE_TIME 2
#define ES_LC_NUM_TITLES 3
#define ES_LC_NUM_LAUNCH 4
#define ES_LC_ELAPSED_TIME 5

// ES title type
#define ES_TITLE_TYPE_NORMAL 1
#define ES_TITLE_TYPE_DATA 8
#define ES_TITLE_TYPE_CT_TITLE 0x40

// ES content type
#define ES_CONTENT_TYPE_ENCRYPTED 0x1
#define ES_CONTENT_TYPE_DISC 0x2
#define ES_CONTENT_TYPE_CFM 0x4
#define ES_CONTENT_TYPE_OPTIONAL 0x4000
#define ES_CONTENT_TYPE_SHARED 0x8000

// Certificate and signature sizes
#define ES_DEVICE_CERT_SIZE 384
#define ES_SIGNING_CERT_SIZE 384
#define ES_SIGNATURE_SIZE 60

// Key Identifiers
#define ES_KEYID_NORMAL 0 /* Normal common key */
#define ES_KEYID_NEW 1    /* "New common key" (AKA Korean Common Key/"Common Key 2") */
#define ES_KEYID_CAFE 1   /* Wii U common key */

#define ES_CONTENT_INDEX_MAX 512

typedef s32 ESFd;
typedef s32 ESError;

typedef u32 ESDeviceId;
typedef u64 ESTicketId;
typedef u64 ESTitleId;
typedef u32 ESContentId;
typedef u16 ESContentIndex;

typedef u16 ESTitleVersion;
typedef u16 ESTicketVersion;
typedef u16 ESMinorTitleVersion;
typedef ESTitleId ESSysVersion;

typedef u32 ESSigType;
typedef u8 ESLicenseType;
typedef u32 ESTitleType;
typedef u16 ESContentType;
typedef u8 ESSysAccessMask[2];

#pragma pack(push, 4)

typedef struct {
    ESContentId id;        // 0x00
    ESContentIndex index;  // 0x04
    ESContentType type;    // 0x06
    u64 size;              // 0x08
} ESContentInfo;

typedef u32 ESLimitCode;

typedef struct {
    ESLimitCode code;  // 0x00
    u32 limit;         // 0x04
} ESLpEntry;

#pragma pack(pop)

#define ES_TITLE_TYPE(t64) ((((ESTitleId)t64 & 0xFFFFFFFF00000000) >> 32))
#define ES_TITLE_CODE(t64) (((ESTitleId)t64 & 0x00000000FFFFFFFF))

#define ES_TITLE_TYPE_NOMASK(t64) ((((ESTitleId)t64) >> 32))
#define ES_TITLE_CODE_NOMASK(t64) (((ESTitleId)t64))

#define ES_TITLE_ID(type, code) ((ESTitleId)((ESTitleId)type << 32 | code))

#endif  // REVOLUTION_ES_TYPES_H
