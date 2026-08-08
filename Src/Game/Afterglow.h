#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include "Engine/Types.h"

struct GameMemory;
struct GameInput;
struct RenderCommands;
struct AssetManager;
struct GameAssets;

struct GameState
{
};

struct GameContext
{
    GameMemory* Memory;
    GameInput* Input;
    RenderCommands* Render;
    AssetManager* Loader;
    GameAssets* Assets;

    int32 ScreenWidth;
    int32 ScreenHeight;

    real32 DeltaTime;
};

void GameInit(GameContext* context);
void GameUpdateAndRender(GameContext* context);

#endif
