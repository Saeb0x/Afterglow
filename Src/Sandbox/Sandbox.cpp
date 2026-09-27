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
static UIPanel DebugPanel = { 20.0f, 20.0f, 300.0f, 400.0f, 300.0f, 400.0f };
#endif

// NOTE(saeb): A square in the middle of the design area, for the debug panel to edit.
static bool ShowSquare = true;
static real32 SquareSize = 200.0f;
static real32 SquareRed = 1.0f;

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
    // The game first, in design space; the debug UI draws over it.
    if(ShowSquare)
    {
        RendererQuad square = {};
        square.X = 500.0f - SquareSize * 0.5f; square.Y = 500.0f - SquareSize * 0.5f;
        square.Width = SquareSize; square.Height = SquareSize;
        square.R = SquareRed; square.G = 0.2f; square.B = 0.2f; square.A = 1.0f;

        RendererPushQuad(&square);
    }
    
#if SSTL_DEBUG
    UIBegin(&DebugUIContext, &DebugFont);
    UIPanelBegin(&DebugUIContext, &DebugPanel, SV8(u8"Debug"));

    UILabel(&DebugUIContext, SV8(u8"Square"));
    UICheckbox(&DebugUIContext, SV8(u8"Show"), &ShowSquare);
    UISlider(&DebugUIContext, SV8(u8"Size"), &SquareSize, 50.0f, 400.0f);
    UISlider(&DebugUIContext, SV8(u8"Red"), &SquareRed, 0.0f, 1.0f);

    if(UIButton(&DebugUIContext, SV8(u8"Reset")))
    {
        ShowSquare = true;
        SquareSize = 200.0f;
        SquareRed = 1.0f;
        LogPrint(allocator, SV8(u8"Square reset"));
    }

    UIPanelEnd(&DebugUIContext);
    UIEnd(&DebugUIContext);
#endif
}

void GameShutdown(StackAllocator* allocator)
{
}
