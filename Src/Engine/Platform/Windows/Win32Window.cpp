#include "Win32Window.h"
#include "Engine/Platform/Window.h"
#include "Win32Input.h"

struct Window
{
    HWND Handle;
    StringView8 InitialTitle; // Used once, when the window is created
    String8 Title;
    uint32 InitialWidth, InitialHeight; // Client area to start with; 0 = the default
    uint32 Width, Height;
    uint32 MinWidth, MinHeight; // Client area; 0 = no limit
    bool Minimized;
    bool CloseRequested;
    uint32 Flags;
};
static Window WindowData;

static LRESULT CALLBACK Win32WindowProcedure(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    Win32InputProcess(windowHandle, message, wParam, lParam);

    switch(message)
    {
        case WM_CLOSE:
        {
            // NOTE(saeb): Don't destroy here; the renderer still owns resources tied to this window. Flag it and let the main loop shut down in order.
            WindowData.CloseRequested = true;
        } break;

        case WM_DESTROY:
        {
            // NOTE(saeb): Safety net; if the window is destroyed by anything other than Win32WindowShutdown(), still end the loop.
            WindowData.CloseRequested = true;
        } break;

        case WM_GETMINMAXINFO:
        {
            // NOTE(saeb): Windows limits the outer window size, frame included, so add this window's frame to the minimum client area. Only the sides with a limit are changed; the rest keep Windows' defaults.
            if(WindowData.MinWidth > 0 || WindowData.MinHeight > 0)
            {
                RECT minimum = { 0, 0, (LONG)WindowData.MinWidth, (LONG)WindowData.MinHeight };
                AdjustWindowRectEx(&minimum, (DWORD)GetWindowLongPtrW(windowHandle, GWL_STYLE), FALSE, (DWORD)GetWindowLongPtrW(windowHandle, GWL_EXSTYLE));

                MINMAXINFO* info = (MINMAXINFO*)lParam;
                if(WindowData.MinWidth > 0)
                {
                    info->ptMinTrackSize.x = minimum.right - minimum.left;
                }

                if(WindowData.MinHeight > 0)
                {
                    info->ptMinTrackSize.y = minimum.bottom - minimum.top;
                }
            }
        } break;

        case WM_SIZE:
        {
            if(wParam == SIZE_MINIMIZED)
            {
                WindowData.Minimized = true;
                break;
            }

            WindowData.Width = LOWORD(lParam);
            WindowData.Height = HIWORD(lParam);
            WindowData.Minimized = false;
        } break;

        default:
        {
            return(DefWindowProcW(windowHandle, message, wParam, lParam));
        }
    }

    return(0);
}

bool Win32WindowCreate(StackAllocator* allocator)
{
    StringView8 title = (WindowData.InitialTitle.Data && WindowData.InitialTitle.Length > 0) ? WindowData.InitialTitle : SV8(u8"Afterglow");
    uint32 width = (WindowData.InitialWidth > 0) ? WindowData.InitialWidth : 1280;
    uint32 height = (WindowData.InitialHeight > 0) ? WindowData.InitialHeight : 720;

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

    uint32 windowStyle = WS_OVERLAPPEDWINDOW;
    uint32 windowWidth = width;
    uint32 windowHeight = height;
    uint32 windowX = CW_USEDEFAULT;
    uint32 windowY = CW_USEDEFAULT;

    if(WindowData.Flags & WindowFlags_Fullscreen)
    {
        windowStyle = WS_POPUP;
        windowWidth = GetSystemMetrics(SM_CXSCREEN);
        windowHeight = GetSystemMetrics(SM_CYSCREEN);
        windowX = 0;
        windowY = 0;
    }
    else
    {
        RECT windowClientArea = { 0, 0, (LONG)width, (LONG)height };
        AdjustWindowRectEx(&windowClientArea, (DWORD)windowStyle, FALSE, 0);

        windowWidth = (uint32)(windowClientArea.right - windowClientArea.left);
        windowHeight = (uint32)(windowClientArea.bottom - windowClientArea.top);
    }

    Frame frameScratch = GetFrame(allocator, Heap::Upper);
    HWND windowHandle = CreateWindowExW(0,
                                        L"AfterglowWin32WindowClass",
                                        (LPCWSTR)((SV8ToSV16(allocator, title)).Data),
                                        (DWORD)windowStyle,
                                        (int)windowX, (int)windowY,
                                        (int)windowWidth, (int)windowHeight,
                                        nullptr,
                                        nullptr,
                                        GetModuleHandleW(nullptr),
                                        nullptr);
    ReleaseFrame(allocator, frameScratch);

    if(!windowHandle)
    {
        UnregisterClassW(L"AfterglowWin32WindowClass", GetModuleHandleW(nullptr));
        return(false);
    }

    WindowData.Handle = windowHandle;
    WindowData.Title = String8FromView(allocator, title);

    RECT windowClientArea = {};
    GetClientRect(windowHandle, &windowClientArea);
    WindowData.Width = (uint32)(windowClientArea.right - windowClientArea.left);
    WindowData.Height = (uint32)(windowClientArea.bottom - windowClientArea.top);

    WindowData.Minimized = false;

    return(true);
}

bool Win32WindowPumpEvents()
{
    Win32InputBegin();

    MSG message;
    while(PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        // NOTE(saeb): Nothing in the engine posts WM_QUIT, but anything calling PostQuitMessage() should still end the loop.
        if(message.message == WM_QUIT)
        {
            return(false);
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return(!WindowData.CloseRequested);
}

void Win32WindowShutdown()
{
    if(WindowData.Handle)
    {
        DestroyWindow(WindowData.Handle);
        WindowData.Handle = nullptr;
    }

    UnregisterClassW(L"AfterglowWin32WindowClass", GetModuleHandleW(nullptr));
}

HWND Win32WindowGetHandle()
{
    return(WindowData.Handle);
}

void WindowSetFlags(uint32 windowFlags)
{
    WindowData.Flags = windowFlags;
}

void WindowSetTitle(StringView8 title)
{
    WindowData.InitialTitle = title;
}

void WindowSetClientAreaDimensions(uint32 width, uint32 height)
{
    WindowData.InitialWidth = width;
    WindowData.InitialHeight = height;
}

void WindowSetMinClientAreaDimensions(uint32 width, uint32 height)
{
    WindowData.MinWidth = width;
    WindowData.MinHeight = height;
}

void WindowGetClientAreaDimensions(uint32* width, uint32* height)
{
    *width = WindowData.Width;
    *height = WindowData.Height;
}

bool WindowGetMinimized()
{
    return(WindowData.Minimized);
}
