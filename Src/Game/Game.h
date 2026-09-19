#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include "Engine/Window.h"
#include "Engine/Input.h"

inline bool GameInit()
{
    WindowSetFlags(WindowFlags_None);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);

    return(true);
}

inline void GameUpdate(real64 deltaTime = 0.0)
{
}

inline void GameShutdown()
{
}

#endif
