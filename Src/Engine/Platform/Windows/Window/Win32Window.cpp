#include "Win32Window.h"
#include "Engine/Platform/Windows/Input/Win32Input.h"

#include "SSTL/Memory.h"

static bool8 PlatformInitialized = false;

struct PlatformData
{
    HWND WindowHandle;
    bool32 WindowShouldQuit;
    bool32 WindowMinimized;
    bool32 WindowResizePending;
    WindowDimensions WindowPendingResizeDimensions;

    GameMemory Memory;
};
static PlatformData Data;

static void Win32UpdateWindowDimensions(HWND windowHandle, WindowDimensions* outDims)
{
    RECT windowClientRect;
    GetClientRect(windowHandle, &windowClientRect);
    outDims->Width = windowClientRect.right - windowClientRect.left;
    outDims->Height = windowClientRect.bottom - windowClientRect.top;
}

static LRESULT CALLBACK Win32WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
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
            Data.WindowMinimized = (wParam == SIZE_MINIMIZED);

            if(!Data.WindowMinimized)
            {
                Data.WindowResizePending = true;
                Win32UpdateWindowDimensions(windowHandle, &Data.WindowPendingResizeDimensions);
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

bool8 PlatformInit(const char* title, int32 width, int32 height, GameMemory** outMemory)
{
    uint64 engineArenaSize = 16 * sstl::Megabytes;
    uint64 permanentArenaSize = 64 * sstl::Megabytes;
    uint64 transientArenaSize = 1 * sstl::Gigabytes;
    uint64 totalSize = engineArenaSize + permanentArenaSize + transientArenaSize;

    void* memoryBlock = (void*)VirtualAlloc(0, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    sstl::InitializeArena(&Data.Memory.Engine, memoryBlock, engineArenaSize);
    sstl::InitializeArena(&Data.Memory.Permanent, (uint8*)memoryBlock + engineArenaSize, permanentArenaSize);
    sstl::InitializeArena(&Data.Memory.Transient, (uint8*)memoryBlock + engineArenaSize + permanentArenaSize, transientArenaSize);

    if(Data.Memory.Engine.Base)
    {
        Data.Memory.Initialized = true;

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
            VirtualFree(Data.Memory.Engine.Base, 0, MEM_RELEASE);
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
            VirtualFree(Data.Memory.Engine.Base, 0, MEM_RELEASE);
            return(false);
        }

        Data.WindowHandle = windowHandle;
        *outMemory = &Data.Memory;

        PlatformInitialized = true;
        return(true);
    }

    return(false);
}

void PlatformPumpEvents(GameInput* input)
{
    MSG message;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        if(message.message == WM_QUIT)
        {
            Data.WindowShouldQuit = true;
            continue;
        }

        Win32ProcessInputMessage(input, message.message, message.wParam, message.lParam);

        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

void PlatformShutdown()
{
    PlatformInitialized = false;

    if(Data.WindowHandle)
    {
        DestroyWindow(Data.WindowHandle);
        Data.WindowHandle = nullptr;
    }

    if(Data.Memory.Engine.Base)
    {
        VirtualFree(Data.Memory.Engine.Base, 0, MEM_RELEASE);
    }
}

void Win32ShowWindow()
{
    if(PlatformInitialized)
    {
        ShowWindow(Data.WindowHandle, SW_SHOW);
    }
}

bool8 Win32WindowConsumeResize(WindowDimensions* outDims)
{
    if(PlatformInitialized && Data.WindowResizePending)
    {
        *outDims = Data.WindowPendingResizeDimensions;
        Data.WindowResizePending = false;
        return(true);
    }

    return(false);
}

bool8 Win32WindowShouldQuit()
{
    return(PlatformInitialized && Data.WindowShouldQuit);
}

HWND Win32GetWindowHandle()
{
    if(PlatformInitialized)
    {
        return(Data.WindowHandle);
    }

    return(nullptr);
}

void Win32GetWindowDimensions(WindowDimensions* outDims)
{
    if(PlatformInitialized)
    {
        Win32UpdateWindowDimensions(Data.WindowHandle, outDims);
    }
}

bool8 Win32WindowIsMinimized()
{
    return(PlatformInitialized && Data.WindowMinimized);
}
