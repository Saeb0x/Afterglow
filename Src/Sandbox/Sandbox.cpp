// NOTE(saeb): The engine's own test game: it defines the four functions Engine/Game.h declares, so the engine builds and runs without a real game. Games replace this file with their own.
#include "Engine/Game.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Text.h"
#include "Engine/Asset/Asset.h"

static Font DebugFont;

void GameConfigure()
{
    WindowSetFlags(WindowFlags_None);
    WindowSetTitle(SV8(u8"Afterglow Sandbox"));
    WindowSetClientAreaDimensions(1280, 720);
    WindowSetMinClientAreaDimensions(360, 360);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);
    RendererSetFlags(RendererFlags_VSync);
    RendererSetDesignSize(1000.0f, 1000.0f);
}

bool GameInit(StackAllocator* allocator)
{
    AssetLoadFont(allocator, SV8(u8"Data/Engine/LiberationMono-Regular.aga"), &DebugFont);
    return(true);
}

void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
    // NOTE(saeb): One background over everything the window shows, so the design area has no visible edge.
    real32 visibleX, visibleY, visibleWidth, visibleHeight;
    RendererGetVisibleArea(&visibleX, &visibleY, &visibleWidth, &visibleHeight);

    RendererQuad background = {};
    background.X = visibleX; background.Y = visibleY;
    background.Width = visibleWidth; background.Height = visibleHeight;
    background.U1 = 1.0f; background.V1 = 1.0f;
    background.R = 0.5294f; background.G = 0.8078f; background.B = 0.9216f; background.A = 1.0f;
    RendererPushQuad(&background);

    TextDraw(&DebugFont, 200.0f, 300.0f, 100.0f, 1.0f, 0.0f, 0.0f, 1.0f, SV8(u8"Afterglow"));
}

void GameShutdown(StackAllocator* allocator)
{
}
