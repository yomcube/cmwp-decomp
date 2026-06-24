#include <revolution/os.h>
#include <revolution/rso.h>
#include <string.h>

void RSONotifyModuleLoaded() {
    return;
}

void RSONotifyModuleUnloaded() {
    return;
}

void RSONotifyPreRSOLink() {
    return;
}

void RSONotifyPostRSOLink() {
    return;
}

void RSONotifyPreRSOLinkFar() {
    return;
}

void RSONotifyPostRSOLinkFar() {
    return;
}

static int LocateObject(void* newModule, void* bss, int i_fixed_level) {
    int i;
    RSOObjectHeader* moduleHeader = (RSOObjectHeader*)newModule;
    moduleHeader->bssSection = 0;
    ASSERTLINE(201, moduleHeader->info.version <= RSO_VERSION);

    moduleHeader->info.sectionInfoOffset = (u32)((u32)newModule + moduleHeader->info.sectionInfoOffset);
    moduleHeader->info.nameOffset = (u32)(moduleHeader->info.nameOffset + (u32)newModule);
    moduleHeader->internalRelOffset = (u32)(moduleHeader->internalRelOffset + (u32)newModule);
    moduleHeader->externalRelOffset = (u32)(moduleHeader->externalRelOffset + (u32)newModule);
    moduleHeader->expHeader.tableOffset = (u32)(moduleHeader->expHeader.tableOffset + (u32)newModule);
    moduleHeader->expHeader.stringOffset = (u32)(moduleHeader->expHeader.stringOffset + (u32)newModule);
    moduleHeader->impHeader.tableOffset = (u32)(moduleHeader->impHeader.tableOffset + (u32)newModule);
    moduleHeader->impHeader.stringOffset = (u32)(moduleHeader->impHeader.stringOffset + (u32)newModule);

    for (i = 1; i < moduleHeader->info.numSections; i++) {
    }

    RSOUnresolveImportSymbolAll(newModule);
    if (i_fixed_level <= 1) {
        memset(bss, 0, moduleHeader->bssSize);
    }
    RSONotifyModuleLoaded();
    return 1;
}

int RSOLocateObject(void* newModule, void* bss) {
    return LocateObject(newModule, bss, 0);
}

int RSOLocateObjectFixed(void* newModule, void* bss) {
    return LocateObject(newModule, bss, 1);
}

RSOImportTable* RSOGetImport(const RSOSymbolHeader* imp) {
    return (RSOImportTable*)imp->tableOffset;
}

RSOImportTable* RSOGetExport(const RSOSymbolHeader* exp) {
    return (RSOImportTable*)exp->tableOffset;
}

int RSOGetNumImportSymbols(const RSOSymbolHeader* imp) {
    return imp->tableSize / sizeof(RSOImportTable);
}

char* RSOGetImportSymbolName(RSOSymbolHeader* imp, int index) {
    RSOImportTable* impTab = (RSOImportTable*)(imp->tableOffset + index * sizeof(RSOImportTable));
    return (char*)(impTab->strOffset + imp->stringOffset);
}

BOOL RSOIsImportSymbolResolved(const RSOObjectHeader* rso, int index) {
    u32 v = RSOGetImport(&rso->impHeader)[index].value;

    if (v != rso->unresolved) {
        return TRUE;
    }

    return FALSE;
}

BOOL RSOIsImportSymbolResolvedAll(const RSOObjectHeader* rso) {
    int numSymbols, i;
    numSymbols = RSOGetNumImportSymbols(&rso->impHeader);
    for (i = 0; i < numSymbols; i++) {
        if (!RSOIsImportSymbolResolved(rso, i)) {
            return FALSE;
        }
    }

    return TRUE;
}

void RSOResolveImportSymbol(RSOObjectHeader* rsoImp, int index, void* addr) {
    RSOImportTable* impTab = RSOGetImport(&rsoImp->impHeader);
    impTab[index].value = (u32)addr;
    RSORelocateImportSymbol(rsoImp, impTab, (u32)addr);
}

void RSOUnresolveImportSymbol(RSOObjectHeader* rsoImp, int index) {
    RSOImportTable* impTab = RSOGetImport(&rsoImp->impHeader);
    impTab[index].value = rsoImp->unresolved;
    RSORelocateImportSymbol(rsoImp, impTab, index);
}

int RSOGetNumExportSymbols(const RSOSymbolHeader* exp) {
    return exp->tableSize >> 4;
}

char* RSOGetExportSymbolName(RSOSymbolHeader* exp, int index) {
    RSOExportTable* expTab = (RSOExportTable*)(exp->tableOffset + index * sizeof(RSOExportTable));
    return (char*)(expTab->strOffset + exp->stringOffset);
}

RSOHash RSOGetHash(const char* symbolName) {
    char v2;
    int v3;
    unsigned int v4;
    RSOHash hash = 0;

    while (*symbolName != 0) {
        v2 = *symbolName++;
        v3 = 16 * hash + v2;
        v4 = v3 & 0xF0000000;
        if (v4 != 0) {
            v3 ^= v4 >> 24;
        }

        hash = v3 & ~v4;
    }

    return hash;
}

static void makeCode(u32 addr, u32* i_buff) {
    i_buff[0] = 0x91810004;
    i_buff[1] = (addr >> 0x10) + ((addr & 0x8000) ? 1 : 0) & 0xffff | 0x3d800000;
    i_buff[2] = addr & 0xffff | 0x398c0000;
    i_buff[3] = 0x7d8903a6;
    i_buff[4] = 0x81810004;
    i_buff[5] = 0x4e800420;
}

static void cnvFarCode(RSOObjectHeader* rso, RSOImportTable* impTab, int impIndex, u32 addr, u32* i_buff) {
}

int RSOGetJumpCodeSize(const RSOObjectHeader* pHeader) {
    return RSO_FAR_JUMP_SIZE * (pHeader->expHeader.tableSize >> 4);
}
