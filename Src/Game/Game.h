#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"

inline bool GameInit()
{
    WindowSetFlags(WindowFlags_None);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);

    return(true);
}

inline void GameUpdate(real64 deltaTime)
{
}

inline void GameShutdown()
{
}

#endif
