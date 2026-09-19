#include "Engine/Platform/Windows/Win32Window.h"

#include "Game/Game.h"

#include <SSTL/Core/Utility.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static StackAllocator PlatformMemory;

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
    InitStackAllocator(&PlatformMemory, SSTL_MIB(1024));

    if(GameInit() && Win32WindowCreate(&PlatformMemory, SV8ToSV16(&PlatformMemory, SV8(u8"AfterglowGame")), 1280, 720))
    {
        while(Win32WindowPumpEvents())
        {
            GameUpdate();
        }

        GameShutdown();

        Win32WindowShutdown();
        ShutdownStackAllocator(&PlatformMemory);
    }

    return(0);
}
