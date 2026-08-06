#include "Win32Window.h"
#include "Engine/Platform/Windows/Input/Win32Input.h"

struct PlatformSurface
{
    HWND Handle;
    bool32 ShouldQuit;
    bool32 Suspended;
    bool32 ResizePending;
    SurfaceDimensions PendingResizeDimensions;
};
static PlatformSurface Surface;

HWND Win32GetWindowHandle(PlatformSurface* surface)
{
    return(surface->Handle);
}

static void Win32UpdateSurfaceDimensions(HWND windowHandle, SurfaceDimensions* outDims)
{
    RECT windowClientRect;
    GetClientRect(windowHandle, &windowClientRect);
    outDims->Width = windowClientRect.right - windowClientRect.left;
    outDims->Height = windowClientRect.bottom - windowClientRect.top;
}

void PlatformShowSurface(PlatformSurface* surface)
{
    ShowWindow(surface->Handle, SW_SHOW);
}

void PlatformGetSurfaceDimensions(PlatformSurface* surface, SurfaceDimensions* outDims)
{
    Win32UpdateSurfaceDimensions(surface->Handle, outDims);
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
            Surface.Suspended = (wParam == SIZE_MINIMIZED);

            if(!Surface.Suspended)
            {
                Surface.ResizePending = true;
                Win32UpdateSurfaceDimensions(windowHandle, &Surface.PendingResizeDimensions);
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

bool32 PlatformCreateSurface(const char* title, int32 width, int32 height, PlatformSurface** outSurface)
{
    HINSTANCE instance = GetModuleHandle(0);

    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = Win32WindowCallback;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
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

    Surface.Handle = windowHandle;
    *outSurface = &Surface;

    return(true);
}
void PlatformPumpEvents(GameInput* input)
{
    MSG message;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        if(message.message == WM_QUIT)
        {
            Surface.ShouldQuit = true;
            continue;
        }

        Win32ProcessInputMessage(input, message.message, message.wParam, message.lParam);

        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

bool32 PlatformShouldQuit()
{
    return(Surface.ShouldQuit);
}

bool32 PlatformConsumeResize(SurfaceDimensions* outDims)
{
    if(Surface.ResizePending)
    {
        *outDims = Surface.PendingResizeDimensions;
        Surface.ResizePending = false;
        return(true);
    }

    return(false);
}

bool32 PlatformIsSuspended()
{
    return(Surface.Suspended);
}
