#ifndef PRIVATE_ES_TYPES_H
#define PRIVATE_ES_TYPES_H

#include <revolution/types.h>

#include <private/ios/iosctypes.h>

/**
 * https://wiibrew.org/wiki/Title_metadata
 * https://wiibrew.org/wiki/Ticket
 * https://github.com/devkitPro/libogc/blob/master/gc/ogc/es.h
 * https://github.com/iversonjimmy/acer_cloud_wifi_copy/blob/master/sw_x/es_core/esc/core/base/include/estypes.h
 * https://github.com/iversonjimmy/acer_cloud_wifi_copy/blob/master/sw_x/es_core/esc/core/base/include/esitypes.h
 */

typedef s32 ESError;

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
#define ES_ERR_MEMORY_ERROR -1024
#define ES_ERR_NO_TMD_FILE_FOUND -1025
#define ES_ERR_TMD_INVALID_RIGHT -1026
#define ES_ERR_ISSUER_NOT_FOUND -1027
#define ES_ERR_TICKET_NOT_FOUND -1028
#define ES_ERR_INVALID_TICKET -1029
#define ES_ERR_INVALID_BOOT2 -1031
#define ES_ERR_UNKNOWN_FATAL -1032
#define ES_ERR_TICKET_EXPIRED -1033
#define ES_ERR_INVALID_TITLE_VER -1035
#define ES_ERR_BAD_SYSMENU_TICKET -1036
#define ES_ERR_BAD_SYSMENU_CONTENTS -1037
#define ES_ERR_NO_DISC_NAND_TMD -1039

typedef s32 ESFd;

/* COMMON TYPES */

typedef u32 ESDeviceId;
typedef u64 ESTicketId;
typedef u64 ESTitleId;
typedef u32 ESContentId;
typedef u16 ESContentIndex;

#define ES_TITLE_TYPE(t64) ((((ESTitleId)t64 & 0xFFFFFFFF00000000) >> 32))
#define ES_TITLE_CODE(t64) (((ESTitleId)t64 & 0x00000000FFFFFFFF))

#define ES_TITLE_TYPE_NOMASK(t64) ((((ESTitleId)t64) >> 32))
#define ES_TITLE_CODE_NOMASK(t64) (((ESTitleId)t64))

#define ES_TITLE_ID(type, code) ((ESTitleId)((ESTitleId)type << 32 | code))

typedef u16 ESTitleVersion;
typedef u16 ESMinorTitleVersion;
typedef ESTitleId ESSysVersion;

typedef u8 ESLicenseType;
typedef u32 ESTitleType;
typedef u32 ESTitleCode;
typedef u16 ESContentType;
typedef u8 ESSysAccessMask[2];

typedef u8 ESVersion;

typedef u16 ESPropertyMask;
typedef u8 ESCidxMask[64];

/* TICKET TYPES */

typedef u16 ESTicketVersion;

typedef u8 ESTicketCustomData[20];
typedef u8 ESTicketReserved[25];

#define ES_MAX_LIMIT_TYPE 8

#define ES_LC_DURATION_TIME 1
#define ES_LC_ABSOLUTE_TIME 2
#define ES_LC_NUM_TITLES 3
#define ES_LC_NUM_LAUNCH 4
#define ES_LC_ELAPSED_TIME 5

#pragma pack(push, 4)
typedef u32 ESLimitCode;
typedef struct {
    ESLimitCode code;  // 0x00
    u32 limit;         // 0x04
} ESLpEntry;

typedef struct {
    IOSCSigRsa2048 sig;                   // 0x00; RSA 2048-bit sign of the ticket
    IOSCEccPublicKey serverPubKey;        // 0x180; Ticketing server public key
    ESVersion version;                    // 0x1BC; Ticket data structure version number
    ESVersion caCrlVersion;               // 0x1BD; CA CRL version number
    ESVersion signerCrlVersion;           // 0x1BE; Signer CRL version number
    IOSCAesKey titleKey;                  // 0x1BF; Published title key
    ESTicketId ticketId;                  // 0x1D0; Unique 64-bit ticket ID
    ESDeviceId deviceId;                  // 0x1D8; Unique 32-bit device ID
    ESTitleId titleId;                    // 0x1DC; Unique 64-bit title ID
    ESSysAccessMask sysAccessMask;        // 0x1E4; 16-bit cidx mask to indicate which
                                          //        of the first 16 pieces of contents
                                          //        can be accessed by the system app
    ESTicketVersion ticketVersion;        // 0x1E6; 16-bit ticket version
    u32 accessTitleId;                    // 0x1E8; 32-bit title ID for access control
    u32 accessTitleMask;                  // 0x1EC; 32-bit title ID mask
    ESLicenseType licenseType;            // 0x1F0
    u8 keyId;                             // 0x1F1; Common key ID
    ESPropertyMask propertyMask;          // 0x1F2; 16-bit property mask
    ESTicketCustomData customData;        // 0x1F4; 20-byte custom data
    ESTicketReserved reserved;            // 0x208; 25-byte reserved info
    u8 audit;                             // 0x221
    ESCidxMask cidxMask;                  // 0x222; Bit-mask of the content indices
    ESLpEntry limits[ES_MAX_LIMIT_TYPE];  // 0x264; Limited play entries
} ESTicket;

typedef struct {
    ESVersion version;                    // 0x00; Ticket data structure version number
    ESTicketId ticketId;                  // 0x04; Unique 64-bit ticket ID
    ESDeviceId deviceId;                  // 0x0C; Unique 32-bit device ID
    ESTitleId titleId;                    // 0x10; Unique 64-bit title ID
    ESSysAccessMask sysAccessMask;        // 0x18; 16-bit cidx mask to indicate which
                                          //       of the first 16 pieces of contents
                                          //       can be accessed by the system app
    ESTicketVersion ticketVersion;        // 0x1A; 16-bit ticket version
    u32 accessTitleId;                    // 0x1C; 32-bit title ID for access control
    u32 accessTitleMask;                  // 0x20; 32-bit title ID mask
    ESLicenseType licenseType;            // 0x24
    u8 keyId;                             // 0x25; Common key ID
    ESPropertyMask propertyMask;          // 0x26; 16-bit property mask
    ESTicketCustomData customData;        // 0x28; 20-byte custom data
    ESTicketReserved reserved;            // 0x3C; 25-byte reserved info
    u8 audit;                             // 0x55
    ESCidxMask cidxMask;                  // 0x56; Bit-mask of the content indices
    ESLpEntry limits[ES_MAX_LIMIT_TYPE];  // 0x98; Limited play entries
} ESTicketView;
#pragma pack(pop)

/* TMD TYPES */

// typedef u8 ESTmdCustomData[32];
// typedef u8 ESTmdReserved[30];
#pragma pack(push, 1)
typedef struct ESTmdCustomData {
    u32 region;      // 0x00
    u8 ratings[16];  // 0x04
    u8 unk_0x14[4];

    u8 driveSpin;  // 0x18
    u8 unk_0x19[7];
} ESTmdCustomData;
typedef struct ESTmdReserved {
    u8 ipcMask[12];     // 0x00
    u8 empty_0x2C[18];  // 0x0C
} ESTmdReserved;
#pragma pack(pop)

#pragma pack(push, 4)
typedef struct {
    ESContentId cid;       // 0x00; 32-bit content ID
    ESContentIndex index;  // 0x04; Content index, unique per title
    ESContentType type;    // 0x06; Content type
    u64 size;              // 0x08; Unencrypted content size in bytes
    IOSCHash256 hash;      // 0x10; Hash of the content
} ESContentMeta;

typedef struct {
    ESVersion version;                      // 0x00; TMD version number
    ESVersion caCrlVersion;                 // 0x01; CA CRL version number
    ESVersion signerCrlVersion;             // 0x02; Signer CRL version number
    ESSysVersion sysVersion;                // 0x04; System software version number
    ESTitleId titleId;                      // 0x0C; 64-bit title id
    ESTitleType type;                       // 0x14; 32-bit title type
    u16 groupId;                            // 0x18
    ESTmdCustomData customData;             // 0x1A; 32-byte custom data
    ESTmdReserved reserved;                 // 0x3A; 30-byte reserved info
    u32 accessRights;                       // 0x58 Rights to system resources
    ESTitleVersion titleVersion;            // 0x5C; 16-bit title version
    u16 numContents;                        // 0x5E; Number of contents
    ESContentIndex bootIndex;               // 0x60; Boot content index
    ESMinorTitleVersion minorTitleVersion;  // 0x62 16-bit minor title version
} ESTitleMetaHeader;

#define ES_MAX_META_CONTENT 512
typedef struct {
    IOSCSigRsa2048 sig;                           // 0x00; RSA 2048-bit sign of the TMD header
    ESTitleMetaHeader head;                       // 0x180
    ESContentMeta contents[ES_MAX_META_CONTENT];  // 0x1E4; CMD array sorted by content index
} ESTitleMeta;

typedef struct {
    ESContentId cid;       // 0x00; 32-bit content ID
    ESContentIndex index;  // 0x04; Content index, unique per title
    ESContentType type;    // 0x06; Content type
    u64 size;              // 0x08; Unencrypted content size in bytes
} ESContentMetaView;

typedef struct {
    ESVersion version;            // 0x00; TMD version number
    ESVersion caCrlVersion;       // 0x01; CA CRL version number
    ESVersion signerCrlVersion;   // 0x02; Signer CRL version number
    ESSysVersion sysVersion;      // 0x04; System software version number
    ESTitleId titleId;            // 0x0C; 64-bit title id
    ESTitleType type;             // 0x14; 32-bit title type
    u16 groupId;                  // 0x18
    ESTmdCustomData customData;   // 0x1A; 32-byte custom data
    ESTmdReserved reserved;       // 0x3A; 30-byte reserved info
    ESTitleVersion titleVersion;  // 0x58; 16-bit title version
    u16 numContents;              // 0x5A; Number of contents
} ESTmdViewHeader;

typedef struct {
    ESTmdViewHeader head;                             // 0x00
    ESContentMetaView contents[ES_MAX_META_CONTENT];  // 0x5C; CMD array sorted by content index
} ESTmdView;
#pragma pack(pop)

/* MACRO UTILITIES */

#define ES_TITLE_TYPE(t64) ((((ESTitleId)t64 & 0xFFFFFFFF00000000) >> 32))
#define ES_TITLE_CODE(t64) (((ESTitleId)t64 & 0x00000000FFFFFFFF))

#define ES_TITLE_TYPE_NOMASK(t64) ((((ESTitleId)t64) >> 32))
#define ES_TITLE_CODE_NOMASK(t64) (((ESTitleId)t64))

#define ES_TITLE_ID(type, code) ((ESTitleId)((ESTitleId)type << 32 | code))

#endif  // PRIVATE_ES_TYPES_H
