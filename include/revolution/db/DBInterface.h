#ifndef _DOLPHIN_DBINTERFACE_H_
#define _DOLPHIN_DBINTERFACE_H_

#include <revolution/types.h>

#include <revolution/os/OSException.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DBInterface {
    u32 bPresent;
    u32 exceptionMask;
    void (*ExceptionDestination)();
    void* exceptionReturn;
} DBInterface;

extern DBInterface* __DBInterface;

void DBInit();
void DBInitComm(int* inputFlagPtr, int* mtrCallback);
void __DBExceptionDestination();
void __DBExceptionDestinationAux();
BOOL __DBIsExceptionMarked(__OSException exception);
void __DBMarkException(u8 exception, int value);
void __DBSetPresent(u32 value);

#ifdef __cplusplus
}
#endif

#endif
