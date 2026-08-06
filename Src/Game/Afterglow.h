#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include "Engine/Types.h"
#include "SSTL/Memory.h"

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
};

void GameInit(GameContext* context);
void GameUpdateAndRender(GameContext* context);

#endif
