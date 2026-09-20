#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Render/Render.h"

#include <SSTL/Memory/StackAllocator.h>

inline bool GameInit(StackAllocator* allocator)
{
    WindowSetFlags(WindowFlags_None);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);

    return(true);
}

inline void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
    Quad quad = {};
    quad.Position[0] = 540;
    quad.Position[1] = 260;
    quad.Size[0] = 200;
    quad.Size[1] = 200;
    quad.Color = { 1.0f, 0.0f, 0.0f, 1.0f };

    RenderDrawQuad(quad);
}

inline void GameShutdown(StackAllocator* allocator)
{
}

#endif
