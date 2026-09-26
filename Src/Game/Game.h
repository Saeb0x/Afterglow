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
    WindowSetMinClientAreaDimensions(360, 360);
    RendererSetFlags(RendererFlags_VSync);
    RendererSetDesignSize(1000.0f, 1000.0f);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);
}

inline bool GameInit(StackAllocator* allocator)
{
    return(true);
}

inline void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
    // NOTE(saeb): One background over everything the window shows, so the design area has no visible edge.
    real32 visibleX, visibleY, visibleWidth, visibleHeight;
    RendererGetVisibleArea(&visibleX, &visibleY, &visibleWidth, &visibleHeight);

    RendererQuad background = {};
    background.X = visibleX; background.Y = visibleY;
    background.Width = visibleWidth; background.Height = visibleHeight;
    background.U1 = 1.0f; background.V1 = 1.0f;
    background.R = 0.180f; background.G = 0.275f; background.B = 0.212f; background.A = 1.0f;
    RendererPushQuad(&background);
}

inline void GameShutdown(StackAllocator* allocator)
{
}

#endif
