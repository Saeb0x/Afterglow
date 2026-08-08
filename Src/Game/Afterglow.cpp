#include "Afterglow.h"
#include "Engine/Platform.h"
#include "Engine/Input.h"
#include "Engine/Render/RenderCommands.h"
#include "Engine/Assets/AssetManager.h"
#include "GameAssets.h"

void GameInit(GameContext* context)
{
    GameState* state = sstl::PushStruct<GameState>(&context->Memory->Permanent);
}

void GameUpdateAndRender(GameContext* context)
{
    GameState* state = (GameState*)context->Memory->Permanent.Base;
}
