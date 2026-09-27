// NOTE(saeb): The engine's own test game: it defines the four functions Engine/Game.h declares, so the engine builds and runs without a real game. Games replace this file with their own.
#include "Engine/Game.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Text.h"
#include "Engine/Asset/Asset.h"

#include <SSTL/Core/Config.h>

#if SSTL_DEBUG
static Font DebugFont;
#endif

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
#if SSTL_DEBUG
    AssetLoadFont(allocator, SV8(u8"Data/Engine/LiberationMono-Regular.aga"), &DebugFont);
#endif

    return(true);
}

void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
    RendererSetSpace(RendererSpace::Window);

    RendererQuad panel = {};
    panel.X = 10.0f; panel.Y = 10.0f; panel.Width = 300.0f; panel.Height = 200.0f;
    panel.R = 0.1f; panel.G = 0.1f; panel.B = 0.1f; panel.A = 0.8f;
    RendererPushQuad(&panel);

#if SSTL_DEBUG
    TextDraw(&DebugFont, 20.0f, 20.0f, 20.0f, 1, 1, 1, 1, SV8(u8"Window space"));
#endif

    RendererSetSpace(RendererSpace::Design);

#if SSTL_DEBUG
    TextDraw(&DebugFont, 200.0f, 200.0f, 50.0f, 1.0f, 0.0f, 0.0f, 1.0f, SV8(u8"Design space"));
#endif
}

void GameShutdown(StackAllocator* allocator)
{
}
