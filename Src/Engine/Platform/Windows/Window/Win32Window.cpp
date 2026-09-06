#include "Win32Window.h"
#include "Engine/Window.h"
#include "Engine/Platform/Windows/Input/Win32Input.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct Window
{
    HWND Handle;
    cstring16 Title;
    uint32 Width, Height;
    uint32 Flags;
};
static Window WindowData = {};

static LRESULT CALLBACK Win32WindowProcedure(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_CLOSE:
        {
            DestroyWindow(windowHandle);
        } break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;

        default:
        {
            return(DefWindowProcW(windowHandle, message, wParam, lParam));
        }
    }

    return(0);
}

bool8 Win32WindowCreate(cstring16 title, uint32 width, uint32 height)
{
    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = Win32WindowProcedure;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW));
    windowClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    windowClass.lpszClassName = L"AfterglowWin32WindowClass";

    if(!RegisterClassExW(&windowClass))
    {
        return(false);
    }

    DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    int windowWidth = (int)width;
    int windowHeight = (int)height;
    int windowX = CW_USEDEFAULT;
    int windowY= CW_USEDEFAULT;

    if(WindowData.Flags & WindowFlags_Fullscreen)
    {
        windowStyle = WS_POPUP | WS_VISIBLE;
        windowWidth = GetSystemMetrics(SM_CXSCREEN);
        windowHeight = GetSystemMetrics(SM_CYSCREEN);
        windowX = 0;
        windowY = 0;
    }

    HWND windowHandle = CreateWindowExW(0,
                                        L"AfterglowWin32WindowClass",
                                        reinterpret_cast<LPCWSTR>(title),
                                        windowStyle,
                                        windowX, windowY,
                                        windowWidth, windowHeight,
                                        nullptr,
                                        nullptr,
                                        GetModuleHandleW(nullptr),
                                        nullptr);

    if(!windowHandle)
    {
        UnregisterClassW(L"AfterglowWin32WindowClass", GetModuleHandleW(nullptr));
        return(false);
    }

    WindowData.Handle = windowHandle;
    WindowData.Title = title;
    WindowData.Width = (uint32)windowWidth;
    WindowData.Height = (uint32)windowHeight;

    return(true);
}

bool8 Win32WindowPumpEvents()
{
    Win32InputBegin();

    MSG message;
    while(PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if(message.message == WM_QUIT)
        {
            return(false);
        }

        Win32InputProcess(message.message, message.wParam, message.lParam);

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return(true);
}

void Win32WindowShutdown()
{
    UnregisterClassW(L"AfterglowWin32WindowClass", GetModuleHandleW(nullptr));
}

void WindowSetFlags(uint32 windowFlags)
{
    WindowData.Flags = windowFlags;
}
