#if !defined(AFTERGLOW_H)
#define AFTERGLOW_H

#include "Engine/Types.h"
#include "SSTL/Memory.h"

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
    sstl::Arena Permanent;
    sstl::Arena Transient;
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

#endif
