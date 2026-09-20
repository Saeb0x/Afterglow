#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"

#include <SSTL/Memory/StackAllocator.h>

inline bool GameInit(StackAllocator* allocator)
{
    WindowSetFlags(WindowFlags_None);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);

    return(true);
}

inline void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
}

inline void GameShutdown(StackAllocator* allocator)
{
}

#endif
