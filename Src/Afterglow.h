#if !defined(AFTERGLOW_H)

#include "Core/Types.h"
#include "Core/MemoryArena.h"

struct GameMemory
{
    bool32 Initialized;
    MemoryArena PermanentArena;
    MemoryArena TransientArena;
};

static void GameUpdateAndRender(GameMemory* gameMemory);

#define AFTERGLOW_H
#endif
