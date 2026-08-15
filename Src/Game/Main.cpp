#include "Engine/Window.h"
#include "Engine/Input.h"

#include <SSTL/Config.h>

#if SSTL_PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
    if(WindowCreate(u"Afterglow Game", 1280, 720))
    {
        while(WindowPumpEvents())
        {
        }

        WindowShutdown();
    }

    return(0);
}

#else
    #error Afterglow only supports Windows for now!

#endif
