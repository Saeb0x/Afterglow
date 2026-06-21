#if !defined(MEMORY_ARENA_H)

#include "Types.h"

#include <string.h>

struct MemoryArena
{
    uint8* Base;
    uint64 Size;
    uint64 Used;
};

static void InitializeArena(MemoryArena* arena, uint64 size, void* base)
{
    arena->Base = (uint8*)base;
    arena->Size = size;
    arena->Used = 0;
}

static void* PushSize(MemoryArena* arena, uint64 size)
{
    Assert(arena->Used + size <= arena->Size);

    void* result = arena->Base + arena->Used;
    memset(result, 0, size);

    arena->Used += size;

    return(result);
}

#define PushStruct(arena, type) (type*)PushSize(arena, sizeof(type))
#define PushArray(arena, type, count) (type*)PushSize(arena, sizeof(type) * (count))

#define MEMORY_ARENA_H
#endif
