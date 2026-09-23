#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Render/Render.h"

#include <SSTL/Memory/StackAllocator.h>

// NOTE(saeb): Runs before the window exists; only set flags here, never acquire resources (nothing tears this down).
inline void GameConfigure()
{
    WindowSetFlags(WindowFlags_None);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);
}

inline bool GameInit(StackAllocator* allocator)
{
    return(true);
}

inline void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
}

inline void GameShutdown(StackAllocator* allocator)
{
}

#endif
