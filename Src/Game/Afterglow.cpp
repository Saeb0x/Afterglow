#include "Afterglow.h"
#include "Engine/RenderCommands.h"
#include "Engine/AssetManager.h"
#include "GameAssets.h"
#include "Engine/Input.h"

#include <stdio.h>

#if defined(AG_DEBUG)
static void CommandPerf(Console* console, const char* args, void* data)
{
    GameContext* context = (GameContext*)data;
    GameState* state = (GameState*)context->Memory->Permanent.BaseAddress;

    state->ShowPerf = !state->ShowPerf;
    ConsolePrint(console, state->ShowPerf ? "perf: on" : "perf: off");
}

static void DrawPerfOverlay(GameContext* context, Font* font)
{
    char buffer[128];
    real32 scale = 0.5f;
    real32 lineHeight = (real32)font->LineHeight * scale;
    real32 x = 8.0f;
    real32 y = context->ScreenHeight - 100.0f;

    real32 frameMs = context->DeltaTime * 1000.0f;
    real32 fps = (context->DeltaTime > 0.0f) ? (1.0f / context->DeltaTime) : 0.0f;

    sprintf_s(buffer, sizeof(buffer), "FPS: %.1f", fps);
    PushText(context->Render, font, x, y, buffer, WHITE, scale);
    y += lineHeight;

    sprintf_s(buffer, sizeof(buffer), "Frame: %.2f ms", frameMs);
    PushText(context->Render, font, x, y, buffer, WHITE, scale);
    y += lineHeight;

    sprintf_s(buffer, sizeof(buffer), "Draw calls: %u", context->DrawCallCount);
    PushText(context->Render, font, x, y, buffer, WHITE, scale);
}
#endif

void GameInit(GameContext* context)
{
    GameState* state = PushStruct(&context->Memory->Permanent, GameState);

    AssetManagerLoadFont(context->Loader, &context->Memory->Transient, &context->Assets->UIFont, "Data/UIFont.aga");

#if defined(AG_DEBUG)
    ConsoleInit(&state->Console, context);
    ConsoleRegisterCommand(&state->Console, "perf",  CommandPerf);

    state->ShowPerf = false;
#endif
}

void GameUpdateAndRender(GameContext* context)
{
    GameState* state = (GameState*)context->Memory->Permanent.BaseAddress;

#if defined(AG_DEBUG)
    if(state->ShowPerf)
    {
        DrawPerfOverlay(context, &context->Assets->UIFont);
    }

    ConsoleUpdateAndRender(&state->Console, context->Input, context->Render, &context->Assets->UIFont, context->ScreenWidth, context->ScreenHeight);
#endif
}
