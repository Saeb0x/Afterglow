#if !defined(ARENA_H)

#include "Types.h"

struct Arena
{
    uint8* BaseAddress;
    uint64 Size;
    uint64 Used;
};

static void InitializeArena(Arena* arena, void* baseAddress, uint64 size)
{
    arena->BaseAddress = (uint8*)baseAddress;
    arena->Size = size;
    arena->Used = 0;
}

static void* PushSize(Arena* arena, uint64 size)
{
    Assert(arena->Used + size <= arena->Size);

    void* result = arena->BaseAddress + arena->Used;
    arena->Used += size;

    return(result);
}

#define PushStruct(arena, type) (type*)PushSize(arena, sizeof(type))
#define PushArray(arena, type, count) (type*)PushSize(arena, sizeof(type) * (count))

#define ARENA_H
#endif
