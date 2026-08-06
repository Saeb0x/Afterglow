#include "Afterglow.h"
#include "Engine/RenderCommands.h"
#include "Engine/AssetManager.h"
#include "GameAssets.h"
#include "Engine/Input.h"

#include <stdio.h>

void GameInit(GameContext* context)
{
    GameState* state = sstl::PushStruct<GameState>(&context->Memory->Permanent);
}

void GameUpdateAndRender(GameContext* context)
{
    GameState* state = (GameState*)context->Memory->Permanent.Base;
}
