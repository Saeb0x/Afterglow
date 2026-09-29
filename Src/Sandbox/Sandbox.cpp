#include "Engine/Game.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Text.h"
#include "Engine/UI/UI.h"
#include "Engine/Asset/Asset.h"

#include <SSTL/Core/String.h>

#include <DirectXMath.h>

static Font DebugFont;
static UIContext DebugUI;
static UIPanel DebugPanel = { 20.0f, 20.0f, 320.0f, 300.0f, 320.0f, 300.0f };

static Camera WorldCamera = { { 0.0f, 0.0f }, { 20.0f, 20.0f }, 1.0f, 0.0f };

static real32 BeamRotation = 0.0f;

void GameConfigure()
{
    WindowSetFlags(WindowFlags_None);
    WindowSetTitle(SV8(u8"Afterglow Sandbox"));
    WindowSetClientAreaDimensions(1280, 720);
    WindowSetMinClientAreaDimensions(360, 360);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);
    RendererSetFlags(RendererFlags_VSync);
}

bool GameInit(StackAllocator* allocator)
{
    AssetLoadFont(allocator, SV8(u8"Data/Engine/LiberationMono-Regular.aga"), &DebugFont);

    return(true);
}

void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
    RendererSetCamera(&WorldCamera);
    
    RendererQuad beam = {};
    beam.X = -2.0f; beam.Y = -0.25f; beam.Width = 4.0f; beam.Height = 0.5f;
    beam.R = 0.8f; beam.G = 0.5f; beam.B = 0.2f; beam.A = 1.0f;
    beam.Rotation = BeamRotation;
    RendererPushQuad(&beam);

    UIBegin(&DebugUI, &DebugFont);
    UIPanelBegin(&DebugUI, &DebugPanel, SV8(u8"Debug"));
    UISlider(&DebugUI, SV8(u8"Beam"), &BeamRotation, -DirectX::XM_PI, DirectX::XM_PI);
    UIPanelEnd(&DebugUI);
    UIEnd(&DebugUI);
}

void GameShutdown(StackAllocator* allocator)
{
}
