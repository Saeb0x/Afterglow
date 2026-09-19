#include "Engine/Platform/Windows/Win32Window.h"
#include "Engine/Platform/Windows/Win32Time.h"
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
        Win32TimeInit();

        while(Win32WindowPumpEvents())
        {
            GameUpdate(Win32TimeTick());
        }

        GameShutdown();
        Win32WindowShutdown();

        ShutdownStackAllocator(&PlatformMemory);
    }

    return(0);
}
