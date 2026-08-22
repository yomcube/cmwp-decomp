#ifndef REVOLUTION_EUART_H
#define REVOLUTION_EUART_H

#include <revolution/types.h>

#include <revolution/os/OSContext.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EUART_ERR_OK 0
#define EUART_ERR_NOT_INITIALIZED 1
#define EUART_ERR_INVALID 2
#define EUART_ERR_BUSY 3
#define EUART_ERR_READ_UNSUPPORTED 4 /* ? */
#define EUART_ERR_EXI_ERROR 5

BOOL EUARTInit();
void EUARTExit();

int EUARTPrintf(const char* msg, ...);
BOOL EUARTSetStatus(int status);
int EUARTGetLastError();

int InitializeUART();

int ReadUARTN();
int WriteUARTN(void* buf, u32 len);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_EUART_H
