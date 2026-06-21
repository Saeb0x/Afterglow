#include "Afterglow.h"

struct GameState
{
    uint32 FrameCount;
};

static void GameUpdateAndRender(GameMemory* gameMemory)
{
    GameState* gameState;

    if(!gameMemory->Initialized)
    {
        gameState = PushStruct(&gameMemory->PermanentArena, GameState);
        gameState->FrameCount = 0;

        gameMemory->Initialized = true;
    }
    else
    {
        gameState = (GameState*)gameMemory->PermanentArena.Base;
    }

    gameState->FrameCount++;
}
