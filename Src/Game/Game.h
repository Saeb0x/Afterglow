#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include <SSTL/Config.h>

#include "Engine/Window.h"
#include "Engine/Input.h"

#if SSTL_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>

#endif

inline bool8 GameInit()
{
    WindowSetFlags(WindowFlags_None);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);

    return(true);
}

inline void GameUpdate(real64 deltaTime = 0.0)
{
    int32 mouseX;
    int32 mouseY;
    InputMouseXY(&mouseX, &mouseY);

#if SSTL_PLATFORM_WINDOWS
    wchar_t debugBuffer[256];
    swprintf_s(debugBuffer, 256, L"Mouse Coords(x,y): (%d, %d)\n", mouseX, mouseY);

    // OutputDebugStringW(debugBuffer);

    if(InputKeyPressed(InputKey::Space))
    {
        OutputDebugStringW(L"Space is pressed!\n");
    }
#endif 
}

inline void GameShutdown()
{
}

#endif
