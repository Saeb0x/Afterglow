#include "Afterglow.cpp"
#include "Window/Win32Window.cpp"
#include "Direct3D/D3D11Renderer.cpp"
#include "Direct3D/D3D11QuadBatcher.cpp"

#include <stdlib.h>

int WINAPI WinMain(HINSTANCE instance,
                   HINSTANCE,
                   LPSTR,
                   int)
{
    uint64 permanentArenaSize = Megabytes(64);
    uint64 transientArenaSize = Gigabytes(2);
    LPVOID gameMemoryBlock = VirtualAlloc(0, permanentArenaSize + transientArenaSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    GameMemory gameMemory = {};
    InitializeArena(&gameMemory.PermanentArena, permanentArenaSize, gameMemoryBlock);
    InitializeArena(&gameMemory.TransientArena, transientArenaSize, (uint8*)gameMemoryBlock + permanentArenaSize);

    if(gameMemory.PermanentArena.Base)
    {
        HWND windowHandle;
        if(Win32CreateWindow(instance, "Afterglow", 1280, 720, &windowHandle))
        {
            WindowDimensions dims;
            Win32GetWindowDimensions(windowHandle, &dims);

            if(D3D11InitRenderer(windowHandle, dims.Width, dims.Height))
            {
                if(D3D11QuadBatcherInit(&gameMemory.PermanentArena, &gameMemory.TransientArena, 16384))
                {
                    Win32ShowWindow(windowHandle);

                    bool32 running = true;
                    while(running)
                    {
                        Win32ProcessPendingMessages();

                        if(Win32WindowShouldQuit())
                        {
                            running = false;
                            break;
                        }

                        if(Win32WindowConsumeResize(&dims))
                        {
                            D3D11ResizeRenderer(dims.Width, dims.Height);
                        }

                        gameMemory.TransientArena.Used = 0;
                        if(!Win32WindowIsMinimized())
                        {
                            D3D11BeginFrame();
                            D3D11QuadBatcherBegin(dims.Width, dims.Height);

                            GameUpdateAndRender(&gameMemory);

                            // TEMP(saeb): Many random quads to verify batching.
                            for(uint32 testIndex = 0; testIndex < 1000; ++testIndex)
                            {
                                real32 x = (real32)(rand() % dims.Width);
                                real32 y = (real32)(rand() % dims.Height);
                                real32 w = (real32)(20 + rand() % 80);
                                real32 h = (real32)(20 + rand() % 80);

                                uint8 r = (uint8)(rand() % 256);
                                uint8 g = (uint8)(rand() % 256);
                                uint8 b = (uint8)(rand() % 256);

                                D3D11QuadBatcherPushQuad(x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, 0, PackColor(r, g, b, 255));
                            }

                            D3D11QuadBatcherEnd();
                            D3D11Present();
                        }
                    }

                    VirtualFree(gameMemory.PermanentArena.Base, 0, MEM_RELEASE);
                }
                else
                {
                    abort();
                }
            }
            else
            {
                abort();
            }
        }
        else
        {
            abort();
        }
    }
    else
    {
        abort();
    }

    D3D11ShutdownRenderer();

    return(0);
}
