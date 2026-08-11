#include "Engine/Window.h"
#include "Engine/Input.h"

#include <SSTL/Config.h>

#if SSTL_PLATFORM_WINDOWS

#include <windows.h>

static GameInput input = {};

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
    Window* window = WindowCreate("Afterglow", 1280, 720);

    if(window)
    {
        WindowShow(window);
        while(!WindowShouldClose(window))
        {
            InputBegin(&input);
            WindowPumpEvents(window, &input);
        }

        WindowShutdown(window);
    }

    return(0);
}

#else
    #error Afterglow only supports Windows for now!
#endif
