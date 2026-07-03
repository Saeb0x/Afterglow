#if !defined(AFTERGLOW_H)

#include "Core/Types.h"
#include "Core/Arena.h"

struct RenderCommands;
struct GameAssets;

struct GameMemory
{
    bool32 Initialized;
    Arena Permanent;
    Arena Transient;
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
