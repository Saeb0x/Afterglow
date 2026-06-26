#include "Afterglow.cpp"
#include "Window/Win32Window.cpp"
#include "Direct3D/D3D11Renderer.cpp"
#include "Direct3D/D3D11QuadBatcher.cpp"

#include <stdlib.h>

#define EMB(content) \
    do { \
        MessageBox(0, content, "Afterglow Error", MB_OK | MB_ICONERROR); \
        abort(); \
    } while(0)

static const char* WindowTitle =
#if defined(AG_DEBUG)
    "Afterglow - DX11 | Debug";
#else
    "Afterglow";
#endif

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
        if(Win32CreateWindow(instance, WindowTitle, 1280, 720, &windowHandle))
        {
            WindowDimensions dims;
            Win32GetWindowDimensions(windowHandle, &dims);

            D3D11RendererState renderer = {};
            if(D3D11InitRenderer(&renderer, windowHandle, dims.Width, dims.Height))
            {
                if(D3D11InitQuadBatcher(&renderer, &gameMemory.PermanentArena, &gameMemory.TransientArena, 16384))
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
                            D3D11ResizeRenderer(&renderer, dims.Width, dims.Height);
                        }

                        gameMemory.TransientArena.Used = 0;
                        if(!Win32WindowIsMinimized())
                        {
                            D3D11BeginFrame(&renderer);
                            D3D11QuadBatcherBegin(dims.Width, dims.Height);

                            GameUpdateAndRender(&gameMemory);

                            D3D11QuadBatcherEnd();
                            D3D11Present(&renderer);
                        }
                    }

                    VirtualFree(gameMemory.PermanentArena.Base, 0, MEM_RELEASE);
                    D3D11ShutdownRenderer(&renderer);
                }
                else
                {
                    D3D11ShutdownRenderer(&renderer);
                    EMB("Quad batcher initialization failed!");
                }
            }
            else
            {
                EMB("Renderer initialization failed!");
            }
        }
        else
        {
            EMB("Window creation failed!");
        }
    }
    else
    {
        EMB("Game memory allocation failed!");
    }

    return(0);
}
