#include "Afterglow.h"

#include <windows.h>

static bool Running;

LRESULT CALLBACK WindowCallback(HWND handle,
                                UINT message,
                                WPARAM wParam,
                                LPARAM lParam)
{
    LRESULT result = 0;
    
    switch(message)
    {
        case WM_CLOSE:
        {
            // TODO(saeb): User chose to close - show confirmation UI?
            DestroyWindow(handle);
        } break;
        case WM_DESTROY:
        {
            // TODO(saeb): Window is being destroyed; perform shutdown cleanup.
            PostQuitMessage(0);
        } break;
        default:
        {
            return DefWindowProc(handle, message, wParam, lParam);
        } break;
    }
    
    return(result);
}

int WINAPI WinMain(HINSTANCE instance,
                   HINSTANCE,
                   LPSTR,
                   int)
{
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = WindowCallback;
    windowClass.hInstance = instance;
    // windowClass.hIcon = ;
    // windowClass.hCursor = ;
    windowClass.lpszClassName = "AfterglowWindowClass";
    // windowClass.hIconSm = ;
    
    if(RegisterClassEx(&windowClass))
    {
        HWND windowHandle = CreateWindowEx(0,
                                           windowClass.lpszClassName,
                                           "Afterglow Game",
                                           WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                           CW_USEDEFAULT, CW_USEDEFAULT,
                                           CW_USEDEFAULT, CW_USEDEFAULT,
                                           0,
                                           0,
                                           windowClass.hInstance,
                                           0);
        
        if(windowHandle)
        {
            Running = true;
            while(Running)
            {
                MSG message;
                while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    if(message.message == WM_QUIT)
                    {
                        Running = false;
                        break;
                    }
                    
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
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
