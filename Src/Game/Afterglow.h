#if !defined(AFTERGLOW_H)

#include "Core/Types.h"
#include "Core/Arena.h"

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

#define AFTERGLOW_H
#endif
