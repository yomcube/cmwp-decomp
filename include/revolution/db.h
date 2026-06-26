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

#if SDK_VERSION < 20091211
#define DB_PRINT(...) DBPrintf(__VA_ARGS__)
#else
#define DB_PRINT(...)
#endif

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // REVOLUTION_DB_H
