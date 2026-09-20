#include "Engine/Platform/Windows/Win32Window.h"
#include "Engine/Platform/Windows/Win32Time.h"
#include "Engine/Platform/Windows/Win32Render.h"
#include "Game/Game.h"

#include <SSTL/Core/Utility.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static StackAllocator EngineMemory;

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    InitStackAllocator(&EngineMemory, SSTL_MIB(64));

    if(GameInit(&EngineMemory) && Win32WindowCreate(&EngineMemory, SV8(u8"Afterglow Game"), 1280, 720))
    {
        uint32 windowWidth, windowHeight;
        WindowGetDimensions(&windowWidth, &windowHeight);

        if(Win32RenderInit(Win32WindowGetHandle(), windowWidth, windowHeight))
        {
            ShowWindow(Win32WindowGetHandle(), SW_SHOW);

            Win32TimeInit();

            while(Win32WindowPumpEvents())
            {
                Frame frameScratch = GetFrame(&EngineMemory, Heap::Upper);

                Win32RenderClear(Color{ 0.0f, 0.0f, 0.0f, 1.0f });
                GameUpdate(&EngineMemory, Win32TimeTick());
                Win32RenderPresent();

                ReleaseFrame(&EngineMemory, frameScratch);
            }

            GameShutdown(&EngineMemory);
            Win32RenderShutdown();
        }

        Win32WindowShutdown();

        ShutdownStackAllocator(&EngineMemory);
    }

    return(0);
}
