#include "Engine/Game.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Text.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/UI/UI.h"
#include "Engine/Asset/Asset.h"

static Camera WorldCamera = { { 0.0f, 0.0f }, { 20.0f, 20.0f }, 1.0f, 0.0f };

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
    return(true);
}

void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
    RendererSetCamera(&WorldCamera);
}

void GameShutdown(StackAllocator* allocator)
{
}
