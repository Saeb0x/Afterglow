#include "Win32Window.h"

static bool32 ShouldQuit;
static bool32 ResizePending;
static WindowDimensions PendingResizeDimensions;
static bool32 Minimized;

void Win32ShowWindow(HWND windowHandle)
{
    ShowWindow(windowHandle, SW_SHOW);
}

void Win32GetWindowDimensions(HWND windowHandle, WindowDimensions* outDims)
{
    RECT windowClientRect;
    GetClientRect(windowHandle, &windowClientRect);

    outDims->Width = windowClientRect.right - windowClientRect.left;
    outDims->Height = windowClientRect.bottom - windowClientRect.top;
}

static LRESULT CALLBACK Win32WindowCallback(HWND windowHandle,
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
            DestroyWindow(windowHandle);
        } break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;

        case WM_SIZE:
        {
            Minimized = (wParam == SIZE_MINIMIZED);

            if(!Minimized)
            {
                ResizePending = true;
                Win32GetWindowDimensions(windowHandle, &PendingResizeDimensions);
            }
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            BeginPaint(windowHandle, &paint);
            EndPaint(windowHandle, &paint);
        } break;

        default:
        {
            result = DefWindowProc(windowHandle, message, wParam, lParam);
        } break;
    }

    return(result);
}

bool32 Win32CreateWindow(HINSTANCE instance, const char* title, int32 width, int32 height, HWND* outWindowHandle)
{
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = Win32WindowCallback;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "AfterglowWindowClass";

    if(!RegisterClassEx(&windowClass))
    {
        return(false);
    }
    
    HWND windowHandle = CreateWindowEx(0,
                                       windowClass.lpszClassName,
                                       title,
                                       WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT,
                                       width, height,
                                       0,
                                       0,
                                       instance,
                                       0);

    if(!windowHandle)
    {
        return(false);
    }
    
    *outWindowHandle = windowHandle;
    return(true);
}

void Win32ProcessPendingMessages()
{
    MSG message;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        if(message.message == WM_QUIT)
        {
            ShouldQuit = true;
            continue;
        }
        
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

bool32 Win32WindowShouldQuit()
{
    return(ShouldQuit);
}

bool32 Win32WindowConsumeResize(WindowDimensions* outDims)
{
    if(ResizePending)
    {
        *outDims = PendingResizeDimensions;
        ResizePending = false;
        return(true);
    }
    
    return(false);
}

bool32 Win32WindowIsMinimized()
{
    return(Minimized);
}
