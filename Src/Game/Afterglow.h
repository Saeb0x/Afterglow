#if !defined(AFTERGLOW_H)

#include "Core/Types.h"
#include "Core/Arena.h"

#if defined(AG_DEBUG)
#include "Engine/Console.h"
#endif

struct RenderCommands;
struct AssetManager;
struct GameAssets;
struct GameInput;

struct GameMemory
{
    bool32 Initialized;
    Arena Permanent;
    Arena Transient;
};

struct GameState
{
#if defined(AG_DEBUG)
    Console Console;
    bool32 ShowPerf;
#endif
};

struct GameContext
{
    GameMemory* Memory;
    RenderCommands* Render;
    AssetManager* Loader;
    GameInput* Input;

    GameAssets* Assets;

    int32 ScreenWidth;
    int32 ScreenHeight;

    real32 DeltaTime;

#if defined(AG_DEBUG)
    uint32 DrawCallCount;
#endif
};

void GameInit(GameContext* context);
void GameUpdateAndRender(GameContext* context);

#define AFTERGLOW_H
#endif
