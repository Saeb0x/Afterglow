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
        if(D3D11InitRenderer(windowHandle, 1280, 720))
        {
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

                GameUpdateAndRender();

                D3D11Present();
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

    return(0);
}
