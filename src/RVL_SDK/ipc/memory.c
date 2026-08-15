#include <revolution/ios.h>
#include <revolution/ipc.h>

#include <revolution/os.h>

#define MAX_HEAP 8

#define CHUNK_MAGIC_FREE 0xBABE0000
#define CHUNK_MAGIC_USED 0xBABE0001
#define CHUNK_MAGIC_FORALIGN 0xBABE0002

typedef struct HeapChunk HeapChunk;
struct HeapChunk {
    u32 magic;        // 0x00
    u32 size;         // 0x04
    HeapChunk* prev;  // 0x08
    HeapChunk* next;  // 0x0C
};

typedef struct HeapDesc {
    void* start;      // 0x00
    u32 unk_0x04;     // 0x04
    u32 size;         // 0x08
    HeapChunk* head;  // 0x0C
} HeapDesc;

static HeapDesc __heaps[MAX_HEAP];

static void __iosCoalesceChunk(HeapChunk* chunk) {
    if (chunk && (u8*)chunk->next == (u8*)chunk + chunk->size + sizeof(HeapChunk)) {
        HeapChunk* oldChunk = chunk->next;
        chunk->next = oldChunk->next;
        if (chunk->next) {
            chunk->next->prev = chunk;
        }
        chunk->size += oldChunk->size + sizeof(HeapChunk);
    }
}

IOSHeapId iosCreateHeap(void* start, u32 size) {
    IOSHeapId result = IOS_ERROR_INVALID;
    BOOL enabled = OSDisableInterrupts();
    IOSHeapId i;
    HeapDesc* heap;

    if (((u32)start & 31) != 0) {
        goto exit;
    }

    for (i = 0; (u32)i < MAX_HEAP; i++) {
        if (!__heaps[i].start) {
            break;
        }
    }

    if (i == MAX_HEAP) {
        result = IOS_ERROR_MAX;
        goto exit;
    }

    heap = &__heaps[i];
    heap->start = start;
    heap->size = size;
    heap->head = start;
    heap->head->magic = CHUNK_MAGIC_FREE;
    heap->head->size = size - sizeof(HeapChunk);
    heap->head->prev = NULL;
    heap->head->next = NULL;

    result = i;

exit:
    OSRestoreInterrupts(enabled);

    return result;
}

IOSError iosDestroyHeap(IOSHeapId id) {
    IOSError result = IOS_ERROR_INVALID;
    BOOL enabled = OSDisableInterrupts();
    IOSHeapId i;
    HeapDesc* heap;

    if (id < 0 || id >= MAX_HEAP || !__heaps[id].start) {
        result = IOS_ERROR_INVALID;  // already set but ok
        goto exit;
    }

    heap = &__heaps[id];

    // first heap ID is reserved
    if (id == IOS_SYSTEM_HEAP_ID) {
        result = IOS_ERROR_ACCESS;
        goto exit;
    }

    heap->start = NULL;
    heap->size = 0;
    heap->unk_0x04 = 0;
    heap->head = NULL;

    result = IOS_ERROR_OK;

exit:
    OSRestoreInterrupts(enabled);

    return result;
}

void* __iosAlloc(IOSHeapId id, u32 size, u32 align) {
    BOOL enabled;

    HeapChunk* chunk;
    HeapDesc* heapDesc;
    HeapChunk* best;
    HeapChunk* block = NULL;

    enabled = OSDisableInterrupts();

    // Check arguments
    if (size == 0 || align == 0 || (align & (align - 1)) != 0) {
        goto exit;
    }

    // Limit alignment minimum
    if (align < DEFAULT_ALIGN) {
        align = DEFAULT_ALIGN;
    }

    size = OSRoundUp32B(size);

    // Check heap id
    if (id < 0 || id >= MAX_HEAP || !__heaps[id].start) {
        block = NULL;
        goto exit;
    }

    heapDesc = &__heaps[id];
    best = heapDesc->head;

    // Find chunk best fit for specified size
    for (chunk = NULL; best; best = best->next) {
        HeapChunk* chunkMem = (HeapChunk*)(best + 1);
        u32 chunkUnalign = (align - 1) & (align - ((u32)chunkMem & (align - 1)));

        if (best->size == size && chunkUnalign == 0) {
            chunk = best;
            break;
        } else {
            if (best->size >= (size + chunkUnalign) && (!chunk || best->size < chunk->size)) {
                chunk = best;
            }
        }
    }

    best = chunk;

    if (best) {
        HeapChunk* bestMem = (HeapChunk*)(best + 1);
        u32 bestUnalign = (align - 1) & (align - ((u32)bestMem & (align - 1)));

        // Split off extra size that won't be used by this allocation
        if (best->size > (size + bestUnalign + sizeof(HeapChunk))) {
            HeapChunk* rest = (HeapChunk*)((u8*)best + size + bestUnalign + sizeof(HeapChunk));

            rest->magic = CHUNK_MAGIC_FREE;
            rest->size = ((best->size - size) - bestUnalign) - sizeof(HeapChunk);
            rest->next = best->next;
            if (rest->next) {
                rest->next->prev = rest;
            }
            best->next = rest;
            best->size = size + bestUnalign;
        }

        best->magic = CHUNK_MAGIC_USED;
        if (best->prev) {
            best->prev->next = best->next;
        } else {
            heapDesc->head = best->next;
        }
        if (best->next) {
            best->next->prev = best->prev;
        }
        best->prev = best->next = NULL;

        // For unaligned chunks, create a header where one would normally be.
        // To find the real chunk, you take the FORALIGN one and jump back one.
        block = (void*)((u8*)best + bestUnalign + sizeof(HeapChunk));
        if (bestUnalign != 0) {
            HeapChunk* var_r31_unk8;
            var_r31_unk8 = (HeapChunk*)(block - 1);
            var_r31_unk8->magic = CHUNK_MAGIC_FORALIGN;
            var_r31_unk8->prev = best;
        }
    }

exit:
    OSRestoreInterrupts(enabled);
    return block;
}

void* iosAlloc(IOSHeapId id, u32 size) {
    return __iosAlloc(id, size, DEFAULT_ALIGN);
}

void* iosAllocAligned(IOSHeapId id, u32 size, u32 align) {
    return __iosAlloc(id, size, align);
}

IOSError iosFree(IOSHeapId id, void* block) {
    HeapChunk* chunk;
    HeapChunk* it;
    HeapDesc* desc;
    BOOL enabled;
    s32 result = IOS_ERROR_INVALID;

    enabled = OSDisableInterrupts();
    if (!block) {
        goto exit;
    }

    if (id < 0 || id >= MAX_HEAP || !__heaps[id].start) {
        result = IOS_ERROR_INVALID;
        goto exit;
    }
    desc = &__heaps[id];
    if ((u8*)block < ((u8*)desc->start + sizeof(HeapChunk)) || (u8*)block > ((u8*)desc->start + desc->size)) {
        goto exit;
    }

    chunk = (HeapChunk*)block - 1;
    if (chunk->magic == CHUNK_MAGIC_FORALIGN) {
        chunk = chunk->prev;
    }

    if (chunk->magic != CHUNK_MAGIC_USED) {
        goto exit;
    }

    chunk->magic = CHUNK_MAGIC_FREE;
    for (it = desc->head; it; it = it->next) {
        if (!it->next || it->next > chunk) {
            break;
        }
    }
    if (it && chunk > it) {
        chunk->prev = it;
        chunk->next = it->next;
        it->next = chunk;
        if (chunk->next) {
            chunk->next->prev = chunk;
        }
    } else {
        chunk->next = desc->head;
        desc->head = chunk;
        chunk->prev = NULL;
        if (chunk->next) {
            chunk->next->prev = chunk;
        }
    }
    __iosCoalesceChunk(chunk);
    __iosCoalesceChunk(chunk->prev);

    result = IOS_ERROR_OK;

exit:
    OSRestoreInterrupts(enabled);
    return result;
}
