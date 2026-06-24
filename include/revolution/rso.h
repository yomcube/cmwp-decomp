#ifndef RSO_H
#define RSO_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RSO_VERSION 1
#define RSO_FAR_JUMP_SIZE 24

typedef struct RSOObjectHeader RSOObjectHeader;
typedef u32 RSOHash;
typedef struct RSOObjectList RSOObjectList;
typedef struct RSOObjectLink RSOObjectLink;
typedef struct RSOObjectInfo RSOObjectInfo;
typedef struct RSOSymbolHeader RSOSymbolHeader;
typedef struct RSOSectionInfo RSOSectionInfo;
typedef struct RSOExportTable RSOExportTable;
typedef struct RSOImportTable RSOImportTable;
typedef struct RSORel RSORel;
typedef struct RSOExportFuncTable RSOExportFuncTable;

enum RSOFL { RSO_FL_NON, RSO_FL_INTERNAL, RSO_FL_EXTERNAL };

struct RSOImportTable {
    u32 strOffset;
    u32 value;
    u32 relOffset;
};
struct RSOExportTable {
    u32 strOffset;
    u32 value;
    u32 section;
    u32 hash;
};

struct RSOExportFuncTable {
    const char* symbol_name;
    u32* symbol_ptr;
};

struct RSOObjectList {
    RSOObjectInfo* head;
    RSOObjectInfo* tail;
};

struct RSOObjectLink {
    RSOObjectInfo* next;
    RSOObjectInfo* prev;
};

struct RSOObjectInfo {
    /*0x00*/ RSOObjectLink link;
    /*0x08*/ u32 numSections;
    /*0x0C*/ u32 sectionInfoOffset;
    /*0x10*/ u32 nameOffset;
    /*0x14*/ u32 nameSize;
    /*0x18*/ u32 version;
};

struct RSOSymbolHeader {
    u32 tableOffset;
    u32 tableSize;
    u32 stringOffset;
};

struct RSOObjectHeader {
    /*0x00*/ RSOObjectInfo info;
    /*0x1C*/ u32 bssSize;
    /*0x20*/ u8 prologSection;
    /*0x21*/ u8 epilogSection;
    /*0x22*/ u8 unresolvedSection;
    /*0x23*/ u8 bssSection;
    /*0x24*/ u32 prolog;
    /*0x28*/ u32 epilog;
    /*0x2C*/ u32 unresolved;
    /*0x30*/ u32 internalRelOffset;
    /*0x34*/ u32 internalRelSize;
    /*0x38*/ u32 externalRelOffset;
    /*0x3C*/ u32 externalRelSize;
    /*0x40*/ RSOSymbolHeader expHeader;
    /*0x4C*/ RSOSymbolHeader impHeader;
};

struct RSOSectionInfo {
    u32 offset;
    u32 size;
};

struct RSORel {
    u32 offset;
    RSOObjectInfo* info;
    u32 addend;
};

int RSOLocateObject(void*, void*);
int RSOLocateObjectFixed(void*, void*);

void RSOUnresolveImportSymbolAll(void* rso);

BOOL RSOListInit(void*);
BOOL RSOLinkList(void*, void*);

BOOL RSOIsImportSymbolResolvedAll(const RSOObjectHeader*);

int RSOGetJumpCodeSize(const RSOObjectHeader*);
int RSOGetJumpCodeSize(const RSOObjectHeader*);
void RSOMakeJumpCode(const RSOObjectHeader*, void*);

const void* RSOFindExportSymbolAddr(const RSOObjectHeader*, const char*);
int RSOLinkJump(RSOObjectHeader*, const RSOObjectHeader*, void*);

void RSORelocateImportSymbol(RSOObjectHeader* rso, RSOImportTable* impTab, int impIndex);

#ifdef __cplusplus
}
#endif

#endif  // RSO_H
