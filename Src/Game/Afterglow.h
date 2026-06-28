#if !defined(AFTERGLOW_H)

#include "Core/Types.h"
#include "Core/MemoryArena.h"
#include "Engine/RenderCommands.h"
#include "GameAssets.h"

struct GameMemory
{
    bool32 Initialized;
    MemoryArena Permanent;
    MemoryArena Transient;
};

struct GameContext
{
    GameMemory* Memory;
    GameAssets* Assets;
    RenderCommands* Render;

    int32 ScreenWidth;
    int32 ScreenHeight;    
};

void GameUpdateAndRender(GameContext* context);

#define AFTERGLOW_H
#endif
