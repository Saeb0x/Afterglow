#if !defined(ARENA_H)

#include "Types.h"

struct Arena
{
    uint8* BaseAddress;
    uint64 Size;
    uint64 Used;
};

struct TempMemory
{
    Arena* Arena;
    uint64 Used;
};

static void InitializeArena(Arena* arena, void* baseAddress, uint64 size)
{
    arena->BaseAddress = (uint8*)baseAddress;
    arena->Size = size;
    arena->Used = 0;
}

static void* PushSize(Arena* arena, uint64 size, uint64 alignment)
{
    uint64 currentAddress = (uint64)(arena->BaseAddress + arena->Used);
    uint64 alignmentMask = alignment - 1; // Power-of-two only (1, 2, 4, 8, 16)
    uint64 alignmentOffset = 0;
    if(currentAddress & alignmentMask) // Not already aligned?
    {
        alignmentOffset = alignment - (currentAddress & alignmentMask);
    }

    Assert(arena->Used + alignmentOffset + size <= arena->Size); // Padding counts against budget

    void* result = arena->BaseAddress + arena->Used + alignmentOffset;
    arena->Used += alignmentOffset + size;

    return(result);
}

#define PushStruct(arena, type) (type*)PushSize(arena, sizeof(type), alignof(type))
#define PushArray(arena, type, count) (type*)PushSize(arena, sizeof(type) * (count), alignof(type))

static TempMemory BeginTempMemory(Arena* arena)
{
    TempMemory result;
    result.Arena = arena;
    result.Used = arena->Used;

    return(result);
}

static void EndTempMemory(TempMemory temp)
{
    Assert(temp.Arena->Used >= temp.Used); // Must roll backward, never forward
    temp.Arena->Used = temp.Used;
}

#define ARENA_H
#endif
