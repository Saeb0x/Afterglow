#include "Engine/Platform/Windows/Window/Win32Window.h"
#include "Game/Game.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
    if(GameInit() && Win32WindowCreate(u"Afterglow Game", 1280, 720))
    {
        while(Win32WindowPumpEvents())
        {
            GameUpdate();
        }

        GameShutdown();
        Win32WindowShutdown();
    }

    return(0);
}
