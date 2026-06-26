#if !defined(AFTERGLOW_H)

#include "Core/Types.h"
#include "Core/MemoryArena.h"
#include "RenderCommands.h"

struct GameMemory
{
    bool32 Initialized;
    MemoryArena PermanentArena;
    MemoryArena TransientArena;
};

void GameUpdateAndRender(GameMemory* gameMemory, RenderCommands* renderCommands);

#define AFTERGLOW_H
#endif
