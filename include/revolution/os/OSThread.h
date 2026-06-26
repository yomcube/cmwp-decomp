#ifndef REVOLUTION_OS_THREAD_H
#define REVOLUTION_OS_THREAD_H

#include <revolution/types.h>

#include <revolution/os/OSContext.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef s32 OSPriority;

enum OS_THREAD_STATE {
    OS_THREAD_STATE_READY = 1,
    OS_THREAD_STATE_RUNNING = 2,
    OS_THREAD_STATE_WAITING = 4,
    OS_THREAD_STATE_MORIBUND = 8,
};

#define OS_PRIORITY_MIN 0
#define OS_PRIORITY_MAX 31
#define OS_PRIORITY_IDLE OS_PRIORITY_MAX

#define OS_THREAD_SPECIFIC_MAX 2

#define OS_THREAD_ATTR_DETACH 0x0001

#define OS_THREAD_STACK_MAGIC 0xDEADBABE

typedef struct OSThread OSThread;
typedef struct OSMutex OSMutex;
typedef struct OSThreadQueue OSThreadQueue;
typedef struct OSMutexQueue OSMutexQueue;
typedef struct OSThreadLink OSThreadLink;
typedef struct OSMutexLink OSMutexLink;

struct OSThreadQueue {
    OSThread* head;  // 0x00
    OSThread* tail;  // 0x04
};

struct OSThreadLink {
    OSThread* next;  // 0x00
    OSThread* prev;  // 0x04
};

struct OSMutexQueue {
    OSMutex* head;  // 0x00
    OSMutex* tail;  // 0x04
};

struct OSMutexLink {
    OSMutex* next;  // 0x00
    OSMutex* prev;  // 0x04
};

struct OSThread {
    OSContext context;  // 0x000

    u16 state;    // 0x2C8
    u16 attr;     // 0x2CA
    s32 suspend;  // 0x2CC

    OSPriority priority;  // 0x2D0
    OSPriority base;      // 0x2D4

    void* val;  // 0x2D8

    OSThreadQueue* queue;     // 0x2DC
    OSThreadLink link;        // 0x2E0
    OSThreadQueue queueJoin;  // 0x2E8

    OSMutex* mutex;           // 0x2F0
    OSMutexQueue queueMutex;  // 0x2F4
    OSThreadLink linkActive;  // 0x2FC

    u8* stackBase;  // 0x304
    u32* stackEnd;  // 0x308

    s32 error;  // 0x30C

    void* specific[OS_THREAD_SPECIFIC_MAX];  // 0x310
};

typedef void (*OSSwitchThreadCallback)(OSThread*, OSThread*);
typedef void (*OSIdleFunction)(void*);

void OSInitThreadQueue(OSThreadQueue* queue);

void OSSleepThread(OSThreadQueue* queue);
void OSWakeupThread(OSThreadQueue* queue);

s32 OSSuspendThread(OSThread* thread);
s32 OSResumeThread(OSThread* thread);

OSThread* OSGetCurrentThread();

s32 OSEnableScheduler();
s32 OSDisableScheduler();

void OSCancelThread(OSThread* thread);
void OSClearStack(u8 val);

BOOL OSIsThreadSuspended(OSThread* thread);
BOOL OSIsThreadTerminated(OSThread* thread);

void OSYieldThread();

BOOL OSCreateThread(OSThread* thread, void* (*func)(void*), void* param, void* stack, u32 stackSize, OSPriority priority, u16 attr);
void OSExitThread(void* val);

BOOL OSJoinThread(OSThread* thread, void** val);
void OSDetachThread(OSThread* thread);

BOOL OSSetThreadPriority(OSThread* thread, OSPriority priority);
s32 OSGetThreadPriority(OSThread* thread);

OSThread* OSSetIdleFunction(OSIdleFunction idleFunction, void* param, void* stack, u32 stackSize);
OSThread* OSGetIdleFunction();

s32 OSCheckActiveThreads();

void OSSetThreadSpecific(s32 index, void* ptr);
void* OSGetThreadSpecific(s32 index);

OSSwitchThreadCallback OSSetSwitchThreadCallback(OSSwitchThreadCallback callback);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // REVOLUTION_OS_THREAD_H
