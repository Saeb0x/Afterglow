#include "Engine/Platform/Windows/Win32Window.h"
#include "Engine/Platform/Windows/Win32Time.h"
#include "Game/Game.h"

#include <SSTL/Core/Utility.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static StackAllocator EngineMemory;

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
    InitStackAllocator(&EngineMemory, SSTL_MIB(64));

    if(GameInit() && Win32WindowCreate(&EngineMemory, SV8(u8"Afterglow Game"), 1280, 720))
    {
        Win32TimeInit();

        while(Win32WindowPumpEvents())
        {
            Frame frameScratch = GetFrame(&EngineMemory, Heap::Upper);

            GameUpdate(Win32TimeTick());

            ReleaseFrame(&EngineMemory, frameScratch);
        }

        GameShutdown();
        Win32WindowShutdown();

        ShutdownStackAllocator(&EngineMemory);
    }

    return(0);
}
