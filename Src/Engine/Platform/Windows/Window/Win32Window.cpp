#include "Engine/Window.h"
#include "Engine/Platform/Windows/Input/Win32Input.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct Window
{
    HWND Handle;

    cstring16 Title;
    uint32 Width, Height;
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

bool8 WindowCreate(cstring16 title, uint32 width, uint32 height)
{
    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = Win32WindowProcedure;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW));
    windowClass.lpszClassName = L"AfterglowWin32WindowClass";

    if(!RegisterClassExW(&windowClass))
    {
        return(false);
    }

    HWND windowHandle = CreateWindowExW(0,
                                        L"AfterglowWin32WindowClass",
                                        reinterpret_cast<LPCWSTR>(title),
                                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                        CW_USEDEFAULT, CW_USEDEFAULT,
                                        (int)width, (int)height,
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
    WindowData.Width = width;
    WindowData.Height = height;

    return(true);
}

bool8 WindowPumpEvents()
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

void WindowShutdown()
{
    UnregisterClassW(L"AfterglowWin32WindowClass", GetModuleHandleW(nullptr));
}
