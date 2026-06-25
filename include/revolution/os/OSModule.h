#ifndef REVOLUTION_OS_MODULE_H
#define REVOLUTION_OS_MODULE_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OS_MODULE_VERSION 3
typedef struct OSModuleHeader OSModuleHeader;

typedef u32 OSModuleID;
typedef struct OSModuleQueue OSModuleQueue;
typedef struct OSModuleLink OSModuleLink;
typedef struct OSModuleInfo OSModuleInfo;
typedef struct OSSectionInfo OSSectionInfo;
typedef struct OSImportInfo OSImportInfo;
typedef struct OSRel OSRel;

struct OSModuleQueue {
    OSModuleInfo* head;  // 0x00
    OSModuleInfo* tail;  // 0x04
};

struct OSModuleLink {
    OSModuleInfo* next;  // 0x04
    OSModuleInfo* prev;  // 0x08
};

struct OSModuleInfo {
    OSModuleID id;          // unique identifier for the module
    OSModuleLink link;      // doubly linked list of modules
    u32 numSections;        // # of sections
    u32 sectionInfoOffset;  // offset to section info table
    u32 nameOffset;         // offset to module name
    u32 nameSize;           // size of module name
    u32 version;            // version number
};

struct OSModuleHeader {
    // CAUTION: info must be the 1st member
    OSModuleInfo info;

    // OS_MODULE_VERSION == 1
    u32 bssSize;  // total size of bss sections in bytes
    u32 relOffset;
    u32 impOffset;
    u32 impSize;           // size in bytes
    u8 prologSection;      // section # for prolog function
    u8 epilogSection;      // section # for epilog function
    u8 unresolvedSection;  // section # for unresolved function
    u8 bssSection;         // section # for bss section (set at run-time)
    u32 prolog;            // prolog function offset
    u32 epilog;            // epilog function offset
    u32 unresolved;        // unresolved function offset

    // OS_MODULE_VERSION == 2
#if (2 <= OS_MODULE_VERSION)
    u32 align;     // module alignment constraint
    u32 bssAlign;  // bss alignment constraint
#endif

    // OS_MODULE_VERSION == 3
#if (3 <= OS_MODULE_VERSION)
    u32 fixSize;
#endif
};

struct OSSectionInfo {
    u32 offset;
    u32 size;
};

struct OSImportInfo {
    OSModuleID id;  // external module id
    u32 offset;     // offset to OSRel instructions
};

struct OSRel {
    u16 offset;  // byte offset from the previous entry
    u8 type;
    u8 section;
    u32 addend;
};

void OSSetStringTable(void* stringTable);
BOOL OSLink(OSModuleInfo* newModule, void* bss);

#if (3 <= OS_MODULE_VERSION)
BOOL OSLinkFixed(OSModuleInfo* newModule, void* bss);
#endif

BOOL OSUnlink(OSModuleInfo* oldModule);

OSModuleInfo* OSSearchModule(void* ptr, u32* section, u32* offset);

// debugger notification
void OSNotifyLink(OSModuleInfo* module);
void OSNotifyUnlink(OSModuleInfo* module);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_OS_MODULE_H
