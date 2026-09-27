// NOTE(saeb): The engine's own test game: it defines the four functions Engine/Game.h declares, so the engine builds and runs without a real game. Games replace this file with their own.
#include "Engine/Game.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Platform/Log.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Text.h"
#include "Engine/UI/UI.h"
#include "Engine/Asset/Asset.h"

#include <SSTL/Core/Config.h>

#if SSTL_DEBUG
static UIContext DebugUIContext;
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
#if SSTL_DEBUG
    UIBegin(&DebugUIContext, &DebugFont);
    if(UIButton(&DebugUIContext, SV8(u8"Click me"), 20.0f, 20.0f, 160.0f, 40.0f))
    {
        LogPrint(allocator, SV8(u8"Button Clicked!"));
    }
    UIEnd(&DebugUIContext);
#endif
}

void GameShutdown(StackAllocator* allocator)
{
}
