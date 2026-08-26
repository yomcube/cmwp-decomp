#ifndef REVOLUTION_IOS_CRYPTO_TYPES_H
#define REVOLUTION_IOS_CRYPTO_TYPES_H

#include <revolution/types.h>

/**
 * https://wiibrew.org/wiki/Certificate_chain
 * https://github.com/iversonjimmy/acer_cloud_wifi_copy/blob/master/sw_x/es_core/esc/core/base/include/iosctypes.h
 * https://github.com/iversonjimmy/acer_cloud_wifi_copy/blob/master/sw_x/es_core/esc/core/base/include/iosccert.h
 */

#define IOSC_ERROR_OK 0
#define IOSC_ERROR_ACCESS -2000
#define IOSC_ERROR_EXISTS -2001
#define IOSC_ERROR_INVALID -2002
#define IOSC_ERROR_MAX -2003
#define IOSC_ERROR_NOEXISTS -2004
#define IOSC_ERROR_INVALID_OBJTYPE -2005
#define IOSC_ERROR_INVALID_RNG -2006
#define IOSC_ERROR_INVALID_FLAG -2007
#define IOSC_ERROR_INVALID_FORMAT -2008
#define IOSC_ERROR_INVALID_VERSION -2009
#define IOSC_ERROR_INVALID_SIGNER -2010
#define IOSC_ERROR_FAIL_CHECKVALUE -2011
#define IOSC_ERROR_FAIL_INTERNAL -2012
#define IOSC_ERROR_FAIL_ALLOC -2013
#define IOSC_ERROR_INVALID_SIZE -2014
#define IOSC_ERROR_INVALID_ADDR -2015
#define IOSC_ERROR_INVALID_ALIGN -2016

typedef u32 IOSCCertSigType;
#define IOSC_SIG_RSA4096 0x00010000      /* RSA 4096 bit signature */
#define IOSC_SIG_RSA2048 0x00010001      /* RSA 2048 bit signature */
#define IOSC_SIG_ECC 0x00010002          /* ECC signature 512 bits*/
#define IOSC_SIG_RSA4096_H256 0x00010003 /* RSA 4096 bit sig using SHA-256 */
#define IOSC_SIG_RSA2048_H256 0x00010004 /* RSA 2048 bit sig using SHA-256 */
#define IOSC_SIG_ECC_H256 0x00010005     /* ECC sig 512 bits using SHA-256 */
#define IOSC_SIG_HMAC_SHA1 0x00010006    /* HMAC-SHA1 160 bit signature */

typedef u8 IOSCRsaSig2048[256];
typedef u8 IOSCSigDummy[60];
typedef u8 IOSCName[64];

#define IOSC_ECC_KEYSIZE_BYTES 30
typedef u8 IOSCEccSig[IOSC_ECC_KEYSIZE_BYTES * 2];
typedef u8 IOSCEccPublicKey[IOSC_ECC_KEYSIZE_BYTES * 2];

#define IOSC_AES_KEYSIZE_BYTES 16
typedef u8 IOSCAesKey[IOSC_AES_KEYSIZE_BYTES];

typedef u8 IOSCEccPublicPad[4];

typedef u8 IOSCHash256[20];

typedef struct {
    IOSCCertSigType sigType;  // 0x00
    IOSCRsaSig2048 sig;       // 0x04
    IOSCSigDummy dummy;       // 0x104
    IOSCName issuer;          // 0x140
} IOSCSigRsa2048;

#endif  // REVOLUTION_IOS_CRYPTO_TYPES_H
