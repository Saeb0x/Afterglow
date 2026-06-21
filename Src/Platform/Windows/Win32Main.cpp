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
            GameMemory gameMem = {};

            gameMem.PermanentDataSize = Megabytes(64);
            gameMem.TransientDataSize = Gigabytes(2);
            uint64 totalSize = gameMem.PermanentDataSize + gameMem.TransientDataSize;

            gameMem.PermanentData = VirtualAlloc(0, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            gameMem.TransientData = (uint8*)gameMem.PermanentData + gameMem.PermanentDataSize;

            if(gameMem.PermanentData)
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

                    GameUpdateAndRender(&gameMem);

                    D3D11Present();
                }

                VirtualFree(gameMem.PermanentData, 0, MEM_RELEASE);
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
