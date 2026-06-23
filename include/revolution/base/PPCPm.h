#ifndef REVOLUTION_BASE_PPC_PM_H
#define REVOLUTION_BASE_PPC_PM_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void PMBegin();
void PMEnd();
void PMCycles();
void PML1FetchMisses();
void PML1MissCycles();
void PMInstructions();

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // REVOLUTION_BASE_PPC_PM_H
