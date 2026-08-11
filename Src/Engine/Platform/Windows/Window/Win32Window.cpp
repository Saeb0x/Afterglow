#include "Win32Window.h"
#include "Engine/Platform/Windows/Input/Win32Input.h"

#include <windows.h>

static Window WindowData;

static HMODULE Instance = GetModuleHandle(0);
static const char* WindowClassName = "AfterglowWindowClass";

static void Win32UpdateWindowDimensions(HWND windowHandle, WindowDimensions* outDims)
{
    RECT windowClientRect;
    GetClientRect(windowHandle, &windowClientRect);
    outDims->Width = windowClientRect.right - windowClientRect.left;
    outDims->Height = windowClientRect.bottom - windowClientRect.top;
}

static LRESULT CALLBACK Win32WindowProcedure(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch(message)
    {
        case WM_CLOSE:
        {
            WindowData.NativeHandler = nullptr;
            DestroyWindow(windowHandle);
        } break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;

        case WM_SIZE:
        {
            WindowData.Minimized = (wParam == SIZE_MINIMIZED);

            if(!WindowData.Minimized)
            {
                Win32UpdateWindowDimensions(windowHandle, &WindowData.Dimensions);
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

Window* WindowCreate(const char* title, uint32 width, uint32 height)
{
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = Win32WindowProcedure;
    windowClass.hInstance = Instance;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.lpszClassName = WindowClassName;

    if(!RegisterClassEx(&windowClass))
    {
        return(nullptr);
    }

    HWND windowHandle = CreateWindowEx(0,
                                       WindowClassName,
                                       title,
                                       WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT, CW_USEDEFAULT,
                                       width, height,
                                       0,
                                       0,
                                       Instance,
                                       0);

    if(!windowHandle)
    {
        UnregisterClass(WindowClassName, Instance);
        return(nullptr);
    }

    WindowData.NativeHandler = (void*)windowHandle;
    WindowData.Title = title;
    WindowData.Dimensions.Width = width;
    WindowData.Dimensions.Height = height;
    WindowData.Minimized = false;
    WindowData.ShouldClose = false;

    return(&WindowData);
}

void WindowShow(Window* window)
{
    if(window->NativeHandler)
    {
        ShowWindow((HWND)window->NativeHandler, SW_SHOW);
    }
}

void WindowShutdown(Window* window)
{
    if(!window->NativeHandler)
    {
        UnregisterClass(WindowClassName, Instance);
    }
}

void WindowPumpEvents(Window* window, GameInput* input)
{
    MSG message;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        if(message.message == WM_QUIT)
        {
            WindowData.ShouldClose = true;
            break;
        }

        Win32InputProcess(input, message.message, message.wParam, message.lParam);

        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

bool8 WindowShouldClose(Window* window)
{
    return(window && window->ShouldClose);
}

void WindowGetDimensions(Window* window, WindowDimensions* outDims)
{
    if(window->NativeHandler)
    {
        Win32UpdateWindowDimensions((HWND)window->NativeHandler, outDims);
    }
}

bool8 WindowGetMinimized(Window* window)
{
    return(window && window->Minimized);
}
