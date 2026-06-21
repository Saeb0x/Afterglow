#include "Afterglow.cpp"
#include "Window/Win32Window.cpp"
#include "Direct3D/D3D11Renderer.cpp"

int WINAPI WinMain(HINSTANCE instance,
                   HINSTANCE,
                   LPSTR,
                   int)
{
    HWND windowHandle;
    if(Win32CreateWindow(instance, "Afterglow", 1280, 720, &windowHandle))
    {
        WindowDimensions dims;
        Win32GetWindowDimensions(windowHandle, &dims);

        if(D3D11InitRenderer(windowHandle, dims.Width, dims.Height))
        {
            uint64 permanentArenaSize = Megabytes(64);
            uint64 transientArenaSize = Gigabytes(2);
            LPVOID gameMemoryBlock = VirtualAlloc(0, permanentArenaSize + transientArenaSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            GameMemory gameMemory = {};

            InitializeArena(&gameMemory.PermanentArena, permanentArenaSize, gameMemoryBlock);
            InitializeArena(&gameMemory.TransientArena, transientArenaSize, (uint8*)gameMemoryBlock + permanentArenaSize);

            if(gameMemory.PermanentArena.Base)
            {
                Win32ShowWindow(windowHandle);

                bool running = true;
                while(running)
                {
                    Win32ProcessPendingMessages();

                    if(Win32WindowShouldQuit())
                    {
                        running = false;
                        break;
                    }

                    WindowDimensions newDimensions;
                    if(Win32WindowConsumeResize(&newDimensions))
                    {
                        D3D11ResizeRenderer(newDimensions.Width, newDimensions.Height);
                    }

                    D3D11BeginFrame();

                    GameUpdateAndRender(&gameMemory);

                    D3D11Present();
                }

                VirtualFree(gameMemory.PermanentArena.Base, 0, MEM_RELEASE);
            }
            else
            {
                // TODO(saeb): Logging.
            }
        }
        else
        {
            // TODO(saeb): Logging.
        }
    }
    else
    {
        // TODO(saeb): Logging.
    }

    D3D11ShutdownRenderer();

    return(0);
}
