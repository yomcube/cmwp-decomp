#ifndef REVOLUTION_RSO_H
#define REVOLUTION_RSO_H

#include <revolution/types.h>

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

typedef enum {
    RSO_FL_NON = 0,
    RSO_FL_INTERNAL,
    RSO_FL_EXTERNAL,
} RSOFixedLevel;

struct RSOImportTable {
    u32 strOffset;  // 0x00
    u32 value;      // 0x04
    u32 relOffset;  // 0x08
};
struct RSOExportTable {
    u32 strOffset;  // 0x00
    u32 value;      // 0x04
    u32 section;    // 0x08
    RSOHash hash;   // 0x0C
};

// not part of DWARF from the original object
// but is part of DWARF from other games
struct RSOExportFuncTable {
    const char* symbol_name;  // 0x00
    u32* symbol_ptr;          // 0x04
};

struct RSOObjectList {
    RSOObjectInfo* head;  // 0x00
    RSOObjectInfo* tail;  // 0x04
};

struct RSOObjectLink {
    RSOObjectInfo* next;  // 0x00
    RSOObjectInfo* prev;  // 0x04
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
    u32 tableOffset;   // 0x00
    u32 tableSize;     // 0x04
    u32 stringOffset;  // 0x08
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
    u32 offset;  // 0x00
    u32 size;    // 0x04
};

struct RSORel {
    u32 offset;  // 0x00
    u32 info;    // 0x04
    u32 addend;  // 0x08
};

DECL_WEAK void RSONotifyModuleLoaded(RSOObjectHeader* moduleHeader);
DECL_WEAK void RSONotifyModuleUnloaded(RSOObjectHeader* moduleHeader);
DECL_WEAK void RSONotifyPreRSOLink(RSOObjectHeader* impHeader, const RSOObjectHeader* expHeader);
DECL_WEAK void RSONotifyPostRSOLink(RSOObjectHeader* impHeader, const RSOObjectHeader* expHeader);
DECL_WEAK void RSONotifyPreRSOLinkFar(RSOObjectHeader* moduleHeader, const RSOObjectHeader* expHeader, void* buff);
DECL_WEAK void RSONotifyPostRSOLinkFar(RSOObjectHeader* moduleHeader, const RSOObjectHeader* expHeader, void* buff);

BOOL RSOLocateObject(void* newModule, void* bss);
BOOL RSOLocateObjectFixed(void* newModule, void* bss);
BOOL RSOStaticLocateObject(void* newModule);

BOOL RSOUnLocateObject(void* oldModule);

RSOImportTable* RSOGetImport(const RSOSymbolHeader* imp);
RSOExportTable* RSOGetExport(const RSOSymbolHeader* exp);

int RSOLink(RSOObjectHeader* rsoImp, const RSOObjectHeader* rsoExp);
void RSOUnLink(RSOObjectHeader* rsoImp, const RSOObjectHeader* rsoExp)
#if SDK_VERSION < 20091211  // :/
    NO_INLINE
#endif
    ;

RSOHash RSOGetHash(const char* symbolname);

int RSOGetNumImportSymbols(const RSOSymbolHeader* imp);
int RSOGetNumImportSymbolsUnresolved(const RSOObjectHeader* rso);
char* RSOGetImportSymbolName(const RSOSymbolHeader* imp, int index);
BOOL RSOIsImportSymbolResolved(const RSOObjectHeader* rso, int index);
BOOL RSOIsImportSymbolResolvedAll(const RSOObjectHeader* rso);

int RSOGetNumExportSymbols(const RSOSymbolHeader* exp);
char* RSOGetExportSymbolName(const RSOSymbolHeader* exp, int index);
void* RSOGetExportSymbolAddr(const RSOObjectHeader* rso, int index);
void* RSOFindExportSymbolAddr(const RSOObjectHeader* rso, const char* name);
RSOExportTable* RSOFindExportSymbol(const RSOObjectHeader* rso, const char* name);

BOOL RSOListInit(void* staticRso);
BOOL RSOLinkList(void* newRso, void* bss);
BOOL RSOLinkListFixed(void* newRso, void* bss, RSOFixedLevel fixed_level);
BOOL RSOUnLinkList(void* oldRso);

u32 RSOGetFixedSize(void* rso, RSOFixedLevel fixed_level);

int RSOGetFarCodeSize(RSOObjectHeader* rsoImp, const RSOObjectHeader* rsoExp);
int RSOLinkFar(RSOObjectHeader* rsoImp, const RSOObjectHeader* rsoExp, void* buff);

int RSOGetJumpCodeSize(const RSOObjectHeader* rsoExp);
void RSOMakeJumpCode(const RSOObjectHeader* rsoExp, void* buff);
int RSOLinkJump(RSOObjectHeader* rsoImp, const RSOObjectHeader* rsoExp, void* buff);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_RSO_H
