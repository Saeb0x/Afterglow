#include "Win32Window.h"
#include "Win32Time.h"
#include "Engine/Render/D3D11/D3D11Render.h"

#include "Game/Game.h"

#include <SSTL/Core/Utility.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static StackAllocator EngineMemory;

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
    if(InitStackAllocator(&EngineMemory, SSTL_MIB(64)))
    {
        GameConfigure();

        if(Win32WindowCreate(&EngineMemory, SV8(u8"Afterglow"), 1280, 720))
        {
            if(D3D11RenderInit())
            {
                if(GameInit(&EngineMemory))
                {
                    ShowWindow(Win32WindowGetHandle(), SW_SHOW);

                    Win32TimeInit();

                    while(Win32WindowPumpEvents())
                    {
                        // NOTE(saeb): Nothing to show while minimized; sleep until a message (restore, quit) arrives instead of spinning.
                        if(WindowGetMinimized())
                        {
                            WaitMessage();
                            continue;
                        }

                        Frame frameScratch = GetFrame(&EngineMemory, Heap::Upper);

                        D3D11RenderClear();
                        GameUpdate(&EngineMemory, Win32TimeTick());
                        D3D11RenderPresent();

                        ReleaseFrame(&EngineMemory, frameScratch);
                    }

                    GameShutdown(&EngineMemory);
                }

                D3D11RenderShutdown();
            }

            Win32WindowShutdown();
        }

        ShutdownStackAllocator(&EngineMemory);
    }

    return(0);
}
