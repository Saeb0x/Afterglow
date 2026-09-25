#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Platform/File.h"
#include "Engine/Asset/Asset.h"
#include "Engine/Renderer/Renderer.h"

#include <SSTL/Memory/StackAllocator.h>

// NOTE(saeb): Only set flags here, never acquire resources (nothing tears this down).
inline void GameConfigure()
{
    WindowSetFlags(WindowFlags_None);
    RendererSetFlags(RendererFlags_VSync);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);
}

inline bool GameInit(StackAllocator* allocator)
{
    return(true);
}

inline void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
    RendererQuad quad = {};
    quad.X = 100.0f; quad.Y = 200.0f;
    quad.Width = 300.0f; quad.Height = 300.0f;
    quad.U1 = 1.0f; quad.V1 = 1.0f;
    quad.A = 1.0f; quad.R = 1.0f; quad.G = 0.5f;

    RendererPushQuad(&quad);
}

inline void GameShutdown(StackAllocator* allocator)
{
}

#endif
