#ifndef REVOLUTION_DB_H
#define REVOLUTION_DB_H

#include <revolution/types.h>

#include <revolution/db/DBInterface.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define OS_DBINTERFACE_ADDR 0x00000040

BOOL DBIsDebuggerPresent();
void DBPrintf(char* str, ...);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // REVOLUTION_DB_H
