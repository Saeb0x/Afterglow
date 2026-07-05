#include "Afterglow.h"
#include "Engine/RenderCommands.h"
#include "Engine/AssetManager.h"
#include "GameAssets.h"

void GameInit(GameContext* context)
{
    AssetManagerLoadFont(context->Loader, &context->Memory->Transient, &context->Assets->UIFont, "Data/UIFont.aga");
}

static bool32 BounceAxis(real32* position, real32* velocity, real32 min, real32 max, real32 deltaTime)
{
    *position += *velocity * deltaTime;

    if(*position <= min || *position >= max)
    {
        *velocity = -(*velocity);
        return(true);
    }

    return(false);
}

void GameUpdateAndRender(GameContext* context)
{
    static real32 textX = 100.0f;
    static real32 textY = 100.0f;
    static real32 velocityX = 200.0f;
    static real32 velocityY = 200.0f;

    static const uint32 BounceColors[] =
    {
        PackColor(255, 0, 0, 255),
        PackColor(0, 255, 0, 255),
        PackColor(0, 0, 255, 255),
    };
    static uint32 colorIndex = 0;

    const char* text = "Afterglow";
    int32 textWidth = TextWidth(&context->Assets->UIFont, text);
    int32 textHeight = context->Assets->UIFont.LineHeight;

    bool32 bouncedX = BounceAxis(&textX, &velocityX, 0.0f, (real32)(context->ScreenWidth - textWidth), context->DeltaTime);
    bool32 bouncedY = BounceAxis(&textY, &velocityY, 0.0f, (real32)(context->ScreenHeight - textHeight), context->DeltaTime);

    if(bouncedX || bouncedY)
    {
        colorIndex = (colorIndex + 1) % ArrayCount(BounceColors);
    }

    PushText(context->Render, &context->Assets->UIFont, textX, textY, text, BounceColors[colorIndex]);
}
