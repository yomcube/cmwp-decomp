#ifndef REVOLUTION_ESI_TYPES_H
#define REVOLUTION_ESI_TYPES_H

#include <revolution/types.h>

#include <revolution/es/estypes.h>
#include <revolution/ios/iosctypes.h>

/**
 * https://wiibrew.org/wiki/Title_metadata
 * https://wiibrew.org/wiki/Ticket
 * https://github.com/devkitPro/libogc/blob/master/gc/ogc/es.h
 * https://github.com/iversonjimmy/acer_cloud_wifi_copy/blob/master/sw_x/es_core/esc/core/base/include/esitypes.h
 */

#define ES_APP_CERT_PREFIX "AP"
#define ES_CP_PREFIX "CP"
#define ES_XS_PREFIX "XS"
#define ES_MS_PREFIX "MS"
#define ES_SP_PREFIX "SP"

typedef u8 ESVersion;
typedef u8 ESTicketCustomData[20];
typedef u8 ESTicketReserved[25];
typedef u16 ESPropertyMask;
typedef u8 ESCidxMask[64];

// typedef u8 ESTmdCustomData[32];
#pragma pack(push, 1)
typedef struct ESTmdCustomData {
    u32 region;      // 0x00
    u8 ratings[16];  // 0x04
    u8 unk_0x14[4];
    u8 driveSpin;  // 0x18
    u8 unk_0x19[7];
} ESTmdCustomData;
#pragma pack(pop)

// typedef u8 ESTmdReserved[30];
#pragma pack(push, 1)
typedef struct ESTmdReserved {
    u8 ipcMask[12];     // 0x00
    u8 empty_0x2C[18];  // 0x0C
} ESTmdReserved;
#pragma pack(pop)

#pragma pack(push, 4)

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

typedef struct {
    IOSCSigRsa2048 sig;                            // 0x00; RSA 2048-bit sign of the TMD header
    ESTitleMetaHeader head;                        // 0x180
    ESContentMeta contents[ES_CONTENT_INDEX_MAX];  // 0x1E4; CMD array sorted by content index
} ESTitleMeta;

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
    ESTmdViewHeader head;                          // 0x00
    ESContentInfo contents[ES_CONTENT_INDEX_MAX];  // 0x5C; CMD array sorted by content index
} ESTmdView;

#pragma pack(pop)

#endif  // REVOLUTION_ESI_TYPES_H
